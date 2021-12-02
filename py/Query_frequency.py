from argparse import ArgumentParser, Namespace, RawTextHelpFormatter
import os, re, ctypes
from typing import List, Tuple
import nltk.tokenize as nltoken
import time


class QueryResult(ctypes.Structure):
    _fields_ = [
        ('doc_id', ctypes.c_uint32),
        ('score', ctypes.c_float),
        ('url', ctypes.c_char_p),
        ('offset', ctypes.c_uint64),
        ('len', ctypes.c_uint32)
    ]
class QueryResultArray(ctypes.Structure):
    _fields_ = [
        ('n', ctypes.c_uint32),
        ('arr', ctypes.POINTER(QueryResult))
    ]
class QueryTerms(ctypes.Structure):
    _fields_ = [
        ('n', ctypes.c_uint32),
        ('idxs', ctypes.POINTER(ctypes.c_uint32)),
        ('freqs', ctypes.POINTER(ctypes.c_uint32))
    ]
class QueryTermsArray(ctypes.Structure):
    _fields_ = [
        ('terms_n_docs', ctypes.POINTER(ctypes.c_uint32)),
        ('n', ctypes.c_uint32),
        ('arr', ctypes.POINTER(QueryTerms))
    ]


def get_arguments():
    parser = ArgumentParser(formatter_class=RawTextHelpFormatter)
    parser.add_argument('--buf', type=int, required=True, help='index LRU cache size in MB')
    parser.add_argument('-i', type=str, required=True, help='documents dataset filename')
    parser.add_argument('--dll', type=str, required=True, help='c++ dll filename')
    parser.add_argument('--index', type=str, required=True, help='index filename')
    parser.add_argument('--lexicon', type=str, required=True, help='lexicon filename')
    parser.add_argument('--doctable', type=str, required=True, help='doctable filename')
    parser.add_argument('--load-cache', type=str, help='load LRUCache from file')
    parser.add_argument('--dump-cache', type=str, help='dump LRUCache on file')
    parser.add_argument('-k', type=int, default=10, help='top K result')
    parser.add_argument('-s', type=int, default=50, help='snippet length for each query word')

    return parser.parse_args()


def init_dll() -> ctypes.CDLL:
    cdll = ctypes.CDLL(arg.dll)
    
    cdll.init.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.set_LRUCache_size.argtypes = [ctypes.c_uint32]
    cdll.load_LRUCache.argtypes = [ctypes.c_char_p]
    cdll.dump_LRUCache.argtypes = [ctypes.c_char_p]

    cdll.get_SingleQueryProcessor.argtypes = [ctypes.c_char_p]
    cdll.get_SingleQueryProcessor.restype = ctypes.c_void_p
    cdll.get_ANDQueryProcessor.argtypes = [ctypes.py_object]
    cdll.get_ANDQueryProcessor.restype = ctypes.c_void_p
    cdll.get_ORQueryProcessor.argtypes = [ctypes.py_object]
    cdll.get_ORQueryProcessor.restype = ctypes.c_void_p
    cdll.get_QueryResult.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    cdll.get_QueryResult.restype = ctypes.c_void_p

    cdll.delete_QueryProcessor.argtypes = [ctypes.c_void_p]
    cdll.delete_QueryResult.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    # cdll.delete_all.argtypes = []

    cdll.get_QueryTerms.argtypes = [ctypes.py_object, ctypes.c_void_p]
    cdll.get_QueryTerms.restype = ctypes.c_void_p
    # cdll.test_query_result.restype = ctypes.c_void_p
    return cdll


re_url = re.compile(r'https?://\S+')
re_numbers = re.compile(r'0*(\d+)')
re_tk = re.compile(r'[^\W_]+')

def tokenize(s):
    # remove url-like string
    s = re_url.sub('', s)
    # separate numbers with space but exclude leading zeros
    s = re_numbers.sub(r' \1 ', s)
    # tokenize
    return re_tk.findall(s.lower())


def parse_query(query: str) -> Tuple[int, List[List[str]]]:
    '''
    2-layer query with <AND>, <OR>, single term. <OR> query can only be root query. \\
    query: "#microsoft#, apple. || band" -> "(microsoft && apple) || (band)" \\
    query: "**.,!microsoft!, apple# band" -> "microsoft && apple && band" \\
    query: "xx999yy999zz || Apple" -> "Apple" \\
        because another term doesn't exist in Lexicon \\
    query: "xx999yy999zz, Apple" -> NONE \\
        because it's <AND> query but one of the term doesn't exist in Lexicon
    [return]
    (C pointer of QueryProcessor, list of valid terms in subqueries)
    '''
    valid_terms: List[List[str]] = []
    query_processors: List[int] = []
    # split query string by <OR>
    # iterate each sub query
    for sub_query in query.split('||'):
        query_processor, sub_valid_terms = parse_AND_Single_query(sub_query)
        if query_processor is None:
            continue
        valid_terms.append(sub_valid_terms)
        query_processors.append(query_processor)
    
    if len(query_processors) == 0:
        return None, ''
    elif len(query_processors) == 1:
        root_processor = query_processors[0]
    else:
        root_processor = cdll.get_ORQueryProcessor(query_processors)
    
    return root_processor, valid_terms


def parse_AND_Single_query(query: str) -> Tuple[int, List[str]]:
    '''
    parse query with <AND>, or single term. No <OR>.
    [return]
    (C pointer of QueryProcessor, list of valid terms)
    '''
    terms = sorted(set(tokenize(query)))
    if len(terms) == 0: return None, None
    # single term query
    elif len(terms) == 1:
        query_processor = cdll.get_SingleQueryProcessor(terms[0].encode('utf-8'))
        if query_processor is None:
            return None, None
    else: # <AND> query
        terms_b = [term.encode('utf-8') for term in terms]
        query_processor = cdll.get_ANDQueryProcessor(terms_b)
        if query_processor is None:
            return None, None
    return query_processor, terms


def merge_intervals(intervals: List[Tuple[int,int]]) -> List[Tuple[int,int]]:
    intervals = sorted(intervals, key=lambda x:x[0])
    ret = []
    for interval in intervals:
        new_interval = interval
        #compare with the last one in ret
        if len(ret) > 0 and ret[-1][1] >= interval[0]:
            #merge and then replace the last one in ret
            new_interval = ret.pop()
            if interval[1] > new_interval[1]:
                new_interval = (new_interval[0], interval[1])
        ret.append(new_interval)
    return ret


def generate_snippet(terms: List[str], offset: int) -> str:
    # load document text based on offset
    file.seek(offset)
    is_text = False

    line = file.readline()
    while line:
        if line == '<TEXT>\n':
            # next line should be url according to .trec file
            line = file.readline()
            # further lines should be text content
            text = []
            is_text = True
        elif line == '</TEXT>\n':
            is_text = False
            break
        elif is_text:
            text.append(line.rstrip())
        line = file.readline()
    text = ' '.join(text)
    # return text

    # intervals for each term in text
    intervals = []
    for term in terms:
        # search if the term exist in this document text
        res = re.search(f'(^|[\W_])({term})([\W_]|$)', text, re.IGNORECASE)
        if res is None: continue
        left, right = res.span()
        # create an interval centered around this term with length
        interval = (max(0,left-arg.s), min(right+arg.s,len(text)))
        intervals.append(interval)
    intervals = merge_intervals(intervals)

    snippet = ' ... '.join(text[left:right] for left,right in intervals)
    return snippet


def format_query(valid_terms: List[List[str]]) -> str:
    if len(valid_terms) == 1:
        formatted_query = ' && '.join(valid_terms[0])
    else:
        formatted_query = []
        for sub_query_terms in valid_terms:
            formatted_query.append('({})'.format(' && '.join(sub_query_terms)))
        formatted_query = ' || '.join(formatted_query)
    return formatted_query


def print_result(terms: List[str], query_res, query_terms):
    terms_index_len = []
    for i, term in enumerate(terms):
        n_docs = query_terms.terms_n_docs[i]
        terms_index_len.append('"{:s}" {:d}'.format(term, n_docs))
    print('[Terms index len] {:s}'.format(' :: '.join(terms_index_len)))

    for i in range(query_res.n):
        print('[{:02d}] doc_id {:d} :: BM25 score {:.2f} :: doc len {:d} :: {}'.format(i,
            query_res.arr[i].doc_id,
            query_res.arr[i].score,
            query_res.arr[i].len,
            query_res.arr[i].url.decode('ascii')))
        
        terms_freqs = []
        for j in range(query_terms.arr[i].n):
            term_i = query_terms.arr[i].idxs[j]
            freq = query_terms.arr[i].freqs[j]
            terms_freqs.append('"{:s}" {:d}'.format(terms[term_i], freq))
        print('   [Terms frequency] {:s}'.format(' :: '.join(terms_freqs)))

        print(generate_snippet(terms, query_res.arr[i].offset))
        print()


def execute_query(query: str):
    dt = time.time()
    root_processor, valid_terms = parse_query(query)
    if root_processor is None:
        print('[ERROR] No word found in Lexicon!')
        return
    
    query_res_p = cdll.get_QueryResult(root_processor, arg.k)
    query_res = ctypes.cast(query_res_p, ctypes.POINTER(QueryResultArray)).contents
    print('[Formatted query] "{:s}"'.format(format_query(valid_terms)))

    if query_res.n == 0:
        print('No result found.')
        return
    
    # flatten the list
    terms = sorted(set(term for ls in valid_terms for term in ls))
    terms_b = [term.encode('utf-8') for term in terms]
    query_terms_p = cdll.get_QueryTerms(terms_b, query_res_p)
    query_terms = ctypes.cast(query_terms_p, ctypes.POINTER(QueryTermsArray)).contents

    print_result(terms, query_res, query_terms)
    dt = time.time() - dt
    print('[Query time] {:.3f} seconds\n'.format(dt))
    
    cdll.delete_QueryProcessor(root_processor)
    cdll.delete_QueryResult(query_res_p, query_terms_p)


if __name__ == '__main__':
    global arg, cdll, file
    arg = get_arguments()
    cdll = init_dll()
    file = open(arg.i, 'r', encoding='utf-8')

    print('Loading data structures......')
    cdll.init(arg.index.encode('ascii'), arg.lexicon.encode('ascii'), arg.doctable.encode('ascii'))
    cdll.set_LRUCache_size(arg.buf)
    if arg.load_cache:
        cdll.load_LRUCache(arg.load_cache.encode('ascii'))

    while True:
        query = input('____________________\n[Enter] ')
        if query == ':q!':
            break
        execute_query(query)
    
    if arg.dump_cache:
        cdll.dump_LRUCache(arg.dump_cache.encode('ascii'))
    cdll.delete_all()
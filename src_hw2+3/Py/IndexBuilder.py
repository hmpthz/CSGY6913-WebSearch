from argparse import ArgumentParser, Namespace, RawTextHelpFormatter
import os, re, ctypes
import nltk.tokenize as nltoken
import math, time


def get_arguments() -> Namespace:
    parser = ArgumentParser(formatter_class=RawTextHelpFormatter)
    parser.add_argument('-o', type=str, required=True, help='output directory')
    parser.add_argument('--dll', type=str, required=True, help='c++ dll filename')
    parser.add_argument('--ibuf', type=int, required=True, help='total input buffer size in MB')
    parser.add_argument('--obuf', type=int, required=True, help='total output buffer size in MB')
    parser.add_argument('--ext', type=str, default='bin', help='extension of output files, used in debug')

    parser.add_argument('--step1', action='store_true',
            help=('execute the first step of index building\n'
            'step1: parse input .trec file, tokenize the text, add to postings, construct doc table\n'
            'then transfer and convert postings to output index buffer with memory limit\n'
            'when reach memory limit, write separate sub-index files.'))
    parser.add_argument('-i', type=str, help='(step1) input filename')

    parser.add_argument('--step2', action='store_true',
            help=('execute the second step of index building\n'
            'step2: n-way merge sub-index files from the output of step1'))
    parser.add_argument('-m', type=int, help='(step2) # of files from the output of step1')
    parser.add_argument('--way', type=int, default=2, help='(step2) n-way merge')

    return parser.parse_args()


def init_dll(arg) -> ctypes.CDLL:
    cdll = ctypes.CDLL(arg.dll)
    # check PyInterface.h to see function descriptions
    cdll.get_Builder.restype = ctypes.c_void_p
    cdll.delete_Builder.argtypes = [ctypes.c_void_p]

    cdll.set_postingbuf_size.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    cdll.set_inputbufs_size.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32]
    cdll.set_outputbuf_size.argtypes = [ctypes.c_void_p, ctypes.c_uint32]
    cdll.open_output_doctable_file.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    cdll.open_inputbufs_file.argtypes = [ctypes.c_void_p, ctypes.py_object, ctypes.py_object]
    cdll.open_outputbuf_file.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p]

    cdll.add_doc_info.argtypes = [ctypes.c_void_p, ctypes.c_uint64, ctypes.c_uint32, ctypes.c_char_p]
    cdll.doc_tokens_to_postings.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.py_object]
    cdll.doc_tokens_to_postings.restype = ctypes.c_bool
    cdll.postings_to_index.argtypes = [ctypes.c_void_p]
    cdll.write_doctable.argtypes = [ctypes.c_void_p]
    cdll.is_postingsbuf_empty.argtypes = [ctypes.c_void_p]
    cdll.is_postingsbuf_empty.restype = ctypes.c_bool
    cdll.merge.argtypes = [ctypes.c_void_p]
    return cdll


def step1(arg) -> int:
    cdll = init_dll(arg)
    cbuilder = cdll.get_Builder()
    cdll.set_postingbuf_size(cbuilder, arg.ibuf)
    cdll.set_outputbuf_size(cbuilder, arg.obuf)
    doctable_filename = os.path.join(arg.o, 'doctable.'+arg.ext)
    index_filename = os.path.join(arg.o, 'index0_{}.'+arg.ext)
    lexicon_filename = os.path.join(arg.o, 'lexicon0_{}.'+arg.ext)

    tk = nltoken.RegexpTokenizer('[^\W_]+')
    pattern_no_url = re.compile(r'\w+://\w+\.\S+')

    f = open(arg.i, 'r', encoding='utf-8')
    is_text = False
    tokens = []
    doc_id = 0
    file_id = 0

    # .trec file parsing
    line = f.readline()
    while line:
        if line == '<DOC>\n':
            start_offset = f.tell() - 6
        elif line == '<TEXT>\n':
            # next line should be url according to .trec file
            line = f.readline()
            url = line.rstrip().encode('ascii')
            # further lines should be text content
            is_text = True
        elif line == '</TEXT>\n':
            # end of doc content
            if doc_id % 500 == 0: print('{} docs'.format(doc_id))
            # add doctable
            doc_len = len(tokens)
            cdll.add_doc_info(cbuilder, start_offset, doc_len, url)
            # add postings
            is_memoryfull = cdll.doc_tokens_to_postings(cbuilder, doc_id, tokens)
            if is_memoryfull:
                # open file
                bindex_file = index_filename.format(file_id).encode('ascii')
                blexicon_file = lexicon_filename.format(file_id).encode('ascii')
                cdll.open_outputbuf_file(cbuilder, bindex_file, blexicon_file)
                # write
                cdll.postings_to_index(cbuilder)
                file_id += 1
            tokens.clear()
            is_text = False
            doc_id += 1
        elif is_text:
            # remove url-like string
            line = pattern_no_url.sub('', line)
            tokens.extend([s.encode('utf-8') for s in tk.tokenize(line)])
        line = f.readline()
    # final
    cdll.open_output_doctable_file(cbuilder, doctable_filename.encode('ascii'))
    cdll.write_doctable(cbuilder)
    if not cdll.is_postingsbuf_empty(cbuilder):
        # write remaining data
        bindex_file = index_filename.format(file_id).encode('ascii')
        blexicon_file = lexicon_filename.format(file_id).encode('ascii')
        cdll.open_outputbuf_file(cbuilder, bindex_file, blexicon_file)
        cdll.postings_to_index(cbuilder)
        file_id += 1
    cdll.delete_Builder(cbuilder)
    print('')
    print('[Total # of documents] {}\n[Total # of output files] {}'.format(doc_id, file_id))
    return file_id


def step2(arg):
    cdll = init_dll(arg)
    cbuilder = cdll.get_Builder()
    cdll.set_inputbufs_size(cbuilder, arg.way, arg.ibuf)
    cdll.set_outputbuf_size(cbuilder, arg.obuf)
    # merge based on filename
    # the first int is sequence of merge run, second int is sequence of this run
    index_filename = os.path.join(arg.o, 'index{}_{}.'+arg.ext)
    lexicon_filename = os.path.join(arg.o, 'lexicon{}_{}.'+arg.ext)
    # merge run
    run = 0
    # num of input or output files for each run
    num_in = arg.m
    num_out = math.ceil(num_in / arg.way)
    # num_out == 1 is the final merge
    while num_out > 1:
        print('[merge run {}]'.format(run+1))

        for out_i in range(num_out):
            # for each merged output file, get corresponding input files
            in_start = out_i * arg.way
            in_end = min(in_start+arg.way, num_in)
            in_index_filelist = []
            in_lexicon_filelist = []
            for in_i in range(in_start, in_end):
                in_index_filelist.append(index_filename.format(run, in_i).encode('ascii'))
                in_lexicon_filelist.append(lexicon_filename.format(run, in_i).encode('ascii'))
            # ouput file
            out_index_file = index_filename.format(run+1, out_i).encode('ascii')
            out_lexicon_file = lexicon_filename.format(run+1, out_i).encode('ascii')
            # open file
            cdll.open_inputbufs_file(cbuilder, in_index_filelist, in_lexicon_filelist)
            cdll.open_outputbuf_file(cbuilder, out_index_file, out_lexicon_file)
            print('\ninput  index:  ', in_index_filelist)
            print('input  lexicon:', in_lexicon_filelist)
            print('output index:   ', out_index_file)
            print('output lexicon: ', out_lexicon_file)
            print('')
            # merge
            cdll.merge(cbuilder)

        # next run
        run += 1
        num_in = num_out
        num_out = math.ceil(num_in / arg.way)
        print('')
    
    print('[merge run final]')
    # get input files
    in_end = min(arg.way, num_in)
    in_index_filelist = []
    in_lexicon_filelist = []
    for in_i in range(0, in_end):
        in_index_filelist.append(index_filename.format(run, in_i).encode('ascii'))
        in_lexicon_filelist.append(lexicon_filename.format(run, in_i).encode('ascii'))
    # output file 
    out_index_file = os.path.join(arg.o, 'index.'+arg.ext).encode('ascii')
    out_lexicon_file = os.path.join(arg.o, 'lexicon.'+arg.ext).encode('ascii')
    # open file
    cdll.open_inputbufs_file(cbuilder, in_index_filelist, in_lexicon_filelist)
    cdll.open_outputbuf_file(cbuilder, out_index_file, out_lexicon_file)
    print('\ninput  index:  ', in_index_filelist)
    print('input  lexicon:', in_lexicon_filelist)
    print('output index:   ', out_index_file)
    print('output lexicon: ', out_lexicon_file)
    print('')
    # merge
    cdll.merge(cbuilder)
    cdll.delete_Builder(cbuilder)



if __name__ == '__main__':
    arg = get_arguments()
    if not os.path.exists(arg.o):
        os.makedirs(arg.o)
    if arg.step1:
        assert(arg.i is not None)
        t = time.time()
        step1(arg)
        t = time.time() - t
        print('[Elapsed time] {:d}min {:.1f}sec'.format(int(t//60), t-(t//60)*60))
    elif arg.step2:
        assert(arg.m >=1 and arg.way >= 2)
        t = time.time()
        step2(arg)
        t = time.time() - t
        print('[Elapsed time] {:d}min {:.1f}sec'.format(int(t//60), t-(t//60)*60))
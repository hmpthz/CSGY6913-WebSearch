from random import random
from typing import List, Tuple
import ctypes
import re
from collections import Counter
import time
import pickle
import multiprocessing as mp
import os

s1 = 'Caroline · 4 years ago4 1 Comment (Stefan-Boltzmann law) 4L\n = 4pi*R^2*sigma*T^4 Solving for R we get: => R = (1/ (2T^2)) * sqrt (L/ (pi*sigmá)) Plugging in your values you should get: => R = (1/ (2 (11,000K)^2)) *sqrt ( (2.7*10^32W)/ (pi * (5.67*10^-8 W/m^2K^4))) R = 1.609 * 10^11 m?\n'
s2 = '* machacársela = jerk + ®  ®  Reflexivo + off ; wank ; jag off. á® ĐĐĐĐ'
s3 = '000000001dss Standard Test\'s I\'m 00000 0 000234 9990001á®sDFD88sderbVV 0099d测试dfd000880sdĐĐĐĐv11mmmm'
s4 = 'Use the standard get and set methods to query and control the standard MATLAB ® axes properties of a map axes.'
s5 = 'Geográphic látitude limits of the display area, specified as a  two-element vector of the form  [southern_limit  northern_limit].'
s6 = 'Join us right here!04-05-2015 03:29 AM 381 2tools156https://forums.androidcentral.com/showthread.php?t=473134&p=4131822&viewfull=1#post4131822Chris Beaudoin R. aahttp://www.medicalnewstoday.com/articles/311152.php\n'
s7 = 'P+��OT#�����귴!��a�\5 (6Z4��m�����FC��I��\'뽝�i��P��#�"Yd���3��O���o�>\'t9�=l����"�A=��ڹbe����է����4��x���QP���*�����֟���1�S37ܙ�a�L���q�2�@w�R�i [��m���B��Vy����o'


def read_obj(filename):
    with open(filename, 'rb') as f:
        obj = pickle.load(f)
    return obj

def write_obj(obj, filename):
    with open(filename, 'wb') as f:
        pickle.dump(obj, f)

def functime(func, n, *args, **kwargs):
    t = time.time()
    for i in range(n):
        func(*args, *kwargs)
    t = time.time() - t
    print(t/n)


def f():
    MB = 2**20
    with open('D:\\Downloads\\fulldocs-new.trec', 'rb') as f:
        content = f.read(int(20*MB))

    with open('test.xml', 'wb') as f:
        f.write(content)

def merge_intervals(intervals: List[Tuple[int,int]]) -> List[Tuple[int,int]]:
    intervals = sorted(intervals, key=lambda x:x[0])
    ret = []
    for i in intervals:
        newInterval = i
        #compare with the last one in ret
        if len(ret) > 0 and ret[-1][1] >= i[0]:
            #merge and then replace the last one in ret
            newInterval = ret.pop()
            if i[1] > newInterval[1]:
                newInterval = (newInterval[0], i[1])
        ret.append(newInterval)
    return ret


def test_tokenize(s) -> List[str]:
    re_url = re.compile(r'https?://\S+')
    re_numbers = re.compile(r'0*(\d+)')
    re_tk = re.compile(r'[^\W_]+')

    s = re_url.sub('', s)
    s = re_numbers.sub(r' \1 ', s)
    return re_tk.findall(s.lower())

def test_parse1():
    '''
    apply regex and tokenize for every line
    '''
    file = open('test.xml', 'r', encoding='utf-8')
    is_text = False

    line = file.readline()
    while line:
        if line == '<DOC>\n':
            start_off = file.tell() - 6
        elif line == '<TEXT>\n':
            # next line should be url according to .trec file
            line = file.readline()
            url = line.rstrip().encode('ascii')
            # further lines should be text content
            tokens = []
            is_text = True
        elif line == '</TEXT>\n':
            for i in range(len(tokens)):
                a = tokens[i]
            is_text = False
        elif is_text:
            tokens.extend(s.encode('utf-8') for s in test_tokenize(line))
        line = file.readline()


def test_parse2():
    '''
    join all lines in one text string then apply regex and tokenize
    (slightly faster)
    '''
    file = open('test.xml', 'r', encoding='utf-8')
    is_text = False

    line = file.readline()
    while line:
        if line == '<DOC>\n':
            start_off = file.tell() - 6
        elif line == '<TEXT>\n':
            # next line should be url according to .trec file
            line = file.readline()
            url = line.rstrip().encode('ascii')
            # further lines should be text content
            lines = []
            is_text = True
        elif line == '</TEXT>\n':
            tokens = [s.encode('utf-8') for s in test_tokenize(''.join(lines))]
            for i in range(len(tokens)):
                a = tokens[i]
            is_text = False
        elif is_text:
            lines.append(line)
        line = file.readline()


def print_doc_tokens(offset):
    file = open('D:\\Downloads\\fulldocs-new.trec', 'r', encoding='utf-8')
    file.seek(offset)
    is_text = False

    line = file.readline()
    while line:
        if line == '<TEXT>\n':
            # next line should be url according to .trec file
            line = file.readline()
            # further lines should be text content
            lines = []
            is_text = True
        elif line == '</TEXT>\n':
            is_text = False
            break
        elif is_text:
            lines.append(line)
        line = file.readline()
    tokens = [s.encode('utf-8') for s in test_tokenize(''.join(lines))]
    print('the'.encode('utf-8') in tokens)


A = 9
def test_multiprocess():
    print(mp.current_process().name, A)

if __name__ == '__main__':
    # print(merge_intervals([(1,4),(7,10),(3,6),(9,13)]))
    # functime(test_parse1, 10)
    # functime(test_parse2, 10)
    # print_doc_tokens(5176544968)
    f()
    
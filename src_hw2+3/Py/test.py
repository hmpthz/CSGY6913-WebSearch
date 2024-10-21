from random import random
from typing import List, Tuple
import nltk.tokenize as TK
import ctypes
import re
from collections import Counter
import time
import pickle
import random

s1 = 'Caroline · 4 years ago4 1 Comment (Stefan-Boltzmann law) 4L\n = 4pi*R^2*sigma*T^4 Solving for R we get: => R = (1/ (2T^2)) * sqrt (L/ (pi*sigmá)) Plugging in your values you should get: => R = (1/ (2 (11,000K)^2)) *sqrt ( (2.7*10^32W)/ (pi * (5.67*10^-8 W/m^2K^4))) R = 1.609 * 10^11 m?\n'
s2 = '* machacársela = jerk + Reflexivo + off ; wank ; jag off. á® ĐĐĐĐ'
s3 = 'Your\'s insurer is “subrogated” to the rights of your\'ve policy and can\'t “step in your shoes” to recover any amount paid out on your behalf. \'\'\'a \n'
s4 = 'Use the standard get and set methods to query and control the standard MATLAB ® axes properties of a map axes.'
s5 = 'Geográphic látitude limits of the display area, specified as a  two-element vector of the form  [southern_limit  northern_limit].'
s6 = 'http://www.medicalnewstoday.com/articles/311152.aaphp^ PMID 10195220.^ http://www.medicalnewstoday.com/articles/311152.php^ Nielsen, R. http://www.medicalnewstoday.com/articles/311152.php^'
s7 = 'P+��OT#�����귴!��a�\5 (6Z4��m�����FC��I��\'뽝�i��P��#�"Yd���3��O���o�>\'t9�=l����"�A=��ڹbe����է����4��x���QP���*�����֟���1�S37ܙ�a�L���q�2�@w�R�i [��m���B��Vy����o'


def read_obj(filename):
    with open(filename, 'rb') as f:
        obj = pickle.load(f)
    return obj

def write_obj(obj, filename):
    with open(filename, 'wb') as f:
        pickle.dump(obj, f)


def f():
    MB = 10**6
    with open('D:\\Downloads\\fulldocs-new.trec', 'r', encoding='utf-8') as f:
        content = f.read(220*MB)

    with open('test.xml', 'w', encoding='utf-8') as f:
        f.write(content)


def get_file_offset():
    offsets = []
    f = open('D:\\Downloads\\fulldocs-new.trec', 'r', encoding='utf-8')

    t = time.time()
    line = f.readline()
    while line:
        if line == '<DOC>\n':
            i = f.tell() - 6
            print(i)
            offsets.append(i)
        line = f.readline()
    t = time.time() - t

    print('get file offset time', t, 'seconds')


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



#most_common_terms()
print(merge_intervals([(1,4),(7,10),(3,6),(9,13)]))
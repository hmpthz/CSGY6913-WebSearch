from argparse import ArgumentParser, Namespace, RawTextHelpFormatter
import os, ctypes, time
pjoin = os.path.join


def get_arguments() -> Namespace:
    parser = ArgumentParser(formatter_class=RawTextHelpFormatter)
    parser.add_argument('-d', type=str, required=True, help='directory for input and output files')
    parser.add_argument('--dll', type=str, required=True, help='c++ dll filename')
    parser.add_argument('--buf', type=int, required=True, help='buffer size in MB for input buffer and output buffer')
    parser.add_argument('--task', type=str, required=True, help='name of the task to be executed')
    parser.add_argument('--type', type=str, help='type of index to be used for the task')
    parser.add_argument('--bits', type=int, help='bit width of quantized score')

    return parser.parse_args()


def init_dll() -> ctypes.CDLL:
    cdll = ctypes.CDLL(arg.dll)
    # check PyInterface.h to see function descriptions
    cdll.load_doctable.argtypes = [ctypes.c_char_p]
    
    cdll.get_inputbuf_Origin.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_Origin.restype = ctypes.c_void_p
    cdll.get_inputbuf_UncompressedScore.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_UncompressedScore.restype = ctypes.c_void_p

    cdll.get_outputbuf_UncompressedScore.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_UncompressedScore.restype = ctypes.c_void_p

    cdll.precompute_to_UncompressedScore.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

    return cdll


def task_precompute():
    filename_doctbale = pjoin(arg.d, 'doctable.bin')
    cdll_get_input = index_cdll_gets['input']['origin']
    filenames_input = index_filenames['origin']
    cdll_get_output = index_cdll_gets['output']['uncompressed-score']
    filenames_output = index_filenames['uncompressed-score']
    print('#### precompute BM25 score ####')
    print('[input files] ', filename_doctbale, *filenames_input)
    print('[output files]', *filenames_output)
    print()

    index_input = cdll_get_input(arg.buf, *(f.encode('ascii') for f in filenames_input))
    index_output = cdll_get_output(arg.buf, True, *(f.encode('ascii') for f in filenames_output))

    cdll.load_doctable(filename_doctbale.encode('ascii'))
    cdll.precompute_to_UncompressedScore(index_input, index_output)


def task_quantize():
    pass


def task_speed_test():
    pass


def task_MSE():
    pass


if __name__ =='__main__':
    global arg, cdll
    global index_cdll_gets, index_filenames, tasks
    arg = get_arguments()
    cdll = init_dll()

    index_cdll_gets = {
        'input': {
            'origin': cdll.get_inputbuf_Origin,
            'uncompressed-score': cdll.get_inputbuf_UncompressedScore,
            'linear': {
                6: None,
                8: None,
                12: None
            },
            'log': {
                6: None,
                8: None,
                12: None
            },
            'adaptive-float': {
                6: None,
                8: None,
                12: None
            }
        },
        'output': {
            'uncompressed-score': cdll.get_outputbuf_UncompressedScore,
            'linear': {
                6: None,
                8: None,
                12: None
            },
            'log': {
                6: None,
                8: None,
                12: None
            },
            'adaptive-float': {
                6: None,
                8: None,
                12: None
            }
        }
    }

    index_filenames = {
        'origin': (pjoin(arg.d, 'lexicon.bin'), pjoin(arg.d, 'index.bin')),
        'uncompressed-score': (pjoin(arg.d, 's_lex.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 'score.bin')),
        'linear': {
            6: (pjoin(arg.d, 's_lex_linear6.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_linear6.bin')),
            8: (pjoin(arg.d, 's_lex_linear8.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_linear8.bin')),
            12: (pjoin(arg.d, 's_lex_linear12.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_linear12.bin'))
        },
        'log': {
            6: (pjoin(arg.d, 's_lex_log6.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_log6.bin')),
            8: (pjoin(arg.d, 's_lex_log8.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_log8.bin')),
            12: (pjoin(arg.d, 's_lex_log12.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_log12.bin'))
        },
        'adaptive-float': {
            6: (pjoin(arg.d, 's_lex_adapt6.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_adapt6.bin')),
            8: (pjoin(arg.d, 's_lex_adapt8.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_adapt8.bin')),
            12: (pjoin(arg.d, 's_lex_adapt12.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_adapt12.bin'))
        }
    }

    tasks = {
        'precompute': {
            'f': task_precompute
        },
        'quantize': {
            'f': task_quantize,
            'cdll_f': {
                'linear': {
                    6: None,
                    8: None,
                    12: None
                },
                'log': {
                    6: None,
                    8: None,
                    12: None
                },
                'adaptive-float': {
                    6: None,
                    8: None,
                    12: None
                }
            }
        },
        'speed-test': {
            'f': task_speed_test,
            'cdll_f': {
                'origin': None,
                'uncompressed-score': None,
                'linear': {
                    6: None,
                    8: None,
                    12: None
                },
                'log': {
                    6: None,
                    8: None,
                    12: None
                },
                'adaptive-float': {
                    6: None,
                    8: None,
                    12: None
                }
            }
        },
        'MSE': {
            'f': task_MSE,
            'cdll_f': {
                'linear': {
                    6: None,
                    8: None,
                    12: None
                },
                'log': {
                    6: None,
                    8: None,
                    12: None
                },
                'adaptive-float': {
                    6: None,
                    8: None,
                    12: None
                }
            }
        }
    }

    t = time.time()
    tasks[arg.task]['f']()
    t = time.time() - t
    print('[Elapsed time] {:d}min {:.1f}sec'.format(int(t//60), t-(t//60)*60))

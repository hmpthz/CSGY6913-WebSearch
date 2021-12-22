from argparse import ArgumentParser, Namespace, RawTextHelpFormatter
import os, ctypes, time
pjoin = os.path.join


def task_precompute():
    global t
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
    t = time.time()
    cdll.precompute_to_UncompressedScore(index_input, index_output)
    t = time.time() - t


def task_quantize():
    global t
    pass


def task_speed_test():
    global t
    if arg.type == 'origin':
        filename_doctbale = pjoin(arg.d, 'doctable.bin')
        cdll.load_doctable(filename_doctbale.encode('ascii'))
    

def task_MSE():
    global t
    pass



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
    cdll.get_inputbuf_linear6.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_linear6.restype = ctypes.c_void_p
    cdll.get_inputbuf_linear8.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_linear8.restype = ctypes.c_void_p
    cdll.get_inputbuf_linear10.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_linear10.restype = ctypes.c_void_p
    cdll.get_inputbuf_linear12.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_linear12.restype = ctypes.c_void_p
    cdll.get_inputbuf_log6.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_log6.restype = ctypes.c_void_p
    cdll.get_inputbuf_log8.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_log8.restype = ctypes.c_void_p
    cdll.get_inputbuf_log10.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_log10.restype = ctypes.c_void_p
    cdll.get_inputbuf_log12.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_log12.restype = ctypes.c_void_p
    cdll.get_inputbuf_adaptive6.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_adaptive6.restype = ctypes.c_void_p
    cdll.get_inputbuf_adaptive8.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_adaptive8.restype = ctypes.c_void_p
    cdll.get_inputbuf_adaptive10.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_adaptive10.restype = ctypes.c_void_p
    cdll.get_inputbuf_adaptive12.argtypes = [ctypes.c_uint32, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_inputbuf_adaptive12.restype = ctypes.c_void_p

    cdll.get_outputbuf_UncompressedScore.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_UncompressedScore.restype = ctypes.c_void_p
    cdll.get_outputbuf_linear6.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_linear6.restype = ctypes.c_void_p
    cdll.get_outputbuf_linear8.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_linear8.restype = ctypes.c_void_p
    cdll.get_outputbuf_linear10.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_linear10.restype = ctypes.c_void_p
    cdll.get_outputbuf_linear12.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_linear12.restype = ctypes.c_void_p
    cdll.get_outputbuf_log6.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_log6.restype = ctypes.c_void_p
    cdll.get_outputbuf_log8.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_log8.restype = ctypes.c_void_p
    cdll.get_outputbuf_log10.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_log10.restype = ctypes.c_void_p
    cdll.get_outputbuf_log12.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_log12.restype = ctypes.c_void_p
    cdll.get_outputbuf_adaptive6.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_adaptive6.restype = ctypes.c_void_p
    cdll.get_outputbuf_adaptive8.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_adaptive8.restype = ctypes.c_void_p
    cdll.get_outputbuf_adaptive10.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_adaptive10.restype = ctypes.c_void_p
    cdll.get_outputbuf_adaptive12.argtypes = [ctypes.c_uint32, ctypes.c_bool, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    cdll.get_outputbuf_adaptive12.restype = ctypes.c_void_p

    cdll.precompute_to_UncompressedScore.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_linear6.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_linear8.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_linear10.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_linear12.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_log6.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_log8.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_log10.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_log12.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_adaptive6.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_adaptive8.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_adaptive10.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.quantize_adaptive12.argtypes = [ctypes.c_void_p, ctypes.c_void_p]

    cdll.test_Origin.argtypes = [ctypes.c_void_p]
    cdll.test_UncompressedScore.argtypes = [ctypes.c_void_p]
    cdll.test_linear6.argtypes = [ctypes.c_void_p]
    cdll.test_linear8.argtypes = [ctypes.c_void_p]
    cdll.test_linear10.argtypes = [ctypes.c_void_p]
    cdll.test_linear12.argtypes = [ctypes.c_void_p]
    cdll.test_log6.argtypes = [ctypes.c_void_p]
    cdll.test_log8.argtypes = [ctypes.c_void_p]
    cdll.test_log10.argtypes = [ctypes.c_void_p]
    cdll.test_log12.argtypes = [ctypes.c_void_p]
    cdll.test_adaptive6.argtypes = [ctypes.c_void_p]
    cdll.test_adaptive8.argtypes = [ctypes.c_void_p]
    cdll.test_adaptive10.argtypes = [ctypes.c_void_p]
    cdll.test_adaptive12.argtypes = [ctypes.c_void_p]

    cdll.MSE_linear6.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_linear8.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_linear10.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_linear12.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_log6.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_log8.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_log10.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_log12.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_adaptive6.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_adaptive8.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_adaptive10.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    cdll.MSE_adaptive12.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    return cdll


if __name__ =='__main__':
    global arg, cdll
    global index_cdll_gets, index_filenames, tasks
    global t # time
    arg = get_arguments()
    cdll = init_dll()
    t = 0.0

    index_cdll_gets = {
        'input': {
            'origin': cdll.get_inputbuf_Origin,
            'uncompressed-score': cdll.get_inputbuf_UncompressedScore,
            'linear': {
                6: cdll.get_inputbuf_linear6,
                8: cdll.get_inputbuf_linear8,
                10: cdll.get_inputbuf_linear10,
                12: cdll.get_inputbuf_linear12
            },
            'log': {
                6: cdll.get_inputbuf_log6,
                8: cdll.get_inputbuf_log8,
                10: cdll.get_inputbuf_log10,
                12: cdll.get_inputbuf_log12
            },
            'adaptive-float': {
                6: cdll.get_inputbuf_adaptive6,
                8: cdll.get_inputbuf_adaptive8,
                10: cdll.get_inputbuf_adaptive10,
                12: cdll.get_inputbuf_adaptive12
            }
        },
        'output': {
            'uncompressed-score': cdll.get_outputbuf_UncompressedScore,
            'linear': {
                6: cdll.get_outputbuf_linear6,
                8: cdll.get_outputbuf_linear8,
                10: cdll.get_outputbuf_linear10,
                12: cdll.get_outputbuf_linear12
            },
            'log': {
                6: cdll.get_outputbuf_log6,
                8: cdll.get_outputbuf_log8,
                10: cdll.get_outputbuf_log10,
                12: cdll.get_outputbuf_log12
            },
            'adaptive-float': {
                6: cdll.get_outputbuf_adaptive6,
                8: cdll.get_outputbuf_adaptive8,
                10: cdll.get_outputbuf_adaptive10,
                12: cdll.get_outputbuf_adaptive12
            }
        }
    }

    index_filenames = {
        'origin': (pjoin(arg.d, 'lexicon.bin'), pjoin(arg.d, 'index.bin')),
        'uncompressed-score': (pjoin(arg.d, 's_lex.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 'score.bin')),
        'linear': {
            6: (pjoin(arg.d, 's_lex_linear6.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_linear6.bin')),
            8: (pjoin(arg.d, 's_lex_linear8.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_linear8.bin')),
            10: (pjoin(arg.d, 's_lex_linear10.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_linear10.bin')),
            12: (pjoin(arg.d, 's_lex_linear12.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_linear12.bin'))
        },
        'log': {
            6: (pjoin(arg.d, 's_lex_log6.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_log6.bin')),
            8: (pjoin(arg.d, 's_lex_log8.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_log8.bin')),
            10: (pjoin(arg.d, 's_lex_log10.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_log10.bin')),
            12: (pjoin(arg.d, 's_lex_log12.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_log12.bin'))
        },
        'adaptive-float': {
            6: (pjoin(arg.d, 's_lex_adapt6.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_adapt6.bin')),
            8: (pjoin(arg.d, 's_lex_adapt8.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_adapt8.bin')),
            10: (pjoin(arg.d, 's_lex_adapt10.bin'), pjoin(arg.d, 's_index.bin'), pjoin(arg.d, 's_adapt10.bin')),
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
                    6: cdll.quantize_linear6,
                    8: cdll.quantize_linear8,
                    10: cdll.quantize_linear10,
                    12: cdll.quantize_linear12
                },
                'log': {
                    6: cdll.quantize_log6,
                    8: cdll.quantize_log8,
                    10: cdll.quantize_log10,
                    12: cdll.quantize_log12
                },
                'adaptive-float': {
                    6: cdll.quantize_adaptive6,
                    8: cdll.quantize_adaptive8,
                    10: cdll.quantize_adaptive10,
                    12: cdll.quantize_adaptive12
                }
            }
        },
        'speed-test': {
            'f': task_speed_test,
            'cdll_f': {
                'origin': cdll.test_Origin,
                'uncompressed-score': cdll.test_UncompressedScore,
                'linear': {
                    6: cdll.test_linear6,
                    8: cdll.test_linear8,
                    10: cdll.test_linear10,
                    12: cdll.test_linear12
                },
                'log': {
                    6: cdll.test_log6,
                    8: cdll.test_log8,
                    10: cdll.test_log10,
                    12: cdll.test_log12
                },
                'adaptive-float': {
                    6: cdll.test_adaptive6,
                    8: cdll.test_adaptive8,
                    10: cdll.test_adaptive10,
                    12: cdll.test_adaptive12
                }
            }
        },
        'MSE': {
            'f': task_MSE,
            'cdll_f': {
                'linear': {
                    6: cdll.MSE_linear6,
                    8: cdll.MSE_linear8,
                    10: cdll.MSE_linear10,
                    12: cdll.MSE_linear12
                },
                'log': {
                    6: cdll.MSE_log6,
                    8: cdll.MSE_log8,
                    10: cdll.MSE_log10,
                    12: cdll.MSE_log12
                },
                'adaptive-float': {
                    6: cdll.MSE_adaptive6,
                    8: cdll.MSE_adaptive8,
                    10: cdll.MSE_adaptive10,
                    12: cdll.MSE_adaptive12
                }
            }
        }
    }

    tasks[arg.task]['f']()
    print('[Elapsed time] {:d}min {:.1f}sec'.format(int(t//60), t-(t//60)*60))

#Reference paper: https://arxiv.org/pdf/1909.13271.pdf (T. Tambe et al.)

import numpy as np
np.set_printoptions(suppress=True)

def adaptivfloat_origin(float_arr, n_bits=8, n_exp=4, bias = None):
    n_mant = n_bits-1-n_exp

    # 1. store sign value and do the following part as unsigned value
    sign = np.sign(float_arr)
    float_arr = abs(float_arr)

    # 1.5  if bias not determined, auto set exponent bias by the maximum input 
    if (bias == None):
        bias_temp = np.frexp(float_arr.max())[1]-1
        bias = bias_temp - (2**n_exp - 1)

    # 2. limits the range of output float point
    min_exp = 0+bias
    max_exp = 2**(n_exp)-1+bias

    ## min and max values of adaptivfloat
    min_value = 2.0**min_exp*(1+2.0**(-n_mant))
    max_value = (2.0**max_exp)*(2.0-2.0**(-n_mant))
    # print(min_exp, bias_temp, max_exp, min_value, max_value)

    #print(min_value, max_value)
    ## 2.1. reduce too small values to zero
            
    float_arr[float_arr < 0.5*min_value] = 0
    float_arr[(float_arr > 0.5*min_value)*(float_arr < min_value)] = min_value

    ## 2.2. reduce too large values to max value of output format
    float_arr[float_arr > max_value] = max_value

    # 3. get mant, exp (the format is different from IEEE float)
    mant, exp = np.frexp(float_arr)
    # print(mant, exp)

    # 3.1 change mant, and exp format to IEEE float format
    # no effect for exponent of 0 outputs
    mant = 2*mant
    exp = exp-1
    power_exp = np.exp2(exp)
    ## 4. quantize mantissa
    scale = 2**(-n_mant) ## e.g. 2 bit, scale = 0.25
    q_mant = (mant/scale).round()
    q_mant[q_mant > 0] = q_mant[q_mant > 0] - 2**n_mant
    mant = q_mant.copy()
    mant[mant > 0] = mant[mant > 0] + 2**n_mant
    mant = mant*scale
    print(q_mant)

    float_out = sign*power_exp*mant
        
    float_out = float_out.astype("float32")
    print(float_out)
    return float_out


def quantize_mant(sign, abs_mant, n_mant):
    eps = 0.501
    # mant: [0.5, 1.0) -> [0.0, 1.0)
    abs_mant = 2*abs_mant-1
    q_mant = np.zeros(abs_mant.shape[0], dtype=int)
    # q_mant: [2**m, 2**(m+1)-1]
    q_mant[sign > 0] = np.round(abs_mant[sign > 0] * (2**n_mant-eps)) + 2**n_mant
    # q_mant: [1, 2**m-1]
    q_mant[sign < 0] = np.round(abs_mant[sign < 0] * (2**n_mant-1-eps)) + 1
    return q_mant

def dequantize_mant(q_mant, n_mant):
    eps = 0.501
    pos_mask = q_mant >= 2**n_mant
    neg_mask = (q_mant >= 1) & (q_mant < 2**n_mant)
    q_mant[pos_mask] -= 2**n_mant
    q_mant[neg_mask] -= 1
    mant = np.zeros(q_mant.shape[0], dtype=float)
    mant[pos_mask] = q_mant[pos_mask] / (2**n_mant-eps)
    mant[neg_mask] = q_mant[neg_mask] / (2**n_mant-1-eps)
    mant[mant != 0] = 0.5 * (mant[mant != 0] + 1)
    return mant


def adaptivfloat(arr, n_bits=8, n_exp=4):
    n_mant = n_bits-1-n_exp

    # 1. store sign value and do the following part as unsigned value
    sign = np.sign(arr)
    arr = np.abs(arr)

    # 1.5  if bias not determined, auto set exponent bias by the maximum input 
    exp_max = np.frexp(arr.max())[1]-1
    exp_bias = exp_max - (2**n_exp - 1)

    ## min and max values of adaptivfloat
    value_min = 2.0**exp_bias*(1+2.0**(-n_mant))
    value_max = (2.0**exp_max)*(2.0-2.0**(-n_mant))

    ## 2.1. reduce too small values to zero
    arr[arr < 0.5*value_min] = 0
    arr[(arr > 0.5*value_min) & (arr < value_min)] = value_min

    ## 2.2. reduce too large values to max value of output format
    arr[arr > value_max] = value_max

    # 3. get mant, exp (the format is different from IEEE float)
    mant, exp = np.frexp(arr)
    q_mant = quantize_mant(sign, mant, n_mant)
    mant = dequantize_mant(q_mant, n_mant)
    print(sign * mant * 2.0**exp)


def test_round_strategy(arr, n):
    # strategy 1
    arr1 = arr.copy()
    arr1 = np.round(arr1 * (2**n-0.501))
    arr1 = arr1 / (2**n-0.501)
    # strategy 2
    arr2 = arr.copy()
    arr2 = np.round(arr2 * (2**n-1))
    arr2 = arr2 / (2**n-1)
    # for x1, x2, x3 in zip(arr, arr1, arr2):
    #     print(x1, x2, x3)
    mse1 = np.square(arr-arr1).mean()
    mse2 = np.square(arr-arr2).mean()
    print(mse1, mse2)

# adaptivfloat_origin(np.array([-0.0015, -0.74, 12, 0]))
adaptivfloat(np.array([-0.0015, -0.74, 12, 0]))
# test_round_strategy(np.random.rand(100), 3)
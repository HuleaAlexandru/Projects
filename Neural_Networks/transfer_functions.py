import numpy as np

def identity(x, derivate = False):
    return x if not derivate else np.ones(x.shape)

def logisticOnValue(x):
	return  1. / (1 + np.power(np.e, -x))

def logisticOnValueDerivate(x):
	return x * (1 - x)

def logistic(x, derivate = False):
    return np.array(map(logisticOnValue, x) if not derivate else map(logisticOnValueDerivate, x))

def hyperbolicTangentOnValue(x):
	return ((np.power(np.e, x)) - (np.power(np.e, -x))) * 1./ ((np.power(np.e, x)) + (np.power(np.e, -x)))

def hyperbolicTangentOnValueDerivate(x):
	return 1 - x * x

def hyperbolic_tangent(x, derivate = False):
    result = np.array(map(hyperbolicTangentOnValue, x) if not derivate else map(hyperbolicTangentOnValueDerivate, x))
    return result

def soft(x):
    result = np.zeros((len(x), 1))
    sum = 0
    for i in range(len(x)):
        sum += (np.power(np.e, x[i]))

    for i in range(len(x)):
        result[i] = (np.power(np.e, x[i])) / sum 
    return result

def softDerivate(x):
    return x - x * x

def softMaxFunction(x, derivate = False):
    result = soft(x) if not derivate else softDerivate(x)
    return result

def relu(x):
    (m,h,w) = x.shape
    result = np.zeros((m,h,w))
    for i in range(m):
        for j in range(h):
            for k in range(w):
                result[i][j][k] = np.max(x[i][j][k], 0)
    return result

def reluDerivate(x):
    (m,h,w) = x.shape
    result = np.zeros((m,h,w))
    for i in range(m):
        for j in range(h):
            for k in range(w):
                if (x[i][j][k] > 0):
                     result[i][j][k] = 1
    return result

def reluFunction(x, derivate = False):
    result = relu(x) if not derivate else reluDerivate(x)
    return result

def maxPooling(x, stride):
    (m,h,w) = x.shape
    result = np.zeros((m, h / stride, w / stride))
    ids = np.zeros((m, h / stride, w / stride, 3))
    for i in range(m):
        for j in range(h):
            for k in range(w):
                if (j % stride == 0 and k % stride == 0):
                    result[i][j / stride][k / stride] = x[i][j][k]
                    ids[i][j / stride][k / stride] = [i, j, k]
                else:
                    if (result[i][j / stride][k / stride] < x[i][j][k]):
                        result[i][j / stride][k / stride] = x[i][j][k]
                        ids[i][j / stride][k / stride] = [i, j, k]
    return (result, ids)


def maxPoolingDerivate(x):
	(mo, ho, wo) = x.shape
	result = np.ones((mo, ho, wo))
	return result

def maxPoolingFunction(x, stride, derivate = False):
    result = maxPooling(x, stride) if not derivate else maxPoolingDerivate(x)
    return result

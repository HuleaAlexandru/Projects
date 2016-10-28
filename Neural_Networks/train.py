# Tudor Berariu, 2015
import numpy as np
from argparse import ArgumentParser

from data_loader_mnist import load_mnist
from data_loader_cifrar import load_cifrar
from feed_forward import FeedForward, LINEARIZE, FULLY_CONNECTED, TANH, SOFTMAX, CONV, MAX_POOLING, RELU
from transfer_functions import identity, logistic, hyperbolic_tangent

def eval_nn(nn, imgs, labels, maximum = 0):
	# Compute the confusion matrix
	confusion_matrix = np.zeros((10, 10), dtype = np.int)
	correct_no = 0
	how_many = imgs.shape[0] if maximum == 0 else maximum
	for i in range(imgs.shape[0])[:how_many]:
		y = np.argmax(nn.forward(imgs[i]))
		t = labels[i]
		if y == t:
			correct_no += 1
		confusion_matrix[t, y] += 1

	return float(correct_no) / float(how_many), confusion_matrix

def train_nn(nn, data, learning_rate, eval_every, stop):
	#pylab.ion()
	index = 0

	inputTrain = []
	outputTrain = []
	inputTest = []
	outputTest = []

	for i in np.random.permutation(data["train_no"]):

		index += 1
		print(index)
		nn.zero_gradients()
		outputs = nn.forward(data["train_imgs"][i])
		t = np.zeros((10, 1))
		t[data["train_labels"][i]] = 1.0
		error = -t / outputs
		nn.backward(data["train_imgs"][i], error)
		nn.update_parameters(learning_rate)

		# Evaluate the network
		if index % eval_every == 0:
			test_acc, test_cm = \
				eval_nn(nn, data["test_imgs"], data["test_labels"])
			train_acc, train_cm = \
				eval_nn(nn, data["train_imgs"], data["train_labels"], 5000)
			print("Train acc: %2.6f ; Test acc: %2.6f" % (train_acc, test_acc))
			#pylab.imshow(test_cm)
			print(test_cm)
			#pylab.draw()
			inputTrain.append(index)
			outputTrain.append(train_acc)
			inputTest.append(index)
			outputTest.append(test_acc)

		if (index == stop):
			break

	return (inputTrain, outputTrain, inputTest, outputTest)

def run(dataset, arhitecture, learning_rate, eval_every, stop):
	input_size = dataset["train_imgs"][0].shape
	nn = FeedForward(input_size, arhitecture)
	print(nn.to_string())

	return train_nn(nn, dataset, learning_rate, eval_every, stop)


if __name__ == "__main__":
	parser = ArgumentParser()
	parser.add_argument("--learning_rate", type = float, default = 0.001,
						help="Learning rate")
	parser.add_argument("--eval_every", type = int, default = 2000,
						help="Learning rate")
	args = parser.parse_args()
	
	#dataset = load_mnist()
	dataset = load_cifrar()

	input_size = dataset["train_imgs"][0].shape

	nn = FeedForward(input_size, [(CONV, (6, 28, 28), 5, 1), (RELU, -1), (MAX_POOLING, (6, 14, 14)), (CONV, (16, 10, 10), 5, 1), (RELU, -1), (MAX_POOLING, (16, 5, 5)), 
		(LINEARIZE, -1), (FULLY_CONNECTED, 120), (FULLY_CONNECTED, 84), (FULLY_CONNECTED, 10) ,(SOFTMAX, -1)])
	#nn = FeedForward(input_size, [(LINEARIZE, -1), (FULLY_CONNECTED, 300), (TANH, -1), (FULLY_CONNECTED, 100), (TANH, -1), (FULLY_CONNECTED, 10), (SOFTMAX, -1)])
	print(nn.to_string())

	train_nn(nn, dataset, args.learning_rate, args.eval_every, 10000)

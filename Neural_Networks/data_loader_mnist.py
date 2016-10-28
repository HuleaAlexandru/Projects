import numpy as np

from os.path import exists
from os import mkdir, system
from mnist import MNIST

import matplotlib
matplotlib.use('TkAgg')
import pylab

MNIST_PATH = "./MNIST"
FILES = [
	"train-images-idx3-ubyte.gz",
	"train-labels-idx1-ubyte.gz",
	"t10k-images-idx3-ubyte.gz",
	"t10k-labels-idx1-ubyte.gz",
]
BASE_URL = "http://yann.lecun.com/exdb/mnist"

def download_mnist():
	if not exists(MNIST_PATH):
		mkdir(MNIST_PATH)
	for f in FILES:
		if not exists("%s/%s" % (MNIST_PATH, f[:-3])):
			system("wget %s/%s -O %s/%s" % (BASE_URL, f, MNIST_PATH, f))
			system("gunzip %s/%s" % (MNIST_PATH, f[:-3]))


def preprocess(train_imgs, test_imgs):
	avg = np.mean(train_imgs)
	dev = np.std(train_imgs)

	train_imgs -= avg
	train_imgs /= dev
	test_imgs -= avg
	test_imgs /= dev


def load_mnist():
	download_mnist()
	mnist_data = MNIST(MNIST_PATH)
	train_imgs, train_labels = mnist_data.load_training()
	test_imgs, test_labels = mnist_data.load_testing()
	data = {}

	for i in range(len(train_imgs)):
		square = []
		for j in range(1024):
			row = j / 32
			col = j % 32
			if (row < 2 or col < 2 or row > 29 or col > 29):
				square.append(0)
			else:
				val = train_imgs[i][(row - 2) * 28 + col - 2]
				square.append(val)
		train_imgs[i] = square

	for i in range(len(test_imgs)):
		square = []
		for j in range(1024):
			row = j / 32
			col = j % 32
			if (row < 2 or col < 2 or row > 29 or col > 29):
				square.append(0)
			else:
				val = test_imgs[i][(row - 2) * 28 + col - 2]
				square.append(val)
		test_imgs[i] = square

	data["train_imgs"] = np.array(train_imgs, dtype="f").reshape(60000, 1, 32, 32)
	data["test_imgs"] = np.array(test_imgs, dtype="f").reshape(10000, 1, 32, 32)
	data["train_labels"] = np.array(train_labels)
	data["test_labels"] = np.array(test_labels)

	preprocess(data["train_imgs"], data["test_imgs"])

	data["train_no"] = 60000
	data["test_no"] = 10000

	return data

if __name__ == "__main__":
	data = load_mnist()
	
	rows_no, cols_no = (10, 10)
	full_img = np.zeros((0, 32 * cols_no))
	labels = np.zeros((rows_no, cols_no), dtype=int)
	for row_no in range(rows_no):
		row = np.zeros((32, 0))
		for col_no in range(cols_no):
			idx = np.random.randint(data["train_imgs"].shape[0])
			labels[(row_no, col_no)] = data["train_labels"][idx]
			row = np.concatenate((row, data["train_imgs"][idx].reshape(32, 32)), axis = 1)
		full_img = np.concatenate((full_img, row), axis = 0)

	#x = full_img.shape
	#if (len(x) == 2):
	#	x = x + (1,)
	#print(x)
	
	print(labels)
	print(full_img)
	pylab.imshow(full_img, cmap="Greys_r")
	pylab.show()
	
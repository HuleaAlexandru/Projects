# Tudor Berariu, 2016

import numpy as np
import tarfile as tar
import cPickle as cp

from os.path import exists
from os import mkdir, system, listdir, remove, rename, rmdir


import matplotlib
matplotlib.use('TkAgg')
import pylab

CIFRAR_PATH = "./CIFRAR"
ARCHIVE_NAME = "archive.tar.gz"
FOLDER_NAME = "cifar-10-batches-py"

DATA_FILES = [
	"data_batch_1",
	"data_batch_2",
	"data_batch_3",
	"data_batch_4",
	"data_batch_5"
]

TEST_FILES = ["test_batch"]

META_FILES = ["batches.meta"]

BASE_URL = "https://www.cs.toronto.edu/~kriz/cifar-10-python.tar.gz"

def download_cifrar():
	if not exists(CIFRAR_PATH):
		mkdir(CIFRAR_PATH)
		system("wget %s -O %s/%s" % (BASE_URL, CIFRAR_PATH, ARCHIVE_NAME))
		tarFile = tar.open(CIFRAR_PATH + "/" + ARCHIVE_NAME, "r:gz")
		tarFile.extractall(CIFRAR_PATH)
		tarFile.close()

		remove(CIFRAR_PATH + "/" + ARCHIVE_NAME)
		for f in listdir(CIFRAR_PATH + "/" + FOLDER_NAME):
			rename(CIFRAR_PATH + "/" + FOLDER_NAME + "/" + f, CIFRAR_PATH + "/" + f)
		rmdir(CIFRAR_PATH + "/" + FOLDER_NAME)

def unpickle(file):
    fo = open(file, 'rb')
    dict = cp.load(fo)
    fo.close()
    return dict

def preprocess(train_imgs, test_imgs):
	avg = np.mean(train_imgs)
	dev = np.std(train_imgs)

	train_imgs -= avg
	train_imgs /= dev
	test_imgs -= avg
	test_imgs /= dev

def load_cifrar():
	download_cifrar()
	
	train_imgs = np.zeros((0, 3072))
	train_labels = []
	test_imgs = np.zeros((0, 3072))
	test_labels = []
	data = {}

	for f in DATA_FILES:
		dict = unpickle(CIFRAR_PATH + "/" + f)
		extractedData = dict["data"]
		labels = dict["labels"]
		train_imgs = np.concatenate((train_imgs, extractedData), axis = 0)
		train_labels += labels

	data["train_imgs"] = np.array(train_imgs, dtype="f").reshape(50000, 3, 32, 32)
	data["train_labels"] = np.array(train_labels)

	for f in TEST_FILES:
		dict = unpickle(CIFRAR_PATH + "/" + f)
		extractedData = dict["data"]
		labels = dict["labels"]
		test_imgs = np.concatenate((test_imgs, extractedData), axis = 0)
		test_labels += labels
	
	data["test_imgs"] = np.array(test_imgs, dtype="f").reshape(10000, 3, 32, 32)
	data["test_labels"] = np.array(test_labels)

	label_names = []
	for f in META_FILES:
		dict = unpickle(CIFRAR_PATH + "/" + f)
		label_names += dict["label_names"]

	data["label_names"] = np.array(label_names)

	preprocess(data["train_imgs"], data["test_imgs"])

	data["train_no"] = 50000
	data["test_no"] = 10000

	return data
	
if __name__ == "__main__":
	data = load_cifrar()

	rows_no, cols_no = (10, 10)
	full_img = np.zeros((0, 32 * cols_no, 3))
	labels = np.zeros((rows_no, cols_no), dtype=int)
	for row_no in range(rows_no):
		row = np.zeros((32, 0, 3))
		for col_no in range(cols_no):
			idx = np.random.randint(data["train_imgs"].shape[0])
			labels[(row_no, col_no)] = data["train_labels"][idx]
			row = np.concatenate((row, data["train_imgs"][idx].reshape(32, 32, 3)), axis = 1)
		full_img = np.concatenate((full_img, row), axis = 0)

	print(labels)
	print(data["label_names"])
	pylab.imshow(full_img)
	pylab.show()
	
#!/usr/bin/env python3

import numpy as np
import sys

def rand_matrix(N, M, seed):
	return np.arange(seed, seed+N*M, dtype=np.float32).reshape(N, M) * 3.141

def emit(name, array, alignment='8'):
	print(".global %s" % name)
	print(".align " + alignment)
	print("%s:" % name)
	bs = array.tobytes()
	for i in range(0, len(bs), 4):
		s = ""
		for n in range(4):
			if i+3-n < len(bs):
				s += "%02x" % bs[i+3-n]
			else:
				s += "00"
		print("    .word 0x%s" % s)

# Define the filter size
if len(sys.argv) > 1:
	AVL = int(sys.argv[1])

else:
	AVL = 32


matrix1 = np.around(rand_matrix(AVL, AVL, 1)).astype(np.int64) % (10000)
np.random.shuffle(matrix1.flat)
matrix2 = np.around(rand_matrix(AVL, AVL, 1)).astype(np.int64) % (10000)
np.random.shuffle(matrix2.flat)

# Create the empty output matrix
empty_o = np.zeros((AVL, AVL)).astype(np.int64)

# Calculate the output matrix
result = np.matmul(matrix1, matrix2).astype(np.int64)

# Transpose matrix2
matrix2 = np.transpose(matrix2)

# Print information on file
with open('data.S', 'w') as f:
	sys.stdout = f
	print(".section .data,\"aw\",@progbits")
	emit("AVL", np.array(AVL, dtype=np.uint32))
	emit("matrix1", matrix1, '8')
	emit("matrix2", matrix2, '8')
	emit("output", empty_o, '8')
	emit("golden_o", result, '8')
	sys.stdout = sys.__stdout__


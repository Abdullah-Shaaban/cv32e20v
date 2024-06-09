#!/usr/bin/env python3

import numpy as np
import sys

def rand_matrix(N, M, seed):
	return np.arange(seed, seed+N*M, dtype=np.float32).reshape(N, M) * 3.141

def emit(name, array, alignment='4'):
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


# Generate a random int8 input padded image. Limit the values to 9 to avoid overflow
matrix1 = np.around(rand_matrix(AVL, AVL, 1)).astype(np.int8) % (10)
np.random.shuffle(matrix1.flat)

# Generate a random int8 input padded image. Limit the values to 9 to avoid overflow
matrix2 = np.around(rand_matrix(AVL, AVL, 1)).astype(np.int8) % (10)
np.random.shuffle(matrix2.flat)

# Create the empty output matrix
empty_o = np.zeros((AVL, AVL)).astype(np.int8)

# Calculate the output matrix
result = np.matmul(matrix1, matrix2).astype(np.int8)

# Calculate a checksum
checksum = np.sum(result, dtype=np.int8)

# Print information on file
with open('data.S', 'w') as f:
	sys.stdout = f
	print(".section .data,\"aw\",@progbits")
	emit("AVL", np.array(AVL, dtype=np.uint32))
	emit("matrix1", matrix1, '4')
	emit("matrix2", matrix2, '4')
	emit("output", empty_o, '4')
	emit("golden_o", result, '4')
	emit("o_checksum", checksum)
	sys.stdout = sys.__stdout__


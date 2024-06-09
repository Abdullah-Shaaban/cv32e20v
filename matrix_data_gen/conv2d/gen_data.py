#!/usr/bin/env python3

import numpy as np
import scipy.signal
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
    matrix_width = int(sys.argv[1])
    filter_size = int(sys.argv[2])
	# Filter size must be odd
    assert(filter_size % 2 == 1), "The filter size must be an odd integer number"
else:
    matrix_width = 6
    filter_size = 3

# Input image
M = matrix_width
N = matrix_width
padding = int(filter_size/2)
M_pad = M + 2*padding
N_pad = N + 2*padding
# assert(M % 4 == 0), "Output image dimension must be divisible by 4, pad the input image accordingly"
# assert(N % 4 == 0), "Output image dimension must be divisible by 4, pad the input image accordingly"

# Generate a random int8 input padded image. Limit the values to 9 to avoid overflow
image = np.around(rand_matrix(M_pad, N_pad, 1)).astype(np.int8) % (10)
np.random.shuffle(image.flat)

# Generate a random int8 filter. Limit the values to 4 to avoid overflow
gen_filter = np.around(rand_matrix(filter_size, filter_size, 0)).astype(np.int8) % (5)
np.random.shuffle(gen_filter.flat)

# Create the empty o matrix
empty_o = np.zeros((M, N)).astype(np.int8)

# Calculate the output matrix
result = np.around(scipy.signal.convolve2d(np.flip(gen_filter), image, 'valid')).astype(np.int8) # https://stackoverflow.com/questions/41613155/what-does-scipy-signal-convolve2d-calculate

# Calculate a checksum
checksum = np.sum(result, dtype=np.int8)

# Print matrices as a table of Hex values to a text file
with open('data.txt', 'w') as f:
	f.write("Image\n")
	np.savetxt(f, image.astype(np.ubyte), fmt='%02x')
	f.write("\nFilter\n")
	np.savetxt(f, gen_filter.astype(np.ubyte), fmt='%02x')
	f.write("\nResult\n")
	np.savetxt(f, result.astype(np.ubyte), fmt='%02x')

# Print information on file
with open('data.S', 'w') as f:
	sys.stdout = f
	print(".section .data,\"aw\",@progbits")
	emit("M", np.array(M, dtype=np.uint32))
	emit("N", np.array(N, dtype=np.uint32))
	emit("F", np.array(filter_size, dtype=np.uint32))
	emit("image", image, '4')
	emit("filter", gen_filter, '4')
	emit("output", empty_o, '4')
	emit("golden_o", result, '4')
	emit("o_checksum", checksum)
	sys.stdout = sys.__stdout__


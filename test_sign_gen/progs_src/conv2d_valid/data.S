.section .data,"aw",@progbits
.global M
.align 4
M:
    .word 0x00000008
.global N
.align 4
N:
    .word 0x00000008
.global F
.align 4
F:
    .word 0x00000003
.global image
.align 4
image:
    .word 0x05090208
    .word 0x02030501
    .word 0x02040004
    .word 0x07030202
    .word 0x08090303
    .word 0x03060605
    .word 0x00090502
    .word 0x02010109
    .word 0x01090803
    .word 0x00030807
    .word 0x05060507
    .word 0x06000604
    .word 0x06090607
    .word 0x01090004
    .word 0x07080509
    .word 0x07040602
.global filter
.align 4
filter:
    .word 0x02040001
    .word 0x03030004
    .word 0x00000001
.global output
.align 4
output:
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
.global golden_o
.align 4
golden_o:
    .word 0x5255524f
    .word 0x5c444440
    .word 0x4a4f4a62
    .word 0x64548569
    .word 0x5f763963
    .word 0x3d594368
    .word 0x59766179
    .word 0x77773048
    .word 0x67285a6f

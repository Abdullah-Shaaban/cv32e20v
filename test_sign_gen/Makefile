# COMP_OPTS = -static -mcmodel=medany -std=gnu99 -O2 -ffast-math -O2 -fno-common -Wall -march=rv32imacv
# LINK_OPTS = -static -nostdlib -nostartfiles -lm -lgcc -T $(PROG_PATH)/link.ld
PK_PATH=/home/abal/riscv32-unknown-elf/bin
SCRIPTS_PATH=./py_scripts
PROG_PATH=./
BIN_PATH=./programs_bin
HAMMER_PATH=/home/abal/hammer_vector
PROG ?= vssrl
VLEN ?= 128
TEST_TEST_DIR := ./progs_src/$(PROG)

# compile:
# 	riscv32-unknown-elf-gcc -o $(BIN_PATH)/$(PROG).elf $(PROG).c -static -march=rv32gcv
# 	riscv32-unknown-elf-objdump -D $(BIN_PATH)/$(PROG).elf > $(PROG).dasm

# Compile all files in the prog folder
TEST_FILES = $(filter %.c %.S,$(wildcard  $(TEST_TEST_DIR)/*))
compile: 
	riscv32-unknown-elf-gcc -O2 -g -static -mabi=ilp32 -march=rv32gcv -Wall -pedantic \
	-fno-tree-vectorize -fno-tree-loop-vectorize -fno-tree-slp-vectorize \
	-o $(BIN_PATH)/$(PROG).elf \
	$(TEST_FILES)
	riscv32-unknown-elf-objdump -d -S -M no-aliases -M numeric $(BIN_PATH)/$(PROG).elf > $(BIN_PATH)/$(PROG).dasm

simulate:
	spike --isa=RV32GCV --varch=vlen:$(VLEN),elen:32 $(PK_PATH)/pk $(BIN_PATH)/$(PROG).elf 
# spike --isa=RV32GCV --varch=vlen:$(VLEN),elen:32 -d $(PK_PATH)/pk $(BIN_PATH)/$(PROG).elf

# matmul-eth-64
do_64:
	riscv64-unknown-elf-gcc -O2 -g -static -march=rv64gcv -Wall -pedantic \
	-fno-tree-vectorize -fno-tree-loop-vectorize -fno-tree-slp-vectorize \
	-o $(BIN_PATH)/$(PROG).elf \
	$(TEST_FILES)
	riscv64-unknown-elf-objdump -d -S -M no-aliases -M numeric $(BIN_PATH)/$(PROG).elf > $(BIN_PATH)/$(PROG).dasm
	spike --isa=RV64GCV --varch=vlen:$(VLEN),elen:64 pk $(BIN_PATH)/$(PROG).elf

do_spfmatmul:
	riscv32-unknown-elf-gcc -O2 -g -static -march=rv32gcv -Wall -pedantic \
	-fno-tree-vectorize -fno-tree-loop-vectorize -fno-tree-slp-vectorize \
	-o $(BIN_PATH)/$(PROG).elf \
	$(TEST_FILES)
	riscv32-unknown-elf-objdump -d -S -M no-aliases -M numeric $(BIN_PATH)/$(PROG).elf > $(BIN_PATH)/$(PROG).dasm
	spike --isa=RV32GCV --varch=vlen:$(VLEN),elen:32 $(PK_PATH)/pk $(BIN_PATH)/$(PROG).elf

simulate_with_cmd:
	spike --isa=RV32GCV --varch=vlen:128,elen:32 -d --debug-cmd=$(PROG)_spike_commands $(PK_PATH)/pk $(BIN_PATH)/$(PROG).elf 2> logs/$(PROG)_log.txt
#-l --log=$(PROG)_spike_log
gen_golden_output:
	python3 $(SCRIPTS_PATH)/gen_golden_out.py $(HAMMER_PATH) $(BIN_PATH)/$(PROG).elf
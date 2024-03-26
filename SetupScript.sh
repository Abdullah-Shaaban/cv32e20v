#!/bin/bash

######################
# Load required tools
######################
module load questasim/2023.3
module load syn
module load formality
module load synopsys/ppower
module load verdi   #needs verdi to use fsdb2vcd
module load altair/grid-engine

##################################
# Environment and other variables
##################################
### Simulation variables
export CV_CORE=CV32E20 # When this is set, there is probably no need to set run from cv32e20/sim/uvmt
export CV_SW_TOOLCHAIN=/work/abal/riscv
export CV_SW_PREFIX=riscv32-corev-elf-
export CV_SW_MARCH=rv32imcv_zicsr
export CV_CORE_PATH=/pri/abal/V_Unit/cv32e20v/CVE2X # When this variable is defined, Common.mk will use it instead of cloning CVE2 repository
export CV_SIMULATOR=vsim # When this is set, there is no need for SIMULATOR=vsim in the make command
export CV_SW_CFLAGS="-fno-tree-vectorize -fno-tree-loop-vectorize -fno-tree-slp-vectorize"
# export CV_SIM_PREFIX= # "Prepended to all simulation compile and/or execution invocations. Can be used to invoke job-scheduling tool (e.g. LSF)."
                # This can be probably used to run on Grid Engine
SIM_DIR=cv32e20/sim/uvmt

### Synthesis variables
SYNTH_DIR=/pri/abal/V_Unit/cv32e20v/syn

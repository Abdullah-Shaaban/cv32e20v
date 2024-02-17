#!/bin/bash

######################
# Load required tools
######################
module load questasim/2022.3
module load syn/1912
module load formality/2018.06-sp1

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
# export CV_SIM_PREFIX= # "Prepended to all simulation compile and/or execution invocations. Can be used to invoke job-scheduling tool (e.g. LSF)."
                # This can be probably used to run on Grid Engine
SIM_DIR=cv32e20/sim/uvmt

### Synthesis variables
export IP_NAME=cve2x
export DESIGN_NAME=cve2_top
export VC_WORKSPACE=/pri/abal/V_Unit/Workspace_V # Necessary for some things in Nordic's scripts
export DESIGN_RTL_DIR=/pri/abal/V_Unit/cv32e20v/CVE2X/rtl # Necessary for filelist
SYNTH_DIR=/pri/abal/V_Unit/cv32e20v/syn


#############################
# Simulation
#############################

### Run the Hello World test
# make -C $SIM_DIR test TEST=hello-world SIMULATOR=vsim USE_ISS=0 ADV_DEBUG=YES GUI=NO WAVES=NO
# GUI=1 to open the simulator GUI
# WAVES=YES to enable waveform generation
# ADV_DEBUG=YES to enable the advanced debugging program for the selected simulator (e.g. Visualizer for QuestaSim)
# USE_ISS=1 to enable the ISS
# USER_RUN_FLAGS for Passing run-time arguments to the simulator
    # Example for UVM quit count: USER_RUN_FLAGS=+UVM_MAX_QUIT_COUNT=10,NO

### Run benchmark tests
make -C $SIM_DIR test TEST=arr_add SIMULATOR=vsim USE_ISS=0 ADV_DEBUG=YES GUI=NO WAVES=YES

### Open generated waveforms for debugging
# make -C $SIM_DIR waves TEST=hello-world SIMULATOR=vsim ADV_DEBUG=YES 

### Short CI Check
# cd core-v-verif/bin
# ci_check --core cv32e20 --simulator vsim --iss 0



#############################
# Synthesis (and LEC)
#############################

make -C $SYNTH_DIR synthesis LIB_SET=SC7P5T NOM_VOLTAGE=0P80V
make -C $SYNTH_DIR lec LIB_SET=SC7P5T NOM_VOLTAGE=0P80V
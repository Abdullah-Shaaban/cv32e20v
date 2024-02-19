#!/bin/bash

source SetupScript.sh

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
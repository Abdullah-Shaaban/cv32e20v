#!/bin/bash

source SetupScript.sh

#############################
# Initialize Variables
#############################
test=""
bench=""
def_bench=""
def_gls=""
ip_name="cve2v"
name_map_en="false"
waves_en="0"
error=0
activity_files_path=/pri/abal/V_Unit/cv32e20v/syn/activity_files
tests_path=/pri/abal/V_Unit/core-v-verif/cv32e20/sim/uvmt/vsim_results/default
sim_manifest=/pri/abal/V_Unit/cv32e20v/cv32e20v_manifest.flist
syn_run_name="cve2v"

# Help message
function help_message {
    echo "Usage: "
    echo "  source MasterScript.sh -do <sim, wave, syn, lec, pwr, name_map_check, gen_saif> \\"
    echo "       -ip_name <name> -syn_run_name <name> \\" 
    echo "       -test <test> -bench <scalar, vector> \\"
    echo "       -en_name_map <true, false> -en_waves <1,0> -en_gls_sim <1,0>"
    echo "Options:"
    echo "  -do: Function to run"
    echo "  -ip_name: Name of the IP (scalar: cve2 - Vector: cve2v)"
    echo "  -syn_run_name: Name of the synthesis run, used for results directory name (needed by both Syn and Pwr flows)"
    echo "  -test: Choose test to simulate (default: hello-world)"
    echo "  -bench: Enable scalar/vector benchmarking."
    echo "  -en_name_map: Enable name-map generation during synthesis (true, false)"
    echo "  -en_waves: Enable waveform dumping (1, 0)"
    echo "  -en_gls_sim: Enable GLS simulation (1, 0)"
    echo "  -help : Display help message"
    echo "Example 1: source MasterScript.sh -do sim -test axpy -bench vector -en_waves 1"
    echo "Example 2: source MasterScript.sh -do syn -en_name_map true -ip_name cve2v -syn_run_name cve2v_vlen64 -test conv2d-VLEN64-Mat8"
    echo "Example 3: source MasterScript.sh -do pwr -ip_name cve2v -syn_run_name cve2v_vlen64 -test conv2d-VLEN64-Mat8"
    echo "Example 4: source MasterScript.sh -do name_map_check -ip_name cve2v -test matmul-vec"
}

#############################
# Simulation
#############################

### Run benchmark tests
function handle_sim {

    if [[ $def_gls == "1" ]]
    then
        echo "Doing Gate-level Simulation"
        def_gls="+GLS"
        sim_manifest=/pri/abal/V_Unit/cv32e20v/cv32e20v_manifest-GLS.flist
    else
        echo "Doing RTL Simulation"
    fi

    echo "Test: $test"
    
    if [[ "$bench" == "scalar" ]] 
    then
        def_bench="+BENCHMARK"
        # Compile without vector extension
        export CV_SW_MARCH=rv32imc_zicsr
        echo "Benchmarking Scalar"
    
    elif [[ "$bench" == "vector" ]]
    then
        def_bench="+BENCHMARK+BNCH_VECTOR"
        echo "Benchmarking Vector"
    
    else
        echo "No Benchmarking"
    fi

    if [[ $waves_en == "1" ]]
    then
        echo "Waveform Dumping Enabled"
    fi

    make -C $SIM_DIR test TEST=$test SIMULATOR=vsim USE_ISS=0 ADV_DEBUG=1 GUI=0 WAVES=$waves_en USER_DEFINES=+define+CVE2V_SYNTH$def_bench$def_gls CV_CORE_MANIFEST=$sim_manifest
}

### Open generated waveforms for debugging
function open_wave {
    echo "Opening Waves"
    echo "Test: $test"
    make -C $SIM_DIR waves TEST=$test SIMULATOR=vsim ADV_DEBUG=1
}

### Short CI Check
# cd core-v-verif/bin
# ci_check --core cv32e20 --simulator vsim --iss 0


#############################
# Synthesis (and LEC)
#############################
function handle_syn {
    echo "Running Synthesis"
    make -C $SYNTH_DIR synthesis LIB_SET=SC7P5T NOM_VOLTAGE=0P80V IP_NAME=$ip_name RUN_NAME=$syn_run_name N_GEN_SAIF_MAP=$name_map_en SAIF_NAME=${ip_name}_${test}
}
function handle_lec {
    echo "Running LEC"
    make -C $SYNTH_DIR lec LIB_SET=SC7P5T NOM_VOLTAGE=0P80V IP_NAME=$ip_name
}

###########################
# Power Analysis
###########################
function handle_power {
    # Get benchmark start and end time from simulation results (vcd in ps, hence removing '.')
    logfile=$tests_path/$test/0/benchmark_results.log
    bnch_start_time=$(grep "Benchmark start time" "$logfile" | awk '{print $4}' | tr -d '.')
    bnch_end_time=$(grep "Benchmark end time" "$logfile" | awk '{print $4}' | tr -d '.')
    echo "Running Power Analysis"
    make -C $POWER_DIR power LIB_SET=SC7P5T NOM_VOLTAGE=0P80V IP_NAME=$ip_name TEST_NAME=$test SYN_RUN_NAME=$syn_run_name VCD_TIME_START=$bnch_start_time VCD_TIME_END=$bnch_end_time
}

###########################
# Name Mapping Check
###########################
function check_name_map {
    # # Convert qwave.db to vcd
    # qwave2vcd -wavefile $tests_path/$test/0/qwave.db -outfile $activity_files_path/$ip_name.vcd
    # # Get benchmark start and end time from simulation results (vcd in ps, hence removing '.')
    # logfile=$tests_path/$test/0/benchmark_results.log
    # bnch_start_time=$(grep "Benchmark start time" "$logfile" | awk '{print $4}' | tr -d '.')
    # bnch_end_time=$(grep "Benchmark end time" "$logfile" | awk '{print $4}' | tr -d '.')
    # # Convert vcd to saif
    # vcd2saif -input $activity_files_path/$ip_name.vcd -output $activity_files_path/$ip_name.saif -top uvmt_cv32e20_tb -instance uvmt_cv32e20_tb/dut_wrap/cv32e20_top_i -time $bnch_start_time $bnch_end_time
    echo "Checking Name Mapping"
    # make -C $SYNTH_DIR check_name_map LIB_SET=SC7P5T NOM_VOLTAGE=0P80V IP_NAME=$ip_name
}

###########################
# Make SAIF
###########################
function make_saif {
    # Get benchmark start and end time from simulation results (vcd in ps, hence removing '.')
    logfile=$tests_path/$test/0/benchmark_results.log
    bnch_start_time=$(grep "Benchmark start time" "$logfile" | awk '{print $4}' | tr -d '.')
    bnch_end_time=$(grep "Benchmark end time" "$logfile" | awk '{print $4}' | tr -d '.')
    # Convert qwave.db to vcd. The -designfile flag caused problems, so it was removed
    qwave2vcd -wavefile $tests_path/$test/0/qwave.db -outfile $activity_files_path/${ip_name}_$test.vcd -begin_time $bnch_start_time -end_time $bnch_end_time &> $activity_files_path/${ip_name}_${test}_qwave2vcd.log
    # Convert vcd to saif
    vcd2saif -input $activity_files_path/${ip_name}_$test.vcd -output $activity_files_path/${ip_name}_$test.saif -top uvmt_cv32e20_tb -instance uvmt_cv32e20_tb/dut_wrap/cv32e20_top_i &> $activity_files_path/${ip_name}_${test}_vcd2saif.log -time $bnch_start_time $bnch_end_time
}

# Check if no arguments are provided
if [ $# -eq 0 ]; then
    echo "No arguments provided"
    error=1
    help_message
fi

# Loop through arguments
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -do)
            func="$2"
            shift
            ;;
        -ip_name)
            ip_name="$2"
            shift
            ;;
        -syn_run_name)
            syn_run_name="$2"
            shift
            ;;
        -test)
            test="$2"
            shift
            ;;
        -bench)
            bench="$2"
            shift
            ;;
        -en_name_map)
            name_map_en="$2"
            shift
            ;;
        -en_waves)
            waves_en="$2"
            shift
            ;;
        -en_gls_sim)
            def_gls="$2"
            shift
            ;;
        -help)
            help_message
            ;;
        *)
            echo "Unknown argument: $key"
            error=1
            help_message
            ;;
    esac
    shift
done

# Run the function
if [ $error -eq 0 ]; then
    case $func in
        sim)
            handle_sim
            ;;
        wave)
            open_wave
            ;;
        syn)
            handle_syn
            ;;
        lec)
            handle_lec
            ;;
        pwr)
            handle_power
            ;;
        name_map_check)
            check_name_map
            ;;
        gen_saif)
            make_saif
            ;;
        *)
            echo "Unknown command: $func"
            help_message
            ;;
    esac
fi
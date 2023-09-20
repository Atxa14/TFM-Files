#!/bin/bash

# Function to execute the ns-3 scenario
run_scenario() {
    # Change directory to the ns-3 installation directory
    cd /home/user/Documents/ns3/ns-3-dev

    # Execute the ns-3 scenario with the incremented parameter
    ./ns3 run "scenario1.cc --mcs=7 --channelWidth=80 --nNetwork=4 --nStaA=10 --nStaB=10 --nStaC=10 --nStaD=10 --tracing=false --frequency=5 --seedNumber=$1 --runNumber=$2"
    ./ns3 run "scenario2.cc --mcs=7 --channelWidth=80 --nNetwork=3 --nStaA=25 --nStaB=25 --nStaC=25 --tracing=false --frequency=5 --seedNumber=$1 --runNumber=$2"
    ./ns3 run "scenario3.cc --mcs=7 --channelWidth=80 --nNetwork=5 --nStaA=50 --nStaB=50 --nStaC=50 --nStaD=50 --nStaE=50 --tracing=false --frequency=5 --seedNumber=$1 --runNumber=$2"
    ./ns3 run "scenario4.cc --mcs=7 --channelWidth=80 --nNetwork=5 --nStaA=50 --nStaB=50 --nStaC=50 --nStaD=50 --nStaE=50 --tracing=false --frequency=5 --seedNumber=$1 --runNumber=$2"
}

# Initialize the parameter value
runNumber=0
seedNumber=123
max_executions=200

# Loop to run the scenario every 10 minutes
while [ $seedNumber -lt $max_executions ]; do
    ((seedNumber++))
    run_scenario $seedNumber $runNumber
    while [ $runNumber -lt $max_executions ]; do
        ((runNumber++))
        run_scenario $seedNumber $runNumber
    done
    runNumber=1
done



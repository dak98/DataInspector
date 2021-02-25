#/bin/sh

source base.sh

# Go to the project's root directory
cd ..

DEVICES1="non-existent"
DEVICES2="eta-and-phi-histograms"
DEVICES3="eta-and-phi-histograms,etaphi-histogram"

# @Test
echo "> Selecting non-existent device for inspection should do nothing"
## Check if no proxy is running
before

start_analysis_task "-d=${DEVICES1}"

get_inspected_data "${DEVICES1}"
test_no_messages

after

printf "${PASS}\n"

# @Test
echo "> /inspected-data for non-existent device should return nothing"
## Check if no proxy is running
before

start_analysis_task "-d=${DEVICES3}"

get_inspected_data "${DEVICES1}"
test_no_messages

after

printf "${PASS}\n"

# Return to the catalog
cd test

#/bin/sh

source base.sh

# Go to the project's root directory
cd ..

DEVICES1="eta-and-phi-histograms,etaphi-histogram"
DEVICES2="eta-and-phi-histograms"

# @Test
echo "> /inspected-data should return nothing if no devices are selected"
## Check if no proxy is running
before

start_analysis_task

get_inspected_data "${DEVICES1}"
test_no_messages

after

printf "${PASS}\n"

# @Test
echo "> /inspected-data should return nothing if no messages are available"
## Check if no proxy is running
before

start_analysis_task

curl -X GET "${ADDRESS}/select-all" > /dev/null 2>&1

get_inspected_data "${DEVICES1}"
test_no_messages

after

printf "${PASS}\n"

# @Test
echo "> /inspected-data should return nothing for device which is not inspected"
## Check if no proxy is running
before

start_analysis_task "-d=${DEVICES1}"

get_inspected_data "output-wrapper"
test_no_messages

after

printf "${PASS}\n"

# @Test
echo "> /inspected-data should return inspected messages (1)"
## Check if no proxy is running
before

start_analysis_task "-d=${DEVICES1}"

get_inspected_data "${DEVICES1}"
test_result_nonempty "/inspected-data should return messages"

after

printf "${PASS}\n"

# @Test
echo "> /inspected-data should return inspected messages (2)"
## Check if no proxy is running
before

start_analysis_task "-d=${DEVICES1}"

get_inspected_data "${DEVICES2}"
test_result_nonempty "/inspected-data should return messages"

after

printf "${PASS}\n"

# @Test
echo "> /inspected-data should return inspected messages (3)"
## Check if no proxy is running
before

start_analysis_task "-d=${DEVICES2}"

get_inspected_data "${DEVICES1}"
test_result_nonempty "/inspected-data should return messages"

after

printf "${PASS}\n"

# Return to the catalog
cd test

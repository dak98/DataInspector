#/bin/sh

# Colors
GREEN='\033[1;32m'
NO_COLOR='\033[0m'
RED='\033[1;31m'

# Constants
ERROR_CODE=-1
FAIL="${RED}FAIL${NO_COLOR}"
PASS="${GREEN}PASS${NO_COLOR}"

# Common tests
HOST="127.0.0.1"
BASE="o2-analysistutorial-histograms --aod-file test/AO2D.root -b"
PORT=3000

ADDRESS="http://${HOST}:${PORT}"

test_base() {
    alienv enter O2/latest <<< "${BASE} $@" 2>&1 
}

test() {
    LOCATION="-a=${HOST} -p=${PORT}"
    alienv enter O2/latest <<< "./inspector ${BASE} ${LOCATION} $@" 2>&1
}

error() {
    printf "${FAIL}: $@\n"
    exit ${ERROR_CODE}
}    

test_result_empty() {
    if [[ -n "${RESULT}" ]]; then
	error "$@"
    fi
}

test_result_nonempty() {
    if [[ -z "${RESULT}" ]]; then
	error "$@"
    fi
}

is_proxy_running() {
    RESULT=`pgrep "\bproxy\b"`
}

before() {
    is_proxy_running
    test_result_empty "The proxy program is already running!"
}

# O2 analysis task commands
after() {
    curl -X GET "${ADDRESS}/stop"

    ## There is a delay before the proxy program stops after the command
    sleep 3

    is_proxy_running
    test_result_empty "The proxy program should not be running"
}

start_analysis_task() {
    test "$@" > /dev/null

    is_proxy_running
    test_result_nonempty "The proxy program should be running"
}

get_inspected_data() {
    ENDPOINT="${ADDRESS}/inspected-data"
    ARG1="devices: $@"
    ARG2="count: 100"
    RESULT="$(curl -X GET "${ENDPOINT}" -H "${ARG1}" -H "${ARG2}" 2> /dev/null)"
}

test_no_messages() {
    if [[ "${RESULT}" != "[]" ]]; then
	error "/inspected-data should return nothing"
    fi
}

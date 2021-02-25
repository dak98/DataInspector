#/bin/sh

source base.sh

# Go to the project's root directory
cd ..

after() {
    curl -X GET "${ADDRESS}/stop"

    ## There is a delay before the proxy program stops after the command
    sleep 3

    is_proxy_running
    test_result_empty "The proxy program should not be running"
}

# @Test
echo "> /stop should stop the proxy program"
## Check if no proxy is running
before

test > /dev/null

is_proxy_running
test_result_nonempty "The proxy program should be running"

after

printf "${PASS}\n"

# @Test
echo "> /available-devices should return the names"
## Check if no proxy is running
before

test > /dev/null

is_proxy_running
test_result_nonempty "The proxy program should be running"

RESULT=`curl -X GET "${ADDRESS}/available-devices" 2> /dev/null`
RESULT=`echo "${RESULT}" | wc -l`

if [[ "${RESULT}" -ne "4" ]]; then
    error "The workflow has 4 devices but "${RESULT}" returned"
fi

after

printf "${PASS}\n"

# @Test
echo "> /available-devices should return the same result every time"
## Check if no proxy is running
before

test > /dev/null

is_proxy_running
test_result_nonempty "The proxy program should be running"

RESULT1=`curl -X GET "${ADDRESS}/available-devices" 2> /dev/null`
RESULT2=`curl -X GET "${ADDRESS}/available-devices" 2> /dev/null`

if [[ "${RESULT1}" != "${RESULT2}" ]]; then
    error "Results of two /available-devices calls differ"
fi

after

printf "${PASS}\n"

# Return to the catalog
cd test

#/bin/sh

source base.sh

# Check if the proxy program is started (or not) correctly

# Go to the project's root directory
cd ..

# Common tests
after() {
    kill -9 "${RESULT}"
}

# @Test
echo "> Launching only the BASE case should not start the proxy"
## Check if no proxy is running
before

test_base > /dev/null

## Check if no proxy is running
is_proxy_running
test_result_empty "The proxy program should not be running!"

printf "${PASS}\n"

# @Test
echo "> Launching an O2 analysis task with --inspector should add DataInspector"
## Check if no DataInspector is added if launched without --inspector
RESULT=`test_base "-g"`
RESULT=`echo "${RESULT}" | grep "\bDataInspector\b"`
test_result_empty "DataInspector should be added only with --inspector"

## Check if --inspector adds DataInspector
RESULT=`test_base "--inspector -g"`
RESULT=`echo "${RESULT}" | grep "\bDataInspector\b"`
test_result_nonempty "--inspector should add DataInspector"

printf "${PASS}\n"

# @Test
echo "> Launching the BASE case with the plugin should not launch the proxy"
before

test_base "--inspector -S plugin/build -P plugin" > /dev/null

## Check if no proxy is running
is_proxy_running
test_result_empty "The proxy program should not be running!"

printf "${PASS}\n"

# @Test
echo "> Launching the inspector script with arguments should start the proxy"
before

test > /dev/null

## Check if no proxy is running
is_proxy_running
test_result_nonempty "The proxy program should be running!"

after

printf "${PASS}\n"

# Return to the catalog
cd test

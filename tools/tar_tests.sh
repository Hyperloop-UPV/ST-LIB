## shell script for CI automation and tests compressing

#only compress if using simulator
# otherwise it would create empty folder

#!/bin/sh
if [[ $# -eq 0 ]] ; then
    echo "ERROR, provide a build configuration"
    exit 1
fi
mode=$1


if [[ "$mode" == "simulator" ]]; then
    tar -cvf test_files.tar out/build/simulator/Tests/stlib-test
else
    exit 1
fi # mode=simulator
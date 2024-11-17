## shell script for CI automation
#!/bin/sh
if [[ $# -eq 0 ]] ; then
    echo "ERROR, provide a build configuration"
    exit 1
fi
mode=$1
echo $mode
out_dir="out/build/$mode"

if [[ "$mode"="simulator" ]]; then
    cmake -DCMAKE_BUILD_TYPE=Debug -S . -B $out_dir -G "Unix Makefiles"
    cd $out_dir 
    make -j 8
else
    echo "hardware build"
fi # mode=simulator
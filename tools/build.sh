## shell script for CI automation

mode=$1

out_dir="out/build/$mode"

if $mode=="simulator"
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B $out_dir -G "Unix Makefiles"
cd $out_dir 
(make -j 8)
else
    echo "hardware build"
fi # mode=simulator
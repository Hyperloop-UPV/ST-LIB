## shell script for CI automation

out_dir = "out/build/simulator"

cmake -DCMAKE_BUILD_TYPE=Debug -S . -B $out_dir -G "Unix Makefiles"
cd $out_dir 
status=$(make -j 8) 
if  $status=0
    $(cd ${out_dir}/Tests) 
    ./stlib_test
fi
#/bin/bash
clear 
export LD_LIBRARY_PATH=$PWD/lib64:$PWD/lib:$LD_LIBRARY_PATH
export DEBUG_MALLOC=

make clean
make intel-all

if [ $? -eq 0 ]; then
	cd Test
	make
	./tryme
	cd ..
fi

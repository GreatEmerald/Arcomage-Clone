#!/bin/sh
cd ../src
make -f Makefile-32
mv arcomage ../bin/linux-i686
make -f Makefile-32 clean
cd ../utils

#!/bin/sh
cd ../src
make -f Makefile.dev
mv arcomage ../bin/linux-x86_64
make -f Makefile.dev clean
cd ../utils

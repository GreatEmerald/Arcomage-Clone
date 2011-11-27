#!/bin/sh
cd ../src
make -f Makefile.dev
mv arcomage ../bin
make -f Makefile.dev clean
cd ../utils

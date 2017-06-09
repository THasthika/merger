#!/bin/bash

autoreconf -i

cd libal

./autogen.sh
./configure
make

cd ../

#!/bin/bash

memCheck()
{
    G_SLICE=always-malloc valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20  --log-file=valgrind.log  ./eggUtest "ItfTest"
}

docmd()
{
    echo "BEGIN--------------------- $*"
    $*
    echo "END----------------------- $*"
}

#docmd ./eggUtest fileTest
#docmd ./eggUtest ViewStreamTest
#docmd ./eggUtest btreeTest
#docmd ./eggUtest RecoveryLogTest
#docmd ./eggUtest listTest
#docmd ./eggUtest configTest
#docmd ./eggUtest IdTableTest
#docmd ./eggUtest eggtpTest
#docmd ./eggUtest eggPathTest
#docmd ./eggUtest eggWeightTest
#docmd ./eggUtest ItfTest

#setup enviroment
#pgrep eggChunk.fcgi &>/dev/null || { echo "please start fcgi"; exit 1; }
#pgrep eggServiceSe &>/dev/null || { echo "please start eggServiceServer"; exit 1; }

./test_eggFieldModify.sh
./test_eggIncrementModifyDocument.sh
./test_eggModifyDocument.sh
./test_eggModifyDocument2.sh

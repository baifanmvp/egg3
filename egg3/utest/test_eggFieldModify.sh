#!/bin/bash

prg=test_eggFieldModify
gcc -O0 -g3 -Wall ./${prg}.c -o $prg `pkg-config glib-2.0 --cflags --libs` -legg3 -ldl

[ $? != 0 ] && exit 1

expectfile=`pwd`/utest_data/$prg.0
resultfile=`pwd`/utest_data/$prg.1
outfile=`pwd`/utest_data/$prg.$$
workdir=`pwd`/utest_data/$prg
tmpfiles="$expectfile $resultfile $outfile"

echo BEGIN---------------------$prg
teststart()
{
    mkdir -p $workdir 2>/dev/null
    rm -rf $workdir/* 2>/dev/null

        cat >$expectfile <<EOF
=TEST=body: 456 OtherAnalyzed:ImC2LexAnalyzer String 
=TEST=body: 456 OtherAnalyzed:ImC2LexAnalyzer String 
=TEST=price: 712 NotAnalyzed Int32 
=TEST=: 456 OtherAnalyzed:ImC2LexAnalyzer String 
=TEST=price: 712 NotAnalyzed Int32 
=TEST=body2: 968 NotAnalyzed Int32 
=TEST=: 456 OtherAnalyzed:ImC2LexAnalyzer String 
=TEST=: 712 NotAnalyzed Int32 
=TEST=body2: 968 NotAnalyzed Int32 
EOF

}
testend()
{
    rm $tmpfiles
    rm -rf $workdir
    echo END-----------------------$prg
}

dotest()
{
    echo "$*"
    $* &>$outfile
}

# check
check()
{
    grep '=TEST=' $outfile > $resultfile
    if diff -u $expectfile $resultfile ; then
        echo Test OK
        testend
    else
        echo Test FAIL
        echo "Leaving file: " $tmpfiles
    fi
}

teststart; dotest ./$prg $workdir/ ;check
#teststart; dotest ./$prg tcp:127.0.0.1:3000$workdir/ ;check
#teststart; dotest ./$prg unixsock:/home/ape/Dev/egg2-trunk/bin/server/20120215/egg.sock$workdir/ ;check
#teststart; dotest ./$prg fcgi:127.0.0.1:80$workdir/ ;check

memcheck()
{
    G_SLICE=always-malloc valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20  --log-file=valgrind.log  $*
}
#teststart; memcheck ./$prg $workdir/

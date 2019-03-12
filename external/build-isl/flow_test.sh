#!/bin/sh

EXEEXT=
srcdir=/home/adilla/Documents/SchedTrees/islutils/external/isl

failed=0

for i in $srcdir/test_inputs/flow/*.ai; do
	echo $i;
	base=`basename $i .ai`
	test=test-$base.flow
	dir=`dirname $i`
	ref=$dir/$base.flow
	(./isl_flow$EXEEXT < $i > $test &&
	./isl_flow_cmp$EXEEXT $ref $test && rm $test) || failed=1
done

test $failed -eq 0 || exit

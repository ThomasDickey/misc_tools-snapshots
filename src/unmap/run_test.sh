#!/bin/sh
# $Id: run_test.sh,v 1.2 1997/06/10 00:11:34 tom Exp $
for O in *.o
do
	S=`basename $O .o`.s
	T=`basename $O .o`.t
	./unmap $O > $S
	./map $S | ./unmap >$T
	if (diff -s $S $T)
	then
		echo ok $O
	else
		echo fail $O
	fi
	rm -f $S $T
done

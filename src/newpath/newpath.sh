#!/bin/sh
# $Id: newpath.sh,v 1.2 1994/06/24 14:18:42 tom Exp $
# crude, slow, portable
Popt="no"
Bopt="no"
Dopt="no"
newpath=`echo $PATH | sed -e 's/:/ /g'`
while test $# != 0
do
	n=$1
	case $n in
	-b)
		Bopt=yes
		;;
	-d)
		Dopt=yes
		;;
	-p)	Popt=yes
		;;
	-n)	newpath=`echo $2 | sed -e 's/:/ /g'`
		shift
		;;
	-*)
		a=`echo $n | sed -e 's/-\(.\).*/-\1/'`
		b=`echo $n | sed -e 's/-.\(.*\)/-\1/'`
		set -- dummy $a $b $2 $3 $4 $5 $6 $7 $8
		;;
	*)
		if test $Bopt = yes
		then
			newpath="$n $newpath"
		else
			newpath="$newpath $n"
		fi
		;;
	esac
	shift
done
if test $Dopt = yes
then
	TMP=/tmp/newpath$$
	rm -f $TMP
	trap "rm -f $TMP" 0 1 2 5 15
	for n in $newpath
	do
		if test -d $n
		then
			if test ! -f $TMP
			then
				echo $n >>$TMP
			elif ( grep '^'$n'$' $TMP >/dev/null )
			then
				echo >/dev/null
			else
				echo $n >>$TMP
			fi
		fi
	done
	newpath=`cat $TMP`
fi
if test $Popt = no
then
	newpath=`echo $newpath | sed -e 's/ /:/g'`
fi
echo $newpath

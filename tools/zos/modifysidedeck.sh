#!/usr/bin/env bash

if [ "$#" -ne 3 ] || ! [ -f "$1" ]; then
  echo ===========================
  echo "Script to modify sidedeck references to a new DLL name"
  echo ===========================
  echo "Usage: $0 originalsidedeck modifiedsidedeck newdllreference" >&2
  exit 1
fi

originalsidedeck=$1
outputsidedeck=$2
newdllname=$3

ID=`date +%C%y%m%d_%H%M%S`
TMP="/tmp/sidedeck-$(basename $0).$ID.tmp"
TMP2="/tmp/sidedeck-$(basename $0).$ID.tmp.2"

# Remove on exit/interrupt
trap "/bin/rm -rf $TMP $TMP2 && exit" EXIT INT TERM QUIT HUP

set -x
dd conv=unblock cbs=80 if=$originalsidedeck of=$TMP
chtag -tc 1047 $TMP
cat $TMP | sdwrap -u | sed -e "s/\(^ IMPORT \(DATA\|CODE\)64,\)'[^']*'/\1'$newdllname'/g"  | sdwrap -w > $TMP2

# Reformat sidedeck to be USS compatible
iconv -f ISO8859-1 -t IBM-1047 $TMP2 > $TMP
mv $TMP $TMP2
dd conv=block cbs=80 if=${TMP2} of=$outputsidedeck

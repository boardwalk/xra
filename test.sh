#!/bin/bash
set -e
export PATH="`pwd`/src:$PATH"
for exe_file in test/*.xra; do
  echo "Testing $(basename $exe_file)"
  ref_file=${exe_file%.xra}.ref
  out_file=${exe_file%.xra}.out
  $exe_file > $out_file
  diff -u $ref_file $out_file
done

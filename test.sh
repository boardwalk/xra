#!/bin/bash
set -e
for exe_file in test/*.xra; do
  echo "Testing $(basename $exe_file)"
  ref_file=${exe_file%.xra}.ref
  out_file=${exe_file%.xra}.out
  ./test-one.pl $exe_file > $out_file
  diff -u $ref_file $out_file
done

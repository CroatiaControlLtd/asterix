#!/bin/bash

failedtests=0
exec="../../obj/main/release/asterix"
execd="../../obj/main/debug/asterix"
config="../config/asterix.ini"

test_compare () {
	result=$($2)
	expected=$(cat $3)

	if [ "$result" == "$expected" ]; then
	  echo "OK      " $1
	else
	  echo "Failed  " $1
	  failedtests=$[$failedtests +1]
	fi
}

test_output () {
  $2 > valgrind.result 2>&1
	result=$?
	expected=$3

	if [ "$result" == "$expected" ]; then
	  echo "OK      " $1
	else
	  echo "Failed  " $1
	  failedtests=$[$failedtests +1]
	fi
}


test_compare "Test filter" "$exec -d $config -L" "../sample_output/filter.txt"
test_compare "Test json CAT_062_065" "$exec -d $config -P -j -f ../sample_data/cat_062_065.pcap" "../sample_output/cat_062_065_json.txt"
test_compare "Test json CAT_001_002" "$exec -d $config -R -j -f ../sample_data/cat_001_002.pcap -LF ../sample_output/filter.txt" "../sample_output/cat_001_002_json.txt"
test_compare "Test jsonh CAT_001_002" "$exec -d $config -R -jh -f ../sample_data/cat_001_002.pcap -LF ../sample_output/filter.txt" "../sample_output/cat_001_002_jsonh.txt"
test_compare "Test xml CAT_001_002" "$exec -d $config -R -x -f ../sample_data/cat_001_002.pcap -LF ../sample_output/filter.txt" "../sample_output/cat_001_002.xml"
test_compare "Test txt CAT_001_002" "$exec -d $config -R -f ../sample_data/cat_001_002.pcap -LF ../sample_output/filter.txt" "../sample_output/cat_001_002.txt"
test_compare "Test line CAT_001_002" "$exec -d $config -R -l -f ../sample_data/cat_001_002.pcap -LF ../sample_output/filter.txt" "../sample_output/cat_001_002_line.txt"
test_compare "Test filtered txt CAT_034_048" "$exec -d $config -P -f ../sample_data/cat_034_048.pcap -LF ../sample_output/filter.txt" "../sample_output/cat_034_048_filtered.txt"
test_compare "Test unfiltered txt CAT_034_048" "$exec -d $config -P -f ../sample_data/cat_034_048.pcap" "../sample_output/cat_034_048.txt"
test_compare "Test GPS parsing" "$exec -d $config -G -f ../sample_data/parsegps.gps" "../sample_output/parsegps.txt"


#test_output  "Test Memory leak with valgrind" "valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $execd -P -d $config -f ../sample_data/cat_062_065.pcap -jh" "0"
test_output  "Test Memory leak with valgrind (1)" "valgrind --leak-check=full --error-exitcode=1 $execd -P -d $config -f ../sample_data/cat_062_065.pcap -jh" "0"
test_output  "Test Memory leak with valgrind (2)" "valgrind --leak-check=full --error-exitcode=1 $execd -G -d $config -f ../sample_data/parsegps.gps -jh" "0"


if [ "$failedtests" == "0" ]; then
  echo "All tests OK"
  exit 0
else
  echo $failedtests " tests failed."
  exit 1
fi

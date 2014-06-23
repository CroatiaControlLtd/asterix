failedtests=0

test () {
#	$2 > $3
	result=$($2)
	expected=$(cat $3)

	if [ "$result" == "$expected" ]; then
	  echo "OK      " $1
	else
	  echo "Failed  " $1
	  failedtests=$[$failedtests +1]
	fi
}

test "Test filter" "../obj/main/release/asterix.exe -d config/asterix.ini -L sample_data/filter.txt" "sample_data/filter.txt"
test "Test json CAT_062_065" "../obj/main/release/asterix.exe -d config/asterix.ini -P -j -f sample_data/cat_062_065.pcap" "sample_data/cat_062_065_json.txt"
test "Test json CAT_001_002" "../obj/main/release/asterix.exe -d config/asterix.ini -R -j -f sample_data/cat_001_002.pcap -LF sample_data/filter.txt" "sample_data/cat_001_002_json.txt"
test "Test jsonh CAT_001_002" "../obj/main/release/asterix.exe -d config/asterix.ini -R -jh -f sample_data/cat_001_002.pcap -LF sample_data/filter.txt" "sample_data/cat_001_002_jsonh.txt"
test "Test xml CAT_001_002" "../obj/main/release/asterix.exe -d config/asterix.ini -R -x -f sample_data/cat_001_002.pcap -LF sample_data/filter.txt" "sample_data/cat_001_002.xml"
test "Test txt CAT_001_002" "../obj/main/release/asterix.exe -d config/asterix.ini -R -f sample_data/cat_001_002.pcap -LF sample_data/filter.txt" "sample_data/cat_001_002.txt"
test "Test line CAT_001_002" "../obj/main/release/asterix.exe -d config/asterix.ini -R -l -f sample_data/cat_001_002.pcap -LF sample_data/filter.txt" "sample_data/cat_001_002_line.txt"

if [ "$failedtests" == "0" ]; then
  echo "All tests OK"
else
  echo $failedtests " tests failed."
fi
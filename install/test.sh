echo "Test CAT_062_065"

result1=$(../obj/main/release/asterix.exe -d config/asterix.ini -P -j -f sample_data/cat_062_065.pcap)
expected1=$(cat sample_data/cat_062_065.txt)

if [ "$result1" == "$expected1" ]; then
  echo "OK"
else
  echo "Failed"
fi


echo "Test CAT_001_002"

result2=$(../obj/main/release/asterix.exe -d config/asterix.ini -R -j -f sample_data/cat_001_002.pcap)
expected2=$(cat sample_data/cat_001_002.txt)

if [ "$result2" == "$expected2" ]; then
  echo "OK"
else
  echo "Failed"
fi

echo "Test filter"

result3=$(../obj/main/release/asterix.exe -d config/asterix.ini -R -j -f sample_data/cat_001_002.pcap -FL sample_data/filter.txt)
expected3=$(cat sample_data/cat_001_002.txt)

if [ "$result3" == "$expected3" ]; then
  echo "OK"
else
  echo "Failed"
fi
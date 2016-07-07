#!/bin/bash

exec="../../obj/main/release/asterix"
config="../config/asterix.ini"

echo Updating tests

$exec -d $config -L > "../sample_output/filter.txt"
$exec -d $config -P -j -f ../sample_data/cat_062_065.pcap > "../sample_output/cat_062_065_json.txt"
$exec -d $config -R -j -f ../sample_data/cat_001_002.pcap -FL ../sample_output/filter.txt > "../sample_output/cat_001_002_json.txt"
$exec -d $config -R -jh -f ../sample_data/cat_001_002.pcap -FL ../sample_output/filter.txt > "../sample_output/cat_001_002_jsonh.txt"
$exec -d $config -R -x -f ../sample_data/cat_001_002.pcap -FL ../sample_output/filter.txt > "../sample_output/cat_001_002.xml"
$exec -d $config -R -f ../sample_data/cat_001_002.pcap -FL ../sample_output/filter.txt > "../sample_output/cat_001_002.txt"
$exec -d $config -R -l -f ../sample_data/cat_001_002.pcap -FL ../sample_output/filter.txt > "../sample_output/cat_001_002_line.txt"

echo Done

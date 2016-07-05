valgrind -v --leak-check=full --show-leak-kinds=all --error-exitcode=1 ../../obj/main/debug/asterix -P -d ../config/asterix.ini -f ../sample_data/cat_062_065.pcap -jh

if [ $? = 1 ]; then
        echo "Memory leak test failed"
else
        echo "Memory leak test OK"
fi
#!/bin/bash
###############################################################
## ASTERIX WIRESHARK PLUGIN PACKAGE
## Pack and compress asterix sources, executable and sample data
## Copyright (C) 2012 Croatia Control Ltd
## revision history:
##              20121202 dsalantic v1.0 created, initial version
###############################################################
x=`date +%Y_%m_%d`

rm -rf wireshark
mkdir wireshark
cp -p CMakeLists.txt wireshark
cp -p configure.ac wireshark
cp -p HowToBuild.txt wireshark
cp -p Makefile.am wireshark
cp -rp epan wireshark
cp -rp packaging wireshark
cp -rp plugins wireshark
cp -p plugins/asterix/*.* wireshark/plugins/asterix
cp -p ../../* wireshark/plugins/asterix/
mkdir -p wireshark/usr/local/lib/wireshark/plugins/asterix
cp -p ../../../../install/config/asterix.ini wireshark/usr/local/lib/wireshark/plugins/asterix
cp -p ../../../../install/config/*.xml wireshark/usr/local/lib/wireshark/plugins/asterix

cd wireshark
tar cvfz ../asterix_wireshark_src_$x.tar.gz *
cd ..
rm -rf wireshark


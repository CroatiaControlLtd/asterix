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
cp CMakeLists.txt wireshark
cp configure.in wireshark
cp HowToBuild.txt wireshark
cp Makefile.am wireshark
cp -rp epan wireshark
cp -rp packaging wireshark
cp -rp plugins wireshark
cp plugins/asterix/* wireshark/plugins/asterix
cp ../*.* wireshark/plugins/asterix
#cp ../expat/*.* wireshark/plugins/asterix
mkdir wireshark/usr
mkdir wireshark/usr/local
mkdir wireshark/usr/local/lib
mkdir wireshark/usr/local/lib/wireshark
mkdir wireshark/usr/local/lib/wireshark/plugins
mkdir wireshark/usr/local/lib/wireshark/plugins/asterix
cp ../../../install/config/asterix.ini wireshark/usr/local/lib/wireshark/plugins/asterix
cp ../../../install/config/*.xml wireshark/usr/local/lib/wireshark/plugins/asterix

cd wireshark
tar cvfz ../asterix_wireshark_src_$x.tar.gz *
cd ..
rm -rf wireshark


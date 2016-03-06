#!/bin/bash
###############################################################
## ASTERIX ETHEREAL PLUGIN PACKAGE
## Pack and compress asterix sources, executable and sample data
## Copyright (C) 2013 Croatia Control Ltd / Thales Air System
## revision history:
##              20121202 dsalantic v1.0 created, initial version
##              20130718 nbendafi  v1.1 wireshark -> ethereal 0.99.0
###############################################################
x=`date +%Y_%m_%d`

rm -rf ethereal
mkdir ethereal
cp configure.in ethereal
cp HowToBuild.txt ethereal
cp Makefile.am ethereal
cp -rp epan ethereal
cp -rp packaging ethereal
cp -rp plugins ethereal
cp ../*.* ethereal/plugins/asterix
mkdir -p ethereal/asterix
cp ../../../install/config/asterix.ini ethereal/asterix
cp ../../../install/config/*.xml ethereal/asterix

cd ethereal
tar cvfz ../asterix_ethereal_src_$x.tgz *
cd ..
rm -rf ethereal


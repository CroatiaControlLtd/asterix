#!/bin/bash
###############################################################
## ASTERIX WIRESHARK PLUGIN CONFIGURATION PACKAGE
## Pack config files for asterix plugin
## Copyright (C) 2012 Croatia Control Ltd
## revision history:
##              20121202 dsalantic v1.0 created, initial version
###############################################################
x=`date +%Y_%m_%d`

rm -rf usr
mkdir usr
mkdir usr/local
mkdir usr/local/lib
mkdir usr/local/lib/wireshark
mkdir usr/local/lib/wireshark/plugins
mkdir usr/local/lib/wireshark/plugins/asterix
cp ../../../install/config/asterix.ini usr/local/lib/wireshark/plugins/asterix
cp ../../../install/config/*.xml usr/local/lib/wireshark/plugins/asterix

tar cvfz asterix_wireshark_config_$x.tar.gz usr

rm -rf usr


#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Created by xiaojian on 2020/6/23
# Read xml file and parser for create table sql

__author__ = 'xiaojian'

import logging
import json
import xml.dom.minidom
from xml.dom import Node
from xml.dom.minidom import Text
from xml.dom.minidom import Element
from xml.dom.minidom import parseString

logging.basicConfig(level=logging.INFO)

xml_file = "asterix/config/asterix_cat062_1_18.xml"

dom = xml.dom.minidom.parse(xml_file)
root = dom.documentElement

VariableId = 1
resultJS = {}

def printJS(line):
    print(line)
    row = line.split("\t", -1)
    item_id = row[0]
    presence = row[1]
    presence_info = row[3]
    field = row[2]
    field_info = row[4]
    if len(presence) > 0:
        if len(field) > 0:
            resultJS[str.lower("_".join([item_id, presence, field]))] = field_info
        else:
            resultJS[str.lower("_".join([item_id, presence]))] = presence_info

    elif len(field) > 0:
        resultJS[str.lower("_".join([item_id, field]))] = field_info


def getBit(Bit, **kwargs):
    for text in Bit.childNodes:
        return text.nodeValue


def printBits(Bits, **kwargs):
    BitsShortName = ""
    BitsName = ""
    valIndex = 0
    for Bit in Bits.childNodes:
        if type(Bit) is not Element:
            continue
        if Bit.nodeName == "BitsShortName":
            BitsShortName = getBit(Bit, **kwargs)
        if Bit.nodeName == "BitsName":
            BitsName = getBit(Bit, **kwargs)
            if BitsName is None:
                BitsName = ""
        if Bit.nodeName == "BitsValue":
            BitsName += "{}:{};".format(valIndex,getBit(Bit, **kwargs))
            valIndex += 1
    if "Variable" in kwargs:
        global VariableId
        printJS("{}\t{}\t{}\t{}\t{}".format(kwargs["DataItemId"], kwargs["Variable"][str(VariableId)][0], BitsShortName,
                                            kwargs["Variable"][str(VariableId)][1], BitsName))
    else:
        printJS("{}\t{}\t{}\t{}\t{}".format(kwargs["DataItemId"], "", BitsShortName, "", BitsName))


def getBits(Bits, **kwargs):
    BitsPresence = ""
    BitsShortName = ""
    BitsName = ""
    for Bit in Bits.childNodes:
        if type(Bit) is not Element:
            continue
        if Bit.nodeName == "BitsPresence":
            BitsPresence = getBit(Bit, **kwargs)
        if Bit.nodeName == "BitsShortName":
            BitsShortName = getBit(Bit, **kwargs)
        if Bit.nodeName == "BitsName":
            BitsName = getBit(Bit, **kwargs)
    if BitsShortName == "spare":
        BitsPresence = "-1"
    if BitsShortName == "FX":
        BitsPresence = "0"
    return [BitsPresence, [BitsShortName, BitsName]]


def printFixed(Fixed, **kwargs):
    global VariableId
    for Bits in Fixed.childNodes:
        if type(Bits) is not Element:
            continue
        printBits(Bits, **kwargs)
    VariableId += 1


def gitFixed(Fixed, **kwargs):
    Variable = {}
    for Bits in Fixed.childNodes:
        if type(Bits) is not Element:
            continue
        [BitsPresence, [BitsShortName, BitsName]] = getBits(Bits, **kwargs)
        if BitsShortName is None:
            BitsShortName = ""
        if BitsName is None:
            BitsName = ""
        Variable[BitsPresence] = [BitsShortName, BitsName]
    return Variable


def getDataItemId(DataItem):
    if type(DataItem._attrs) == dict:
        return "I" + DataItem._attrs["id"]._value
    else:
        return -1


def printVariable(Variable, **kwargs):
    for Fixed in Variable.childNodes:
        if type(Fixed) is not Element:
            continue
        if Fixed.nodeName == "Fixed":
            printFixed(Fixed, DataItemId=DataItemId)


def getVariable(Variable, **kwargs):
    Variables = {}
    for Fixed in Variable.childNodes:
        if type(Fixed) is not Element:
            continue
        if Fixed.nodeName == "Fixed":
            Variable = gitFixed(Fixed, DataItemId=DataItemId)
            for key in Variable.keys():
                Variables[key] = Variable[key]
    return Variables


def printRepetitive(Repetitive, **kwargs):
    for DataItemFormat in Repetitive.childNodes:
        if type(DataItemFormat) is not Element:
            continue
        if DataItemFormat.nodeName == "Fixed":
            printFixed(DataItemFormat, **kwargs)


def printCompound(Compound, **kwargs):
    for DataItemFormat in Compound.childNodes:
        if type(DataItemFormat) is not Element:
            continue
        if DataItemFormat.nodeName == "Variable":
            kwargs["Variable"] = getVariable(DataItemFormat)
        else:
            if DataItemFormat.nodeName == "Fixed":
                printFixed(DataItemFormat, **kwargs)
            elif DataItemFormat.nodeName == "Repetitive":
                printRepetitive(DataItemFormat, **kwargs)
    for BitsPresence in ["-1", "0"]:
        if BitsPresence in kwargs["Variable"]:
            printJS("{}\t{}\t{}\t{}\t{}".format(kwargs["DataItemId"], kwargs["Variable"][BitsPresence][0], "",
                                                kwargs["Variable"][BitsPresence][1], ""))


def printExplicit(Explicit, **kwargs):
    for DataItemFormat in Explicit.childNodes:
        if type(DataItemFormat) is not Element:
            continue
        if DataItemFormat.nodeName == "Compound":
            printCompound(DataItemFormat, DataItemId=DataItemId)


def map2sql(map, key, depth):
    rows = []
    for k in map.keys():
        if type(map[k]) == str:
            rows.append(struct_str2sql(map, k, depth + 1))
        elif type(map[k]) == dict:
            rows.append(map2sql(map[k], k, depth + 1))
    if depth == 1:
        return """{}`_{}` struct \n{}<\n{}\n{}>""".format("\t\t" * depth, key, "\t\t" * depth, ",\n".join(rows),
                                                          "\t\t" * depth)
    else:
        return """{}`_{}`:struct \n{}<\n{}\n{}>""".format("\t\t" * depth, key, "\t\t" * depth, ",\n".join(rows),
                                                          "\t\t" * depth)


def str2sql(map, key, depth):
    return """ {}`{}` STRING comment "{}" """.format("\t\t" * depth, key, map[key])


def struct_str2sql(map, key, depth):
    return """ {}`{}`:STRING""".format("\t\t" * depth, key)


def json2sql(map):
    rows = []
    for key in map.keys():
        # if key != "I110":
        #     continue
        if type(map[key]) == str:
            rows.append(str2sql(map, key, 1))
        elif type(map[key]) == dict:
            rows.append(map2sql(map[key], key, 1))
    return rows


if __name__ == '__main__':

    for DataItem in root.childNodes:
        if type(DataItem) is not Element:
            continue

        DataItemId = getDataItemId(DataItem)
        for DataItemFormats in DataItem.childNodes:
            if type(DataItemFormats) is not Element:
                continue
            if DataItemFormats.nodeName == "DataItemFormat":
                for DataItemFormat in DataItemFormats.childNodes:
                    VariableId = 1
                    if type(DataItemFormat) is not Element:
                        continue
                    if DataItemFormat.nodeName == "Fixed":
                        printFixed(DataItemFormat, DataItemId=DataItemId)
                    if DataItemFormat.nodeName == "Variable":
                        printVariable(DataItemFormat, DataItemId=DataItemId)
                    if DataItemFormat.nodeName == "Compound":
                        printCompound(DataItemFormat, DataItemId=DataItemId)
                    if DataItemFormat.nodeName == "Explicit":
                        printExplicit(DataItemFormat, DataItemId=DataItemId)


    print("\n")
    print("======================================================================================")
    print("======                         json                                            =======")
    print("======================================================================================")

    print(json.dumps(resultJS,sort_keys=True, indent=4))
    print("\n")
    print("======================================================================================")
    print("======                    create table sql                                     =======")
    print("======================================================================================")
    sql = "CREATE TABLE IF NOT EXISTS TABLE_NAME (\n{}\n);".format(",\n".join(json2sql(resultJS)))
    print(sql)

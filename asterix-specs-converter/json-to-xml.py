#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import argparse
import json
import hashlib

def getNumber(value):
    """Get Natural/Real/Rational number as an object."""
    class Integer(object):
        def __init__(self, val):
            self.val = val
        def __str__(self):
            return '{}'.format(self.val)
        def __float__(self):
            return float(self.val)

    class Ratio(object):
        def __init__(self, a, b):
            self.a = a
            self.b = b
        def __str__(self):
            return '{}/{}'.format(self.a, self.b)
        def __float__(self):
            return float(self.a) / float(self.b)

    class Real(object):
        def __init__(self, val):
            self.val = val
        def __str__(self):
            return '{0:f}'.format(self.val).rstrip('0')
        def __float__(self):
            return float(self.val)

    t = value['type']
    val = value['value']

    if t == 'Integer':
        return Integer(int(val))
    if t == 'Ratio':
        x, y = val['numerator'], val['denominator']
        return Ratio(x, y)
    if t == 'Real':
        return Real(float(val))
    raise Exception('unexpected value type {}'.format(t))

def renderRule(rule, subitem, caseUnspecified, caseContextFree, caseDependent):
    rule_type = rule['type']
    if rule_type == 'Unspecified':
        return caseUnspecified()
    elif rule_type == 'ContextFree':
        return caseContextFree(rule, subitem)
    elif rule_type == 'Dependent':
        return caseDependent(rule)
    else:
        raise Exception('unexpected rule type {}'.format(rule_type))

# 'case' statement as a function
def case(msg, val, *cases):
    for (a,b) in cases:
        if val == a: return b
    raise Exception('unexpected {}: {}'.format(msg, val))

accumulator = []
indentLevel = 0

class Indent(object):
    """Simple indent context manager."""
    def __enter__(self):
        global indentLevel
        indentLevel += 1
    def __exit__(self, exec_type, exec_val, exec_tb):
        global indentLevel
        indentLevel -= 1

indent = Indent()

def replaceString(s, mapping):
    for (key,val) in mapping.items():
        s = s.replace(key, val)
    return s

def replaceOutput(s):
    return replaceString(s, {
        u'–': '-',
        u'“': '',
        u'”': '',
        u'°': ' deg',
    })

def tell(s):
    s = replaceOutput(s) # If unicode is not supported on the target.
    s = ' '*indentLevel*4 + s
    accumulator.append(s.rstrip())

def xmlquote(s):
    return replaceString(s, {
        '"': "&quot;",
        "&": "&amp;",
        "'": "&apos;",
        "<":  "&lt;",
        ">": "&gt;",
    })


def itemLine(item):
    itemType = item['element']['type']
    if itemType == 'Fixed':
        length = int(item['element']['size'] / 8)
        return '<{} length="{:d}">'.format(itemType, length)
    elif itemType != 'Group':
        return '<DataItem id="{}" type="{}">'.format(item['name'], itemType)
    else:
        return '<DataItem id="{}">'.format(item['name'])


def render(s):
    root = json.loads(s)
    category = root['number']
    edition = root['edition']
    title = root['title']
    tell('<?xml version="1.0" encoding="UTF-8" ?>')
    tell('')
    tell('<!--')
    with indent:
        tell('Asterix Category {:03d} v{}.{} definition'.format(category, edition['major'], edition['minor']))

        tell('Do not edit directly!')
        tell('This file is auto-generated from the original json specs file.')
        tell('sha1sum of the json input: {}'.format(hashlib.sha1(s).hexdigest()))
    tell('-->')
    tell('')
    tell('<Category id="{:d}" name="{}" ver="{}.{}">'.format(category, title, edition['major'], edition['minor']))
    with indent:
        [renderItem(item) for item in root['catalogue']]
    with indent:
        renderUap(root['uap'])
    tell('</category>')
    return ''.join([line+'\n' for line in accumulator])

def renderItem(item):
    subitem = item['subitem']
    rule = None
    if 'encoding' in item and 'rule' in item['encoding']:
        rule = item['encoding']['rule']
        tell('<DataItem id="{}" rule="{}">'.format(subitem['name'], rule))
    else:
        tell('<DataItem id="{}">'.format(item['name']))

    with indent:
        title = subitem['title']
        if title:
            tell('<DataItemName>{}</DataItemName>'.format(xmlquote(title)))
        if 'definition' in item:
            tell('<DataItemDefinition>{}</DataItemDefinition>'.format(xmlquote(item['definition'])))
        tell('<DataItemFormat desc="TODO"')
        renderSubitem(subitem)
        tell('</DataItemFormat>')
    tell('</DataItem>')
    tell('')

def renderSubitem(subitem):

    def renderInteger(value):
        tell('<convert>')
        with indent:
            sig = '' if value['signed'] else 'unsigned '
            tell('<type>{}</type>'.format(sig + 'integer'))
            for i in value['constraints']:
                if i['type'] in ['>', '>=']:
                    tell('<min>{}</min>'.format(getNumber(i['value'])))
                if i['type'] in ['<', '<=']:
                    tell('<max>{}</max>'.format(getNumber(i['value'])))
        tell('</convert>')

    def renderQuantity(value):
        tell('<convert>')
        with indent:
            sig = '' if value['signed'] else 'unsigned '
            k = getNumber(value['scaling'])
            tell('<type>{}</type>'.format(sig + 'decimal'))

            fract = value['fractionalBits']
            if fract == 0:
                tell('<lsb>{}</lsb>'.format(k))
            else:
                tell('<lsb>{}/{}</lsb>'.format(k, pow(2,fract)))

            unit = value['unit']
            if unit is not None:
                tell('<unit>{}</unit>'.format(unit))

            for i in value['constraints']:
                if i['type'] in ['>', '>=']:
                    tell('<min>{}</min>'.format(getNumber(i['value'])))
                if i['type'] in ['<', '<=']:
                    tell('<max>{}</max>'.format(getNumber(i['value'])))
        tell('</convert>')

    def renderFixed():
        element = subitem['element']
        def case0():
            pass
        def case1(val, subitem):
            rule = val['rule']
            t = rule['type']
            if t == 'Table':
                from_bit = 8
                to_bit = 1
                with indent:
                    tell('<Bits from="{}" to="{}">'.format(from_bit, to_bit))
                    with indent:
                        if 'name' in subitem:
                            tell('<BitsShortName>{}</BitsShortName>'.format(subitem['name']))
                        if 'title' in subitem:
                            tell('<BitsName>{}</BitsName>'.format(subitem['title']))
                        for key,value in rule['values']:
                            tell('<BitsValue val="{}">{}</BitsValue>'.format(key, xmlquote(value)))
                    tell('</Bits>')
            elif t == 'String':
                f = case('string variation', rule['variation'],
                    ('StringAscii', lambda: tell('<convert><type>string</type></convert>')),
                    ('StringICAO', lambda: None),
                    )
                f ()
            elif t == 'Integer':
                renderInteger(rule)
            elif t == 'Quantity':
                renderQuantity(rule)
            else:
                raise Exception('unexpected value type {}'.format(t))
        def case2(val):
            pass
        return renderRule(element['content'], subitem, case0, case1, case2)

    def renderMaybeSubitem(n, subitem):
        if subitem['spare']:
            tell('<item name="spare{}" type="Spare"><len>{}</len></item>'.format(n, subitem['length']))
            return True
        else:
            #tell(itemLine(subitem))
            #with indent:
                #title = subitem['title']
                #if title:
                #    tell('<dsc>{}</dsc>'.format(xmlquote(title)))
            renderSubitem(subitem)
            return False

    def renderGroup():
        with indent:
            tot_len = 0
            for item in element['subitems']:
                if item['spare']:
                    tot_len = item['length']
                else:
                    tot_len += item['element']['size']

            length = int(tot_len / 8)
            tell('<Fixed length="{:d}">'.format(length))

            spareIndex = 1
            for item in element['subitems']:
                isSpare = renderMaybeSubitem(spareIndex, item)
                if isSpare:
                    spareIndex += 1

            tell('</Fixed>')


    def renderExtended():
        n1 = element['first']
        n2 = element['extents']
        tell('<len>({},{})</len>'.format(n1, n2))
        renderGroup()

    def renderRepetitive():
        renderSubitem(element)

    def renderExplicit():
        pass

    def renderCompound():
        tell('<items>')
        with indent:
            spareIndex = 1
            for item in element['subitems']:
                if item is None:
                    tell('<item></item>')
                else:
                    isSpare = renderMaybeSubitem(spareIndex, item)
                    if isSpare:
                        spareIndex += 1
        tell('</items>')

    element = subitem['element']
    return locals()['render'+element['type']]()

def renderUap(uap):
    ut = uap['type']
    if ut == 'uap':
        variations = [{'name': 'uap', 'items': uap['items']}]
    elif ut == 'uaps':
        variations = uap['variations']
    else:
        raise Exception('unexpected uap type {}'.format(ut))
    tell('<uaps>')
    with indent:
        for var in variations:
            name = var['name']
            tell('<{}>'.format(name))
            with indent:
                for i in var['items']:
                    tell('<item>{}</item>'.format(i or ''))
            tell('</{}>'.format(name))
    tell('</uaps>')

# main
parser = argparse.ArgumentParser(description='Render asterix specs from json to custom xml.')
parser.add_argument('infile', nargs='?', type=argparse.FileType('rb'), default=sys.stdin.buffer)
parser.add_argument('outfile', nargs='?', type=argparse.FileType('wt'), default=sys.stdout)

args = parser.parse_args()

s = args.infile.read()
result = render(s)
args.outfile.write(result)


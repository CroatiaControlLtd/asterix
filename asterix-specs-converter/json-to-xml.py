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

def renderRule(rule, caseUnspecified, caseContextFree, caseDependent):
    rule_type = rule['type']
    if rule_type == 'Unspecified':
        return caseUnspecified()
    elif rule_type == 'ContextFree':
        return caseContextFree(rule)
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
    if itemType != 'Group':
        return '<item name="{}" type="{}">'.format(item['name'], itemType)
    else:
        return '<item name="{}">'.format(item['name'])

def render(s):
    root = json.loads(s)
    category = root['number']
    edition = root['edition']
    tell('<?xml version="1.0" encoding="UTF-8" ?>')
    tell('')
    tell('<!--')
    with indent:
        tell('Do not edit directly!')
        tell('This file is auto-generated from the original json specs file.')
        tell('sha1sum of the json input: {}'.format(hashlib.sha1(s).hexdigest()))
    tell('-->')
    tell('')
    tell('<category cat="{:03d}" edition="{}.{}">'.format(category, edition['major'], edition['minor']))
    with indent:
        tell('<dsc>Asterix Category {:03d}, edition {}.{}</dsc>'.format(category, edition['major'], edition['minor']))
        tell('<items>')
        with indent:
            [renderItem(item) for item in root['catalogue']]
        tell('</items>')
    with indent:
        renderUap(root['uap'])
    tell('</category>')
    return ''.join([line+'\n' for line in accumulator])

def renderItem(item):
    subitem = item['subitem']
    tell(itemLine(subitem))
    with indent:
        title = subitem['title']
        if title:
            tell('<dsc>{}</dsc>'.format(xmlquote(title)))
        renderSubitem(subitem['element'])
    tell('</item>')

def renderSubitem(element):

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
        tell('<len>{}</len>'.format(element['size']))
        def case0():
            pass
        def case1(val):
            rule = val['rule']
            t = rule['type']
            if t == 'Table':
                tell('<values>')
                with indent:
                    for key,value in rule['values']:
                        tell('<value val="{}" dsc="{}"/>'.format(key, xmlquote(value)))
                tell('</values>')
            elif t == 'String':
                f = case('string variatioin', rule['variation'],
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
        return renderRule(element['content'], case0, case1, case2)

    def renderMaybeSubitem(n, subitem):
        if subitem['spare']:
            tell('<item name="spare{}" type="Spare"><len>{}</len></item>'.format(n, subitem['length']))
            return True
        else:
            tell(itemLine(subitem))
            with indent:
                title = subitem['title']
                if title:
                    tell('<dsc>{}</dsc>'.format(xmlquote(title)))
                renderSubitem(subitem['element'])
            tell('</item>')
            return False

    def renderGroup():
        tell('<items>')
        with indent:
            spareIndex = 1
            for item in element['subitems']:
                isSpare = renderMaybeSubitem(spareIndex, item)
                if isSpare:
                    spareIndex += 1
        tell('</items>')

    def renderExtended():
        n1 = element['first']
        n2 = element['extents']
        tell('<len>({},{})</len>'.format(n1, n2))
        renderGroup()

    def renderRepetitive():
        renderSubitem(element['element'])

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
parser.add_argument('outfile', nargs='?', type=argparse.FileType('w'), default=sys.stdout)

args = parser.parse_args()

s = args.infile.read()
result = render(s)
args.outfile.write(result)


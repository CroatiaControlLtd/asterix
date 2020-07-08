#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import argparse
import json
import hashlib
from itertools import chain, repeat

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

def getVariationSize(variation):
    t = variation['type']
    if t == 'Element':
        return variation['size']
    elif t == 'Group':
        return sum([getItemSize(i) for i in variation['items']])
    elif t == 'Repetitive':
        return getVariationSize(variation['variation'])
    else:
        raise Exception('can not determine item size for type {}'.format(t))

def getItemSize(item):
    """Determine the size of an item with apriory known size."""
    if item['spare']:
        return item['length']
    return getVariationSize(item['variation'])

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

def makeVariation(item):
    variation = item['variation']
    vt = variation['type']

    # make a single item Fixed variation
    if vt == 'Element':
        size = getItemSize(item)
        return Fixed(size, [item])

    if vt == 'Group':
        size = getItemSize(item)
        items = variation['items']
        return Fixed(size, items)

    if vt == 'Extended':
        n1 = variation['first']
        n2 = variation['extents']
        items = variation['items']
        return Variable(n1, n2, items)

    if vt == 'Repetitive':
        size = getVariationSize(variation)
        variation2 = variation['variation']
        items = variation2['items']
        return Repetitive(size, items)

    if vt == 'Explicit':
        return Explicit()

    if vt == 'Compound':
        items = variation['items']
        return Compound(items)

    raise Exception('unexpected variation type {}'.format(vt))

class Fx(object):
    def render(self):
        tell('<Bits bit="1" fx="1">')
        with indent:
            tell('<BitsShortName>FX</BitsShortName>')
            tell('<BitsName>Extension Indicator</BitsName>')
            tell('<BitsValue val="0">End of Data Item</BitsValue>')
            tell('<BitsValue val="1">Extension</BitsValue>')
        tell('</Bits>')

class Bits(object):

    def __init__(self, *args):
        self.args = args

    def render(self):
        item, bitsFrom, bitsTo = self.args
        if item['spare']:
            if bitsFrom == bitsTo:
                tell('<Bits bit="{}">'.format(bitsFrom))
            else:
                tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
            with indent:
                tell('<BitsShortName>spare</BitsShortName>')
                tell('<BitsName>Spare bit(s) set to 0</BitsName>')
                tell('<BitsConst>0</BitsConst>')
        else:
            variation = item['variation']
            assert variation['type'] == 'Element'
            content = variation['content']


            # unspecified content (just raw bits)
            def case0():
                if bitsFrom == bitsTo:
                    tell('<Bits bit="{}">'.format(bitsFrom))
                else:
                    tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                with indent:
                    tell('<BitsShortName>{}</BitsShortName>'.format(item['name']))
                    if item['title']:
                        tell('<BitsName>{}</BitsName>'.format(item['title']))

            # defined content
            def case1(val):
                rule = val['rule']
                t = rule['type']

                if t == 'Table':
                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(item['name']))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(item['title']))
                        for key,value in rule['values']:
                            tell('<BitsValue val="{}">{}</BitsValue>'.format(key, xmlquote(value)))

                elif t == 'String':
                    variation = case('string variation', rule['variation'],
                        ('StringAscii', 'ascii'),
                        ('StringICAO', 'icao'),
                        ('StringOctal', 'octal'),
                    )
                    assert bitsFrom != bitsTo
                    tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, variation))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(item['name']))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(item['title']))

                elif t == 'Integer':
                    signed = 'signed' if rule['signed'] else 'unsigned'
                    constraints = rule['constraints']
                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, signed))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(item['name']))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(item['title']))
                        msg = '<BitsInteger'
                        for c in constraints:
                            if c['type'] in ['>=', '>']:
                                msg += ' min="{}"'.format(c['value']['value'])
                                break
                        for c in constraints:
                            if c['type'] in ['<=', '<']:
                                msg += ' max="{}"'.format(c['value']['value'])
                                break
                        msg +='></BitsInteger>'

                        tell(msg)

                elif t == 'Quantity':
                    signed = 'signed' if rule['signed'] else 'unsigned'
                    k = getNumber(rule['scaling'])
                    fract = rule['fractionalBits']
                    unit = rule['unit']
                    constraints = rule['constraints']
                    scale = format(float(k) / (pow(2, fract)), '.29f')
                    scale = scale.rstrip('0')
                    if scale[-1] == '.':
                        scale += '0'

                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, signed))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(item['name']))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(item['title']))
                        msg = '<BitsUnit scale="{}"'.format(scale)
                        for c in constraints:
                            if c['type'] in ['>=', '>']:
                                msg += ' min="{}"'.format(c['value']['value'])
                                break
                        for c in constraints:
                            if c['type'] in ['<=', '<']:
                                msg += ' max="{}"'.format(c['value']['value'])
                                break
                        msg +='>{}</BitsUnit>'.format(unit)

                        tell(msg)

                else:
                    raise Exception('unexpected value type {}'.format(t))

            # complex content rule (process the same as raw)
            def case2(val):
                if bitsFrom == bitsTo:
                    tell('<Bits bit="{}">'.format(bitsFrom))
                else:
                    tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                with indent:
                    tell('<BitsShortName>{}</BitsShortName>'.format(item['name']))
                    if item['title']:
                        tell('<BitsName>{}</BitsName>'.format(item['title']))

            renderRule(content, case0, case1, case2)
        tell('</Bits>')

class Variation(object):
    def __init__(self, *args):
        self.args = args
        self.oldRender = self.render
        self.render = self.realRender

    def realRender(self):
        name = self.__class__.__name__
        tell('<DataItemFormat desc="{} data item.">'.format(name))
        with indent:
            self.oldRender()
        tell('</DataItemFormat>')

class Fixed(Variation):
    def render(self):
        bitSize, items = self.args
        assert (bitSize % 8) == 0, "bit alignment error"
        byteSize = bitSize // 8
        tell('<Fixed length="{}">'.format(byteSize))
        bitsFrom = bitSize
        for item in items:
            n = getItemSize(item)
            bitsTo = bitsFrom - n + 1
            with indent:
                Bits(item, bitsFrom, bitsTo).render()
            bitsFrom -= n
        tell('</Fixed>')

class Variable(Variation):
    def render(self):
        n1, n2, items = self.args
        chunks = chain(repeat(n1,1), repeat(n2))
        tell('<Variable>')
        with indent:
            while True:
                bitSize = next(chunks)
                assert (bitSize % 8) == 0, "bit alignment error"
                byteSize = bitSize // 8
                tell('<Fixed length="{}">'.format(byteSize))
                bitsFrom = bitSize
                with indent:
                    while True:
                        item = items[0]
                        items = items[1:]
                        n = getItemSize(item)
                        bitsTo = bitsFrom - n + 1
                        Bits(item, bitsFrom, bitsTo).render()
                        bitsFrom -= n
                        if bitsFrom <= 1:
                            break
                    Fx().render()
                tell('</Fixed>')
                if not items:
                    break
        tell('</Variable>')

class Repetitive(Variation):
    def render(self):
        bitSize, items = self.args
        assert (bitSize % 8) == 0, "bit alignment error"
        byteSize = bitSize // 8
        tell('<Repetitive length="{}">'.format(byteSize))
        bitsFrom = bitSize
        for item in items:
            n = getItemSize(item)
            bitsTo = bitsFrom - n + 1
            with indent:
                Bits(item, bitsFrom, bitsTo).render()
            bitsFrom -= n
        tell('</Repetitive>')

class Explicit(Variation):
    def render(self):
        tell('<Explicit/>')

class Compound(Variation):
    def render(self):
        (items,) = self.args
        tell('<Compound>')

        with indent:

            # FSPEC
            tell('<Variable>')
            with indent:
                bp = 1
                while True:
                    tell('<Fixed length="1">')
                    n = 8
                    while True:
                        if items:
                            item = items[0]
                            items = items[1:]
                        else:
                            item = None
                        with indent:
                            if item:
                                tell('<Bits bit="{}">'.format(n))
                                with indent:
                                    tell('<BitsShortName>{}</BitsShortName>'.format(item['name']))
                                    tell('<BitsName>{}</BitsName>'.format(item['title']))
                                    tell('<BitsPresence>{}</BitsPresence>'.format(bp))
                                tell('</Bits>')
                                bp += 1
                            else:
                                tell('<Bits bit="{}">'.format(n))
                                with indent:
                                    tell('<BitsShortName>spare</BitsShortName>')
                                    tell('<BitsName>Spare bits set to 0</BitsName>')
                                tell('</Bits>')
                            n -= 1
                            if n <= 1:
                                tell('<Bits bit="1" fx="1">')
                                with indent:
                                    tell('<BitsShortName>FX</BitsShortName>')
                                    tell('<BitsName>Extension indicator</BitsName>')
                                    tell('<BitsValue val="0">no extension</BitsValue>')
                                    tell('<BitsValue val="1">extension</BitsValue>')
                                tell('</Bits>')
                                break
                    tell('</Fixed>')
                    if not items:
                        break
            tell('</Variable>')

            # item list
            (items,) = self.args
            for item in items:
                tell('')
                if item is not None:
                    obj = makeVariation(item)
                    obj.oldRender()
        tell('</Compound>')

class TopItem(object):
    def __init__(self, item):
        self.item = item
        self.variation = makeVariation(item)

    def render(self):
        item = self.item
        tell('')
        tell('<DataItem id="{}" rule="TO BE REMOVED">'.format(item['name']))
        title = item['title']
        definition = item['definition']
        with indent:
            if title:
                tell('<DataItemName>{}</DataItemName>'.format(xmlquote(title)))
            if definition:
                tell('<DataItemDefinition>')
                with indent:
                    for line in definition.splitlines():
                        tell(line)
                tell('</DataItemDefinition>')
            self.variation.render()
        tell('</DataItem>')

class Category(object):
    def __init__(self, root, cks):
        self.root = root
        self.cks = cks
        self.items = [TopItem(item) for item in root['catalogue']]

    def render(self):
        category = self.root['number']
        edition = self.root['edition']
        title = self.root['title']
        tell('<?xml version="1.0" encoding="UTF-8"?>')
        tell('')
        tell('<!--')
        with indent:
            tell('Asterix Category {:03d} v{}.{} definition'.format(category, edition['major'], edition['minor']))

            tell('Do not edit directly!')
            tell('This file is auto-generated from the original json specs file.')
            tell('sha1sum of the json input: {}'.format(self.cks))
        tell('-->')
        tell('')
        tell('<Category id="{:d}" name="{}" ver="{}.{}">'.format(category, title, edition['major'], edition['minor']))
        with indent:
            for i in self.items:
                i.render()
        with indent:
            self.renderUap()
        tell('')
        tell('</Category>')

    def renderUap(self):
        uap = self.root['uap']
        ut = uap['type']
        if ut == 'uap':
            variations = [{'name': 'uap', 'items': uap['items']}]
        elif ut == 'uaps':
            variations = uap['variations']
        else:
            raise Exception('unexpected uap type {}'.format(ut))

        for var in variations:
            tell('')
            tell('<UAP>')
            name = var['name']
            items = var['items']
            with indent:
                bit = 0
                frn = 1
                while True:
                    chunk = items[0:7]
                    items = items[7:]
                    for i in chunk:
                        tell('<UAPItem bit="{}" frn="{}" len="TO BE REMOVED">{}</UAPItem>'.format(bit,frn, i or '-'))
                        bit += 1
                        frn += 1
                    if not items:
                        while bit % 8 != 7:
                            tell('<UAPItem bit="{}" frn="{}" len="TO BE REMOVED">-</UAPItem>'.format(bit,frn))
                            bit += 1
                            frn += 1
                    tell('<UAPItem bit="{}" frn="FX" len="-">-</UAPItem>'.format(bit,frn))
                    bit += 1
                    if not items:
                        break
            tell('</UAP>')

# main
parser = argparse.ArgumentParser(description='Render asterix specs from json to custom xml.')
parser.add_argument('infile', nargs='?', type=argparse.FileType('rb'), default=sys.stdin.buffer)
parser.add_argument('outfile', nargs='?', type=argparse.FileType('wt'), default=sys.stdout)

args = parser.parse_args()

s = args.infile.read()
root = json.loads(s)
cks = hashlib.sha1(s).hexdigest()
cat = Category(root, cks)
cat.render()
result = ''.join([line+'\n' for line in accumulator])
args.outfile.write(result)


#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# item name changes, {category: { (name, name,...): new_name }}
item_renames = {
    11: {
        ('000', '000'): 'MsgTyp',
        ('015', '015'): 'SI',
        ('090', '090'): 'MFL',
        ('092', '092'): 'CTGA',
        ('140', '140'): 'ToTI',
        ('215', '215'): 'RoCD',
        ('300', '300'): 'VFI',
        ('430', '430'): 'FLS',
    },
    21: {
        ('015', '015'): 'id',
        ('020', '020'): 'ECAT',
    },
    62: {
        ('015', '015'): 'SID',
        ('040', '040'): 'TrkN',
        ('070', '070'): 'ToT',

    },
}

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
        "’": "&apos;",
        "<":  "&lt;",
        ">": "&gt;",
    })

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

    def __init__(self, parent, item, bitsFrom, bitsTo):
        self.parent = parent
        self.item = item
        self.bitsFrom = bitsFrom
        self.bitsTo = bitsTo

    @property
    def cat(self):
        return self.parent.cat

    @property
    def name(self):
        old_name = self.item['name']
        full_name = tuple(list(self.parent.full_name[1:]) + [old_name])
        rename_map = item_renames.get(self.cat, {})
        new_name = rename_map.get(full_name, old_name)
        return(new_name)

    def render(self):
        item, bitsFrom, bitsTo = self.item, self.bitsFrom, self.bitsTo
        if item['spare']:
            if bitsFrom == bitsTo:
                tell('<Bits bit="{}">'.format(bitsFrom))
            else:
                tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
            with indent:
                tell('<BitsShortName>spare</BitsShortName>')
                tell('<BitsName>Spare bit(s) set to 0</BitsName>')
                tell('<BitsConst>0</BitsConst>')
            tell('</Bits>')
        else:
            variation = item['variation']
            vt = variation['type']
            if vt == 'Element':
                content = variation['content']
            elif vt == 'Repetitive':
                content = variation['variation']['content']
            else:
                raise Exception('unexpected variation type {}'.format(vt))

            # unspecified content (raw bits)
            def case0():
                if bitsFrom == bitsTo:
                    tell('<Bits bit="{}">'.format(bitsFrom))
                else:
                    tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                with indent:
                    tell('<BitsShortName>{}</BitsShortName>'.format(self.name))
                    if item['title']:
                        tell('<BitsName>{}</BitsName>'.format(item['title']))
                tell('</Bits>')

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
                        tell('<BitsShortName>{}</BitsShortName>'.format(self.name))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(item['title']))
                        for key,value in sorted(rule['values']):
                            tell('<BitsValue val="{}">{}</BitsValue>'.format(key, xmlquote(value)))
                    tell('</Bits>')

                elif t == 'String':
                    variation = case('string variation', rule['variation'],
                        ('StringAscii', 'ascii'),
                        ('StringICAO', '6bitschar'),
                        ('StringOctal', 'octal'),
                    )
                    assert bitsFrom != bitsTo
                    tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, variation))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(self.name))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(item['title']))
                    tell('</Bits>')

                elif t == 'Integer':
                    signed = 'signed' if rule['signed'] else 'unsigned'
                    constraints = rule['constraints']
                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, signed))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(self.name))
                        if item['title']:
                            tell('<BitsName>{}</BitsName>'.format(item['title']))
                        msg = '<BitsUnit'
                        for c in constraints:
                            if c['type'] in ['>=', '>']:
                                msg += ' min="{}"'.format(c['value']['value'])
                                break
                        for c in constraints:
                            if c['type'] in ['<=', '<']:
                                msg += ' max="{}"'.format(c['value']['value'])
                                break
                        msg +='></BitsUnit>'
                        tell(msg)
                    tell('</Bits>')

                elif t == 'Quantity':
                    signed = 'signed' if rule['signed'] else 'unsigned'
                    k = getNumber(rule['scaling'])
                    fract = rule['fractionalBits']
                    unit = rule['unit']
                    constraints = rule['constraints']
                    if fract > 0:
                        scale = format(float(k) / (pow(2, fract)), '.29f')
                        scale = scale.rstrip('0')
                    else:
                        scale = format(float(k))

                    if scale[-1] == '.':
                        scale += '0'

                    if bitsFrom == bitsTo:
                        tell('<Bits bit="{}">'.format(bitsFrom))
                    else:
                        tell('<Bits from="{}" to="{}" encode="{}">'.format(bitsFrom, bitsTo, signed))
                    with indent:
                        tell('<BitsShortName>{}</BitsShortName>'.format(self.name))
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
                    tell('</Bits>')

                elif t == 'Bds':
                    tell('<BDS/>')

                else:
                    raise Exception('unexpected value type {}'.format(t))

            # complex content rule (process the same as raw)
            def case2(val):
                if bitsFrom == bitsTo:
                    tell('<Bits bit="{}">'.format(bitsFrom))
                else:
                    tell('<Bits from="{}" to="{}">'.format(bitsFrom, bitsTo))
                with indent:
                    tell('<BitsShortName>{}</BitsShortName>'.format(self.name))
                    if item['title']:
                        tell('<BitsName>{}</BitsName>'.format(item['title']))
                tell('</Bits>')

            renderRule(content, case0, case1, case2)

class Variation(object):

    @staticmethod
    def create(parent, item):
        variation = item['variation']
        vt = variation['type']

        # single item Fixed variation
        if vt == 'Element':
            size = getItemSize(item)
            return Fixed(parent, item, size, [item])

        if vt == 'Group':
            size = getItemSize(item)
            items = variation['items']
            return Fixed(parent, item, size, items)

        if vt == 'Extended':
            n1 = variation['first']
            n2 = variation['extents']
            items = variation['items']
            return Variable(parent, item, n1, n2, items)

        if vt == 'Repetitive':
            size = getVariationSize(variation)
            return Repetitive(parent, item, size, variation['variation'])

        if vt == 'Explicit':
            return Explicit(parent, item)

        if vt == 'Compound':
            fspec = variation['fspec']
            items = variation['items']
            return Compound(parent, item, fspec, items)

        raise Exception('unexpected variation type {}'.format(vt))

    @property
    def cat(self):
        return self.parent.cat

    @property
    def full_name(self):
        return tuple(list(self.parent.full_name) + [self.item['name']])

    @property
    def name(self):
        old_name = self.item['name']
        full_name = tuple(list(self.parent.full_name[1:]) + [old_name])
        rename_map = item_renames.get(self.cat, {})
        new_name = rename_map.get(full_name, old_name)
        return(new_name)

    def __init__(self, parent, item, *args):
        self.parent = parent
        self.item = item
        self.args = args

        self.oldRender = self.render
        self.render = self.realRender

    @property
    def desc(self):
        return '{} data item.'.format(self.__class__.__name__)

    def realRender(self):
        tell('<DataItemFormat desc="{}">'.format(self.desc))
        with indent:
            self.oldRender()
        tell('</DataItemFormat>')

class Fixed(Variation):

    @property
    def desc(self):
        bitSize, items = self.args
        byteSize = bitSize // 8
        return '{}-octet fixed length data item.'.format(byteSize)

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
                Bits(self, item, bitsFrom, bitsTo).render()
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
                        Bits(self, item, bitsFrom, bitsTo).render()
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
        bitSize, variation = self.args
        assert (bitSize % 8) == 0, "bit alignment error"
        byteSize = bitSize // 8
        tell('<Repetitive>')
        items = variation.get('items')
        if items is None:
            item = self.item
            bitSize = getItemSize(item)
            bitsFrom = bitSize
            bitsTo = bitsFrom - bitSize + 1
            with indent:
                Bits(self, item, bitsFrom, bitsTo).render()
        else:
            with indent:
                tell('<Fixed length="{}">'.format(byteSize))
                bitsFrom = bitSize
                for item in items:
                    n = getItemSize(item)
                    bitsTo = bitsFrom - n + 1
                    with indent:
                        Bits(self, item, bitsFrom, bitsTo).render()
                    bitsFrom -= n
                tell('</Fixed>')
        tell('</Repetitive>')

class Explicit(Variation):
    def render(self):
        definition = None
        if self.parent.item['name'] == 'RE':
            definition = self.parent.category.re
            if definition:
                definition['name'] = 'RE'
        if self.parent.item['name'] == 'SP':
            definition = self.parent.category.sp
            if definition:
                definition['name'] = 'SP'
        tell('<Explicit>')
        if definition:
            Variation.create(self, definition).render()
        else:
            with indent:
                tell('<Fixed length="1">')
                with indent:
                    tell('<Bits from="8" to="1">')
                    with indent:
                        tell('<BitsShortName>VAL</BitsShortName>')
                    tell('</Bits>')
                tell('</Fixed>')
        tell('</Explicit>')

class Compound(Variation):
    def render(self):
        (fspec, items) = self.args
        tell('<Compound>')

        with indent:
            tell('<Variable>')

            # FSPEC with predefined length
            if fspec is not None:
                assert (fspec % 8) == 0, "bit alignment error"
                assert fspec == len(items), "item length mismatch"
                with indent:
                    tell('<Fixed length="{}">'.format(fspec // 8))
                    n = len(items)
                    bp = 1
                    for item in items:
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
                    tell('</Fixed>')

            # FSPEC with FX extension
            else:
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
            (fspec,items) = self.args
            for item in items:
                tell('')
                if item is not None:
                    obj = Variation.create(self, item)
                    obj.oldRender()
        tell('</Compound>')

class TopItem(object):
    def __init__(self, category, item):
        self.category = category
        self.item = item

    @property
    def cat(self):
        return self.category.cat

    @property
    def full_name(self):
        return (self.item['name'],)

    def render(self):
        item = self.item
        tell('')
        tell('<DataItem id="{}">'.format(item['name']))
        title = item['title']
        definition = item['definition']
        with indent:
            if title:
                tell('<DataItemName>{}</DataItemName>'.format(xmlquote(title)))
            if definition:
                tell('<DataItemDefinition>')
                with indent:
                    for line in definition.splitlines():
                        tell(xmlquote(line))
                tell('</DataItemDefinition>')
            Variation.create(self, item).render()
        tell('</DataItem>')

class Category(object):
    def __init__(self, root, cks, re=None, sp=None):
        self.root = root
        self.cks = cks
        self.re = re
        self.sp = sp
        self.items = [TopItem(self, item) for item in root['catalogue']]

    @property
    def cat(self):
        return self.root['number']

    def render(self):
        category = self.cat
        edition = self.root['edition']
        title = self.root['title']
        tell('<?xml version="1.0" encoding="UTF-8"?>')
        tell('<!DOCTYPE Category SYSTEM "asterix.dtd">')
        tell('')
        tell('<!--')
        with indent:
            tell('')
            tell('Asterix Category {:03d} v{}.{} definition'.format(category, edition['major'], edition['minor']))
            tell('')
            tell('This file is auto-generated from json specs file.')
            tell('sha1sum of concatenated json input(s): {}'.format(self.cks))
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
                        tell('<UAPItem bit="{}" frn="{}">{}</UAPItem>'.format(bit,frn, i or '-'))
                        bit += 1
                        frn += 1
                    if not items:
                        while bit % 8 != 7:
                            tell('<UAPItem bit="{}" frn="{}">-</UAPItem>'.format(bit,frn))
                            bit += 1
                            frn += 1
                    tell('<UAPItem bit="{}" frn="FX" len="-">-</UAPItem>'.format(bit,frn))
                    bit += 1
                    if not items:
                        break
            tell('</UAP>')


class AsterixJson2XML(object):
    def __init__(self, root, cks, re=None, sp=None):
        self.cat = Category(root, cks, re, sp)

    def parse(self):
        global accumulator
        accumulator = []
        self.cat.render()
        result = ''.join([line + '\n' for line in accumulator])
        return result

# main
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Render asterix specs from json to custom xml.')
    parser.add_argument('--cat', nargs='?', type=argparse.FileType('rb'), default=sys.stdin.buffer, help="input CATegory JSON file")
    parser.add_argument('--ref', nargs='?', type=argparse.FileType('rb'), help="input REF JSON file")
    parser.add_argument('--outfile', nargs='?', type=argparse.FileType('wt'), default=sys.stdout)

    args = parser.parse_args()

    cat_input = args.cat.read()
    root = json.loads(cat_input)

    ref_input = b''
    ref = None
    if args.ref:
        ref_input = args.ref.read()
        ref = json.loads(ref_input)

    cks = hashlib.sha1(cat_input+ref_input).hexdigest()
    cat = AsterixJson2XML(root, cks, re=ref)
    result = cat.parse()
    args.outfile.write(result)


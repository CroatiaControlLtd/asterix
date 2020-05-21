#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Render json object to rst format."""

import sys
import argparse
import json
from itertools import chain, accumulate, repeat

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

def tell(s):
    s = ' '*indentLevel*4 + s
    accumulator.append(s.rstrip())

def underline(c, s):
    n = len(s)
    assert len(c) == 1, 'expecting single underline character'
    return '{}\n{}\n'.format(s, c*n)

def render(s):
    """Rendering entry point"""
    root = json.loads(s)
    renderHeader(root)

    cat = root['number']
    tell(underline('-', 'Description of standard data items'))
    [renderItem(cat, item) for item in root['catalogue']]

    renderUap(root)

    return ''.join([line+'\n' for line in accumulator])

def renderHeader(root):
    cat = root['number']
    tell(underline('=', "Asterix category " + '{:03d}'.format(cat) + ' - {}'.format(root['title'])))
    tell('**category**: {:03d}'.format(cat))
    tell('')
    edition = root['edition']
    tell('**edition**: {}.{}'.format(edition['major'], edition['minor']))
    tell('')
    date = root['date']
    tell('**date**: {}-{:02d}-{:02d}'.format(date['year'], date['month'], date['day']))
    tell('')
    tell(underline('-', 'Preamble'))
    tell(root['preamble'])
    tell('')

def renderItem(cat, item):
    subitem = item['subitem']
    tell('')
    tell(underline('*', 'I'+'{:03d}'.format(cat)+'/'+subitem['name']+' - ' + subitem['title']))

    def case0():
        tell('*Encoding rule:* Unspecified')
    def case1(enc):
        tell('*Encoding rule:* This item is ``{}``.'.format(enc['rule']))
    def case2(enc):
        tell('*Encoding rule:* Presence of this item depends on the value of item ``{}``. ::'.format('/'.join(enc['name'])))
        tell('')
        with indent:
            [tell('{} -> {}'.format(a,b)) for (a,b) in enc['rules']]
    renderRule(item['encoding'], case0, case1, case2)
    tell('')

    tell('*Definition*: {}'.format(item['definition']))
    tell('*Structure*: ')
    tell('')
    renderSubitem(subitem['element'])
    tell('')
    if subitem['remark']:
        tell(subitem['remark'])

def constrainToString(constrain):
    return '{} {}'.format(constrain['type'], str(getNumber(constrain['value'])))

def bits(n):
    if n == 1:
        return '1 bit'
    return '{} bits'.format(n)

def renderSubitem(element):

    def renderInteger(value):
        tell('- {} integer'.format('signed' if value['signed'] else 'unsigned'))
        for const in value['constraints']:
            tell('- value :math:`{}`'.format(constrainToString(const)))
        tell('')

    def renderQuantity(value):
        tell('- {} quantity'.format('signed' if value['signed'] else 'unsigned'))
        k = getNumber(value['scaling'])
        fract = value['fractionalBits']
        unit = value['unit']
        tell('- scaling factor: {}'.format(k))
        tell('- fractional bits: {}'.format(fract))
        if unit:
            tell('- unit: "{}"'.format(unit))

        if fract == 0:
            lsb = ":math:`{}` {}".format(k, unit or '')
            tell("- LSB = {}".format(lsb))
        else:
            b = '{2^{'+str(fract)+'}}'
            lsb1 = ":math:`{} / {}` {}".format(k, b, unit)
            c = '{'+str(pow(2, fract))+'}'
            d = pow(2, fract)
            lsb2 = ":math:`{} / {}` {}".format(k, c, unit)
            lsb3 = ":math:`\\approx {}` {}".format(float(k)/d, unit)
            tell("- LSB = {} = {} {}".format(lsb1, lsb2, lsb3))
        for const in value['constraints']:
            tell('- value :math:`{}` {}'.format(constrainToString(const),unit))

        tell('')

    def renderFixed():
        n = element['size']
        tell('- {} [``{}``]'.format(bits(n), '.'*n))
        tell('')

        def case0():
            tell('- raw value')
            tell('')
            return n

        def case1(val):
            value = val['rule']
            t = value['type']

            if t == 'Table':
                tell('- values:')
                tell('')
                with indent:
                    for key,value in value['values']:
                        tell('| {}: {}'.format(key, value))
                tell('')
            elif t == 'String':
                if value['variation'] == 'StringAscii':
                    tell('- Ascii string (8-bits per character)')
                elif value['variation'] == 'StringICAO':
                    tell('- Ascii ICAO (6-bits per character)')
                else:
                    raise Exception('unexpected string type {}'.format(value['variation']))
                tell('')
            elif t == 'Integer':
                renderInteger(value)
            elif t == 'Quantity':
                renderQuantity(value)
            else:
                raise Exception('unexpected value type {}'.format(t))
            return n

        def case2(val):
            otherItem = '/'.join(val['name'])
            tell('* Content of this item depends on the value of item ``{}``.'.format(otherItem))
            tell('')
            with indent:
                for (a,b) in val['rules']:
                    tell('* In case of ``{} == {}``:'.format(otherItem, a))
                    with indent:
                        case1({'rule': b})
            tell('')
            return n

        return renderRule(element['content'], case0, case1, case2)

    def renderMaybeSubitem(subitem):
        if subitem['spare']:
            n = subitem['length']
            tell('``(spare)``')
            tell('')
            tell('- {} [``{}``]'.format(bits(n), '.'*n))
            tell('')
            return n
        tit = subitem['title']
        tell('``{}``{}'.format(subitem['name'], ' - *{}*'.format(tit) if tit else ''))
        tell('')
        if subitem['description']:
            tell(' '.join(subitem['description'].splitlines()))
            tell('')
        n = renderSubitem(subitem['element'])
        if subitem['remark']:
            with indent:
                tell('remark')
                with indent:
                    for i in subitem['remark'].strip().splitlines():
                        tell(i)
            tell('')
        return n

    def renderGroup():
        n = 0
        with indent:
            for subitem in element['subitems']:
                n += renderMaybeSubitem(subitem)
        return n

    def renderExtended():
        n1 = element['first']
        n2 = element['extents']
        fx = accumulate(chain(repeat(n1,1), repeat(n2)))
        nextFx = next(fx)
        tell('Extended item with first part ``{} bits`` long and optional ``{} bits`` extends.'.format(n1, n2))
        tell('')
        n = 0
        terminated = False
        with indent:
            for subitem in element['subitems']:
                n += renderMaybeSubitem(subitem)
                terminated = False
                if (n+1) == nextFx:
                    tell('``(FX)``')
                    tell('')
                    tell('- extension bit')
                    tell('')
                    with indent:
                        for key,value in [(0, "End of data item"), (1, "Extension into next extent")]:
                            tell('| {}: {}'.format(key, value))
                    tell('')
                    n += 1
                    nextFx = next(fx)
                    terminated = True
        assert terminated, "wrong termination of extended item"
        return n

    def renderRepetitive():
        rep = element['rep']
        tell('Repetitive item, repetition factor {} octet(s).'.format(rep))
        tell('')
        with indent:
            x = renderSubitem(element['element'])
        return x

    def renderExplicit():
        tell('Explicit item')
        tell('')
        return 0

    def renderCompound():
        tell('Compound item')
        tell('')
        n = 0
        with indent:
            for subitem in element['subitems']:
                if subitem is None:
                    tell('(empty subitem)')
                    tell('')
                else:
                    n += renderMaybeSubitem(subitem)
        return n

    return locals()['render'+element['type']]()

def renderUap(root):
    cat = root['number']
    uap = root['uap']

    def findItem(name):
        for item in root['catalogue']:
            subitem = item['subitem']
            if subitem['name'] == name:
                return subitem['title']
        return '??'

    def dumpUap(items):
        frn = 0
        cnt = 0
        for name in items:
            frn += 1
            cnt += 1

            s = '- ({}) '.format(frn)
            if name is None:
                s += '``(spare)``'
            else:
                s += '``I{:03d}/{}`` - {}'.format(cat, name, findItem(name))
            tell(s)

            if cnt >= 7:
                cnt = 0
                tell('- ``(FX)`` - Field extension indicator')
        tell('')

    tell(underline('=', "User Application Profile for Category {:03d}".format(cat)))
    t = uap['type']
    if t == 'uap':
        dumpUap(uap['items'])
    elif t == 'uaps':
        tell('This category has multiple UAPs.')
        tell('')
        for var in uap['variations']:
            tell(underline('-', var['name']))
            dumpUap(var['items'])
    else:
        raise Exception('unexpected uap type {}'.format(t))

# main
parser = argparse.ArgumentParser(description='Render asterix specs from json to rst.')
parser.add_argument('infile', nargs='?', type=argparse.FileType('rb'), default=sys.stdin.buffer)
parser.add_argument('outfile', nargs='?', type=argparse.FileType('w'), default=sys.stdout)

args = parser.parse_args()

s = args.infile.read()
result = render(s)
args.outfile.write(result)


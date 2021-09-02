#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script that download JSON asterix specifications from upstream repository
# and converts them to XML specification.

import urllib.request, json
import hashlib
from asterixjson2xml import AsterixJson2XML

upstream_repo = 'https://zoranbosnjak.github.io/asterix-specs'

with urllib.request.urlopen(upstream_repo+'/manifest.json') as url:
    data = json.loads(url.read().decode())
    for category in data:
        for ver in category['cats']:
            vermajor, verminor = ver.split('.')
            filename = upstream_repo + '/specs/cat%s/cats/cat%s/definition.json' % (category['category'], ver)
            print('Processing: ' + filename)
            with urllib.request.urlopen(filename) as caturl:
                cat_input = caturl.read()
                root = json.loads(cat_input)

                ref_input = b''
                ref = None
                # todo
                #if args.ref:
                #    ref_input = args.ref.read()
                #    ref = json.loads(ref_input)

                cks = hashlib.sha1(cat_input + ref_input).hexdigest()
                cat = AsterixJson2XML(root, cks, re=ref)
                result = cat.parse()

                outfilename = 'specs/asterix_cat%s_%s_%s.xml' % (category['category'], vermajor, verminor)
                with open(outfilename, 'wt') as jsf:
                    jsf.write(result)

with urllib.request.urlopen(upstream_repo+'/gitrev.txt') as url:
    rev = url.read().decode()[0:7]
    print('Done!')
    print('You might want to examine the changed, then proceed with:')
    print('')
    print('git add specs/*xml')
    print('git status specs')
    print('git commit -m "synced with asterix-specs upstream, revision #'+rev+'"')
    print('git push')


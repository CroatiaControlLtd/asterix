# !/usr/bin/python

import asterix
import unittest
import os
import sys


class AsterixInitTest(unittest.TestCase):

    def assertRaisesRegexpression(self, exc, text):
        if sys.version_info < (3, 0, 0):
            return self.assertRaisesRegexp(exc, text)
        else:
            return self.assertRaisesRegex(exc, text)

    def test_init_ok(self):
        sample_init = os.path.join(os.path.dirname(__file__), '../config/asterix_cat062_1_16.xml')
        ret = asterix.init(sample_init)
        self.assertIs(ret, 0)

    def test_init_type_error(self):
        with self.assertRaises(TypeError):
            asterix.init()

    def test_init_value_error(self):
        with self.assertRaises(ValueError):
            asterix.init(0)

    def test_init_io_error(self):
        with self.assertRaises(IOError):
            asterix.init("unknown_file.xml")

    def test_init_syntax_error(self):

        with self.assertRaisesRegexpression(SyntaxError, "Unknown tag: noCategory*"):
            sample_init = os.path.join(os.path.dirname(__file__), './parsing_error_1.xml')
            asterix.init(sample_init)

        with self.assertRaisesRegexpression(SyntaxError, "XMLParser : <Bits> without <Format>*"):
            sample_init = os.path.join(os.path.dirname(__file__), './parsing_error_2.xml')
            asterix.init(sample_init)


def main():
    unittest.main()

if __name__ == '__main__':
    main()

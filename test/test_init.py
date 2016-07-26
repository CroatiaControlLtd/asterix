# !/usr/bin/python
__author__ = 'dsalanti'

import asterix
import unittest
from pkg_resources import Requirement, resource_filename


class AsterixInitTest(unittest.TestCase):

    def test_reinit(self):
        sample_init = resource_filename(Requirement.parse("asterix"), "install/config/asterix_cat062_1_16.xml")
        ret = asterix.init(sample_init)
        self.assertIs(ret, 0)


def main():
    unittest.main()


if __name__ == '__main__':
    main()

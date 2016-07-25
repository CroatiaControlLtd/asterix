# !/usr/bin/python
__author__ = 'dsalanti'

import asterix
import unittest
from pkg_resources import Requirement, resource_filename

def deep_sort(obj):
    """
    Recursively sort list or dict nested lists
    """

    if isinstance(obj, dict):
        _sorted = {}
        for key in sorted(obj):
            _sorted[key] = deep_sort(obj[key])

    elif isinstance(obj, list):
        new_list = []
        for val in obj:
            new_list.append(deep_sort(val))
        _sorted = sorted(new_list)

    else:
        _sorted = obj

    return _sorted

class AsterixParseTest(unittest.TestCase):

    def test_ParseCAT048(self):
        sample_filename = resource_filename(Requirement.parse("asterix"), "install/sample_data/cat048.raw")
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data)
            self.maxDiff=None
            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(packet), 1)
            self.assertTrue('I220' in packet[0])
            self.assertEqual(packet[0]['category'], 48)
            self.assertEqual(packet[0]['I220']['ACAddr']['val'], '3C660C')
            self.assertEqual(packet[0]['I220']['ACAddr']['desc'], 'AircraftAddress')
            self.assertDictEqual(deep_sort(packet[0]), deep_sort({'I161': {'Tn': {'desc': 'Track Number', 'val': 3563}},
                                         'I090': {'G': {'meaning': 'Default', 'desc': '', 'val': 0},
                                                  'V': {'meaning': 'Code validated', 'desc': '', 'val': 0},
                                                  'FL': {'desc': 'FlightLevel', 'val': 330.0}}, 'I020': {
                    'RAB': {'meaning': 'Report from aircraft transponder', 'desc': 'RAB', 'val': 0},
                    'TYP': {'meaning': 'Single ModeS Roll-Call', 'desc': 'TYP', 'val': 5},
                    'FX': {'meaning': 'End of Data Item', 'desc': 'FX', 'val': 0},
                    'SIM': {'meaning': 'Actual target report', 'desc': 'SIM', 'val': 0},
                    'SPI': {'meaning': 'Absence of SPI', 'desc': 'SPI', 'val': 0},
                    'RDP': {'meaning': 'Report from RDP Chain 1', 'desc': 'RDP', 'val': 0}}, 'I250': [
                    {'DSB2': {'desc': 'Comm B Data Buffer Store 2', 'val': 0},
                     'DSB1': {'desc': 'Comm B Data Buffer Store 1', 'val': 4},
                     'MBdata': {'desc': 'MBdata', 'val': 'C0780031BC0000'}}],
                                         'I230': {'spare': {'desc': 'spare bit set to 0', 'const': 5e-324, 'val': 0},
                                                  'BDS16': {'desc': 'BDS 1,0 bit 16', 'val': 1},
                                                  'AIC': {'meaning': 'Yes', 'desc': 'Aircraft identification capability',
                                                          'val': 1}, 'STAT': {'desc': 'STAT', 'val': 0},
                                                  'COM': {'desc': 'COM', 'val': 1}, 'SI': {'meaning': 'SI-Code Capable',
                                                                                           'desc': 'SI/II Transponder Capability',
                                                                                           'val': 0},
                                                  'ARC': {'meaning': '25ft resolution',
                                                          'desc': 'Altitude reporting capability', 'val': 1},
                                                  'BDS37': {'desc': 'BDS 1,0 bits 37/40', 'val': 5},
                                                  'ModeSSSC': {'meaning': 'Yes',
                                                               'desc': 'ModeS Specific Service Capability', 'val': 1}},
                                         'I070': {'G': {'meaning': 'Default', 'desc': '', 'val': 0},
                                                  'V': {'meaning': 'Code validated', 'desc': '', 'val': 0},
                                                  'spare': {'desc': 'spare bit set to 0', 'const': 5e-324, 'val': 0},
                                                  'Mode3A': {'desc': 'Mode-3/A reply code', 'val': '1000'}, 'L': {
                                                 'meaning': 'Mode-3/A code as derived from the reply of the transponder',
                                                 'desc': '', 'val': 0}},
                                         'I010': {'SAC': {'desc': 'System Area Code', 'val': 25},
                                                  'SIC': {'desc': 'System Identification Code', 'val': 201}},
                                         'I220': {'ACAddr': {'desc': 'AircraftAddress', 'val': '3C660C'}},
                                         'I200': {'CGS': {'desc': 'Calculated groundspeed', 'val': 434.94},
                                                  'CHdg': {'desc': 'Calculated heading', 'val': 124.002685546875}},
                                         'category': 48, 'I140': {'ToD': {'desc': 'Time Of Day', 'val': 27354.6015625}},
                                         'I170': {'DOU': {'meaning': 'Normal confidence', 'desc': 'DOU', 'val': 0},
                                                  'CDM': {'meaning': 'Maintaining', 'desc': 'CDM', 'val': 0},
                                                  'CNF': {'meaning': 'Confirmed Track', 'desc': 'CNF', 'val': 0},
                                                  'MAH': {'meaning': 'No horizontal man. sensed', 'desc': 'MAH', 'val': 0},
                                                  'FX': {'meaning': 'End of Data Item', 'desc': 'FX', 'val': 0},
                                                  'GHO': {'meaning': 'True target track', 'desc': 'GHO', 'val': 0},
                                                  'RAD': {'meaning': 'SSR/ModeS Track', 'desc': 'RAD', 'val': 2},
                                                  'SUP': {'meaning': 'Track from cluster network - NO', 'desc': 'SUP',
                                                          'val': 0},
                                                  'spare': {'desc': 'spare bits set to 0', 'const': 1.5e-323, 'val': 0},
                                                  'TRE': {'meaning': 'Track still alive', 'desc': 'TRE', 'val': 0},
                                                  'TCC': {'meaning': 'Radar plane', 'desc': 'TCC', 'val': 0}},
                                         'I040': {'THETA': {'desc': '', 'val': 340.13671875},
                                                  'RHO': {'desc': '', 'max': 256.0, 'val': 197.68359375}}, 'I240': {
                    'TId': {'desc': 'Characters 1-8 (coded on 6 bits each) defining target identification',
                            'val': 'DLH65A  '}}}))

    def test_ParseCAT062CAT065(self):
        sample_filename = resource_filename(Requirement.parse("asterix"), "install/sample_data/cat062cat065.raw")
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data)

            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(packet), 3)
            self.assertIs(packet[0]['category'], 62)
            self.assertIs(packet[1]['category'], 62)
            self.assertIs(packet[2]['category'], 65)
            self.assertDictEqual(deep_sort(packet[0]), deep_sort({'I340':
                                           {'HEI': {'desc': 'Measured 3-D Height', 'val': 0},
                                            'Mode3A': {
                                                'desc': 'Mode 3/A reply under the form of 4 digits in octal representation',
                                                'val': '4276'},
                                            'ModeC': {'desc': 'Last Measured Mode C Code', 'min': -12.0, 'val': 157.0,
                                                      'max': 1270.0},
                                            'CG': {'desc': '', 'meaning': 'Default', 'val': 0},
                                            'TYP': {'desc': 'Report Type', 'meaning': 'Single SSR detection', 'val': 2},
                                            'spare': {'desc': 'Spare bits set to zero', 'val': 0, 'const': 1e-323},
                                            'POS': {'desc': 'Measured Position', 'val': 1},
                                            'SIM': {'desc': '', 'meaning': 'Actual target report', 'val': 0},
                                            'MDA': {'desc': 'Last Measured Mode 3/A code', 'val': 1},
                                            'SAC': {'desc': 'System Area Code', 'val': 25},
                                            'RHO': {'desc': 'Measured distance', 'val': 186.6875, 'max': 256.0},
                                            'L': {'desc': '',
                                                  'meaning': 'MODE 3/A code as derived from the reply of the transponder',
                                                  'val': 0},
                                            'MDC': {'desc': 'Last Measured Mode C code', 'val': 1},
                                            'SID': {'desc': 'Sensor Identification', 'val': 1},
                                            'G': {'desc': '', 'meaning': 'Default', 'val': 0},
                                            'SIC': {'desc': 'System Identification Code', 'val': 13},
                                            'TST': {'desc': '', 'meaning': 'Real target report', 'val': 0},
                                            'V': {'desc': '', 'meaning': 'Code validated', 'val': 0},
                                            'CV': {'desc': '', 'meaning': 'Code validated', 'val': 0},
                                            'THETA': {'desc': 'Measured azimuth', 'val': 259.453125},
                                            'RAB': {'desc': '', 'meaning': 'Report from aircraft transponder',
                                                    'val': 0},
                                            'FX': {'desc': 'Extension indicator', 'meaning': 'no extension', 'val': 0}},
                                       'I100': {'Y': {'desc': 'Y', 'val': -106114.0}, 'X': {'desc': 'X', 'val': -239083.0}},
                                       'I010': {'SIC': {'desc': 'System Identification Code', 'val': 100},
                                                'SAC': {'desc': 'System Area Code', 'val': 25}},
                                       'I015': {'SID': {'desc': 'Service Identification', 'val': 4}}, 'I136': {
                    'MFL': {'desc': 'Measured Flight Level', 'min': -1500.0, 'val': 15700.0, 'max': 150000.0}},
                                       'I295': {'MDA': {'desc': '', 'val': 0.0}, 'MD5': {'desc': '', 'val': 0},
                                                'FX': {'desc': 'Extension indicator', 'meaning': 'no extension', 'val': 0},
                                                'MD2': {'desc': '', 'val': 0}, 'MD4': {'desc': '', 'val': 0},
                                                'MFL': {'desc': '', 'val': 0.0}, 'MHG': {'desc': '', 'val': 0},
                                                'MD1': {'desc': '', 'val': 0}},
                                       'I210': {'Ay': {'desc': 'Ay', 'val': 0.0}, 'Ax': {'desc': 'Ax', 'val': 0.0}},
                                       'I220': {'RoC': {'desc': 'Rate of Climb/Descent', 'val': -443.75}}, 'I200': {
                    'ADF': {'desc': 'Altitude Discrepancy Flag', 'meaning': 'No altitude discrepancy', 'val': 0},
                    'TRANSA': {'desc': 'Transversal Acceleration', 'meaning': 'Constant Course', 'val': 0},
                    'LONGA': {'desc': 'Longitudinal Acceleration', 'meaning': 'Decreasing Groundspeed', 'val': 2},
                    'VERTA': {'desc': 'Vertical Rate', 'meaning': 'Descent', 'val': 2},
                    'spare': {'desc': 'Spare bit set to zero', 'val': 0, 'const': 5e-324}},
                                       'I070': {'ToT': {'desc': 'Time Of Track Information', 'val': 30911.6640625}},
                                       'I130': {
                                           'Alt': {'desc': 'Altitude', 'min': -1500.0, 'val': 43300.0, 'max': 150000.0}},
                                       'I040': {'TrkN': {'desc': 'Track number', 'val': 4980}}, 'category': 62,
                                       'I185': {'Vx': {'desc': 'Vx', 'min': -8192.0, 'val': -51.25, 'max': 8191.75},
                                                'Vy': {'desc': 'Vy', 'min': -8192.0, 'val': 170.0, 'max': 8191.75}},
                                       'I080': {'FPC': {'desc': '', 'meaning': 'Not flight-plan correlated', 'val': 0},
                                                'KOS': {'desc': '', 'meaning': 'Background service used', 'val': 1},
                                                'MDS': {'desc': '',
                                                        'meaning': 'Age of the last received Mode S track update is higher than system dependent threshold',
                                                        'val': 1},
                                                'MD5': {'desc': '', 'meaning': 'No Mode 5 interrogation', 'val': 0},
                                                'ADS': {'desc': '',
                                                        'meaning': 'Age of the last received ADS-B track update is higher than system dependent threshold',
                                                        'val': 1}, 'MI': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                'SRC': {'desc': 'Source of calculated track altitude for I062/130',
                                                        'meaning': 'height from coverage', 'val': 4},
                                                'SSR': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                'SUC': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                'STP': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                'AMA': {'desc': '',
                                                        'meaning': 'track not resulting from amalgamation process',
                                                        'val': 0}, 'SIM': {'desc': '', 'meaning': 'Actual track', 'val': 0},
                                                'AAC': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                'PSR': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                'MON': {'desc': '', 'meaning': 'Multisensor track', 'val': 0},
                                                'CNF': {'desc': '', 'meaning': 'Confirmed track', 'val': 0},
                                                'CST': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                'MD4': {'desc': '', 'meaning': 'No Mode 4 interrogation', 'val': 0},
                                                'SPI': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                'ME': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                'TSB': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                'MRH': {'desc': 'Most Reliable Height',
                                                        'meaning': 'Barometric altitude (Mode C) more reliable', 'val': 0},
                                                'TSE': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                'FX': {'desc': '', 'meaning': 'End of data item', 'val': 0},
                                                'AFF': {'desc': '', 'meaning': 'default value', 'val': 0}}, 'I135': {
                    'CTBA': {'desc': 'Calculated Track Barometric Alt', 'min': -1500.0, 'val': 15700.0, 'max': 150000.0},
                    'QNH': {'desc': 'QNH', 'meaning': 'No QNH correction applied', 'val': 0}}, 'I105': {
                    'Lon': {'desc': 'Longitude in WGS.84 in twos complement. Range -180 < longitude < 180 deg.',
                            'val': 13.0415278673172},
                    'Lat': {'desc': 'Latitude in WGS.84 in twos complement. Range -90 < latitude < 90 deg.',
                            'val': 44.73441302776337}},
                                       'I060': {'CH': {'desc': 'Change in Mode 3/A', 'meaning': 'No Change', 'val': 0},
                                                'spare': {'desc': 'Spare bits set to 0', 'val': 0, 'const': 5e-324},
                                                'Mode3A': {'desc': 'Mode-3/A reply in octal representation',
                                                           'val': '4276'}},
                                       'I290': {'ES': {'desc': 'ADS-B Extended Squitter age', 'val': 0},
                                                'TRK': {'desc': 'Track age', 'val': 0},
                                                'MDS': {'desc': 'Age of the last Mode S detection used to update the track',
                                                        'val': 63.75}, 'PSR': {
                                               'desc': 'Age of the last primary detection used to update the track',
                                               'val': 7.25},
                                                'FX': {'desc': 'Extension indicator', 'meaning': 'no extension', 'val': 0},
                                                'ADS': {'desc': 'ADS-C age', 'val': 0}, 'SSR': {
                                               'desc': 'Age of the last secondary detection used to update the track',
                                               'val': 0.0}, 'VDL': {'desc': 'ADS-B VDL Mode 4 age', 'val': 0}}}))

            self.assertDictEqual(deep_sort(packet[1]), deep_sort({
                                          'I340': {'HEI': {'desc': 'Measured 3-D Height', 'val': 0}, 'Mode3A': {
                                              'desc': 'Mode 3/A reply under the form of 4 digits in octal representation',
                                              'val': '2535'}, 'ModeC': {'desc': 'Last Measured Mode C Code', 'min': -12.0,
                                                                        'val': 350.0, 'max': 1270.0},
                                                   'CG': {'desc': '', 'meaning': 'Default', 'val': 0},
                                                   'TYP': {'desc': 'Report Type', 'meaning': 'Single ModeS Roll-Call',
                                                           'val': 5},
                                                   'spare': {'desc': 'Spare bits set to zero', 'val': 0, 'const': 1e-323},
                                                   'POS': {'desc': 'Measured Position', 'val': 1},
                                                   'SIM': {'desc': '', 'meaning': 'Actual target report', 'val': 0},
                                                   'MDA': {'desc': 'Last Measured Mode 3/A code', 'val': 1},
                                                   'SAC': {'desc': 'System Area Code', 'val': 25},
                                                   'RHO': {'desc': 'Measured distance', 'val': 93.1953125, 'max': 256.0},
                                                   'L': {'desc': '',
                                                         'meaning': 'MODE 3/A code as derived from the reply of the transponder',
                                                         'val': 0}, 'MDC': {'desc': 'Last Measured Mode C code', 'val': 1},
                                                   'SID': {'desc': 'Sensor Identification', 'val': 1},
                                                   'G': {'desc': '', 'meaning': 'Default', 'val': 0},
                                                   'SIC': {'desc': 'System Identification Code', 'val': 13},
                                                   'TST': {'desc': '', 'meaning': 'Real target report', 'val': 0},
                                                   'V': {'desc': '', 'meaning': 'Code validated', 'val': 0},
                                                   'CV': {'desc': '', 'meaning': 'Code validated', 'val': 0},
                                                   'THETA': {'desc': 'Measured azimuth', 'val': 271.4666748046875},
                                                   'RAB': {'desc': '', 'meaning': 'Report from aircraft transponder',
                                                           'val': 0},
                                                   'FX': {'desc': 'Extension indicator', 'meaning': 'no extension',
                                                          'val': 0}}, 'I100': {'Y': {'desc': 'Y', 'val': -36106.5},
                                                                               'X': {'desc': 'X', 'val': -72564.5}},
                                          'I010': {'SIC': {'desc': 'System Identification Code', 'val': 100},
                                                   'SAC': {'desc': 'System Area Code', 'val': 25}},
                                          'I380': {'ID': {'desc': 'Target Identification', 'val': 1},
                                                   'TID': {'desc': 'Trajectory Intent Data', 'val': 0},
                                                   'TAS': {'desc': 'True Airspeed', 'val': 0},
                                                   'STAT': {'desc': 'Flight Status',
                                                            'meaning': 'No alert, no SPI, aircraft airborne', 'val': 0},
                                                   'TIS': {'desc': 'Trajectory Intent Status', 'val': 0},
                                                   'spare': {'desc': 'Spare bits set to zero', 'val': 0, 'const': 1e-323},
                                                   'COM': {'desc': 'Communications capability of the transponder',
                                                           'meaning': 'Comm. A and Comm. B capability', 'val': 1},
                                                   'ACID': {'desc': 'Target Identification', 'val': 'SXD4723 '},
                                                   'ADR': {'desc': 'Target Address', 'val': '3C0A55'},
                                                   'FSS': {'desc': 'Final State SelectedAltitude', 'val': 0},
                                                   'AIC': {'desc': 'Aircraft identification capability', 'meaning': 'Yes',
                                                           'val': 1}, 'B1B': {'desc': 'BDS 1,0 bits 37/40', 'val': 6},
                                                   'GVR': {'desc': 'Geometric Vertical Rate', 'val': 0},
                                                   'SAL': {'desc': 'Selected Altitude', 'val': 0},
                                                   'BVR': {'desc': 'Barometric Vertical Rate', 'val': 0},
                                                   'ACS': {'desc': 'ACAS Resolution Advisory Report', 'val': 0},
                                                   'IAS': {'desc': 'Indicated Airspeed/Mach Number', 'val': 0},
                                                   'MHG': {'desc': 'Magnetic Heading', 'val': 0},
                                                   'ARC': {'desc': 'Altitude reporting capability',
                                                           'meaning': '25 ft resolution', 'val': 1},
                                                   'B1A': {'desc': 'BDS 1,0 bit 16', 'val': 1},
                                                   'SSC': {'desc': 'Specific service capability', 'meaning': 'Yes',
                                                           'val': 1}, 'SAB': {'desc': 'Status reported by ADS-B', 'val': 0},
                                                   'FX': {'desc': 'Extension indicator', 'meaning': 'no extension',
                                                          'val': 0}},
                                          'I015': {'SID': {'desc': 'Service Identification', 'val': 4}}, 'I136': {
                                          'MFL': {'desc': 'Measured Flight Level', 'min': -1500.0, 'val': 35000.0,
                                                  'max': 150000.0}},
                                          'I295': {'MDA': {'desc': '', 'val': 0.0}, 'MD5': {'desc': '', 'val': 0},
                                                   'FX': {'desc': 'Extension indicator', 'meaning': 'no extension',
                                                          'val': 0}, 'MD2': {'desc': '', 'val': 0},
                                                   'MD4': {'desc': '', 'val': 0}, 'MFL': {'desc': '', 'val': 0.0},
                                                   'MHG': {'desc': '', 'val': 0}, 'MD1': {'desc': '', 'val': 0}},
                                          'I210': {'Ay': {'desc': 'Ay', 'val': 0.0}, 'Ax': {'desc': 'Ax', 'val': 0.0}},
                                          'I220': {'RoC': {'desc': 'Rate of Climb/Descent', 'val': 0.0}}, 'I200': {
                                          'ADF': {'desc': 'Altitude Discrepancy Flag', 'meaning': 'No altitude discrepancy',
                                                  'val': 0},
                                          'TRANSA': {'desc': 'Transversal Acceleration', 'meaning': 'Constant Course',
                                                     'val': 0},
                                          'LONGA': {'desc': 'Longitudinal Acceleration', 'meaning': 'Constant Groundspeed',
                                                    'val': 0},
                                          'VERTA': {'desc': 'Vertical Rate', 'meaning': 'Level', 'val': 0},
                                          'spare': {'desc': 'Spare bit set to zero', 'val': 0, 'const': 5e-324}},
                                          'I070': {'ToT': {'desc': 'Time Of Track Information', 'val': 30911.828125}},
                                          'I130': {
                                              'Alt': {'desc': 'Altitude', 'min': -1500.0, 'val': 35312.5, 'max': 150000.0}},
                                          'I040': {'TrkN': {'desc': 'Track number', 'val': 7977}}, 'category': 62,
                                          'I185': {'Vx': {'desc': 'Vx', 'min': -8192.0, 'val': 141.5, 'max': 8191.75},
                                                   'Vy': {'desc': 'Vy', 'min': -8192.0, 'val': -170.75, 'max': 8191.75}},
                                          'I080': {'FPC': {'desc': '', 'meaning': 'Flight plan correlated', 'val': 1},
                                                   'KOS': {'desc': '', 'meaning': 'Background service used', 'val': 1},
                                                   'MDS': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                   'MD5': {'desc': '', 'meaning': 'No Mode 5 interrogation', 'val': 0},
                                                   'ADS': {'desc': '',
                                                           'meaning': 'Age of the last received ADS-B track update is higher than system dependent threshold',
                                                           'val': 1},
                                                   'MI': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                   'SRC': {'desc': 'Source of calculated track altitude for I062/130',
                                                           'meaning': 'triangulation', 'val': 3},
                                                   'SSR': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                   'SUC': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                   'STP': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                   'AMA': {'desc': '',
                                                           'meaning': 'track not resulting from amalgamation process',
                                                           'val': 0},
                                                   'SIM': {'desc': '', 'meaning': 'Actual track', 'val': 0},
                                                   'AAC': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                   'PSR': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                   'MON': {'desc': '', 'meaning': 'Multisensor track', 'val': 0},
                                                   'CNF': {'desc': '', 'meaning': 'Confirmed track', 'val': 0},
                                                   'CST': {'desc': '', 'meaning': 'Default value', 'val': 0},
                                                   'MD4': {'desc': '', 'meaning': 'No Mode 4 interrogation', 'val': 0},
                                                   'SPI': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                   'ME': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                   'TSB': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                   'MRH': {'desc': 'Most Reliable Height',
                                                           'meaning': 'Barometric altitude (Mode C) more reliable',
                                                           'val': 0},
                                                   'TSE': {'desc': '', 'meaning': 'default value', 'val': 0},
                                                   'FX': {'desc': '', 'meaning': 'End of data item', 'val': 0},
                                                   'AFF': {'desc': '', 'meaning': 'default value', 'val': 0}}, 'I135': {
                                          'CTBA': {'desc': 'Calculated Track Barometric Alt', 'min': -1500.0,
                                                   'val': 35000.0, 'max': 150000.0},
                                          'QNH': {'desc': 'QNH', 'meaning': 'No QNH correction applied', 'val': 0}},
                                          'I105': {
                                              'Lon': {
                                                  'desc': 'Longitude in WGS.84 in twos complement. Range -180 < longitude < 180 deg.',
                                                  'val': 15.13318419456482},
                                              'Lat': {
                                                  'desc': 'Latitude in WGS.84 in twos complement. Range -90 < latitude < 90 deg.',
                                                  'val': 45.40080785751343}},
                                          'I060': {'CH': {'desc': 'Change in Mode 3/A', 'meaning': 'No Change', 'val': 0},
                                                   'spare': {'desc': 'Spare bits set to 0', 'val': 0, 'const': 5e-324},
                                                   'Mode3A': {'desc': 'Mode-3/A reply in octal representation',
                                                              'val': '2535'}},
                                          'I290': {'ES': {'desc': 'ADS-B Extended Squitter age', 'val': 0},
                                                   'TRK': {'desc': 'Track age', 'val': 0}, 'MDS': {
                                                  'desc': 'Age of the last Mode S detection used to update the track',
                                                  'val': 0.0}, 'PSR': {
                                                  'desc': 'Age of the last primary detection used to update the track',
                                                  'val': 1.0},
                                                   'FX': {'desc': 'Extension indicator', 'meaning': 'no extension',
                                                          'val': 0}, 'ADS': {'desc': 'ADS-C age', 'val': 0}, 'SSR': {
                                                  'desc': 'Age of the last secondary detection used to update the track',
                                                  'val': 0.0}, 'VDL': {'desc': 'ADS-B VDL Mode 4 age', 'val': 0}},
                                          'I390': {'TYPE': {'desc': 'Type of Aircraft', 'val': 'B738'},
                                                   'LTR': {'desc': 'Letter', 'val': ' '},
                                                   'TYP': {'desc': '', 'meaning': 'Unit 1 internal flight number',
                                                           'val': 1},
                                                   'STA': {'desc': 'Standard Instrument Arrival', 'val': 0},
                                                   'STS': {'desc': 'Stand Status', 'val': 0},
                                                   'TAG': {'desc': 'FPPS Identification Tag', 'val': 1},
                                                   'PEM': {'desc': 'Pre-emergency Mode 3/A code', 'val': 0},
                                                   'RDS': {'desc': 'Runway Designation', 'val': 1},
                                                   'spare': {'desc': 'spare bit set to zero', 'val': 0, 'const': 5e-324},
                                                   'CTL': {'desc': 'Current Control Position', 'val': 0},
                                                   'DES': {'desc': 'Destination Airport', 'val': 'HELX'},
                                                   'HPR': {'desc': '', 'meaning': 'Normal Priority Flight', 'val': 0},
                                                   'NBR': {'desc': '', 'val': 29233709},
                                                   'FCT': {'desc': 'Flight Category', 'val': 1},
                                                   'SAC': {'desc': 'System Area Code', 'val': 25},
                                                   'NU1': {'desc': 'First number', 'val': ' '},
                                                   'DST': {'desc': 'Destination Airport', 'val': 1},
                                                   'IFI': {'desc': 'IFPS_FLIGHT_ID', 'val': 1},
                                                   'SIC': {'desc': 'System Identification Code', 'val': 100},
                                                   'STD': {'desc': 'Standard Instrument Departure', 'val': 0},
                                                   'PEC': {'desc': 'Pre-emergency Callsign', 'val': 0},
                                                   'TOD': {'desc': 'Time of Departure', 'val': 0},
                                                   'CS': {'desc': 'Callsign', 'val': 'SXD4723'},
                                                   'GATOAT': {'desc': '', 'meaning': 'General Air Traffic', 'val': 1},
                                                   'AST': {'desc': 'Aircraft Stand', 'val': 0},
                                                   'FR1FR2': {'desc': '', 'meaning': 'Instrument Flight Rules', 'val': 0},
                                                   'NU2': {'desc': 'Second number', 'val': ' '},
                                                   'WTC': {'desc': 'Wake Turbulence Category', 'val': 'M'},
                                                   'CFL': {'desc': 'Current Cleared Flight Level', 'val': 350.0},
                                                   'CSN': {'desc': 'Callsign', 'val': 1},
                                                   'DEP': {'desc': 'Departure Airport', 'val': 'EDDL'},
                                                   'RVSM': {'desc': '', 'meaning': 'Approved', 'val': 1},
                                                   'FX': {'desc': 'Extension indicator', 'meaning': 'no extension',
                                                          'val': 0}, 'TAC': {'desc': 'Type of Aircraft', 'val': 1}}}))

            self.assertDictEqual(deep_sort(packet[2]), deep_sort(
                                      {'I000': {'Typ': {'desc': 'Message Type', 'meaning': 'End of Batch', 'val': 2}},
                                       'category': 65, 'I015': {'SID': {'desc': 'Service Identification', 'val': 4}},
                                       'I020': {'BTN': {'desc': 'Batch Number', 'val': 24}},
                                       'I030': {'ToD': {'desc': 'Time Of Message', 'val': 30913.0546875}},
                                       'I010': {'SIC': {'desc': 'Source Identification Code', 'val': 100},
                                                'SAC': {'desc': 'Source Area Code', 'val': 25}}}))


def main():
    unittest.main()


if __name__ == '__main__':
    main()

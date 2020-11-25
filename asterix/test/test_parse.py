import asterix
import unittest


class AsterixParseTest(unittest.TestCase):
    def test_ParseCAT048(self):
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data)
            self.maxDiff = None
            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(packet), 1)
            self.assertTrue('I220' in packet[0])
            self.assertEqual(packet[0]['category'], 48)
            self.assertEqual(packet[0]['len'], 45)
            self.assertEqual(packet[0]['crc'], 'AB659C3E')
            self.assertTrue('ts' in packet[0])
            self.assertEqual(packet[0]['I220']['ACAddr']['val'], '3C660C')
            self.assertEqual(packet[0]['I220']['ACAddr']['desc'], 'AircraftAddress')
            self.assertEqual(packet[0]['I010'], {'SAC': {'desc': 'System Area Code', 'val': 25},
                                                 'SIC': {'desc': 'System Identification Code', 'val': 201}})
            self.assertEqual(packet[0]['I170'], {'GHO': {'desc': 'GHO', 'val': 0, 'meaning': 'True target track'},
                                                 'TCC': {'desc': 'TCC', 'val': 0, 'meaning': 'Radar plane'},
                                                 'RAD': {'desc': 'RAD', 'val': 2, 'meaning': 'SSR/ModeS Track'},
                                                 'spare': {'desc': 'spare bits set to 0', 'const': 0, 'val': 0},
                                                 'TRE': {'desc': 'TRE', 'val': 0, 'meaning': 'Track still alive'},
                                                 'CDM': {'desc': 'CDM', 'val': 0, 'meaning': 'Maintaining'},
                                                 'CNF': {'desc': 'CNF', 'val': 0, 'meaning': 'Confirmed Track'},
                                                 'SUP': {'desc': 'SUP', 'val': 0,
                                                         'meaning': 'Track from cluster network - NO'},
                                                 'FX': {'desc': 'FX', 'val': 0, 'meaning': 'End of Data Item'},
                                                 'DOU': {'desc': 'DOU', 'val': 0, 'meaning': 'Normal confidence'},
                                                 'MAH': {'desc': 'MAH', 'val': 0,
                                                         'meaning': 'No horizontal man. sensed'}})
            self.assertEqual(packet[0]['I200'], {'CGS': {'desc': 'Calculated groundspeed', 'val': 434.3994140625},
                                                 'CHdg': {'desc': 'Calculated heading', 'val': 124.002685546875}})
            self.assertEqual(packet[0]['I220'], {'ACAddr': {'desc': 'AircraftAddress', 'val': '3C660C'}})

            self.assertEqual(packet[0]['I250'][0], {'TARGET_ALT_STATUS': {'desc': 'Status of Target ALT source bits',
                                                                          'meaning': 'No source information provided',
                                                                          'val': 0},
                                                    'res1': {'desc': 'Reserved 1', 'val': 0},
                                                    'res2': {'desc': 'Reserved 2', 'val': 0},
                                                    'FMS_ALT': {'desc': 'FMS Selected Altitude', 'val': 0.0},
                                                    'APP': {'desc': 'APPROACH Mode', 'meaning': 'Not active', 'val': 0},
                                                    'ALT_HOLD': {'desc': 'ALT HOLD Mode', 'meaning': 'Not active',
                                                                 'val': 0},
                                                    'TARGET_ALT_SOURCE': {'desc': 'Target ALT source',
                                                                          'meaning': 'Unknown', 'val': 0},
                                                    'BDS': {'desc': 'BDS register', 'val': '40'},
                                                    'FMS_ALT_STATUS': {'desc': 'FMS Altitude Status', 'val': 0},
                                                    'BP_STATUS': {'desc': 'Barometric Pressure Status', 'val': 1},
                                                    'BP': {'desc': 'Barometric Pressure', 'val': 227.0},
                                                    'MODE_STATUS': {'desc': 'Status of MCP/FCU Mode Bits', 'val': 0},
                                                    'VNAV': {'desc': 'VNAV Mode', 'meaning': 'Not active', 'val': 0},
                                                    'MCP_ALT_STATUS': {'desc': 'MCP Altitude Status', 'val': 1},
                                                    'MCP_ALT': {'desc': 'MCP/FCU Selected Altitude', 'val': 33008.0}})

            self.assertEqual(packet[0]['I040'], {'THETA': {'desc': '', 'val': 340.13671875},
                                                 'RHO': {'desc': '', 'max': 256.0, 'val': 197.68359375}})
            self.assertEqual(packet[0]['I240'],
                             {'TId': {'desc': 'Characters 1-8 (coded on 6 bits each) defining target identification',
                                      'val': 'DLH65A  '}})
            self.assertEqual(packet[0]['I140'], {'ToD': {'desc': 'Time Of Day', 'val': 27354.6015625}})
            self.assertEqual(packet[0]['I070'], {'Mode3A': {'desc': 'Mode-3/A reply code', 'val': '1000'},
                                                 'V': {'desc': '', 'val': 0, 'meaning': 'Code validated'},
                                                 'L': {'desc': '', 'val': 0,
                                                       'meaning': 'Mode-3/A code as derived from the reply of the transponder'},
                                                 'spare': {'desc': 'spare bit set to 0', 'const': 0, 'val': 0},
                                                 'G': {'desc': '', 'val': 0, 'meaning': 'Default'}})
            self.assertEqual(packet[0]['I161'], {'Tn': {'desc': 'Track Number', 'val': 3563}})
            self.assertEqual(packet[0]['I020'], {'SIM': {'desc': 'SIM', 'val': 0, 'meaning': 'Actual target report'},
                                                 'TYP': {'desc': 'TYP', 'val': 5, 'meaning': 'Single ModeS Roll-Call'},
                                                 'RAB': {'desc': 'RAB', 'val': 0,
                                                         'meaning': 'Report from aircraft transponder'},
                                                 'RDP': {'desc': 'RDP', 'val': 0,
                                                         'meaning': 'Report from RDP Chain 1'},
                                                 'FX': {'desc': 'FX', 'val': 0, 'meaning': 'End of Data Item'},
                                                 'SPI': {'desc': 'SPI', 'val': 0, 'meaning': 'Absence of SPI'}})
            self.assertEqual(packet[0]['I090'], {'V': {'desc': '', 'val': 0, 'meaning': 'Code validated'},
                                                 'FL': {'desc': 'FlightLevel', 'val': 330.0},
                                                 'G': {'desc': '', 'val': 0, 'meaning': 'Default'}})
            self.assertEqual(packet[0]['I230'], {'COM': {'desc': 'COM', 'val': 1},
                                                 'BDS37': {'desc': 'BDS 1,0 bits 37/40', 'val': 5},
                                                 'ModeSSSC': {'desc': 'ModeS Specific Service Capability', 'val': 1,
                                                              'meaning': 'Yes'}, 'STAT': {'desc': 'STAT', 'val': 0},
                                                 'AIC': {'desc': 'Aircraft identification capability', 'val': 1,
                                                         'meaning': 'Yes'},
                                                 'BDS16': {'desc': 'BDS 1,0 bit 16', 'val': 1},
                                                 'spare': {'desc': 'spare bit set to 0', 'const': 0, 'val': 0},
                                                 'ARC': {'desc': 'Altitude reporting capability', 'val': 1,
                                                         'meaning': '25ft resolution'},
                                                 'SI': {'desc': 'SI/II Transponder Capability', 'val': 0,
                                                        'meaning': 'SI-Code Capable'}})

    def test_ParseCAT048_nodescription(self):
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data, verbose=False)
            self.maxDiff = None
            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(packet), 1)
            self.assertTrue('I220' in packet[0])
            self.assertEqual(packet[0]['category'], 48)
            self.assertEqual(packet[0]['len'], 45)
            self.assertEqual(packet[0]['crc'], 'AB659C3E')
            self.assertTrue('ts' in packet[0])
            self.assertEqual(packet[0]['I220']['ACAddr']['val'], '3C660C')
            self.assertEqual(packet[0]['I010'], {'SAC': {'val': 25},
                                                 'SIC': {'val': 201}})
            self.assertEqual(packet[0]['I170'], {'GHO': {'val': 0},
                                                 'TCC': {'val': 0},
                                                 'RAD': {'val': 2},
                                                 'spare': {'val': 0},
                                                 'TRE': {'val': 0},
                                                 'CDM': {'val': 0},
                                                 'CNF': {'val': 0},
                                                 'SUP': {'val': 0},
                                                 'FX': {'val': 0},
                                                 'DOU': {'val': 0},
                                                 'MAH': {'val': 0}})
            self.assertEqual(packet[0]['I200'], {'CGS': {'val': 434.3994140625},
                                                 'CHdg': {'val': 124.002685546875}})
            self.assertEqual(packet[0]['I220'], {'ACAddr': {'val': '3C660C'}})
            self.assertEqual(packet[0]['I250'][0], {'TARGET_ALT_STATUS': {'val': 0},
                                                    'res1': {'val': 0},
                                                    'res2': {'val': 0},
                                                    'FMS_ALT': {'val': 0.0},
                                                    'APP': {'val': 0},
                                                    'ALT_HOLD': {'val': 0},
                                                    'TARGET_ALT_SOURCE': {'val': 0},
                                                    'BDS': {'val': '40'},
                                                    'FMS_ALT_STATUS': {'val': 0},
                                                    'BP_STATUS': {'val': 1},
                                                    'BP': {'val': 227.0},
                                                    'MODE_STATUS': {'val': 0},
                                                    'VNAV': {'val': 0},
                                                    'MCP_ALT_STATUS': {'val': 1},
                                                    'MCP_ALT': {'val': 33008.0}})
            self.assertEqual(packet[0]['I040'], {'THETA': {'val': 340.13671875},
                                                 'RHO': {'val': 197.68359375}})
            self.assertEqual(packet[0]['I240'], {'TId': {'val': 'DLH65A  '}})
            self.assertEqual(packet[0]['I140'], {'ToD': {'val': 27354.6015625}})
            self.assertEqual(packet[0]['I070'], {'Mode3A': {'val': '1000'},
                                                 'V': {'val': 0},
                                                 'L': {'val': 0},
                                                 'spare': {'val': 0},
                                                 'G': {'val': 0}})
            self.assertEqual(packet[0]['I161'], {'Tn': {'val': 3563}})
            self.assertEqual(packet[0]['I020'], {'SIM': {'val': 0},
                                                 'TYP': {'val': 5},
                                                 'RAB': {'val': 0},
                                                 'RDP': {'val': 0},
                                                 'FX': {'val': 0},
                                                 'SPI': {'val': 0}})
            self.assertEqual(packet[0]['I090'], {'V': {'val': 0},
                                                 'FL': {'val': 330.0},
                                                 'G': {'val': 0}})
            self.assertEqual(packet[0]['I230'], {'COM': {'val': 1},
                                                 'BDS37': {'val': 5},
                                                 'ModeSSSC': {'val': 1},
                                                 'STAT': {'val': 0},
                                                 'AIC': {'val': 1},
                                                 'BDS16': {'val': 1},
                                                 'spare': {'val': 0},
                                                 'ARC': {'val': 1},
                                                 'SI': {'val': 0}})

    def test_ParseCAT062CAT065(self):
        self.maxDiff = None
        sample_filename = asterix.get_sample_file('cat062cat065.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            packet = asterix.parse(data)
            self.assertIsNotNone(packet)
            self.assertIsNotNone(packet[0])
            self.assertIs(len(data), 195)
            self.assertIs(len(packet), 3)
            self.assertIs(packet[0]['category'], 62)
            self.assertIs(packet[0]['len'], 66)
            self.assertEqual(packet[0]['crc'], '943598B9')
            self.assertIs(packet[1]['category'], 62)
            self.assertIs(packet[1]['len'], 114)
            self.assertEqual(packet[1]['crc'], '0F55FD0E')
            self.assertIs(packet[2]['category'], 65)
            self.assertIs(packet[2]['len'], 9)
            self.assertEqual(packet[2]['crc'], 'ED9D3EB1')
            self.assertEqual(packet[0]['I220'], {'RoC': {'val': -443.75, 'desc': 'Rate of Climb/Descent'}})
            self.assertEqual(packet[0]['I015'], {'SID': {'val': 4, 'desc': 'Service Identification'}})

            self.assertEqual(packet[0]['I290']['MDS'], {
                'MDS': {'val': 63.75, 'desc': 'Age of the last Mode S detection used to update the track'}})
            self.assertEqual(packet[0]['I290']['PSR'], {
                'PSR': {'val': 7.25, 'desc': 'Age of the last primary detection used to update the track'}})
            self.assertEqual(packet[0]['I290']['SSR'], {
                'SSR': {'val': 0.0, 'desc': 'Age of the last secondary detection used to update the track'}})

            self.assertEqual(packet[0]['I135'], {
                'QNH': {'meaning': 'No QNH correction applied', 'val': 0, 'desc': 'QNH'},
                'CTBA': {'max': 150000.0, 'min': -1500.0, 'val': 15700.0,
                         'desc': 'Calculated Track Barometric Alt'}})
            self.assertEqual(packet[0]['I136'], {
                'MFL': {'max': 150000.0, 'min': -1500.0, 'val': 15700.0, 'desc': 'Measured Flight Level'}})
            self.assertEqual(packet[0]['I185'], {'Vx': {'max': 8191.75, 'min': -8192.0, 'val': -51.25, 'desc': 'Vx'},
                                                 'Vy': {'max': 8191.75, 'min': -8192.0, 'val': 170.0, 'desc': 'Vy'}})
            self.assertEqual(packet[0]['I080'], {'STP': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                 'MD5': {'meaning': 'No Mode 5 interrogation', 'val': 0, 'desc': ''},
                                                 'FPC': {'meaning': 'Not flight-plan correlated', 'val': 0, 'desc': ''},
                                                 'AMA': {'meaning': 'track not resulting from amalgamation process',
                                                         'val': 0, 'desc': ''},
                                                 'CNF': {'meaning': 'Confirmed track', 'val': 0, 'desc': ''},
                                                 'TSE': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                 'ME': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                 'FX': {'meaning': 'End of data item', 'val': 0, 'desc': ''},
                                                 'CST': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'PSR': {'meaning': 'Default value', 'val': 0, 'desc': ''}, 'MDS': {
                    'meaning': 'Age of the last received Mode S track update is higher than system dependent threshold',
                    'val': 1, 'desc': ''},
                                                 'MI': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                 'SRC': {'meaning': 'height from coverage', 'val': 4,
                                                         'desc': 'Source of calculated track altitude for I062/130'},
                                                 'SIM': {'meaning': 'Actual track', 'val': 0, 'desc': ''},
                                                 'KOS': {'meaning': 'Background service used', 'val': 1, 'desc': ''},
                                                 'AFF': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                 'MRH': {'meaning': 'Barometric altitude (Mode C) more reliable',
                                                         'val': 0, 'desc': 'Most Reliable Height'},
                                                 'MON': {'meaning': 'Multisensor track', 'val': 0, 'desc': ''},
                                                 'TSB': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                 'SUC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'MD4': {'meaning': 'No Mode 4 interrogation', 'val': 0, 'desc': ''},
                                                 'SPI': {'meaning': 'default value', 'val': 0, 'desc': ''}, 'ADS': {
                    'meaning': 'Age of the last received ADS-B track update is higher than system dependent threshold',
                    'val': 1, 'desc': ''},
                                                 'AAC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                 'SSR': {'meaning': 'Default value', 'val': 0, 'desc': ''}})
            self.assertEqual(packet[0]['I070'], {'ToT': {'val': 30911.6640625, 'desc': 'Time Of Track Information'}})
            self.assertEqual(packet[0]['I100'], {'Y': {'val': -106114.0, 'desc': 'Y'},
                                                 'X': {'val': -239083.0, 'desc': 'X'}})
            self.assertEqual(packet[0]['I200'], {'VERTA': {'meaning': 'Descent', 'val': 2, 'desc': 'Vertical Rate'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bit set to zero'},
                                                 'LONGA': {'meaning': 'Decreasing Groundspeed', 'val': 2,
                                                           'desc': 'Longitudinal Acceleration'},
                                                 'TRANSA': {'meaning': 'Constant Course', 'val': 0,
                                                            'desc': 'Transversal Acceleration'},
                                                 'ADF': {'meaning': 'No altitude discrepancy', 'val': 0,
                                                         'desc': 'Altitude Discrepancy Flag'}})
            self.assertEqual(packet[0]['I130'], {
                'Alt': {'max': 150000.0, 'min': -1500.0, 'val': 43300.0, 'desc': 'Altitude'}})
            self.assertEqual(packet[0]['I060'], {'CH': {'meaning': 'No Change', 'val': 0, 'desc': 'Change in Mode 3/A'},
                                                 'G': {'desc': 'Change in Mode 3/A', 'meaning': 'Default', 'val': 0},
                                                 'V': {'desc': 'Change in Mode 3/A', 'meaning': 'Code validated',
                                                       'val': 0},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bits set to 0'},
                                                 'Mode3A': {'val': '4276',
                                                            'desc': 'Mode-3/A reply in octal representation'}})

            self.assertEqual(packet[0]['I295']['MDA'], {'MDA': {'val': 0, 'desc': ''}})
            self.assertEqual(packet[0]['I295']['MFL'], {'MFL': {'val': 0.0, 'desc': ''}})

            self.assertEqual(packet[0]['I010'], {'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'}})

            self.assertEqual(packet[0]['I340']['TYP'], {
                'TYP': {'val': 2, 'meaning': 'Single SSR detection', 'desc': 'Report Type'},
                'TST': {'val': 0, 'meaning': 'Real target report', 'desc': ''},
                'spare': {'val': 0, 'desc': 'Spare bits set to zero', 'const': 0},
                'RAB': {'val': 0, 'meaning': 'Report from aircraft transponder', 'desc': ''},
                'SIM': {'val': 0, 'meaning': 'Actual target report', 'desc': ''}})

            self.assertEqual(packet[0]['I340']['SID'], {
                'SAC': {'val': 25, 'desc': 'System Area Code'},
                'SIC': {'val': 13, 'desc': 'System Identification Code'}})

            self.assertEqual(packet[0]['I340']['MDC'], {
                'CG': {'val': 0, 'meaning': 'Default', 'desc': ''},
                'CV': {'val': 0, 'meaning': 'Code validated', 'desc': ''},
                'ModeC': {'max': 1270.0, 'val': 157.0, 'min': -12.0, 'desc': 'Last Measured Mode C Code'}})

            self.assertEqual(packet[0]['I340']['MDA'], {
                'L': {'val': 0, 'meaning': 'MODE 3/A code as derived from the reply of the transponder',
                      'desc': ''},
                'V': {'val': 0, 'meaning': 'Code validated', 'desc': ''},
                'Mode3A': {'val': '4276',
                           'desc': 'Mode 3/A reply under the form of 4 digits in octal representation'},
                'G': {'val': 0, 'meaning': 'Default', 'desc': ''},
                'spare': {'val': 0, 'desc': 'Spare bit set to zero', 'const': 0}})

            self.assertEqual(packet[0]['I340']['POS'], {
                'RHO': {'max': 256.0, 'val': 186.6875, 'desc': 'Measured distance'},
                'THETA': {'val': 259.453125, 'desc': 'Measured azimuth'}})

            self.assertEqual(packet[0]['I105'], {
                'Lat': {'val': 44.73441302776337,
                        'desc': 'Latitude in WGS.84 in two\'s complement. Range -90 < latitude < 90 deg.'},
                                                                                                       'Lon': {
                'val': 13.0415278673172,
                'desc': 'Longitude in WGS.84 in two\'s complement. Range -180 < longitude < 180 deg.'}})
            self.assertEqual(packet[0]['I040'], {'TrkN': {'val': 4980, 'desc': 'Track number'}})
            self.assertEqual(packet[0]['I210'],
                                 {'Ax': {'val': 0.0, 'desc': 'Ax'}, 'Ay': {'val': 0.0, 'desc': 'Ay'}})
            self.assertEqual(packet[1]['I220'], {'RoC': {'val': 0.0, 'desc': 'Rate of Climb/Descent'}})
            self.assertEqual(packet[1]['I015'], {'SID': {'val': 4, 'desc': 'Service Identification'}})

            self.assertEqual(packet[1]['I290']['MDS'], {
                'MDS': {'val': 0.0, 'desc': 'Age of the last Mode S detection used to update the track'}})
            self.assertEqual(packet[1]['I290']['SSR'], {
                'SSR': {'val': 0.0, 'desc': 'Age of the last secondary detection used to update the track'}})

            self.assertEqual(packet[1]['I135'], {
                'QNH': {'meaning': 'No QNH correction applied', 'val': 0, 'desc': 'QNH'},
                'CTBA': {'max': 150000.0, 'min': -1500.0, 'val': 35000.0,
                         'desc': 'Calculated Track Barometric Alt'}})
            self.assertEqual(packet[1]['I136'], {
                 'MFL': {'max': 150000.0, 'min': -1500.0, 'val': 35000.0,
                        'desc': 'Measured Flight Level'}})
            self.assertEqual(packet[1]['I185'], {'Vx': {'max': 8191.75, 'min': -8192.0, 'val': 141.5, 'desc': 'Vx'},
                                                 'Vy': {'max': 8191.75, 'min': -8192.0, 'val': -170.75,
                                                        'desc': 'Vy'}})
            self.assertEqual(packet[1]['I080'], {'STP': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                     'MD5': {'meaning': 'No Mode 5 interrogation', 'val': 0,
                                                             'desc': ''},
                                                     'FPC': {'meaning': 'Flight plan correlated', 'val': 1,
                                                             'desc': ''},
                                                     'AMA': {
                                                         'meaning': 'track not resulting from amalgamation process',
                                                         'val': 0, 'desc': ''},
                                                     'CNF': {'meaning': 'Confirmed track', 'val': 0, 'desc': ''},
                                                     'TSE': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                     'ME': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                     'FX': {'meaning': 'End of data item', 'val': 0, 'desc': ''},
                                                     'CST': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'PSR': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MDS': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MI': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                     'SRC': {'meaning': 'triangulation', 'val': 3,
                                                             'desc': 'Source of calculated track altitude for I062/130'},
                                                     'SIM': {'meaning': 'Actual track', 'val': 0, 'desc': ''},
                                                     'KOS': {'meaning': 'Background service used', 'val': 1,
                                                             'desc': ''},
                                                     'AFF': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                     'MRH': {
                                                         'meaning': 'Barometric altitude (Mode C) more reliable',
                                                         'val': 0, 'desc': 'Most Reliable Height'},
                                                     'MON': {'meaning': 'Multisensor track', 'val': 0, 'desc': ''},
                                                     'TSB': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                     'SUC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'MD4': {'meaning': 'No Mode 4 interrogation', 'val': 0,
                                                             'desc': ''},
                                                     'SPI': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                     'ADS': {
                                                         'meaning': 'Age of the last received ADS-B track update is higher than system dependent threshold',
                                                         'val': 1, 'desc': ''},
                                                     'AAC': {'meaning': 'Default value', 'val': 0, 'desc': ''},
                                                     'SSR': {'meaning': 'Default value', 'val': 0, 'desc': ''}})
            self.assertEqual(packet[1]['I070'], {'ToT': {'val': 30911.828125, 'desc': 'Time Of Track Information'}})
            self.assertEqual(packet[1]['I100'], {'Y': {'val': -36106.5, 'desc': 'Y'},
                                                     'X': {'val': -72564.5, 'desc': 'X'}})
            self.assertEqual(packet[1]['I200'], {'VERTA': {'meaning': 'Level', 'val': 0, 'desc': 'Vertical Rate'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bit set to zero'},
                                                 'LONGA': {'meaning': 'Constant Groundspeed', 'val': 0,
                                                           'desc': 'Longitudinal Acceleration'},
                                                 'TRANSA': {'meaning': 'Constant Course', 'val': 0,
                                                            'desc': 'Transversal Acceleration'},
                                                 'ADF': {'meaning': 'No altitude discrepancy', 'val': 0,
                                                         'desc': 'Altitude Discrepancy Flag'}})
            self.assertEqual(packet[1]['I130'], {
                    'Alt': {'max': 150000.0, 'min': -1500.0, 'val': 35312.5, 'desc': 'Altitude'}})
            self.assertEqual(packet[1]['I060'],
                             {'CH': {'meaning': 'No Change', 'val': 0, 'desc': 'Change in Mode 3/A'},
                              'G': {'desc': 'Change in Mode 3/A', 'meaning': 'Default', 'val': 0},
                              'V': {'desc': 'Change in Mode 3/A', 'meaning': 'Code validated',
                                    'val': 0},
                              'spare': {'const': 0, 'val': 0, 'desc': 'Spare bits set to 0'},
                              'Mode3A': {'val': '2535',
                                         'desc': 'Mode-3/A reply in octal representation'}})

            self.assertEqual(packet[1]['I295']['MFL'], {'MFL': {'val': 0.0, 'desc': ''}})

            self.assertEqual(packet[1]['I390']['DEP'], {'DEP': {'desc': 'Departure Airport', 'val': 'EDDL'}})
            self.assertEqual(packet[1]['I390']['TAC'], {'TYPE': {'desc': 'Type of Aircraft', 'val': 'B738'}})
            self.assertEqual(packet[1]['I390']['DST'], {'DES': {'desc': 'Destination Airport', 'val': 'HELX'}})

            self.assertEqual(packet[1]['I390']['IFI'], {
                    'spare': {'const': 0, 'desc': 'spare bits set to zero', 'val': 0},
                    'NBR': {'desc': '', 'val': 29233709},
                    'TYP': {'meaning': 'Unit 1 internal flight number', 'desc': '', 'val': 1}})

            self.assertEqual(packet[1]['I390']['RDS'], {'NU1': {'desc': 'First number', 'val': ' '},
                                                        'LTR': {'desc': 'Letter', 'val': ' '},
                                                        'NU2': {'desc': 'Second number', 'val': ''}})

            self.assertEqual(packet[1]['I390']['WTC'],
                                 {'WTC': {'desc': 'Wake Turbulence Category', 'val': 'M'}})

            self.assertEqual(packet[1]['I390']['CSN'], {'CS': {'desc': 'Callsign', 'val': 'SXD4723'}})
            self.assertEqual(packet[1]['I390']['TAG'], {
                    'SIC': {'desc': 'System Identification Code', 'val': 100},
                    'SAC': {'desc': 'System Area Code', 'val': 25}})

            self.assertEqual(packet[1]['I390']['FCT'], {
                'spare': {'const': 0, 'desc': 'spare bit set to zero', 'val': 0},
                'FR1FR2': {'meaning': 'Instrument Flight Rules', 'desc': '', 'val': 0},
                'RVSM': {'meaning': 'Approved', 'desc': '', 'val': 1},
                'GATOAT': {'meaning': 'General Air Traffic', 'desc': '', 'val': 1},
                'HPR': {'meaning': 'Normal Priority Flight', 'desc': '', 'val': 0}})

            self.assertEqual(packet[1]['I390']['CFL'],
                                 {'CFL': {'desc': 'Current Cleared Flight Level', 'val': 350.0}})
            self.assertEqual(packet[1]['I010'], {'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'}})

            self.assertEqual(packet[1]['I340']['TYP'], {
                    'TYP': {'desc': 'Report Type', 'meaning': 'Single ModeS Roll-Call', 'val': 5},
                    'TST': {'desc': '', 'meaning': 'Real target report', 'val': 0},
                    'SIM': {'desc': '', 'meaning': 'Actual target report', 'val': 0},
                    'RAB': {'desc': '', 'meaning': 'Report from aircraft transponder', 'val': 0},
                    'spare': {'desc': 'Spare bits set to zero', 'val': 0, 'const': 0}})
            self.assertEqual(packet[1]['I340']['POS'], {
                'RHO': {'desc': 'Measured distance', 'val': 93.1953125, 'max': 256.0},
                'THETA': {'desc': 'Measured azimuth', 'val': 271.4666748046875}})
            self.assertEqual(packet[1]['I340']['MDA'], {
                    'G': {'desc': '', 'meaning': 'Default', 'val': 0},
                    'L': {'desc': '', 'meaning': 'MODE 3/A code as derived from the reply of the transponder',
                          'val': 0},
                    'V': {'desc': '', 'meaning': 'Code validated', 'val': 0},
                    'Mode3A': {'desc': 'Mode 3/A reply under the form of 4 digits in octal representation',
                               'val': '2535'},
                    'spare': {'desc': 'Spare bit set to zero', 'val': 0, 'const': 0}})
            self.assertEqual(packet[1]['I340']['MDC'], {
                'ModeC': {'min': -12.0, 'desc': 'Last Measured Mode C Code', 'val': 350.0, 'max': 1270.0},
                'CG': {'desc': '', 'meaning': 'Default', 'val': 0},
                'CV': {'desc': '', 'meaning': 'Code validated', 'val': 0}})
            self.assertEqual(packet[1]['I340']['SID'], {
                    'SIC': {'desc': 'System Identification Code', 'val': 13},
                    'SAC': {'desc': 'System Area Code', 'val': 25}})

            self.assertEqual(packet[1]['I380']['COM'], {
                'COM': {'val': 1, 'meaning': 'Comm. A and Comm. B capability',
                        'desc': 'Communications capability of the transponder'},
                'SSC': {'val': 1, 'meaning': 'Yes', 'desc': 'Specific service capability'},
                'STAT': {'val': 0, 'meaning': 'No alert, no SPI, aircraft airborne', 'desc': 'Flight Status'},
                'spare': {'val': 0, 'const': 0, 'desc': 'Spare bits set to zero'},
                'B1A': {'val': 1, 'desc': 'BDS 1,0 bit 16'},
                'B1B': {'val': 6, 'desc': 'BDS 1,0 bits 37/40'},
                'ARC': {'val': 1, 'meaning': '25 ft resolution', 'desc': 'Altitude reporting capability'},
                'AIC': {'val': 1, 'meaning': 'Yes', 'desc': 'Aircraft identification capability'}})
            self.assertEqual(packet[1]['I380']['ADR'], {'ADR': {'val': '3C0A55', 'desc': 'Target Address'}})
            self.assertEqual(packet[1]['I380']['ID'],
                             {'ACID': {'val': 'SXD4723 ', 'desc': 'Target Identification'}})

            self.assertEqual(packet[1]['I105'], {'Lat': {'val': 45.40080785751343,
                                                             'desc': 'Latitude in WGS.84 in two\'s complement. Range -90 < latitude < 90 deg.'},
                                                                                                         'Lon': {
            'val': 15.13318419456482,
            'desc': 'Longitude in WGS.84 in two\'s complement. Range -180 < longitude < 180 deg.'}})
            self.assertEqual(packet[1]['I040'], {'TrkN': {'val': 7977, 'desc': 'Track number'}})
            self.assertEqual(packet[1]['I210'],
                                 {'Ax': {'val': 0.0, 'desc': 'Ax'}, 'Ay': {'val': 0.0, 'desc': 'Ay'}})
            self.assertEqual(packet[2]['I015'], {'SID': {'val': 4, 'desc': 'Service Identification'}})
            self.assertEqual(packet[2]['I020'], {'BTN': {'val': 24, 'desc': 'Batch Number'}})
            self.assertEqual(packet[2]['I010'], {'SAC': {'val': 25, 'desc': 'Source Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'Source Identification Code'}})
            self.assertEqual(packet[2]['I030'], {'ToD': {'val': 30913.0546875, 'desc': 'Time Of Message'}})
            self.assertEqual(packet[2]['I000'],
                             {'Typ': {'meaning': 'End of Batch', 'val': 2, 'desc': 'Message Type'}})


def main():
    unittest.main()


if __name__ == '__main__':
    main()

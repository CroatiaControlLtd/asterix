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
            self.assertEqual(packet[0]['crc'], 'C150ED0E')
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
            self.assertEqual(packet[0]['I200'], {'CGS': {'desc': 'Calculated groundspeed', 'val': 434.94},
                                                 'CHdg': {'desc': 'Calculated heading', 'val': 124.002685546875}})
            self.assertEqual(packet[0]['I220'], {'ACAddr': {'desc': 'AircraftAddress', 'val': '3C660C'}})
            self.assertEqual(packet[0]['I250'], {'ALT_HOLD': {'val': 0, 'desc': 'ALT HOLD Mode',
                                                               'meaning': 'Not active'},
                                                  'APP': {'val': 0, 'desc': 'APPROACH Mode', 'meaning': 'Not active'},
                                                  'BP_STATUS': {'val': 1, 'desc': 'Barometric Pressure Status'},
                                                  'MODE_STATUS': {'val': 0, 'desc': 'Status of MCP/FCU Mode Bits'},
                                                  'MCP_ALT': {'val': 33008.0, 'desc': 'MCP/FCU Selected Altitude'},
                                                  'VNAV': {'val': 0, 'desc': 'VNAV Mode', 'meaning': 'Not active'},
                                                  'FMS_ALT_STATUS': {'val': 0, 'desc': 'FMS Altitude Status'},
                                                  'FMS_ALT': {'val': 0.0, 'desc': 'FMS Selected Altitude'},
                                                  'res': {'val': 0, 'desc': 'Reserved'},
                                                  'MCP_ALT_STATUS': {'val': 1, 'desc': 'MCP Altitude Status'},
                                                  'TARGET_ALT_STATUS': {'val': 0, 'desc': 'Status of Target ALT source bits',
                                                                        'meaning': 'No source information provided'},
                                                  'BP': {'val': 227.0, 'desc': 'Barometric Pressure'},
                                                  'TARGET_ALT_SOURCE': {'val': 0, 'desc': 'Target ALT source',
                                                                        'meaning': 'Unknown'}})
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

    def test_ParseCAT062CAT065(self):
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
            self.assertEqual(packet[0]['crc'], '9CB473BE')
            self.assertIs(packet[1]['category'], 62)
            self.assertIs(packet[1]['len'], 114)
            self.assertEqual(packet[1]['crc'], '5A6E1F96')
            self.assertIs(packet[2]['category'], 65)
            self.assertIs(packet[2]['len'], 9)
            self.assertEqual(packet[2]['crc'], '8B7DA47A')
            self.assertEqual(packet[0]['I220'], {'RoC': {'val': -443.75, 'desc': 'Rate of Climb/Descent'}})
            self.assertEqual(packet[0]['I015'], {'SID': {'val': 4, 'desc': 'Service Identification'}})
            self.assertEqual(packet[0]['I290'], {
                'MDS': {'val': 63.75, 'desc': 'Age of the last Mode S detection used to update the track'},
                'ES': {'val': 0, 'desc': 'ADS-B Extended Squitter age'},
                'PSR': {'val': 7.25, 'desc': 'Age of the last primary detection used to update the track'},
                'FX': {'meaning': 'no extension', 'val': 0, 'desc': 'Extension indicator'},
                'VDL': {'val': 0, 'desc': 'ADS-B VDL Mode 4 age'}, 'ADS': {'val': 0, 'desc': 'ADS-C age'},
                'TRK': {'val': 0, 'desc': 'Track age'},
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
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bits set to 0'},
                                                 'Mode3A': {'val': '4276',
                                                            'desc': 'Mode-3/A reply in octal representation'}})
            self.assertEqual(packet[0]['I295'], {'MD2': {'val': 0, 'desc': ''}, 'MD5': {'val': 0, 'desc': ''},
                                                 'MFL': {'val': 0.0, 'desc': ''}, 'MDA': {'val': 0.0, 'desc': ''},
                                                 'FX': {'meaning': 'no extension', 'val': 0,
                                                        'desc': 'Extension indicator'}, 'MHG': {'val': 0, 'desc': ''},
                                                 'MD1': {'val': 0, 'desc': ''}, 'MD4': {'val': 0, 'desc': ''}})
            self.assertEqual(packet[0]['I010'], {'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'}})
            self.assertEqual(packet[0]['I340'], {'SID': {'val': 1, 'desc': 'Sensor Identification'},
                                                 'Mode3A': {'val': '4276',
                                                            'desc': 'Mode 3/A reply under the form of 4 digits in octal representation'},
                                                 'HEI': {'val': 0, 'desc': 'Measured 3-D Height'},
                                                 'TST': {'meaning': 'Real target report', 'val': 0, 'desc': ''},
                                                 'G': {'meaning': 'Default', 'val': 0, 'desc': ''},
                                                 'V': {'meaning': 'Code validated', 'val': 0, 'desc': ''},
                                                 'CG': {'meaning': 'Default', 'val': 0, 'desc': ''},
                                                 'FX': {'meaning': 'no extension', 'val': 0,
                                                        'desc': 'Extension indicator'},
                                                 'ModeC': {'max': 1270.0, 'min': -12.0, 'val': 157.0,
                                                           'desc': 'Last Measured Mode C Code'},
                                                 'CV': {'meaning': 'Code validated', 'val': 0, 'desc': ''},
                                                 'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'RHO': {'max': 256.0, 'val': 186.6875, 'desc': 'Measured distance'},
                                                 'SIM': {'meaning': 'Actual target report', 'val': 0, 'desc': ''},
                                                 'MDA': {'val': 1, 'desc': 'Last Measured Mode 3/A code'},
                                                 'SIC': {'val': 13, 'desc': 'System Identification Code'},
                                                 'THETA': {'val': 259.453125, 'desc': 'Measured azimuth'},
                                                 'POS': {'val': 1, 'desc': 'Measured Position'},
                                                 'TYP': {'meaning': 'Single SSR detection', 'val': 2,
                                                         'desc': 'Report Type'},
                                                 'RAB': {'meaning': 'Report from aircraft transponder', 'val': 0,
                                                         'desc': ''},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bits set to zero'},
                                                 'L': {
                                                     'meaning': 'MODE 3/A code as derived from the reply of the transponder',
                                                     'val': 0, 'desc': ''},
                                                 'MDC': {'val': 1, 'desc': 'Last Measured Mode C code'}})
            self.assertEqual(packet[0]['I105'], {
                'Lat': {'val': 44.73441302776337,
                        'desc': 'Latitude in WGS.84 in twos complement. Range -90 < latitude < 90 deg.'},
                'Lon': {'val': 13.0415278673172,
                        'desc': 'Longitude in WGS.84 in twos complement. Range -180 < longitude < 180 deg.'}})
            self.assertEqual(packet[0]['I040'], {'TrkN': {'val': 4980, 'desc': 'Track number'}})
            self.assertEqual(packet[0]['I210'], {'Ax': {'val': 0.0, 'desc': 'Ax'}, 'Ay': {'val': 0.0, 'desc': 'Ay'}})
            self.assertEqual(packet[1]['I220'], {'RoC': {'val': 0.0, 'desc': 'Rate of Climb/Descent'}})
            self.assertEqual(packet[1]['I015'], {'SID': {'val': 4, 'desc': 'Service Identification'}})
            self.assertEqual(packet[1]['I290'], {
                'MDS': {'val': 0.0,
                        'desc': 'Age of the last Mode S detection used to update the track'},
                'ES': {'val': 0, 'desc': 'ADS-B Extended Squitter age'}, 'PSR': {'val': 1.0,
                                                                                 'desc': 'Age of the last primary detection used to update the track'},
                'FX': {'meaning': 'no extension', 'val': 0, 'desc': 'Extension indicator'},
                'VDL': {'val': 0, 'desc': 'ADS-B VDL Mode 4 age'},
                'ADS': {'val': 0, 'desc': 'ADS-C age'},
                'TRK': {'val': 0, 'desc': 'Track age'}, 'SSR': {'val': 0.0,
                                                                'desc': 'Age of the last secondary detection used to update the track'}})
            self.assertEqual(packet[1]['I135'], {
                'QNH': {'meaning': 'No QNH correction applied', 'val': 0, 'desc': 'QNH'},
                'CTBA': {'max': 150000.0, 'min': -1500.0, 'val': 35000.0,
                         'desc': 'Calculated Track Barometric Alt'}})
            self.assertEqual(packet[1]['I136'], {
                'MFL': {'max': 150000.0, 'min': -1500.0, 'val': 35000.0,
                        'desc': 'Measured Flight Level'}})
            self.assertEqual(packet[1]['I185'], {'Vx': {'max': 8191.75, 'min': -8192.0, 'val': 141.5, 'desc': 'Vx'},
                                                 'Vy': {'max': 8191.75, 'min': -8192.0, 'val': -170.75, 'desc': 'Vy'}})
            self.assertEqual(packet[1]['I080'], {'STP': {'meaning': 'default value', 'val': 0, 'desc': ''},
                                                 'MD5': {'meaning': 'No Mode 5 interrogation', 'val': 0, 'desc': ''},
                                                 'FPC': {'meaning': 'Flight plan correlated', 'val': 1, 'desc': ''},
                                                 'AMA': {'meaning': 'track not resulting from amalgamation process',
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
            self.assertEqual(packet[1]['I060'], {'CH': {'meaning': 'No Change', 'val': 0, 'desc': 'Change in Mode 3/A'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bits set to 0'},
                                                 'Mode3A': {'val': '2535',
                                                            'desc': 'Mode-3/A reply in octal representation'}})
            self.assertEqual(packet[1]['I295'], {'MD2': {'val': 0, 'desc': ''}, 'MD5': {'val': 0, 'desc': ''},
                                                 'MFL': {'val': 0.0, 'desc': ''}, 'MDA': {'val': 0.0, 'desc': ''},
                                                 'FX': {'meaning': 'no extension', 'val': 0,
                                                        'desc': 'Extension indicator'}, 'MHG': {'val': 0, 'desc': ''},
                                                 'MD1': {'val': 0, 'desc': ''}, 'MD4': {'val': 0, 'desc': ''}})
            self.assertEqual(packet[1]['I390'], {'FCT': {'val': 1, 'desc': 'Flight Category'},
                                                 'TYPE': {'val': 'B738', 'desc': 'Type of Aircraft'},
                                                 'NBR': {'val': 29233709, 'desc': ''},
                                                 'GATOAT': {'meaning': 'General Air Traffic', 'val': 1, 'desc': ''},
                                                 'STD': {'val': 0, 'desc': 'Standard Instrument Departure'},
                                                 'CTL': {'val': 0, 'desc': 'Current Control Position'},
                                                 'RVSM': {'meaning': 'Approved', 'val': 1, 'desc': ''},
                                                 'CS': {'val': 'SXD4723', 'desc': 'Callsign'},
                                                 'DST': {'val': 1, 'desc': 'Destination Airport'},
                                                 'AST': {'val': 0, 'desc': 'Aircraft Stand'},
                                                 'FX': {'meaning': 'no extension', 'val': 0,
                                                        'desc': 'Extension indicator'},
                                                 'IFI': {'val': 1, 'desc': 'IFPS_FLIGHT_ID'},
                                                 'STS': {'val': 0, 'desc': 'Stand Status'},
                                                 'RDS': {'val': 1, 'desc': 'Runway Designation'},
                                                 'DES': {'val': 'HELX', 'desc': 'Destination Airport'},
                                                 'CFL': {'val': 350.0, 'desc': 'Current Cleared Flight Level'},
                                                 'DEP': {'val': 'EDDL', 'desc': 'Departure Airport'},
                                                 'PEC': {'val': 0, 'desc': 'Pre-emergency Callsign'},
                                                 'TAC': {'val': 1, 'desc': 'Type of Aircraft'},
                                                 'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'NU2': {'val': ' ', 'desc': 'Second number'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'spare bit set to zero'},
                                                 'PEM': {'val': 0, 'desc': 'Pre-emergency Mode 3/A code'},
                                                 'HPR': {'meaning': 'Normal Priority Flight', 'val': 0, 'desc': ''},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'},
                                                 'FR1FR2': {'meaning': 'Instrument Flight Rules', 'val': 0, 'desc': ''},
                                                 'TAG': {'val': 1, 'desc': 'FPPS Identification Tag'},
                                                 'TYP': {'meaning': 'Unit 1 internal flight number', 'val': 1,
                                                         'desc': ''}, 'TOD': {'val': 0, 'desc': 'Time of Departure'},
                                                 'NU1': {'val': ' ', 'desc': 'First number'},
                                                 'LTR': {'val': ' ', 'desc': 'Letter'},
                                                 'CSN': {'val': 1, 'desc': 'Callsign'},
                                                 'STA': {'val': 0, 'desc': 'Standard Instrument Arrival'},
                                                 'WTC': {'val': 'M', 'desc': 'Wake Turbulence Category'}})
            self.assertEqual(packet[1]['I010'], {'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'System Identification Code'}})
            self.assertEqual(packet[1]['I340'], {'SID': {'val': 1, 'desc': 'Sensor Identification'},
                                                 'Mode3A': {'val': '2535',
                                                            'desc': 'Mode 3/A reply under the form of 4 digits in octal representation'},
                                                 'HEI': {'val': 0, 'desc': 'Measured 3-D Height'},
                                                 'TST': {'meaning': 'Real target report', 'val': 0, 'desc': ''},
                                                 'G': {'meaning': 'Default', 'val': 0, 'desc': ''},
                                                 'V': {'meaning': 'Code validated', 'val': 0, 'desc': ''},
                                                 'CG': {'meaning': 'Default', 'val': 0, 'desc': ''},
                                                 'FX': {'meaning': 'no extension', 'val': 0,
                                                        'desc': 'Extension indicator'},
                                                 'ModeC': {'max': 1270.0, 'min': -12.0, 'val': 350.0,
                                                           'desc': 'Last Measured Mode C Code'},
                                                 'CV': {'meaning': 'Code validated', 'val': 0, 'desc': ''},
                                                 'SAC': {'val': 25, 'desc': 'System Area Code'},
                                                 'RHO': {'max': 256.0, 'val': 93.1953125, 'desc': 'Measured distance'},
                                                 'SIM': {'meaning': 'Actual target report', 'val': 0, 'desc': ''},
                                                 'MDA': {'val': 1, 'desc': 'Last Measured Mode 3/A code'},
                                                 'SIC': {'val': 13, 'desc': 'System Identification Code'},
                                                 'THETA': {'val': 271.4666748046875, 'desc': 'Measured azimuth'},
                                                 'POS': {'val': 1, 'desc': 'Measured Position'},
                                                 'TYP': {'meaning': 'Single ModeS Roll-Call', 'val': 5,
                                                         'desc': 'Report Type'},
                                                 'RAB': {'meaning': 'Report from aircraft transponder', 'val': 0,
                                                         'desc': ''},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bits set to zero'},
                                                 'L': {
                                                     'meaning': 'MODE 3/A code as derived from the reply of the transponder',
                                                     'val': 0, 'desc': ''},
                                                 'MDC': {'val': 1, 'desc': 'Last Measured Mode C code'}})
            self.assertEqual(packet[1]['I380'], {'ACID': {'val': 'SXD4723 ', 'desc': 'Target Identification'},
                                                 'FSS': {'val': 0, 'desc': 'Final State SelectedAltitude'},
                                                 'STAT': {'meaning': 'No alert, no SPI, aircraft airborne', 'val': 0,
                                                          'desc': 'Flight Status'},
                                                 'ARC': {'meaning': '25 ft resolution', 'val': 1,
                                                         'desc': 'Altitude reporting capability'},
                                                 'COM': {'meaning': 'Comm. A and Comm. B capability', 'val': 1,
                                                         'desc': 'Communications capability of the transponder'},
                                                 'ID': {'val': 1, 'desc': 'Target Identification'},
                                                 'TID': {'val': 0, 'desc': 'Trajectory Intent Data'},
                                                 'AIC': {'meaning': 'Yes', 'val': 1,
                                                         'desc': 'Aircraft identification capability'},
                                                 'B1B': {'val': 6, 'desc': 'BDS 1,0 bits 37/40'},
                                                 'SAL': {'val': 0, 'desc': 'Selected Altitude'},
                                                 'SAB': {'val': 0, 'desc': 'Status reported by ADS-B'},
                                                 'spare': {'const': 0, 'val': 0, 'desc': 'Spare bits set to zero'},
                                                 'ACS': {'val': 0, 'desc': 'ACAS Resolution Advisory Report'},
                                                 'TAS': {'val': 0, 'desc': 'True Airspeed'},
                                                 'ADR': {'val': '3C0A55', 'desc': 'Target Address'},
                                                 'SSC': {'meaning': 'Yes', 'val': 1,
                                                         'desc': 'Specific service capability'},
                                                 'MHG': {'val': 0, 'desc': 'Magnetic Heading'},
                                                 'IAS': {'val': 0, 'desc': 'Indicated Airspeed/Mach Number'},
                                                 'FX': {'meaning': 'no extension', 'val': 0,
                                                        'desc': 'Extension indicator'},
                                                 'TIS': {'val': 0, 'desc': 'Trajectory Intent Status'},
                                                 'GVR': {'val': 0, 'desc': 'Geometric Vertical Rate'},
                                                 'B1A': {'val': 1, 'desc': 'BDS 1,0 bit 16'},
                                                 'BVR': {'val': 0, 'desc': 'Barometric Vertical Rate'}})
            self.assertEqual(packet[1]['I105'], {'Lat': {'val': 45.40080785751343,
                                                         'desc': 'Latitude in WGS.84 in twos complement. Range -90 < latitude < 90 deg.'},
                                                 'Lon': {'val': 15.13318419456482,
                                                         'desc': 'Longitude in WGS.84 in twos complement. Range -180 < longitude < 180 deg.'}})
            self.assertEqual(packet[1]['I040'], {'TrkN': {'val': 7977, 'desc': 'Track number'}})
            self.assertEqual(packet[1]['I210'], {'Ax': {'val': 0.0, 'desc': 'Ax'}, 'Ay': {'val': 0.0, 'desc': 'Ay'}})
            self.assertEqual(packet[2]['I015'], {'SID': {'val': 4, 'desc': 'Service Identification'}})
            self.assertEqual(packet[2]['I020'], {'BTN': {'val': 24, 'desc': 'Batch Number'}})
            self.assertEqual(packet[2]['I010'], {'SAC': {'val': 25, 'desc': 'Source Area Code'},
                                                 'SIC': {'val': 100, 'desc': 'Source Identification Code'}})
            self.assertEqual(packet[2]['I030'], {'ToD': {'val': 30913.0546875, 'desc': 'Time Of Message'}})
            self.assertEqual(packet[2]['I000'], {'Typ': {'meaning': 'End of Batch', 'val': 2, 'desc': 'Message Type'}})


def main():
    unittest.main()


if __name__ == '__main__':
    main()

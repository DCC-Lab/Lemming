import unittest
import re
from pathlib import Path
import os
from contextlib import suppress
from datetime import datetime


class TimestampPath(Path):
    def __init__(self, *args):
        super().__init__(*args)
        
        self.entries = self.get_possible_timestamps()

    def get_possible_matching_patterns(self):
        d1 = r"(\d)"      # One digit
        d2 = r"(\d\d)"    # Two digits
        year = r"(\d{4})" # Year always 4 digits
        uid = r"(\d{4})"  # Unique id always 4 digits

        patterns = []
        for secs in [d1,d2]:
            for mins in [d1,d2]:
                for hours in [d1,d2]:
                    for days in [d1,d2]:
                        for months in [d1,d2]:
                            patterns.append(f"^{uid}.*-{year}{months}{days}{hours}{mins}{secs}$")

        return patterns

    def get_possible_timestamps(self):
        patterns = self.get_possible_matching_patterns()

        filename = self.name # Don't use directory in name

        entries = []
        for pattern in patterns:
            entry = {}
            match = re.search(pattern, filename)
            if match is not None:
                entry['event'] = int(match.group(1))
                entry['year'] = int(match.group(2))
                if entry['year'] < 2020 or entry['year'] > 2050:
                    continue
                entry['month'] = int(match.group(3))
                if entry['month'] > 12 or entry['month'] < 1:
                    continue
                entry['day'] = int(match.group(4))
                if entry['day'] > 31 or entry['day'] < 1:
                    continue
                entry['hours'] = int(match.group(5))
                if entry['hours'] >= 24:
                    continue
                entry['minutes'] = int(match.group(6))
                if entry['minutes'] >= 60:
                    continue
                entry['seconds'] = int(match.group(7))
                if entry['seconds'] >= 60:
                    continue
                entry['time'] = datetime(year=entry['year'], month=entry['month'], day=entry['day'], hour=entry['hours'], minute=entry['minutes'], second=entry['seconds'])
                entries.append(entry)

        return entries


class TestFilenames(unittest.TestCase):
    def test_init(self):
        self.assertTrue(True)

    def test_timestamp_path(self):
        path1 = TimestampPath("0001-startFile-202475182821")
        self.assertIsNotNone(path1)

        path2 = TimestampPath("0002-202475182821")
        self.assertIsNotNone(path2)

    def test_timestamp_path_entries(self):
        path1 = TimestampPath("0001-startFile-202475182821")
        self.assertTrue(len(path1.entries) >= 1)

    def setUp(self):
        self.path = Path("testdata_timestamps")
        files = os.listdir(self.path)
        files.sort()

        self.filepaths = [ TimestampPath(filename) for filename in files]
        self.filepaths = [ filepath for filepath in self.filepaths if len(filepath.entries) >= 1] # Remove files not matching anything

        self.entries = []
        for filepath in self.filepaths:
            self.entries.extend(filepath.entries)



    def test_extract_general_case(self):

        # Step 1: remove duplicates (for some reason they appear when there is 02 and 2 for instance). 
        # We cannot use a set with a dict, do it manually
        entries_without_duplicates = self.entries.copy() 
        for entry in self.entries:
            entries_without_duplicates.remove(entry) # removes first encountered only
            if entry not in entries_without_duplicates:
                # put it back
                entries_without_duplicates.append(entry)

        self.entries = entries_without_duplicates

        # Step 2: Two entries with same id less than 5 minutes apart are considered equal, second one is dropped
        self.entries.sort(key= lambda entry : (entry['event'], entry['time']) )

        entries_without_very_close_times = []
        entries_without_very_close_times.append(self.entries[0]) # First for sure

        for i in range(1, len(self.entries)):
            entry2 = self.entries[i]
            entry1 = self.entries[i-1]
            if entry1['event'] == entry2['event'] and (entry2['time']-entry1['time']).total_seconds() < 5*60:
                pass # Too close to prior time (essentially equivalent)
            # if entry1['event'] != entry2['event'] and  (entry2['time']-entry1['time']).total_seconds() < 0:
            #     pass # Entry2 has to be wrong: is before previous entry1 on different event
            else:
                entries_without_very_close_times.append(entry2)

        self.entries = entries_without_very_close_times

        # Step 3: If ordered by time first, then id, we MUST be able to find
        # ids sequentially (0001, 0002, etc...) If we find others in the way, 
        # they have to be wrong
        self.entries.sort(key= lambda entry : (entry['time'],entry['event']) )

        in_order = []
        entries = self.entries.copy()
        uid = 1
        count_unique_ids = len(set([int(e['event']) for e in self.entries]))

        while uid <= count_unique_ids:
            entry = entries.pop(0)
            if int(entry['event']) == uid:
                in_order.append(entry)
                uid += 1

        self.entries = in_order

        # We are done and there was no error
        for entry in self.entries:
            print(entry['event'],"\t", entry['time'])


    @unittest.skip('Experimenting with algos')
    def test_create_regex(self):
        d1 = r"(\d)"    # One digit
        d2 = r"(\d\d)"  # Two digits
        year = r"(\d{4})"

        patterns = []
        for secs in [d1,d2]:
            for mins in [d1,d2]:
                for hours in [d1,d2]:
                    for days in [d1,d2]:
                        for months in [d1,d2]:
                            patterns.append(f"{year}{months}{days}{hours}{mins}{secs}")

        timestamp = "202475182821"
        entries = []
        for pattern in patterns:
            entry = {}
            match = re.search(pattern, timestamp)
            if match is not None:
                entry['year'] = int(match.group(1))
                if entry['year'] < 2020 or entry['year'] > 2050:
                    continue
                entry['month'] = int(match.group(2))
                if entry['month'] > 12 or entry['month'] < 1:
                    continue
                entry['day'] = int(match.group(3))
                if entry['day'] > 31 or entry['day'] < 1:
                    continue
                entry['hours'] = int(match.group(4))
                if entry['hours'] >= 24:
                    continue
                entry['minutes'] = int(match.group(5))
                if entry['minutes'] >= 60:
                    continue
                entry['seconds'] = int(match.group(6))
                if entry['seconds'] >= 60:
                    continue
                entry['time'] = datetime(year=entry['year'], month=entry['month'], day=entry['day'], hour=entry['hours'], minute=entry['minutes'], second=entry['seconds'])
                entries.append(entry)

    def test_filepaths_with_dir(self):
        files = os.listdir(self.path)
        files.sort()

        filepaths = [ TimestampPath(filename) for filename in files]
        filepaths = [ filepath for filepath in filepaths if len(filepath.entries) >= 1] # Remove files not matching anything
        self.assertEqual(len(filepaths), 233) # With test_data

        entries = []
        for filepath in filepaths:
            self.assertTrue(len(filepath.entries) >= 1, filepath)
            entries.extend(filepath.entries)

        count_unique_ids = len(set([int(e['event']) for e in entries]))
        self.assertEqual(count_unique_ids, len(filepaths))

    def test_filepaths_cleanup(self):
        files = os.listdir(self.path)
        files.sort()

        filepaths = [ TimestampPath(filename) for filename in files]
        filepaths = [ filepath for filepath in filepaths if len(filepath.entries) >= 1] # Remove files not matching anything    

        self.entries.sort(key= lambda entry : (entry['time'],entry['event']) )
        count_unique_ids = len(set([int(e['event']) for e in self.entries]))
        self.assertEqual(count_unique_ids, 233)

        final_entries = []
        entries = self.entries.copy()
        uid = 1 # Start at 1
        while uid <= count_unique_ids:
            entry = entries.pop(0)
            if int(entry['event']) == uid:
                final_entries.append(entry)
                uid += 1

        entries = final_entries

        # We are done and there was no error
        for entry in final_entries:
            print(entry['event'],"\t", entry['time'])







if __name__ == "__main__":
    unittest.main()
    unittest.main(defaultTest=["TestFilenames.test_create_regex"])


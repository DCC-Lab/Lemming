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
                            patterns.append(f"{uid}.*-{year}{months}{days}{hours}{mins}{secs}")

        return patterns

    def get_possible_timestamps(self):
        patterns = self.get_possible_matching_patterns()

        timestamp = str(self)

        entries = []
        for pattern in patterns:
            entry = {}
            match = re.search(pattern, timestamp)
            if match is not None:
                entry['uid'] = int(match.group(1))
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

def extract_number_in_range(text, min, max):
    options = []

    with suppress(IndexError, ValueError):
        option1 = int(text[0])
        if option1 >= min and option1 <= max:
            options.append( (option1, text[1:]) )

    with suppress(IndexError, ValueError):
        option2 = int(text[0:2])
        if option2 >= min and option2 <= max:
            options.append( (option2, text[2:]) )

    return options

def extract_possible_months(entry):
    updated_entries = []

    possible_months = extract_number_in_range(entry['leftover'],1, 12)
    for month, leftover in possible_months:
        updated_entry = dict(entry)
        updated_entry['month'] = month
        updated_entry['leftover'] = leftover
        updated_entries.append(updated_entry)

    return updated_entries

def extract_possible_days(entry):
    updated_entries = []

    possible_days = extract_number_in_range(entry['leftover'],1, 31)
    for day, leftover in possible_days:
        updated_entry = dict(entry)
        updated_entry['day'] = day
        updated_entry['leftover'] = leftover
        updated_entries.append(updated_entry)

    return updated_entries

def extract_possible_hours(entry):
    updated_entries = []

    possible_hours = extract_number_in_range(entry['leftover'],0, 23)
    for hours, leftover in possible_hours:
        updated_entry = dict(entry)
        updated_entry['hours'] = hours
        updated_entry['leftover'] = leftover
        updated_entries.append(updated_entry)

    return updated_entries

def extract_possible_minutes(entry):
    updated_entries = []

    possible_minutes = extract_number_in_range(entry['leftover'],0, 59)
    for minutes, leftover in possible_minutes:
        updated_entry = dict(entry)
        updated_entry['minutes'] = minutes
        updated_entry['leftover'] = leftover
        updated_entries.append(updated_entry)

    return updated_entries

def extract_possible_seconds(entry):
    updated_entries = []

    possible_seconds = extract_number_in_range(entry['leftover'],0, 59)
    for seconds, leftover in possible_seconds:
        updated_entry = dict(entry)
        updated_entry['seconds'] = seconds
        updated_entry['leftover'] = leftover
        updated_entries.append(updated_entry)

    return updated_entries

class TestFilenames(unittest.TestCase):
    def test_init(self):
        self.assertTrue(True)

    def test_path(self):
        path = Path("/Users/dccote/Downloads/data")
        self.assertTrue(path.exists())

    def setUp(self):
        self.path = Path("/Users/dccote/Downloads/data")

        files = os.listdir(self.path)
        files.sort()

        self.entries = []
        for filepath in files:
            match = re.search(r"(\d{4}).*-(2024)(\d+)", filepath)
            if match is not None:
                self.entries.append({"id":match.group(1), "year":int(match.group(2)), "timestamp":match.group(3), "leftover":match.group(3)})

    def test_extract_general_case(self):
        updated_entries = []
        for entry in self.entries:
            multiple_new_entries = extract_possible_months(entry)
            updated_entries.extend(multiple_new_entries)

        self.entries = updated_entries  

        updated_entries = []
        for entry in self.entries:
            multiple_new_entries = extract_possible_days(entry)
            updated_entries.extend(multiple_new_entries)

        self.entries = updated_entries

        updated_entries = []
        for entry in self.entries:
            multiple_new_entries = extract_possible_hours(entry)
            updated_entries.extend(multiple_new_entries)

        self.entries = updated_entries

        updated_entries = []
        for entry in self.entries:
            multiple_new_entries = extract_possible_minutes(entry)
            updated_entries.extend(multiple_new_entries)

        self.entries = updated_entries

        updated_entries = []
        for entry in self.entries:
            multiple_new_entries = extract_possible_seconds(entry)
            updated_entries.extend(multiple_new_entries)

        self.entries = updated_entries
        for entry in self.entries:
            entry['time'] = datetime(year=entry['year'], month=entry['month'], day=entry['day'], hour=entry['hours'], minute=entry['minutes'], second=entry['seconds'])

        # Step 1: remove if it did not reach the end of the string (means leftover characters, so has to be wrong)
        self.entries = [ entry for entry in self.entries if len(entry['leftover']) == 0]

        # Step 2: remove duplicates (for some reason they appear). We cannot use a set with a dict, do it manually
        entries_without_duplicates = self.entries.copy() 
        for entry in self.entries:
            entries_without_duplicates.remove(entry) # removes first encountered only
            if entry not in entries_without_duplicates:
                # put it back
                entries_without_duplicates.append(entry)

        self.entries = entries_without_duplicates

        # Step 3: Two entries with same id less than 5 minutes apart are considered equal, second one is dropped
        self.entries.sort(key= lambda entry : (entry['id'], entry['time']) )

        entries_without_very_close_times = []
        entries_without_very_close_times.append(self.entries[0]) # First for sure

        for i in range(1, len(self.entries)):
            entry2 = self.entries[i]
            entry1 = self.entries[i-1]
            if entry1['id'] == entry2['id'] and (entry2['time']-entry1['time']).total_seconds() < 5*60:
                pass # Too close to prior time (essentially equivalent)
            # if entry1['id'] != entry2['id'] and  (entry2['time']-entry1['time']).total_seconds() < 0:
            #     pass # Entry2 has to be wrong: is before previous entry1 on different event
            else:
                entries_without_very_close_times.append(entry2)

        self.entries = entries_without_very_close_times

        # Step 4: If ordered by time first, then id, we MUST be able to find
        # ids sequentially (0001, 0002, etc...) If we find others in the way, 
        # they have to be wrong
        self.entries.sort(key= lambda entry : (entry['time'],entry['id']) )

        in_order = []
        entries = self.entries.copy()
        uid = 1
        count_unique_ids = len(set([int(e['id']) for e in self.entries]))

        while uid <= count_unique_ids:
            entry = entries.pop(0)
            if int(entry['id']) == uid:
                in_order.append(entry)
                uid += 1

        self.entries = in_order

        # We are done and there was no error
        for entry in self.entries:
            print(entry['id'],"\t", entry['time'])


    def test_timestamp_path(self):
        path1 = TimestampPath("0001-startFile-202475182821")
        self.assertIsNotNone(path1)
        path2 = TimestampPath("0002-202475182821")
        self.assertIsNotNone(path2)

    def test_timestamp_path_entries(self):
        path1 = TimestampPath("0001-startFile-202475182821")
        print(path1.entries)

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

        print(entries)





if __name__ == "__main__":
    unittest.main()
    unittest.main(defaultTest=["TestFilenames.test_create_regex"])


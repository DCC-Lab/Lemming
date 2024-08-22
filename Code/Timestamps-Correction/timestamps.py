import unittest
import re
from pathlib import Path
import os
from contextlib import suppress
from datetime import datetime
from numpy import mean, std

class TimestampPath(Path):
    def __init__(self, *args):
        super().__init__(*args)
        
        self.timestamp_entries = self.get_possible_timestamps()


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
                            patterns.append(f"^{uid}(.*)-{year}{months}{days}{hours}{mins}{secs}$")

        return patterns

    def get_average_time_std(self):
        times = [entry['time'].timestamp() for entry in self.timestamp_entries]
        return mean(times), std(times)

    def get_possible_timestamps(self):
        patterns = self.get_possible_matching_patterns()

        filename = self.name # Don't use directory in name

        extracted_times = []

        entries = []
        for pattern in patterns:
            entry = {}
            match = re.search(pattern, filename)
            if match is not None:
                entry['event'] = int(match.group(1))
                entry['start-marker'] = match.group(2) # can be empty
                entry['year'] = int(match.group(3))
                if entry['year'] < 2020 or entry['year'] > 2050:
                    continue
                entry['month'] = int(match.group(4))
                if entry['month'] > 12 or entry['month'] < 1:
                    continue
                entry['day'] = int(match.group(5))
                if entry['day'] > 31 or entry['day'] < 1:
                    continue
                entry['hours'] = int(match.group(6))
                if entry['hours'] >= 24:
                    continue
                entry['minutes'] = int(match.group(7))
                if entry['minutes'] >= 60:
                    continue
                entry['seconds'] = int(match.group(8))
                if entry['seconds'] >= 60:
                    continue

                entry['time'] = datetime(year=entry['year'], month=entry['month'], day=entry['day'], hour=entry['hours'], minute=entry['minutes'], second=entry['seconds'])

                entry['corrected_filepath'] = f"{entry['event']:04d}{entry['start-marker']}-{entry['year']:02d}{entry['month']:02d}{entry['day']:02d}{entry['hours']:02d}{entry['minutes']:02d}{entry['seconds']:02d}"
                entry['original_filepath'] = str(self)

                if entry['time'] not in extracted_times: # Don't add times when identical
                    entries.append(entry)
                    extracted_times.append(entry['time'])
                else:
                    pass

        return entries

class LemmingDataDirectory(Path):
    def __init__(self, *args):
        super().__init__(*args)

        if not self.is_dir() or not self.exists() :
            ValueError('LemmingDataDirectory must be an existing directory')

        datafilenames = os.listdir(self)

        self.datafilepaths = [ TimestampPath(datafilename) for datafilename in datafilenames]
        self.datafilepaths = [ datafilepath for datafilepath in self.datafilepaths if len(datafilepath.timestamp_entries) >= 1] # Remove files not matching data file
        self.datafilepaths.sort()
        self.timestamp_entries = []
        for filepath in self.datafilepaths:
            self.timestamp_entries.extend(filepath.timestamp_entries)

    @property
    def datafile_count(self):
        return len(self.datafilepaths)

    @property
    def possible_timestamps_entries(self):
        self.timestamp_entries.sort(key= lambda entry : entry['event'] )
        return self.timestamp_entries

    @property
    def corrected_timestamps_entries(self):
        """
        The general idea here is simple: all events occurred sequentially and their time MUST
        also be sequential in the same order. Starting with event 0001, we need to find event 0002
        and anything that is not event 0002 is obviously incorrect because event 0018 cannot be 
        before event 0002 for instance. We keep going until the end.
        This works well here but could fail. Also, the first correct event is kept, not sure 
        how to check if the second (or third or fifth) occurence was in fact the best one.
        """
        count_unique_ids = self.datafile_count

        self.timestamp_entries.sort(key= lambda entry : (entry['time'],entry['event']) )

        corrected_entries = []
        entries = self.timestamp_entries.copy()
        uid = 1 # Start at 1
        while uid <= count_unique_ids:
            entry = entries.pop(0)
            if entry['event'] == uid:
                corrected_entries.append(entry)
                uid += 1

        return corrected_entries
        
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
        self.assertTrue(len(path1.timestamp_entries) >= 1)

    def setUp(self):
        self.path = Path("testdata_timestamps")
        files = os.listdir(self.path)
        files.sort()

        self.filepaths = [ TimestampPath(filename) for filename in files]
        self.filepaths = [ filepath for filepath in self.filepaths if len(filepath.timestamp_entries) >= 1] # Remove files not matching anything

        self.timestamp_entries = []
        for filepath in self.filepaths:
            self.timestamp_entries.extend(filepath.timestamp_entries)

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
        filepaths = [ filepath for filepath in filepaths if len(filepath.timestamp_entries) >= 1] # Remove files not matching anything
        self.assertEqual(len(filepaths), 233) # With current test_data

        entries = []
        for filepath in filepaths:
            self.assertTrue(len(filepath.timestamp_entries) >= 1, filepath)
            entries.extend(filepath.timestamp_entries)

        count_unique_ids = len(set([int(e['event']) for e in entries]))
        self.assertEqual(count_unique_ids, len(filepaths))

    @unittest.skip('Algorithm development occurred here, everything was moved to class LemmingDataDirectory')
    def test_filepaths_cleanup(self):
        files = os.listdir(self.path)
        files.sort()

        filepaths = [ TimestampPath(filename) for filename in files]
        filepaths = [ filepath for filepath in filepaths if len(filepath.entries) >= 1] # Remove files not matching anything    

        self.timestamp_entries.sort(key= lambda entry : (entry['time'],entry['event']) )
        count_unique_ids = len(set([int(e['event']) for e in self.timestamp_entries]))
        self.assertEqual(count_unique_ids, 233)

        final_entries = []
        entries = self.timestamp_entries.copy()
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

    def test_data_directory(self):
        self.assertTrue(LemmingDataDirectory("testdata_timestamps").exists())

    def test_data_directory_content(self):
        self.assertTrue(LemmingDataDirectory("testdata_timestamps").datafile_count == 233) # My test data has 233

    def test_data_directory_all_timestamps_entries(self):
        testdir = LemmingDataDirectory("testdata_timestamps")
        self.assertTrue(len(testdir.possible_timestamps_entries) > testdir.datafile_count)

    def test_data_directory_all_corrected_timestamps(self):
        testdir = LemmingDataDirectory("testdata_timestamps")
        self.assertTrue(len(testdir.corrected_timestamps_entries) == testdir.datafile_count)

    def test_find_indexes_of_element(self):
        numbers = [ 1,2,3,4,5,4,4,4,5,6,7,7,7,7]

        self.assertEqual([i for i, x in enumerate(numbers) if x == 4], [3,5,6,7])

    @unittest.skip('This is insane to think it is possible.  Too many possibilities')
    def test_more_possibilities(self):
        testdir = LemmingDataDirectory("testdata_timestamps")

        testdir.timestamp_entries.sort(key= lambda entry : (entry['time'],entry['event']) )

        all_options = [ ([], testdir.timestamp_entries) ]
        for event in range(1, testdir.datafile_count):
            options_for_next_event = []

            for extracted, leftover in all_options:
                options_for_this_event = self.extract_event_from_entries(extracted, leftover, event)
                if options_for_this_event is not None:
                    options_for_next_event.extend(options_for_this_event)
                else:
                    pass # This is an invalid option, we don't add it back

            all_options = options_for_next_event

        # for option in all_options:
        #     self.assertEqual(len(option[0]), testdir.datafile_count )

        print(len(all_options))
        for j in range(testdir.datafile_count-1):
            for i in range(len(all_options)):
                extracted, leftover = all_options[i]
                print( f"{extracted[j]['time']:.3f}\t", end='')
            print()

    def test_average_stddev(self):
        testdir = LemmingDataDirectory("testdata_timestamps")

        testdir.timestamp_entries.sort(key= lambda entry : (entry['time'],entry['event']) )

        for filepath in testdir.datafilepaths:
            print(filepath)

    def extract_event_from_entries(self, extracted_entries, leftover_entries, event):
        indexes = list([ind for ind, e in enumerate(leftover_entries) if e['event'] == event])
        if len(indexes) == 0:
            return None

        options = []
        for j in range(len(indexes)):
            i = indexes[j]
            entry = leftover_entries[i]
            options.append( (extracted_entries+[leftover_entries[i]], leftover_entries[i+1:]) )

        return options


if __name__ == "__main__":
    # unittest.main() # Un comment to run code below

    testdir = LemmingDataDirectory("testdata_timestamps")

    print("--- All data file paths ---")
    for filepath in testdir.datafilepaths:
        print(f"{filepath}")

    print("--- All options for each data file path ---")
    for filepath in testdir.datafilepaths:
        print(f"{filepath}")
        print(f"    {len(filepath.timestamp_entries)} option(s): " +", ".join([ str(entry['time']) for entry in filepath.timestamp_entries ]))

    print("--- All possible corrected file paths for each data file path ---")
    for timestamp_entry in testdir.possible_timestamps_entries:
        print(f"{timestamp_entry['corrected_filepath']} -> {timestamp_entry['original_filepath']}")

    print("--- Unique option for each data file path ---")
    for timestamp_entry in testdir.corrected_timestamps_entries:
        print(f"{timestamp_entry['corrected_filepath']} -> {timestamp_entry['original_filepath']}")

    print("--- Time for each data file path ---")
    time_offset = testdir.corrected_timestamps_entries[0]['time'].timestamp()
    for timestamp_entry in testdir.corrected_timestamps_entries:
        print(f"{timestamp_entry['time'].timestamp() - time_offset}")

    # print("--- Average time and std ---")
    # for filepath in testdir.datafilepaths:
    #     avg_time, std_time = filepath.get_average_time_std()
    #     print(f"{filepath}\t{avg_time}\t{std_time}")

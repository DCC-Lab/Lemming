import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import pandas as pd
import numpy as np
from timestamps import LemmingDataDirectory as ldd


testdir = ldd("testdata_timestamps")
event_times = [item['time'] for item in testdir.corrected_timestamps_entries if 'time' in item]
start_times = [item['time'] for item in testdir.corrected_timestamps_entries if item['start-marker'] == '-startFile']

bin_width = pd.Timedelta(minutes=15)
min_time = np.min(event_times)
max_time = np.max(event_times)
bins = pd.date_range(start=min_time, end=max_time + bin_width, freq=bin_width)


plt.figure(5)
plt.vlines(start_times, ymin=0, ymax=len(event_times), color='red', label='start time')
plt.plot(event_times, range(len(event_times)), marker='o', linestyle='-', color='b', label='Detection')
plt.bar(event_times, range(len(event_times)), width=bin_width, color='green', label='Bar plot 15 minutes', alpha=0.5)
# plt.hist(event_times, bins=bins, color='b', edgecolor='black')
# plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
plt.title('Timestamps Plot')
plt.xlabel('Time')
plt.ylabel('Index')
plt.xticks(rotation=45)
plt.legend()
plt.tight_layout()

# Show plot
# plt.show()

# plt.figure(6)
# # plt.bar(event_times, range(len(event_times)), width=0.01, color='b')
# # plt.plot(event_times, range(len(event_times)), marker='o', linestyle='-', color='b')
# plt.hist(event_times, bins=bins, color='b', edgecolor='black')
# plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
# plt.title('Timestamps Plot')
# plt.xlabel('Time')
# plt.ylabel('Index')
# plt.xticks(rotation=45)
# plt.tight_layout()

# Show plot
plt.show()


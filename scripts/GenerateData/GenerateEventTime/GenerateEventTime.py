import csv
import numpy as np
import configparser
import pandas as pd

# read the config file
config = configparser.ConfigParser()
config.read('config.ini')

# get values from the config file
csv_file = config['DEFAULT']['csv_file']
min_time = int(config['DEFAULT']['min_time'])
max_time = int(config['DEFAULT']['max_time'])
output_file = config['DEFAULT']['output_file']
zipf_alpha = float(config['ZIPF']['alpha'])
scale = int(config['ZIPF']['scale'])
df = pd.read_csv(csv_file)
row_count = len(df.index)

# add eventTime
with open(csv_file, 'r') as f:
    reader = csv.reader(f)
    rows = [row for row in reader]
    header = rows[0]
    rows = rows[1:]

    zipf_values = np.random.zipf(zipf_alpha, size=row_count) * scale
    eventTime = (zipf_values - 1) % (max_time - min_time + 1) + min_time
    print(eventTime)
    k = 0
    for row in rows:
        row.append(str(int(eventTime[k])))
        k += 1
    rows.insert(0, header)

# write in the csv file
with open(output_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerows(rows)

import csv
import numpy as np
import configparser
import pandas as pd

# Read configuration from config.ini file
config = configparser.ConfigParser()
config.read('config.ini')

# Get configuration values
csv_file = config['DEFAULT']['csv_file']
min_time = int(config['DEFAULT']['min_time'])
max_time = int(config['DEFAULT']['max_time'])
output_file = config['DEFAULT']['output_file']
zipf_alpha = float(config['ZIPF']['alpha'])
delay_scale = float(config['ZIPF']['delay_scale'])

df = pd.read_csv(csv_file)
row_count = len(df.index)

# Read CSV file and add arrivalTime column
with open(csv_file, 'r') as f:
    reader = csv.reader(f)
    rows = [row for row in reader]
    header = rows[0]
    rows = rows[1:]
    zipf_values = np.random.zipf(zipf_alpha, size=row_count) * delay_scale
    delay = (zipf_values - 1) % (max_time - min_time + 1) + min_time
    print(delay)
    k = 0
    for row in rows:
        eventTime = int(row[2])
        arrivalTime = eventTime + delay[k]
        row.append(str(int(arrivalTime)))
        k += 1
    header.append('arrivalTime')
    rows.insert(0, header)

# Write back to CSV file
with open(output_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerows(rows)

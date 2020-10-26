import getopt
import os
import sys

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator, LogLocator, MaxNLocator
from numpy import double

FIGURE_FOLDER = '/data1/xtra/results/figure'

# example for reading csv file
def ReadFile(id, tuple_cnt):
    # Creates a list containing w lists, each of h items, all set to 0
    global UNC_ARB_TRK_OCCUPANCY, UNC_CLOCK_SOCKET
    w = 8

    bound = id + 1 * w
    colomn = {}
    for i in range(id, bound, 1):
        print(i)
        f = open('/data1/xtra/results/breakdown/profile_{}.txt'.format(i), "r")
        read = f.readlines()
        for line in read:
            if line.startswith("DTLB_MISSES"):
                colomn["TLBD_Misses"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("ITLB_MISSES"):
                colomn["ITLB_MISSES"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L1I"):
                colomn["L1I_MISSES"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L2Misses"):
                colomn["L2_MISSES"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L2Hit"):
                colomn["L2_Hit"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L3Misses"):
                colomn["L3_MISSES"] = float(line.split(" ")[1])/tuple_cnt
                ts_start_ns = float(open('/data1/xtra/time_start_{}.txt'.format(i), "r").read())
                ts_end_ns = float(open('/data1/xtra/time_end_{}.txt'.format(i), "r").read())
                time_interval_s = (ts_end_ns - ts_start_ns) / 1E9
                colomn["MEM_BAND_CAL"] = float(line.split(" ")[1]) * 64 / (time_interval_s*1000000)
            elif line.startswith("BR_MISP_EXEC"):
                colomn["BRANCH_MISP"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("BR_INST_EXEC"):
                colomn["INST_EXEC"] = float(line.split(" ")[1])/tuple_cnt
            # elif line.startswith("MemoryBandwidth"):
            #     colomn["MEM_BAND"] = float(line.split(" ")[1])
            elif line.startswith("UNC_ARB_TRK_OCCUPANCY.ALL"):
                UNC_ARB_TRK_OCCUPANCY = float(line.split(" ")[1])
            elif line.startswith("UNC_CLOCK.SOCKET"):
                UNC_CLOCK_SOCKET = float(line.split(" ")[1])
            elif line.startswith("CPUCycle"):
                ts_start_ns = float(open('/data1/xtra/time_start_{}.txt'.format(i), "r").read())
                ts_end_ns = float(open('/data1/xtra/time_end_{}.txt'.format(i), "r").read())
                time_interval_s = (ts_end_ns - ts_start_ns) / 1E9
                colomn["CPU_UTIL"] = float(line.split(" ")[1])/time_interval_s
        colomn["L1D_MISSES"] = colomn["L2_MISSES"] + colomn["L2_Hit"] - colomn["L1I_MISSES"]
        colomn["MEM_BAND"] = UNC_ARB_TRK_OCCUPANCY/UNC_CLOCK_SOCKET
        print(i, colomn)
        colomn.clear()

if __name__ == "__main__":
    id = 400

    tuple_cnt=140800000
    ReadFile(id, tuple_cnt)  # 55

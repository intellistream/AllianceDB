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

matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'
# example for reading csv file
def ReadFile(id, tuple_cnt):
    # Creates a list containing w lists, each of h items, all set to 0
    global UNC_ARB_TRK_OCCUPANCY, UNC_CLOCK_SOCKET
    global L1D_REPL, INST_RETIRED
    w, h = 8, 10
    data = [[0 for x in range(w+1)] for y in range(h)]

    bound = id + 1 * w
    colomn = {}
    j = 0
    for i in range(id, bound, 1):
        # print(i)
        f = open(exp_dir + '/results/breakdown/profile_{}.txt'.format(i), "r")
        read = f.readlines()
        for line in read:
            if line.startswith("DTLB_MISSES"):
                colomn["TLBD_Misses"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("ITLB_MISSES"):
                colomn["ITLB_MISSES"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L1I"):
                colomn["L1I_MISSES"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("MEM_LOAD_UOPS_RETIRED.L1_MISS"):
                colomn["L1_MISSES"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("MEM_LOAD_UOPS_RETIRED.HIT_LFB"):
                colomn["HIT_LFB"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L2Misses"):
                colomn["L2_MISSES"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L2Hit"):
                colomn["L2_Hit"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("L3Misses"):
                colomn["L3_MISSES"] = float(line.split(" ")[1])/tuple_cnt
                ts_start_ns = float(open(exp_dir + '/results/breakdown/time_start_{}.txt'.format(i), "r").read())
                ts_end_ns = float(open(exp_dir + '/results/breakdown/time_end_{}.txt'.format(i), "r").read())
                time_interval_s = (ts_end_ns - ts_start_ns) / 1E9
                colomn["MEM_BAND_CAL"] = float(line.split(" ")[1]) * 64 / (time_interval_s*1000000)
            elif line.startswith("BR_MISP_EXEC"):
                colomn["BRANCH_MISP"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("BR_INST_EXEC"):
                colomn["INST_EXEC"] = float(line.split(" ")[1])/tuple_cnt
            elif line.startswith("MemoryBandwidth"):
                colomn["MEM_BAND"] = float(line.split(" ")[1])
            elif line.startswith("BytesFromMC"):
                colomn["BytesFromMC"] = float(line.split(" ")[1])
            elif line.startswith("BytesWrittenToMC"):
                colomn["BytesWrittenToMC"] = float(line.split(" ")[1])
            elif line.startswith("CPUCycle"):
                ts_start_ns = float(open(exp_dir + '/results/breakdown/time_start_{}.txt'.format(i), "r").read())
                ts_end_ns = float(open(exp_dir + '/results/breakdown/time_end_{}.txt'.format(i), "r").read())
                time_interval_s = (ts_end_ns - ts_start_ns) / 1E9
                # system wide cpu utilization, should divided by the number of cores.
                colomn["CPU_UTIL"] = float(line.split(" ")[1])/(time_interval_s*8)

        colomn["L1D_MISSES"] = colomn["L1_MISSES"] + colomn["HIT_LFB"] - colomn["L1I_MISSES"]

        ts_start_ns = float(open(exp_dir + '/results/breakdown/time_start_{}.txt'.format(i), "r").read())
        ts_end_ns = float(open(exp_dir + '/results/breakdown/time_end_{}.txt'.format(i), "r").read())
        time_interval_s = (ts_end_ns - ts_start_ns) / 1E9
        colomn["MEM_BAND_CAL"] = (colomn["BytesFromMC"] + colomn["BytesWrittenToMC"]) / (time_interval_s*1000000)

        # print(colomn)

        data[0][j] = format(colomn["TLBD_Misses"], '.3f')
        data[1][j] = format(colomn["ITLB_MISSES"], '.3f')
        data[2][j] = format(colomn["L1I_MISSES"], '.3f')
        data[3][j] = format(colomn["L1D_MISSES"], '.3f')
        data[4][j] = format(colomn["L2_MISSES"], '.3f')
        data[5][j] = format(colomn["L3_MISSES"], '.3f')
        data[6][j] = format(colomn["BRANCH_MISP"], '.3f')
        data[7][j] = format(colomn["INST_EXEC"], '.3f')
        data[8][j] = format(colomn["MEM_BAND_CAL"]/31872.0 * 100, '.3f')
        # data[8][j] = format(colomn["MEM_BAND"]/31872.0 * 100, '.6f')
        # data[8][j] = format(colomn["MEM_BAND"], '.6f')
        data[9][j] = format(colomn["CPU_UTIL"] * 100, '.3f')
        # data[10][j] = format(colomn["MEM_BAND_CAL"]/31872.0, '.6f')
        # data[10][j] = format(colomn["MEM_BAND_CAL"]/31872.0 * 100, '.3f')
        j += 1
        colomn.clear()

    # print(data)


    # col_name = ["TLBD Misses", "TLBI Misses", "L1I Misses", "L1D Misses", "L2 Misses", "L3 Misses", "Branch Mispred.", "Instr. Exec.", "Memory BW.(\%)", "CPU. Util.(\%)", "Memory BW.(CAL)"]
    col_name = ["TLBD Misses", "TLBI Misses", "L1I Misses", "L1D Misses", "L2 Misses", "L3 Misses", "Branch Misp.", "Instr. Exec.", "Mem. BW.(\%)", "CPU. Util.(\%)"]

    i=0
    for val in data:
        val[0], val[1], val[2], val[3] = val[2], val[3], val[0], val[1]
        print(col_name[i], " & ", " & ".join(str(x) for x in val[:-1]), " \\\\ \hline")
        i += 1

if __name__ == "__main__":
    id = 400

    tuple_cnt=10000000 + 1000
    ReadFile(id, tuple_cnt)  # 55

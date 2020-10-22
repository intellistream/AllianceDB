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
    w = 8

    bound = id + 1 * w
    column = {}
    uarch = {}
    for i in range(id, bound, 1):
        print(i)
        f = open('/data1/xtra/results/breakdown/profile_{}.txt'.format(i), "r")
        read = f.readlines()
        for line in read:
            if line.startswith("CPU_CLK_UNHALTED.THREAD"):
                column["CPU_CLK_UNHALTED.THREAD"] = float(line.split(" ")[1])
            elif line.startswith("IDQ_UOPS_NOT_DELIVERED.CORE"):
                column["IDQ_UOPS_NOT_DELIVERED.CORE"] = float(line.split(" ")[1])
            elif line.startswith("UOPS_ISSUED.ANY"):
                column["UOPS_ISSUED.ANY"] = float(line.split(" ")[1])
            elif line.startswith("UOPS_RETIRED.RETIRE_SLOTS"):
                column["UOPS_RETIRED.RETIRE_SLOTS"] = float(line.split(" ")[1])
            elif line.startswith("INT_MISC.RECOVERY_CYCLES"):
                column["INT_MISC.RECOVERY_CYCLES"] = float(line.split(" ")[1])
            elif line.startswith("CYCLE_ACTIVITY.STALLS_MEM_ANY"):
                column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] = float(line.split(" ")[1])
            elif line.startswith("RESOURCE_STALLS.SB"):
                column["RESOURCE_STALLS.SB"] = float(line.split(" ")[1])
            elif line.startswith("UNC_ARB_TRK_OCCUPANCY.ALL"):
                column["UNC_ARB_TRK_OCCUPANCY.ALL"] = float(line.split(" ")[1])

        clocks = column["CPU_CLK_UNHALTED.THREAD"]
        slots = 4*clocks
        uarch["Frontend Bound"] = column["IDQ_UOPS_NOT_DELIVERED.CORE"]/slots
        uarch["Bad Speculation"] = (column["UOPS_ISSUED.ANY"] - column["UOPS_RETIRED.RETIRE_SLOTS"] + 4*column["INT_MISC.RECOVERY_CYCLES"])/slots
        uarch["Retiring"] = column["UOPS_RETIRED.RETIRE_SLOTS"]/slots
        uarch["Backend Bound"] = 1 - (uarch["Frontend Bound"] + uarch["Bad Speculation"] + uarch["Retiring"])
        uarch["Memory Bound"] = (column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] + column["RESOURCE_STALLS.SB"])/clocks
        uarch["Core Bound"] = uarch["Backend Bound"] - uarch["Memory Bound"]
        print(i, uarch)
        column.clear()

if __name__ == "__main__":
    id = 400

    tuple_cnt=140800000
    ReadFile(id, tuple_cnt)  # 55

#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar as groupBar
import groupBar2 as groupBar2
import groupLine as groupLine
from autoParase import *
import itertools as it
import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator, LinearLocator
import os
import pandas as pd
import sys
from OoOCommon import *
from autoParase import *
import time

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 22
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['*', '|', 'v', "^", "", "h", "<", ">", "+", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = (
    '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (["////", "o", "", "||", "-", "//", "\\", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 15.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42


def accumulatedCnt(key, keyVec, timeVec):
    cnt = 0
    ruTime = []
    ruCnt = []
    for i in range(len(timeVec)):
        if (keyVec[i] == key):
            cnt = cnt + 1
            ruTime.append(timeVec[i])
            ruCnt.append(cnt)
    return ruTime, ruCnt


def main():
    exeSpace = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/"
    tupleStatisticName = "default_arrived_tuples.csv"
    arrivalTime = np.array(paraseValidColums(tupleStatisticName, "arrivalTime"))
    keys = np.array(paraseValidColums(tupleStatisticName, "keys"))
    eventTime = np.array(paraseValidColums(tupleStatisticName, "eventTime"))

    # print(keys)
    ruTime, ruCnt = accumulatedCnt(5.0, keys, arrivalTime)
    ruTime = np.array(ruTime) / 1000
    groupLine.DrawFigureYnormal([ruTime], [ruCnt], ['#Arrived Tuples'], " arrival time (ms)", "Count", 0, 1,
                                "arrived_Tuples", True)
    # groupLine.DrawFigureYnormal([periodVec,periodVec,periodVec],[avgLatVecNo,avgLatVecMean,avgLatVecIMA],['w/o AQP (lazy)',"w/ final AQP (lazy)","w/ incremental AQP (eager)"],"watermark time (ms)","95% latency (ms)",0,1,figPath+"wm_Aqps_lat",True)
    # groupLine.DrawFigureYnormal([periodVec,periodVec],[aqpErrVecMean,aqpErrVecIMA],["w/ AQP (lazy)","w/ incremental AQP (eager)"],"watermark time (ms)","Error",0,1,figPath+"wm_Aqps_err_incre",True)
    # draw2yLine("watermark time (ms)",periodVecDisp,lat95Vec,errVec,"95% Latency (ms)","Error","ms","",figPath+"wm_lat")
    # draw2yLine("watermark time (ms)",periodVecDisp,thrVec,errVec,"Throughput (KTp/s)","Error","KTp/s","",figPath+"wm_thr")
    # draw2yLine("watermark time (ms)",periodVecDisp,lat95Vec,compVec,"95% Latency (ms)","Completeness","ms","",figPath+"wm_omp")
    # groupLine.DrawFigureYnormal([periodVec,periodVec],[errVec,aqpErrVec],['w/o aqp',"w/ MeanAqp"],"watermark time (ms)","Error",0,1,figPath+"wm_MeanAqp",True)
    # print(errVec)
    # print(aqpErrVec)
    # print(aqpErrVecIMA,aqpErrVecMean)
    # readResultPeriod(50,resultPath)


if __name__ == "__main__":
    main()

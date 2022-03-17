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

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 16
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (["", 'o', 's', 'v', "^", "", "h", "<", ">", "+", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = (
'#FFFFFF', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (
["", "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
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


def getInfoFromCSV(a):
    # column legengd, row name
    with open(a, 'r') as f:
        reader = csv.reader(f)
        # reader = [each for each in csv.DictReader(f, delimiter=',')]
        result = list(reader)
        rows = len(result)
        # print('rows=',rows)
        firstRow = result[0]
        # print(firstRow)
        index = 0
        # define what may attract our interest
        idxCpu = 0
        idxName = 0
        idxEng = 0
        idxLat = 0
        idxVio = 0
        idxOve = 0
        xt = []
        yt = []
        idxList = []
        nameList = []
        legendList = []
        for i in firstRow:
            # print(i)
            if (i != 'name'):
                idxt = index
                idxList.append(idxt)
            index = index + 1
        idxName = 0
        # get names
        for k in range(len(idxList)):
            namet = float(result[0][idxList[k]])
            nameList.append(namet)
        # get legend
        for k in range(1, rows):
            legt = result[k][0]
            legendList.append(legt)
        # get results
        ruAll = []
        ruRow = []
        for k in range(1, rows):
            ruRow = []
            for j in range(len(idxList)):
                rut = float(result[k][idxList[j]])
                ruRow.append(rut)
            ruAll.append(ruRow)
        return nameList, legendList, ruAll


def drawx(fname, yname, xname):
    name, leg, ru = getInfoFromCSV(fname + ".csv")
    namet = []
    for i in range(len(ru)):
        namet.append(name)
    print(name)
    print(ru)
    groupLine.DrawFigureYnormal(namet, ru, leg, xname, yname, 0, 0, fname, 1)


def main():
    drawx("Time", "ms", "cores")
    drawx("AvgLat", "1k latency/us", "cores")
    # drawx("vio","","frequncy/GHz")
    # drawx("over","","frequncy/GHz")


if __name__ == "__main__":
    main()

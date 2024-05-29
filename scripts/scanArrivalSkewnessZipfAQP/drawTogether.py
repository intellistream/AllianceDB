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


def runArrivalSkew(exePath, arrivalSkew, resultPath, templateName="config.csv"):
    # resultFolder="arrivalSkewTests"
    configFname = "config_arrivalSkew_" + str(arrivalSkew) + ".csv"
    configTemplate = templateName
    # clear old files
    os.system("cd " + exePath + "&& rm *.csv")
    # prepare new file
    editConfig(configTemplate, exePath + configFname, "zipfDataLoader_zipfSkewFactor", arrivalSkew)
    # editConfig(exePath+configFname,exePath+configFname,"aqpScale",aqpScale)
    # run
    os.system("cd " + exePath + "&& ./benchmark " + configFname)
    # copy result
    os.system("rm -rf " + resultPath + "/" + str(arrivalSkew))
    os.system("mkdir " + resultPath + "/" + str(arrivalSkew))
    os.system("cd " + exePath + "&& cp *.csv " + resultPath + "/" + str(arrivalSkew))


def runArrivalSkewVector(exePath, arrivalSkewVec, resultPath, templateName="config.csv"):
    for i in arrivalSkewVec:
        runArrivalSkew(exePath, i, resultPath, templateName)


def readResultarrivalSkew(arrivalSkew, resultPath):
    resultFname = resultPath + "/" + str(arrivalSkew) + "/default_general.csv"
    avgLat = readConfig(resultFname, "AvgLatency")
    lat95 = readConfig(resultFname, "95%Latency")
    thr = readConfig(resultFname, "Throughput")
    err = readConfig(resultFname, "Error")
    aqpErr = readConfig(resultFname, "AQPError")
    return avgLat, lat95, thr, err, aqpErr


def readResultVectorarrivalSkew(arrivalSkewVec, resultPath):
    avgLatVec = []
    lat95Vec = []
    thrVec = []
    errVec = []
    compVec = []
    aqpErrVec = []
    for i in arrivalSkewVec:
        avgLat, lat95, thr, err, aqpErr = readResultarrivalSkew(i, resultPath)
        avgLatVec.append(float(avgLat) / 1000.0)
        lat95Vec.append(float(lat95) / 1000.0)
        thrVec.append(float(thr) / 1000.0)
        errVec.append(abs(float(err)))
        aqpErrVec.append(abs(float(aqpErr)))
        compVec.append(1 - abs(float(err)))
    return avgLatVec, lat95Vec, thrVec, errVec, compVec, aqpErrVec


def main():
    exeSpace = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/"
    resultPath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/aqparrivalSkewTest"
    resultPathNoAqp = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/aqparrivalSkewTest/NoAQP"
    resultPathIMA = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/aqparrivalSkewTest/IMA"
    figPath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/figures/"
    configTemplate = exeSpace + "config.csv"
    arrivalSkewVec = [0, 0.2, 0.4, 0.6, 0.8, 1.0]
    arrivalSkewVecDisp = np.array(arrivalSkewVec)
    arrivalSkewVecDisp = arrivalSkewVecDisp
    print(configTemplate)
    # run
    if (len(sys.argv) < 2):
        os.system("rm -rf " + resultPath)
        os.system("mkdir " + resultPath)
        os.system("mkdir " + resultPathNoAqp)
        # os.system("mkdir " + resultPathMeanAqp)
        os.system("mkdir " + resultPathIMA)
        runArrivalSkewVector(exeSpace, arrivalSkewVec, resultPathNoAqp, "config_noAQP.csv")
        # runArrivalSkewVector(exeSpace, arrivalSkewVec, resultPathMeanAqp, "config_meanAQP.csv")
        runArrivalSkewVector(exeSpace, arrivalSkewVec, resultPathIMA, "config_IMA.csv")
    avgLatVecNo, lat95VecNo, thrVecNo, errVecNo, compVec, aqpErrVecNo = readResultVectorarrivalSkew(arrivalSkewVec,
                                                                                                    resultPathNoAqp)
    avgLatVecIMA, lat95VecIMA, thrVecIMA, errVecIMA, compVec, aqpErrVecIMA = readResultVectorarrivalSkew(arrivalSkewVec,
                                                                                                         resultPathIMA)
    os.system("mkdir " + figPath)
    groupLine.DrawFigureYnormal([arrivalSkewVec, arrivalSkewVec, arrivalSkewVec], [aqpErrVecNo, aqpErrVecIMA],
                                ['w/o AQP (lazy)', "w/ incremental AQP (eager)"],
                                "watermark time (ms)", "Error", 0, 1, figPath + "zipfSkew_Aqps_err", True)
    groupLine.DrawFigureYnormal([arrivalSkewVec, arrivalSkewVec, arrivalSkewVec], [avgLatVecNo, avgLatVecIMA],
                                ['w/o AQP (lazy)', "w/ incremental AQP (eager)"],
                                "watermark time (ms)", "95% latency (ms)", 0, 1, figPath + "zipfSkew_Aqps_lat", True)

    # draw2yLine("watermark time (ms)",arrivalSkewVecDisp,lat95Vec,errVec,"95% Latency (ms)","Error","ms","",figPath+"wm_lat")
    # draw2yLine("watermark time (ms)",arrivalSkewVecDisp,thrVec,errVec,"Throughput (KTp/s)","Error","KTp/s","",figPath+"wm_thr")
    # draw2yLine("watermark time (ms)",arrivalSkewVecDisp,lat95Vec,compVec,"95% Latency (ms)","Completeness","ms","",figPath+"wm_omp")
    # groupLine.DrawFigureYnormal([arrivalSkewVec,arrivalSkewVec],[errVec,aqpErrVec],['w/o aqp',"w/ MeanAqp"],"watermark time (ms)","Error",0,1,figPath+"wm_MeanAqp",True)
    # print(errVec)
    # print(aqpErrVec)
    print(aqpErrVecIMA, aqpErrVecNo)
    # readResultarrivalSkew(50,resultPath)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
import csv
import numpy as np
import accuBar as accuBar
import groupBar as groupBar
import groupBar2 as groupBar2
import groupLine as groupLine
from autoParase import *
import itertools as it
import os

import matplotlib
import numpy as np
import pylab
import matplotlib.font_manager as fm
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator, LinearLocator
import os
import pandas as pd
import sys
from OoOCommon import *
import time

# OPT_FONT_NAME = 'Helvetica'
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

# matplotlib.rcParams['ps.useafm'] = True
# matplotlib.rcParams['pdf.use14corefonts'] = True
# matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
# matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE

# Explicitly specify the font path
# 获取系统中可用的字体路径
# font_paths = fm.findSystemFonts()
#
# # 创建字体列表
# font_list = []
# for font_path in font_paths:
#     try:
#         font_name = fm.FontProperties(fname=font_path).get_name()
#         if font_name not in font_list:
#             font_list.append(font_name)
#     except:
#         pass
#
# # 配置 matplotlib 使用系统中可用的字体
# matplotlib.rcParams['font.family'] = font_list[0]

matplotlib.rcParams['pdf.fonttype'] = 42


def runPeriod(exePath, period, resultPath, templateName="config.csv"):
    # resultFolder="periodTests"
    configFname = "config_period_" + str(period) + ".csv"
    configTemplate = templateName
    # clear old files
    os.system("cd " + exePath + "&& rm *.csv")
    # prepare new file
    if (int(period) < 0):
        editConfig(configTemplate, exePath + configFname, "earlierEmitMs", -int(period))
        # editConfig(configTemplate, exePath + configFname, "latenessMs", 0)
        print('use earlierEmit instead')
    else:
        # editConfig(configTemplate, exePath + configFname, "earlierEmitMs", 0)
        editConfig(configTemplate, exePath + configFname, "latenessMs", period)
    # editConfig(exePath+configFname,exePath+configFname,"aqpScale",aqpScale)
    # run
    os.system("cd " + exePath)
    os.system("pwd")
    os.system("chmod +x benchmark")
    os.system("./benchmark " + configFname)
    os.system("cd ../")
    # copy result
    os.system("rm -rf " + resultPath + "/" + str(period))
    os.system("mkdir " + resultPath + "/" + str(period))
    password = "Rjzhb2326090"
    sudo_command = "cd " + exePath + "&& cp *.csv " + resultPath + "/" + str(period)
    os.system('echo {} | {}'.format(password, sudo_command))


def runPeriodVector(exePath, periodVec, resultPath, templateName="config.csv"):
    for i in periodVec:
        runPeriod(exePath, i, resultPath, templateName)


def readResultPeriod(period, resultPath):
    resultFname = resultPath + "/" + str(period) + "/default_general.csv"
    avgLat = readConfig(resultFname, "AvgLatency")
    lat95 = readConfig(resultFname, "95%Latency")
    thr = readConfig(resultFname, "Throughput")
    err = readConfig(resultFname, "Error")
    aqpErr = readConfig(resultFname, "AQPError")
    return avgLat, lat95, thr, err, aqpErr


def readResultVectorPeriod(periodVec, resultPath):
    avgLatVec = []
    lat95Vec = []
    thrVec = []
    errVec = []
    compVec = []
    aqpErrVec = []
    for i in periodVec:
        avgLat, lat95, thr, err, aqpErr = readResultPeriod(i, resultPath)
        avgLatVec.append(float(avgLat) / 1000.0)
        lat95Vec.append(float(lat95) / 1000.0)
        thrVec.append(float(thr) / 1000.0)
        errVec.append(abs(float(err)))
        aqpErrVec.append(abs(float(aqpErr)))
        compVec.append(1 - abs(float(err)))
    return avgLatVec, lat95Vec, thrVec, errVec, compVec, aqpErrVec


def main():
    exeSpace = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/"
    resultPath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/aqpPeriodTest"
    resultPathNoAqp = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/aqpPeriodTest/NoAQP"
    resultPathMeanAqp = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/aqpPeriodTest/MeanAQP"
    resultPathIMA = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/aqpPeriodTest/IMA"
    figPath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/figures/"
    configTemplate = exeSpace + "config.csv"
    periodVec = [-4, -3, -2, -1, 0, 1, 2, 3, 4]
    periodVecDisp = np.array(periodVec)
    periodVecDisp = periodVecDisp
    print(configTemplate)
    # run
    if (len(sys.argv) < 2):
        os.system("rm -rf " + resultPath)
        os.system("mkdir " + resultPath)
        os.system("mkdir " + resultPathNoAqp)
        os.system("mkdir " + resultPathMeanAqp)
        os.system("mkdir " + resultPathIMA)
        runPeriodVector(exeSpace, periodVec, resultPathNoAqp, "config_noAQP.csv")
        runPeriodVector(exeSpace, periodVec, resultPathMeanAqp, "config_meanAQP.csv")
        runPeriodVector(exeSpace, periodVec, resultPathIMA, "config_IMA.csv")
    avgLatVecNo, lat95VecNo, thrVecNo, errVecNo, compVec, aqpErrVecNo = readResultVectorPeriod(periodVec,
                                                                                               resultPathNoAqp)
    avgLatVecMean, lat95VecMean, thrVecMean, errVecMean, compVec, aqpErrVecMean = readResultVectorPeriod(periodVec,
                                                                                                         resultPathMeanAqp)
    avgLatVecIMA, lat95VecIMA, thrVecIMA, errVecIMA, compVec, aqpErrVecIMA = readResultVectorPeriod(periodVec,
                                                                                                    resultPathIMA)
    # os.system("mkdir " + figPath)
    groupLine.DrawFigureYnormal([periodVec, periodVec, periodVec], [aqpErrVecNo, aqpErrVecMean, aqpErrVecIMA],
                                ['w/o AQP (lazy)', "w/ final AQP (lazy)", "w/ incremental AQP (eager)"],
                                "lateness (ms)", "Error", 0, 1, figPath + "latWm_Aqps_err", True)
    groupLine.DrawFigureYnormal([periodVec, periodVec, periodVec], [avgLatVecNo, avgLatVecMean, avgLatVecIMA],
                                ['w/o AQP (lazy)', "w/ final AQP (lazy)", "w/ incremental AQP (eager)"],
                                "lateness (ms)", "95% latency (ms)", 0, 1, figPath + "latWm_Aqps_lat", True)
    groupLine.DrawFigureYnormal([periodVec, periodVec], [aqpErrVecMean, aqpErrVecIMA],
                                ["w/ AQP (lazy)", "w/ incremental AQP (eager)"], "lateness (ms)", "Error", 0, 1,
                                figPath + "latWm_Aqps_err_incre", True)
    # draw2yLine("watermark time (ms)",periodVecDisp,lat95Vec,errVec,"95% Latency (ms)","Error","ms","",figPath+"wm_lat")
    # draw2yLine("watermark time (ms)",periodVecDisp,thrVec,errVec,"Throughput (KTp/s)","Error","KTp/s","",figPath+"wm_thr")
    # draw2yLine("watermark time (ms)",periodVecDisp,lat95Vec,compVec,"95% Latency (ms)","Completeness","ms","",figPath+"wm_omp")
    # groupLine.DrawFigureYnormal([periodVec,periodVec],[errVec,aqpErrVec],['w/o aqp',"w/ MeanAqp"],"watermark time (ms)","Error",0,1,figPath+"wm_MeanAqp",True)
    # print(errVec)
    # print(aqpErrVec)
    print(aqpErrVecIMA, aqpErrVecMean)
    # readResultPeriod(50,resultPath)


if __name__ == "__main__":
    main()

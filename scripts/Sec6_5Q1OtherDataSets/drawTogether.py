#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
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
from matplotlib import ticker
from matplotlib.ticker import LogLocator, LinearLocator

import os
import pandas as pd
import sys
from OoOCommon import *

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 22
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 22
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


def runPeriod(exePath, periodR,periodS, resultPath, configTemplate="config.csv",prefixTag="null"):
    # resultFolder="periodTests"
    configFname = "config_period_"+prefixTag + ".csv"
    # configTemplate = "config.csv"
    # clear old files
    os.system("cd " + exePath + "&& rm *.csv")
    # editConfig(configTemplate, exePath + configFname, "earlierEmitMs", 0)
    editConfig(configTemplate, exePath+"temp.csv", "fileDataLoader_rFile", periodR)
    editConfig(exePath+"temp.csv",exePath+configFname, "fileDataLoader_sFile", periodS)
    # prepare new file
    # run
    os.system("cd " + exePath + "&& ./benchmark " + configFname)
    # copy result
    os.system("rm -rf " + resultPath + "/" + str(prefixTag))
    os.system("mkdir " + resultPath + "/" + str(prefixTag))
    os.system("cd " + exePath + "&& cp *.csv " + resultPath + "/" + str(prefixTag))


def runPeriodVector (exePath,periodVec,pS,resultPath,prefixTag, configTemplate="config.csv"):
    for i in  range(len(periodVec)):
        rf=periodVec[i]
        sf=pS[i]
        print(sf)
        runPeriod(exePath, rf,sf, resultPath, configTemplate,prefixTag[i])


def readResultPeriod(period, resultPath):
    resultFname = resultPath + "/" + str(period) + "/default_general.csv"
    avgLat = readConfig(resultFname, "AvgLatency")
    lat95 = readConfig(resultFname, "95%Latency")
    thr = readConfig(resultFname, "Throughput")
    err = readConfig(resultFname, "AQPError")
    return avgLat, lat95, thr, err


def readResultVectorPeriod(periodVec, resultPath):
    avgLatVec = []
    lat95Vec = []
    thrVec = []
    errVec = []
    compVec = []
    for i in periodVec:
        avgLat, lat95, thr, err = readResultPeriod(i, resultPath)
        avgLatVec.append(float(avgLat) / 1000.0)
        lat95Vec.append(float(lat95) / 1000.0)
        thrVec.append(float(thr) / 1000.0)
        errVec.append(abs(float(err)))
        compVec.append(1 - abs(float(err)))
    return avgLatVec, lat95Vec, thrVec, errVec, compVec


def compareMethod(exeSpace, commonPathBase, resultPaths, csvTemplates, periodVec,pS,pDisplay,reRun=1):
    lat95All = []
    errAll = []
    periodAll = []
    for i in range(len(csvTemplates)):
        resultPath = commonPathBase + resultPaths[i]
        if (reRun == 1):
            os.system("rm -rf " + resultPath)
            os.system("mkdir " + resultPath)
            runPeriodVector(exeSpace, periodVec,pS, resultPath, pDisplay,csvTemplates[i])
        #exit()
        avgLatVec, lat95Vec, thrVec, errVec, compVec = readResultVectorPeriod(pDisplay, resultPath)
        
        lat95All.append(lat95Vec)
        errAll.append(errVec)
        periodAll.append(pDisplay)
    return lat95All, errAll, periodAll
def draw2yBar(NAME,R1,R2,l1,l2,fname):
    fig, ax1 = plt.subplots(figsize=(10,4)) 
    width = 0.2  # 柱形的宽度  
    x1_list = []
    x2_list = []
    bars=[]
    index = np.arange(len(NAME))
    for i in range(len(R1)):
        x1_list.append(i)
        x2_list.append(i + width)
    #ax1.set_ylim(0, 1)
    bars.append(ax1.bar(x1_list, R1, width=width, color=COLOR_MAP[0], hatch=PATTERNS[0], align='edge',edgecolor='black', linewidth=3))
    ax1.set_ylabel("ms",fontproperties=LABEL_FP)
    ax1.set_xticklabels(ax1.get_xticklabels())  # 设置共用的x轴
    ax2 = ax1.twinx()
    
    #ax2.set_ylabel('latency/us')
    #ax2.set_ylim(0, 0.5)
    bars.append(ax2.bar(x2_list, R2, width=width,  color=COLOR_MAP[1], hatch=PATTERNS[1], align='edge', tick_label=NAME,edgecolor='black', linewidth=3))
  
    ax2.set_ylabel("%",fontproperties=LABEL_FP)
    # plt.grid(axis='y', color='gray')

    #style = dict(size=10, color='black')
    #ax2.hlines(tset, 0, x2_list[len(x2_list)-1]+width, colors = "r", linestyles = "dashed",label="tset") 
    #ax2.text(4, tset, "$T_{set}$="+str(tset)+"us", ha='right', **style)
    if (1):
        plt.legend(bars, [l1,l2],
                   prop=LEGEND_FP,
                   ncol=2,
                   loc='upper center',
                   #                     mode='expand',
                   shadow=False,
                   bbox_to_anchor=(0.55, 1.45),
                   columnspacing=0.1,
                   handletextpad=0.2,
                borderaxespad=-1,
                   #                     bbox_transform=ax.transAxes,
                   #                     frameon=True,
                   #                     columnspacing=5.5,
                   #                     handlelength=2,
                   )
    plt.xlabel(NAME, fontproperties=LABEL_FP)
    plt.xticks(size=TICK_FONT_SIZE)
    ax1.yaxis.set_major_locator(LinearLocator(5))
    ax2.yaxis.set_major_locator(LinearLocator(5))
    ax1.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.1f'))
    ax2.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.1f'))
    plt.tight_layout()
    plt.savefig(fname+".pdf")

def main():
    exeSpace = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/"
    commonBasePath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/Sec6_5Otherq1Normal/"

    figPath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/figures/"
    configTemplate = exeSpace + "config.csv"
    # periodVec = [7, 8, 9, 10, 11, 12]
    periodVec = ["datasets/stock/cj_1000ms_1tLowDelayData_short.csv","datasets/rovio/rovio.csv","datasets/logistics/tr.csv","datasets/retail/tr.csv"]
    pDisplay=["Stock","Rovio","Logistics","Retail"]
    pS=["datasets/stock/sb_1000ms_1tHighDelayData_short.csv","datasets/rovio/rovio.csv","datasets/logistics/ts.csv","datasets/retail/ts.csv"]
    resultPaths = ["SHJ","PRJ","PECJ","PECJL"]
    csvTemplates = ["config_shj.csv","config_prj.csv","config_pecjSel.csv","config_pecjSelLazy.csv"]
   
    periodVecDisp = np.array(periodVec)
    periodVecDisp = periodVecDisp
    print(configTemplate)

    # run
    reRun = 0
    if (len(sys.argv) < 2):
        os.system("rm -rf " + commonBasePath)
        os.system("mkdir " + commonBasePath)
        reRun = 1


    # os.system("mkdir " + figPath)
    # print(lat95All)
    # lat95All[3]=ts
    methodTags = ["SHJ", "PRJ", "PECJ-SHJ","PECJ-PRJ"]
   
    lat95All, errAll, periodAll = compareMethod(exeSpace, commonBasePath, resultPaths, csvTemplates, periodVec,pS,pDisplay, reRun)
    print(lat95All[0][0])
    errAll=np.array(errAll)*100.0
    #draw2yBar(methodTags,[lat95All[0][0],lat95All[1][0],lat95All[2][0],lat95All[3][0]],[errAll[0][0],errAll[1][0],errAll[2][0],errAll[3][0]],'95% latency (ms)','Error (%)',figPath + "sec6_5_stock_q1_normal")
    groupBar2.DrawFigure(pDisplay, np.array(errAll), methodTags, "Datasets", "Error (%)",
                         5, 15, figPath + "sec6_5_datasets_q1_err", False)
    groupBar2.DrawFigure(pDisplay, np.array(lat95All), methodTags, "Datasets", "95% latency (ms)",
                         5, 15, figPath + "sec6_5_datasets_q1_lat", True)
    print(lat95All,errAll)
if __name__ == "__main__":
    main()

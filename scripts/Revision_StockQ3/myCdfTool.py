import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import itertools as it
import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator,LinearLocator
import os
import matplotlib.ticker as mtick
def getInfoFromCSV(a):
    #column legengd, row name
    with open(a, 'r') as f:
        reader = csv.reader(f)
        #reader = [each for each in csv.DictReader(f, delimiter=',')]
        result = list(reader)
        rows=len(result)
        #print('rows=',rows)
        firstRow = result[0]
        #print(firstRow)
        index=0
        #define what may attract our interest
        idxCpu=0
        idxName=0
        idxEng=0
        idxLat=0
        idxVio=0
        idxOve=0
        xt=[]
        yt=[]
        idxList=[]
        nameList=[]
        legendList=[]
        for i in firstRow:
            #print(i)
            if(i!='name'):
               idxt=index
               idxList.append(idxt)
            index=index+1
        idxName=0
        #get names
        for k in range(len(idxList)):
            namet=(result[0][idxList[k]])
            nameList.append(namet)
        #get legend
        for k in range(1,rows):
            legt=result[k][0]
            legendList.append(legt)
        #get results
        ruAll=[]
        ruRow=[]
        for k in range(1,rows):
            ruRow=[]
            for j in range(len(idxList)):
                rut=(result[k][idxList[j]])
                ruRow.append(rut)
            ruAll.append(ruRow)
        return nameList,legendList,ruAll
def copyCol(mat,col):
    ru=np.zeros(len(mat))
    for k in range(0,len(mat)):
        ru[k]=float(mat[k][col])
    return np.array(ru)
def getCdfProbabilities(rawData):
    #sorted_data = np.sort(rawData)

# Calculate the cumulative probabilities
    percentile=[1,10,20,30,40,50,60,70,80,90,95,100]
    ru=np.percentile(rawData, percentile)
    #cumulative_prob = np.arange(1, len(sorted_data) + 1) / len(sorted_data)
    return percentile,ru
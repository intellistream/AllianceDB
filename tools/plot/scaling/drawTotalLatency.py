#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar as groupBar
from autoParase import *
def getLatencyFromCSV(a):
    print(a)
    with open(a, 'r') as f:
        reader = csv.reader(f)
        #reader = [each for each in csv.DictReader(f, delimiter=',')]
        result = list(reader)
        rows=len(result)
        print('rows=',rows)
        firstRow = result[0]
        #print(firstRow)
        index=0
        #define what may attract our interest
        idxCpu=0
        idxName=0
        idxLatency=0
       
        for i in firstRow:
            #print(i)
            if(i=='cpu'):
               idxCpu=index
            if(i=='name'):
               idxName=index
            if(i=='latency'):
               idxLatency=index
        
            index=index+1
        #read the valid stages
        vdataEntries=0
        latencyList=[]
       
        for k in range(1,rows):
            if(result[k][idxName]=='para_all'):
               print(result[k][idxLatency])
               return int(result[k][idxLatency])
def writeCsv(fname,ar):
    with open(fname,"w") as csvfile: 
        writer = csv.writer(csvfile)
        writer.writerow(["LL","LB","BL","BB"])
        writer.writerows(ar)
def main():
  
    fileNames2=[
        'lz4_pipe2_LL_1_pe.csv',
        'lz4_pipe2_LB_1_pe.csv',
        'lz4_pipe2_BL_1_pe.csv',
        'lz4_pipe2_BB_1_pe.csv',
       
    ]
    legend=[
       #'',
       'lz4_pipe2$^{LL}$',
       'lz4_pipe2$^{LB}$',
       'lz4_pipe2$^{BL}$',
       'lz4_pipe2$^{BB}$',
    ]
    xv=['']
    yv=[]
    yv2=[]
   
    #yv.append(0)
    for i in range(len(fileNames2)):
        xt=[]
        yt=[]
        yt=getLatencyFromCSV(fileNames2[i])
        print(yt)
        yv2.append(yt)
    groupBar.DrawFigure(xv,[yv2],legend,'algorithms','latency/us',0,250000,'two_stage_maping_pipeline',True)
    accuBar.DrawFigure(legend,[yv2],['algos'],'','latency/us','lz4_pipe2_totalLatency',True,'')
    writeCsv("LAT.csv",[yv2])
    return yv2
if __name__ == "__main__":
    main()
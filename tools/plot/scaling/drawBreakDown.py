#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar as groupBar
import groupBar2 as groupBar2
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
        xt=[]
        yt=[]
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
        R1=0
        R2=0
        for k in range(1,rows):
            if(result[k][idxName]=='load+reamp '):
                R1=(int(result[k][idxLatency]))
            if(result[k][idxName]=='write'):
                R2=(int(result[k][idxLatency]))
        return R1,R2
def main():
   
    fileNames=[
        'lz4_pipe2_LL_se.csv',
        'lz4_pipe2_LB_se.csv',
        'lz4_pipe2_BL_se.csv',
        'lz4_pipe2_BB_se.csv',
       
    ]
  
    r1=[]
    r2=[]
    for i in range(len(fileNames)):
        R1,R2=getLatencyFromCSV(fileNames[i])
        r1.append(R1)
        r2.append(R2)
   
    accuBar.DrawFigure(['LL','LB','BL','BB'],([r1,r2]),['load+remap','write'],'','latency/us','lz4_pipe2_2stage_breakdown',True,'')
    groupBar2.DrawFigure(['LL','LB','BL','BB'],([r1,r2]),['load+remap','write'],0,0,'algos','latency/us','lz4_pipe2_2stage_bottleneck',1)
    ru,idx=maxInList([r1,r2])
    accuBar.DrawFigure(['LL','LB','BL','BB'],([ru]),['bottleneck'],'','latency/us','lz4_pipe2_2stage_botonly',True,'')
    return ru,['LL','LB','BL','BB']
if __name__ == "__main__":
    main()
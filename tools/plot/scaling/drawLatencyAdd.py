#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar2 as groupBar2
def getLatecnyFromCSV(a):
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
        'lz4_pipe2_LL',
        'lz4_pipe2_LB',
        'lz4_pipe2_BL',
        'lz4_pipe2_BB',
       
    ]
    paraTail='_pe.csv'
    seqTail='_se.csv'
  
    r1=[]
    r2=[]
    rp1=[]
    rp2=[]
    for i in range(len(fileNames)):
        R1,R2=getLatecnyFromCSV(fileNames[i]+paraTail)
        R3,R4=getLatecnyFromCSV(fileNames[i]+seqTail)
        d1=R1-R3
        d2=R2-R4
        r1.append(d1)
        r2.append(d2)
        rp1.append(d1)
        rp2.append(d2)
        print(R1-R3)
        print(R2-R4)
    print([rp1,rp2])
    print(rp2[1])
    accuBar.DrawFigure(['LL','LB','BL','BB'],([r1,r2]),['load+remap','write'],'','additional cycles','lz4_pipe2_2stage_increased_latency',True,'')
    accuBar.DrawPercentageFigure(['LL','LB','BL','BB'],accuBar.normalize([r1,r2]),['load+remap','write'],'','ac percentage','lz4_pipe2_2stage_increased_latency_per',True,'')
   # groupBar2.DrawFigure(['LL','LB','BL','BB'],[rp1,rp2],['load+remap','write'],0,0,'x','y','hahaha',1)
if __name__ == "__main__": 
    main()
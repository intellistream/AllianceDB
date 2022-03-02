#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar as groupBar
import groupBar2 as groupBar2
from autoParase import *
import drawTotalLatency as drawTotalLatency
import drawBreakDown as drawBreakDown
import groupLine as groupLine
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
def getLarry(fnameList,a):
    yv=[]
    yv2=[]
    for i in range(len(fnameList)):
        xt=[]
        yt=[]
        yt=getLatencyFromCSV(fnameList[i]+str(a)+'_pe.csv')
        print(yt)
        yv2.append(yt)
    return yv2
def getBLatencyFromCSV(a):
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
            if(result[k][idxName]=='load&match'):
                R1=(int(result[k][idxLatency]))
            if(result[k][idxName]=='writeToken'):
                R2=(int(result[k][idxLatency]))
        return R1,R2
def getBLArray(fnameList,a):
    r1=[]
    r2=[]
    for i in range(len(fnameList)):
        R1,R2=getBLatencyFromCSV(fnameList[i]+str(a)+'_se.csv')
        r1.append(R1)
        r2.append(R2)
    ru,idx=maxInList([r1,r2])
    return ru,idx,r1,r2

def listSub(a,b):
    ru=[]
    for i in range(len(a)):
        k=a[i]-b[i]
        ru.append(k)
    return ru
def tmain(a):
    fileNames2=[
        'lz4_pipe2_LL_',
        'lz4_pipe2_LB_',
        'lz4_pipe2_BL_',
        'lz4_pipe2_BB_',
       
    ]
    legend=[
       #'',
       '$^{LL}$',
       '$^{LB}$',
       '$^{BL}$',
       '$^{BB}$',
    ]
    total=getLarry(fileNames2,a)
    rub,idx,r1,r2=getBLArray(fileNames2,a)
    lsub=listSub(total,rub)
    idx=[
        'LL',
        'LB',
        'BL',
        'BB'
    ]
    #accuBar.DrawFigure(idx,([lsub]),['add lantency'],'','latency/us',str(a)+'_addLatency',True,'')
    return lsub,r1,r2
def tkmain(a,s0Array,s1Array,s2Array,s3Array):
    lsub,r1,r2=tmain(a)
    s0Array.append([r1[0],r2[0],9328000/a,lsub[0]])
    s1Array.append([r1[1],r2[1],9328000/a,lsub[1]])
    s2Array.append([r1[2],r2[2],9328000/a,lsub[2]])
    s3Array.append([r1[3],r2[3],9328000/a,lsub[3]])
    return s0Array,s1Array,s2Array,s3Array
def writeCsv(fname,ar):
    with open(fname,"w") as csvfile: 
        writer = csv.writer(csvfile)
        writer.writerow(["stage0","stage1","size","addLency"])
        writer.writerows(ar)
def getCol(mat,col):
    ru=[]
    if(col>=len(mat[0])):
        return ru
    for i in range(len(mat)):
        k=mat[i][col]
        ru.append(k)
    return ru
def lineFit(tag,x,y):
    z= np.polyfit(x, y, 1)
    print(tag+": y = %10.5f x + (%10.5f) " % (z[0],z[1] ))
def main():
    s0=[]
    s1=[]
    s2=[]
    s3=[]
    s0,s1,s2,s3=tkmain(1,s0,s1,s2,s3)
    s0,s1,s2,s3=tkmain(2,s0,s1,s2,s3)
    s0,s1,s2,s3=tkmain(5,s0,s1,s2,s3)
    s0,s1,s2,s3=tkmain(10,s0,s1,s2,s3)
    #s0,s1,s2,s3=tkmain(20,s0,s1,s2,s3)
    writeCsv("tao_fetch_LL.csv",s0)
    writeCsv("tao_fetch_LB.csv",s1)
    writeCsv("tao_fetch_BL.csv",s2)
    writeCsv("tao_fetch_BB.csv",s3)
    sz=getCol(s0,2)
    llArray=getCol(s0,3)
    lbArray=getCol(s1,3)
    blArray=getCol(s2,3)
    bbArray=getCol(s3,3)
    legend=[
        'LL',
        'LB',
        'BL',
        'BB',
    ]
    lineFit("LL",sz,llArray)
    lineFit("LB",sz,lbArray)
    lineFit("BL",sz,blArray)
    lineFit("BB",sz,bbArray)
    groupLine.DrawFigure([sz,sz,sz,sz] ,[llArray,lbArray,blArray,bbArray],legend,"size/byte","fetcing latency/us",0,450000,"lz4_tao_fetch",True)
    


    #tmain(50)  
if __name__ == "__main__":
    main()
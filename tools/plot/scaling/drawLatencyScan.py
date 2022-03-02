#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
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
def tmainL(a,r1,r2,sz):
   
    fileNames=[
       'lz4_pipe2_LL_'
       
    ]
    R1,R2=getLatencyFromCSV(fileNames[0]+str(a)+"_se.csv")
    r1.append(R1)
    r2.append(R2)
    sz.append(932800/a)
    return r1,r2,sz
def tmainB(a,r1,r2,sz):
   
    fileNames=[
       'lz4_pipe2_BB_'
       
    ]
    R1,R2=getLatencyFromCSV(fileNames[0]+str(a)+"_se.csv")
    r1.append(R1)
    r2.append(R2)
    sz.append(932800/a)
    return r1,r2,sz

def main():
    r1=[]
    r2=[]
    sz=[]
    #r1,r2,sz=tmainL(50,r1,r2,sz)
    r1,r2,sz=tmainL(20,r1,r2,sz)
    r1,r2,sz=tmainL(10,r1,r2,sz)
    r1,r2,sz=tmainL(5,r1,r2,sz)
    r1,r2,sz=tmainL(2,r1,r2,sz)
    r1,r2,sz=tmainL(1,r1,r2,sz)
    legend=[
        'L-load&match',
        'L-writeToken',
        'B-load&match',
        'B-writeToken',
    ]
    print(r1)
    print(r2)
   
    rb1=[]
    rb2=[]
    szb=[]
    #r1,r2,sz=tmainL(50,r1,r2,sz)
    rb1,rb2,szb=tmainB(20,rb1,rb2,szb)
    rb1,rb2,szb=tmainB(10,rb1,rb2,szb)
    rb1,rb2,szb=tmainB(5,rb1,rb2,szb)
    rb1,rb2,szb=tmainB(2,rb1,rb2,szb)
    rb1,rb2,szb=tmainB(1,rb1,rb2,szb)
    groupLine.DrawFigure([sz,sz,szb,szb] ,[r1,r2,rb1,rb2],legend,"size/byte","latency/us",0,200000,"lz4_tao_run",True)
    lmL= np.polyfit(sz,r1, 1)
    print("L-load&match: y = %10.5f x + %10.5f " % (lmL[0],lmL[1]) )
    lmL= np.polyfit(sz,r2, 1)
    print("L-writeToken: y = %10.5f x + %10.5f " % (lmL[0],lmL[1]) )
    lmL= np.polyfit(sz,rb1, 1)
    print("b-load&match: y = %10.5f x + %10.5f " % (lmL[0],lmL[1]) )
    lmL= np.polyfit(sz,rb2, 1)
    print("b-writeToken: y = %10.5f x + %10.5f " % (lmL[0],lmL[1]) )
if __name__ == "__main__":
    main()
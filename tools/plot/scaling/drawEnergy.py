#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar2 as groupBar2
def getAllNeedFromCSV(a):
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
        idxEnergy=0
        idxPeakPower=0
        for i in firstRow:
            #print(i)
            if(i=='cpu'):
               idxCpu=index
            if(i=='name'):
               idxName=index
            if(i=='latency'):
               idxLatency=index
            if(i=='energy'):
               idxEnergy=index
            if (i=='peakPower'):
                idxPeakPower=index
            index=index+1
        #read the valid stages
        vdataEntries=0
        latencyList=[]
       
        for k in range(1,rows):
            if(result[k][idxName]=='para_all'):
               print(result[k][idxLatency])
               return int(result[k][idxLatency]),float(result[k][idxEnergy]),float(result[k][idxPeakPower])
def writeCsv(fname,ar):
    with open(fname,"w") as csvfile: 
        writer = csv.writer(csvfile)
        writer.writerow(["LL","LB","BL","BB"])
        writer.writerows(ar)
def main():
    fileNames=[
        'lz4_pipe2_LL_1_pe.csv',
        'lz4_pipe2_LB_1_pe.csv',
        'lz4_pipe2_BL_1_pe.csv',
        'lz4_pipe2_BB_1_pe.csv',
       
    ]
   
    legend=[
       '$^{LL}$',
       '$^{LB}$',
       '$^{BL}$',
       '$^{BB}$',
    
    ]
    xv=['']
    ev=[]
    pv=[]
    env=[]
    yv2=[]
    lt=[]
    for i in range(len(fileNames)):
        l,e,peak=getAllNeedFromCSV(fileNames[i])
        #print(yt)
        x=e-l*5*0.62*1e-6
        lt.append(l)
        ev.append(e)
        pv.append(peak)
        env.append(x)
    accuBar.DrawFigure(legend,([pv]),['all'],'','ppwer/mW','lz4_pipe2_2stage_peakPower',True,'')
    groupBar2.DrawFigure(legend,([ev,env]),['all','net'],'algos','energy/J',0,0,'lz4_pipe2_2stage_allEnergy',1)
    writeCsv("NET_ENG.csv",[env])
    print(lt,ev,env)
    return env
if __name__ == "__main__":
    main()

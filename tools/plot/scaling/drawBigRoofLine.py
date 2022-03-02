#!/usr/bin/env python3
import groupLine as groupLine
import csv
def readRoofLineFromCSV(a):
    print('load'+a)
    with open(a, 'r') as f:
        reader = csv.reader(f)
        #reader = [each for each in csv.DictReader(f, delimiter=',')]
        result = list(reader)
        rows=len(result)
        print('rows=',rows)
        firstRow = result[0]
        print(firstRow)
        index=0
        #define what may attract our interest
        idxMflops=0
        idxIPM=0
        idxTime=0
        idxEnergy=0
        idxIns=0
        for i in firstRow:
            #print(i)
            if(i=='mfops'):
               idxMflops=index
            if(i=='IPM'):
               idxIPM=index
            if(i=='time'):
                idxTime=index
            if(i=='energy'):
                idxEnergy=index
            if(i=='instructions'):
                idxIns=index
            index=index+1
            


        #read the valid stages
        vdataEntries=0
        mflopsList=[]
        ipmList=[]
        energyList=[]
        netEnergyList=[]
        insList=[]
        for k in range(1,rows):
            #print(result[k][idxMflops])
          
            if(result[k][1]!='NA'):
                if(k>0):
                    vdataEntries+=1
                    mflopsList.append(int(result[k][idxMflops]))
                    ipmList.append(int(result[k][idxIPM]))
                    energyList.append(float(result[k][idxEnergy]))
                    netEnergyList.append(float(result[k][idxEnergy])-float(result[k][idxTime])*5*0.63*1e-6)
                    insList.append(float(result[k][idxIns])/1000000.0)
        return ipmList, mflopsList,energyList,netEnergyList,insList
def divideList(a,b):
    rul=[]
    for i in range(len(a)):
        ru=float(a[i])/float(b[i])
        rul.append(ru)
    return rul
def main():
    fileNames=[
        'core51800000k_rl.csv',
        'core51608000k_rl.csv',
        'core51416000k_rl.csv',
        'core51200000k_rl.csv',
        'core51008000k_rl.csv',
        'core5816000k_rl.csv',
        'core5600000k_rl.csv',
        'core5408000k_rl.csv',
       
    ]
    legend=[
        'B-1.8',
        'B-1.6',
        'B-1.416',
        'B-1.2',
        'B-1.008',
        'B-0.816',
        'B-0.6',
        'B-0.408',
    ]
    xv=[]
    yv=[]
    ev=[]
    nv=[]
    ipjv=[]
    for i in range(len(fileNames)):
        xt=[]
        yt=[]
        et=[]
        nt=[]
        xt,yt,et,nt,it=readRoofLineFromCSV(fileNames[i])
        xv.append(xt)
        yv.append(yt)
        ev.append(et)
        nv.append(nt)
        ipjv.append(divideList(it,nt))
    print(nv)
    groupLine.DrawFigure(xv,yv,legend,"IPM","MINTOPS",0,5000,"Roof_line_of_BigCore",True)
    groupLine.DrawFigureYnormal(xv,nv,legend,"IPM","Energy/J",0,4,"energy_line_of_BigCore",True)
    print(ipjv)
    groupLine.DrawFigure(xv,ipjv,legend,"IPM","MINTO per J",0,6000,"efficiency_line_of_BigCore",True)
if __name__ == "__main__":
    main()
    
       
    

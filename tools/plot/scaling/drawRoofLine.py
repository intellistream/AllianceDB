import csv
import numpy as np
import matplotlib.pyplot as plt
import groupLine as groupLine
def drawRoofLineFromCSV(a,title,fname):
    print(a)
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
       
        for i in firstRow:
            #print(i)
            if(i=='mfops'):
               idxMflops=index
            if(i=='IPM'):
               idxIPM=index
            index=index+1
        #read the valid stages
        vdataEntries=0
        mflopsList=[]
        ipmList=[]
       
        for k in range(1,rows):
            #print(result[k][idxMflops])
            print(result[k][idxIPM])
            if(result[k][1]!='NA'):
                vdataEntries+=1
                mflopsList.append(int(result[k][idxMflops]))
                ipmList.append(int(result[k][idxIPM]))
              
        print('valid entries=',vdataEntries)
        x_values=[ipmList]
        y_values=[mflopsList]
        legend=['roofLine']
        x_label='IPM'
        y_label='MFlops'
        groupLine.DrawFigure(x_values,y_values,legend,x_label,y_label,0,8000,fname,0)

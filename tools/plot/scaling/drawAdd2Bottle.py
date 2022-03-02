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
def listSub(a,b):
    ru=[]
    for i in range(len(a)):
        k=a[i]-b[i]
        ru.append(k)
    return ru
def main():
    total=drawTotalLatency.main()
    ru,idx=drawBreakDown.main()
    lsub=listSub(total,ru)
    accuBar.DrawFigure(idx,([lsub]),['add lantency'],'','latency/us','tcomp32_2stage_addLatency',True,'')
if __name__ == "__main__":
    main()
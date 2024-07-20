# import pandas as pd
import random
import numpy as np
import sys

out_fstr = ['./Sep_85.txt', './Sep_86.txt']
in_fstr = ['./SEP85L', './SEP86L']

for i in [0,1]:
	out_ln = []
	with open(in_fstr[i], 'r') as fp:
		lns = fp.readlines()
		
		ts_w = np.random.randint(0,100,size=1000)
		ts_buc = []
		for ind, k in enumerate(ts_w):
			for j in range(k):
				ts_buc.append(ind)
		ts = np.random.randint(0,len(ts_buc),size=len(lns))
  
		for k in range(len(ts)):
			ts[k] = ts_buc[ts[k]]
		ts = list(ts)
		ts.sort()
		
		for ind, j in enumerate(lns):
			# k = j[19:24].strip()
			k = ((int(j[9:14].strip())+9000)//int(sys.argv[1]))  * 3600 + (int(j[14:19].strip()) // int(sys.argv[2]))
			out_ln.append(str(k) + '|' + str(ts[ind]) + '\n')
   
	with open(out_fstr[i], 'w') as fp:
		fp.writelines(out_ln)

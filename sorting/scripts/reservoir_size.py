import sys, getopt

opt, arg = getopt.getopt(sys.argv[1:], "e:r:s:n:")
optdict = {}
for i in opt:
	optdict[i[0].strip('-')] = float(i[1])

res = optdict['e']*(optdict['r']+optdict['s'])/(2*optdict['n'])
print(int(res), end='')
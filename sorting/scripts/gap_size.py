import sys, getopt

opt, arg = getopt.getopt(sys.argv[1:], "e:m:")
optdict = {}
for i in opt:
	optdict[i[0].strip('-')] = float(i[1])

res = optdict['m']*optdict['e']*optdict['e']/1000

print(int(res), end='')
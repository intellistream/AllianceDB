import sys, getopt
opt, arg = getopt.getopt(sys.argv[1:], "e:r:s:n:")
# print(opt)
# print(type(opt))
optdict = {}
for i in opt:
	optdict[i[0].strip('-')] = float(i[1])

# print(optdict)

res = optdict['e']*(optdict['r']+optdict['s'])/(2*optdict['n'])

# print(res)
print(int(res), end='')
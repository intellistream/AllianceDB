import sys, getopt
opt, arg = getopt.getopt(sys.argv[1:], "e:m:")
# print(opt)
# print(type(opt))
optdict = {}
for i in opt:
	optdict[i[0].strip('-')] = float(i[1])

# print(optdict)

res = optdict['m']*optdict['e']*optdict['e']/1000

# print(res)
print(int(res), end='')
import itertools as it
import os

import numpy as np

import re
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from pandas import reset_option
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator
from matplotlib.ticker import MultipleLocator
from scipy import optimize

OPT_FONT_NAME = 'Helvetica'
# TICK_FONT_SIZE = 24
TICK_FONT_SIZE = 16
LABEL_FONT_SIZE = 18
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = 16

MARKERS = ([
# 'o', 's', 'v', "^", "h", "v", ">", "X", "",
'o', 's', 'v', "^", "h", "v", ">", "X", "d", "<", ">",  "X", "d", "<"]
)
# you may want to change the color map for different figures
# COLOR_MAP = ('#000000', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#000000', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
COLOR_MAP = (
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#1c7d7e', '#269091',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#1c7d7e', '#269091',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#1c7d7e', '#269091',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#1c7d7e', '#269091',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF',

)

#'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
#'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
#'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
#'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
#'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
#'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
#'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
#'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
#'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
#'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
#'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF',

# you may want to change the patterns for different figures
# PATTERNS = (["", "//", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "//", ".", "|||", "o", "---", "+", "\\\\", "*"])
PATTERNS = [ 
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
]
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 15.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/home/tangxilin/S-AllianceDB/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'

def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LEGEND_FP = FontProperties(style='normal', size=26)
    figlegend = pylab.figure(figsize=(16, 0.5))
    bars = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    width = 0.3
    for i in range(len(FIGURE_LABEL)):
        bars[i] = ax1.bar(x_values, data, width,
                          hatch=PATTERNS[i],
                          color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i],
                          edgecolor='black', linewidth=3)

    # LEGEND

    figlegend.legend(bars, FIGURE_LABEL, prop=LEGEND_FP, \
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=True, \
                     frameon=True, handlelength=2, handletextpad=0.3, columnspacing=0.5,
                     borderaxespad=-0.2, fancybox=True
                     )
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')

# draw a bar chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(5, 4))
    # fig = plt.figure(figsize=(5, 3))
    figure = fig.add_subplot(111)

    # FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    # index = np.arange(len(x_values))
    index = np.arange(1)
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.2
    # draw the bars
    # bars = [None] * (len(x_values))
    # for i in range(len(y_values)):
    #     bars[i] = plt.bar(index + i * width + width / 2,
    #                       y_values[i], width,
    #                       hatch=PATTERNS[i],
    #                       color=LINE_COLORS[i],
    #                       label=FIGURE_LABEL[i],edgecolor='black', linewidth=3)
    # for i in range(len(y_values)):
    #     bars[i] = plt.bar(index + i * width + width / 2,
    #                       y_values[i], width,
    #                       hatch=PATTERNS[i],
    #                       color=LINE_COLORS[i],
    #                       edgecolor='black', linewidth=3)
    # sometimes you may not want to draw legends.
    # if allow_legend == True:
    #     plt.legend(bars, FIGURE_LABEL,
    #                prop=LEGEND_FP,
    #                ncol=4,
    #                loc='upper center',
    #                #                     mode='expand',
    #                shadow=False,
    #                bbox_to_anchor=(0.45, 1.6),
    #                columnspacing=0.1,
    #                handletextpad=0.2,
    #                #                     bbox_transform=ax.transAxes,
    #                #                     frameon=True,
    #                #                     columnspacing=5.5,
    #                #                     handlelength=2,
    #                )
    # you may need to tune the xticks position to get the best figure.
    # index = np.arange(len(x_values))
    # plt.xticks(index * width + width/2, x_values, rotation = 30)
    # lala = []
    # for i in y_values:
    #     if (isinstance(i, float)):
    #         lala.append(i)
    #     else:
    #         lala.append(0.0)
    # plt.yticks(lala,lala)
    # plt.ticklabel_format(axis="y", style="sci", scilimits=(0, 0))
    plt.grid(axis='x', color='gray', ls='--', alpha=0.3)
    plt.grid(axis='y', color='gray', ls='--', alpha=0.3)
    # figure.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())

    # for idx, x in enumerate(x_values):
    for idx in range(len(x_values)-1, -1, -1):
        # if idx != 4 and idx != 9 and idx != 12 and idx != 15 :
        # print(x_values[idx])
        # print(y_values[idx])

        # print(idx)
        # print(x_values[idx], y_values[idx])
        plt.scatter(x_values[idx], y_values[idx], c = LINE_COLORS[idx], marker = MARKERS[idx])

    # you may need to tune the xticks position to get the best figure.
    plt.yscale('log')
    #
    # plt.grid(axis='y', color='gray')
    # figure.yaxis.set_major_locator(LogLocator(base=10, subs=(1.0,1.5,1.7,5.0,) ))
    figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.yaxis.set_major_locator(MultipleLocator(base=10))
    # figure.xaxis.set_major_locator(LinearLocator(5))
    # figure.get_xaxis().set_tick_params(direction='in', pad=10)
    # figure.get_yaxis().set_tick_params(direction='in', pad=10)

    # plt.yscale('log')
    # plt.xscale('log')
    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)
    # plt.show()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')

def Get_Lat(file_name):
    res = 1.
    with open(file_name, "r") as f:
        fc = f.read()
        res = float(re.findall(r'95th latency: \((.*?)\)', fc)[0])
    return res


def Cal_Var(x, *arg):
    # print(arg[0])
    # print('lalas')
    # print((np.array(arg[0])*x-87856849382)**2/len(arg[0]))
    return np.sum( ((np.array(arg[0])*x-87856849382)/87856849382)**2)/len(arg[0])

grp_sz = 50
Datastr = []

# example for reading csv file
def ReadFile_latency_avg():
    
    res = []
######## non-sample
    sort_non_smp_lz = ['m-way','m-pass']
    for algo in sort_non_smp_lz:
        col = []
        # for gp in range(2100, 2100 + (9+1)*15, 15):
        
        if Datastr[0] == 'DEBS_':
            tmp = []
            for gp in range(21000, 21000 + (9)*grp_sz, grp_sz):
                tmp = []
                for i in range(grp_sz):
                    file = exp_dir + '/results/breakdown/' + Datastr[0] + algo + '_profile_' + str(gp + i) + Datastr[1]+ '.txt'
                    tmp.append(Get_Lat(file))
                col.append(np.mean(tmp))
            col.append(np.mean(tmp))
            res.append(col)
        else:
            for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
                tmp = []
                for i in range(grp_sz):
                    file = exp_dir + '/results/breakdown/' + Datastr[0] + algo + '_profile_' + str(gp + i) + Datastr[1]+ '.txt'
                    tmp.append(Get_Lat(file))
                col.append(np.mean(tmp))
            res.append(col)
        
    hash_non_smp_lz = ['NPO','PRO']
    for algo in hash_non_smp_lz:
        col = []
        for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
            tmp = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp + i) + Datastr[1]+ '.txt'
                tmp.append(Get_Lat(file))
            col.append(np.mean(tmp))
        res.append(col)

    hash_non_smp_eg = ['PMJ_JM_NP', 'PMJ_JBCR_NP', 'SHJ_JM_NP', 'SHJ_JBCR_NP', ]
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
            tmp = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp + i) + Datastr[1]+ '.txt'
                tmp.append(Get_Lat(file))
            col.append(np.mean(tmp))
        res.append(col)

######### PROBE ALL
    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(26000, 26000 + (9+1)*grp_sz, grp_sz):
            tmp = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp + i) + Datastr[1]+ '.txt'
                tmp.append(Get_Lat(file))
            col.append(np.mean(tmp))
        res.append(col)
            
########### sraj

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(24000, 24000 + (9+1)*grp_sz, grp_sz):
            tmp = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp + i) + Datastr[1]+ '.txt'
                tmp.append(Get_Lat(file))
            col.append(np.mean(tmp))
        res.append(col)

########### reserv PROBE_ALL

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(28000, 28000 + (9+1)*grp_sz, grp_sz):
            tmp = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp + i) + Datastr[1]+ '.txt'
                tmp.append(Get_Lat(file))
            col.append(np.mean(tmp))
        res.append(col)

    return res

def ReadFile_latency():
    
    res = []
######## non-sample
    sort_non_smp_lz = ['m-way','m-pass']
    for algo in sort_non_smp_lz:
        col = []
        if Datastr[0] == 'DEBS_':
            file = ''
            for gp in range(21000, 21000 + (9)*grp_sz, grp_sz):
                file = exp_dir + '/results/breakdown/' + Datastr[0] + algo + '_profile_' + str(gp) + Datastr[1]+ '.txt'
                col.append(Get_Lat(file))
            col.append(Get_Lat(file))
            res.append(col)
        else:
            for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
                file = exp_dir + '/results/breakdown/' + Datastr[0] + algo + '_profile_' + str(gp) + Datastr[1]+ '.txt'
                col.append(Get_Lat(file))
            res.append(col)

    hash_non_smp_lz = ['NPO','PRO']
    for algo in hash_non_smp_lz:
        col = []
        for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp) + Datastr[1]+ '.txt'
            col.append(Get_Lat(file))
        res.append(col)

    hash_non_smp_eg = ['PMJ_JM_NP', 'PMJ_JBCR_NP', 'SHJ_JM_NP', 'SHJ_JBCR_NP', ]
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp) + Datastr[1]+ '.txt'
            col.append(Get_Lat(file))
        res.append(col)

######### PROBE ALL
    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(26000, 26000 + (9+1)*grp_sz, grp_sz):
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp) + Datastr[1]+ '.txt'
            col.append(Get_Lat(file))
        res.append(col)
            
########### sraj

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(24000, 24000 + (9+1)*grp_sz, grp_sz):
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp) + Datastr[1]+ '.txt'
            col.append(Get_Lat(file))
        res.append(col)

########### reserv PROBE_ALL

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(28000, 28000 + (9+1)*grp_sz, grp_sz):
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp) + Datastr[1]+ '.txt'
            col.append(Get_Lat(file))
        res.append(col)

    return res



def Get_Join_Size(file_name):
    res = 1.
    with open(file_name, "r") as f:
        fc = f.read()
        res = float(re.findall(r'Results = (.*?). DONE.', fc)[0])
    return res

def ReadFile_variance():
    res = []
######## non-sample
    sort_non_smp_lz = ['m-way','m-pass']
    for algo in sort_non_smp_lz:
        col = []
        if Datastr[0] == 'DEBS_':
            varn = []
            for gp in range(21000, 21000 + (9)*grp_sz, grp_sz):
                varn = []
                for i in range(grp_sz):
                    file = exp_dir + '/results/breakdown/' + Datastr[0] + algo + '_profile_' + str(gp+i) + Datastr[1]+ '.txt'
                    varn.append(Get_Join_Size(file))
                # col.append(np.std(varn, ddof = 0) / np.mean(varn))
                col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
            col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
        else:
            for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
                varn = []
                for i in range(grp_sz):
                    file = exp_dir + '/results/breakdown/' + Datastr[0] + algo + '_profile_' + str(gp+i) + Datastr[1]+ '.txt'
                    varn.append(Get_Join_Size(file))
                # col.append(np.std(varn, ddof = 0) / np.mean(varn))
                col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
        res.append(col)
        
    hash_non_smp_lz = ['NPO','PRO']
    for algo in hash_non_smp_lz:
        col = []
        for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
            varn = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp+i) + Datastr[1]+ '.txt'
                varn.append(Get_Join_Size(file))
            # col.append(np.std(varn, ddof = 0) / np.mean(varn))
            col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
        res.append(col)

    hash_non_smp_eg = ['PMJ_JM_NP', 'PMJ_JBCR_NP', 'SHJ_JM_NP', 'SHJ_JBCR_NP', ]
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(21000, 21000 + (9+1)*grp_sz, grp_sz):
            varn = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp+i) + Datastr[1]+ '.txt'
                varn.append(Get_Join_Size(file))
            # col.append(np.std(varn, ddof = 0) / np.mean(varn))
            col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
        res.append(col)

######### PROBE ALL
    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(26000, 26000 + (9+1)*grp_sz, grp_sz):
            varn = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp+i) + Datastr[1]+ '.txt'
                varn.append(Get_Join_Size(file))
            # col.append(np.std(varn, ddof = 0) / np.mean(varn))
            col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
        res.append(col)
            
########### sraj

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(24000, 24000 + (9+1)*grp_sz, grp_sz):
            varn = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp+i) + Datastr[1]+ '.txt'
                varn.append(Get_Join_Size(file))
            # col.append(np.std(varn, ddof = 0) / np.mean(varn))
            col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
        res.append(col)

########### reserv PROBE_ALL

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for algo in hash_non_smp_eg:
        col = []
        for gp in range(28000, 28000 + (9+1)*grp_sz, grp_sz):
            varn = []
            for i in range(grp_sz):
                file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(gp+i) + Datastr[1]+ '.txt'
                varn.append(Get_Join_Size(file))
            # col.append(np.std(varn, ddof = 0) / np.mean(varn))
            col.append(Cal_Var(optimize.fmin(Cal_Var, 1, args=tuple([varn])), varn))
        res.append(col)

    return res

if __name__ == "__main__":
    # x_values = ["Stock", "Rovio", "YSB", "DEBS"]

    for Datastr in [ ['DEBS_', '_41']]:
    # for Datastr in [['Stock_', '_38'], ['DEBS_', '_41'], ['Rovio_', '_39']]:
    # for Datastr in [ ['Rovio_', '_39']]:
        x_values = ReadFile_latency_avg()
        x_values = np.array(x_values) / np.max(x_values)
        y_values = ReadFile_variance()
        with open(Datastr[0]+'variance.txt', "w") as f:
            f.write(str(y_values))

    # y_values = np.array(y_values)/np.max(y_values)
    # y_values = np.array(y_values)*np.array(y_values)

    # print(x_values)
    # print(len(x_values))
    # print(y_values)
    # print(len(y_values))
    # legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
    #                  'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    legend_labels = ['S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'B-SHJ$^{JM}$', 'B-SHJ$^{JB}$', 
                     'S-RSHJ$^{JM}$', 'S-RSHJ$^{JB}$',
                     'B-RSHJ$^{JM}$', 'B-RSHJ$^{JB}$',]
    
    DrawFigure(x_values, y_values, legend_labels,
               'Normalized 95th latency', 'Variance', 0,
               400, 'latency_variance_tradeoff', False)

    # DrawLegend(legend_labels, 'latency_legend')

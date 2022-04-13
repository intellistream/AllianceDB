from audioop import reverse
import getopt
import os
import sys
from math import ceil

import numpy as np

import matplotlib
import matplotlib as mpl
from matplotlib.ticker import PercentFormatter, LogLocator
from numpy import double
from numpy.ma import arange

mpl.use('Agg')

import matplotlib.pyplot as plt
import pylab
from matplotlib.font_manager import FontProperties

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 24
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

# MARKERS = (["", 'o', 's', 'v', "^", "", "h", "v", ">", "x", "d", "<", "|", "", "+", "_"])
MARKERS = ([
'o', 's', 'v', "^", "h", "v", ">", "X", "",
'o', 's', 'v', "^", "h", "v", ">", "X", "d", "<", ">",  "X", "d", "<"]
)
# you may want to change the color map for different figures
# COLOR_MAP = ('#000000', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#000000', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
COLOR_MAP = ('#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF',
)
# you may want to change the patterns for different figures
# PATTERNS = (["", "//", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "//", ".", "|||", "o", "---", "+", "\\\\", "*"])
PATTERNS = [ "//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
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
MARKER_SIZE = 10.0
MARKER_FREQUENCY = 1000

mpl.rcParams['ps.useafm'] = True
mpl.rcParams['pdf.use14corefonts'] = True
mpl.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
mpl.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
mpl.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/home/tangxilin/S-AllianceDB/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'

# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


def getmaxts(id):
    ts = 0

    file = exp_dir + '/results/timestamps/PRJ_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x

    file = exp_dir + '/results/timestamps/NPJ_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x

    file = exp_dir + '/results/timestamps/MPASS_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x

    file = exp_dir + '/results/timestamps/MWAY_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x

    file = exp_dir + '/results/timestamps/SHJ_JM_NP_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x

    file = exp_dir + '/results/timestamps/SHJ_JBCR_NP_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x

    file = exp_dir + '/results/timestamps/PMJ_JM_NP_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x

    file = exp_dir + '/results/timestamps/PMJ_JBCR_NP_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    if (x > ts):
        ts = x
    return ts


def getCount(id):
    file = exp_dir + '/results/timestamps/PRJ_{}.txt'.format(1, id)
    f = open(file, "r")
    read = f.readlines()
    return len(read)

def CalArray(file_name):
    f = open(file_name, "r")
    read = f.readlines()
    print(len(read))
    gap = len(read)/200

    col = []
    for i, r in enumerate(read):
        # print(i, gap)
        j=int(i*gap)
        if j >= len(read):
            break
        else:
            l = read[j]
            value = double(l.strip("\n"))  # timestamp.
            col.append(value)

    # calculate the proportional values of samples
    print(len(col))
    coly = 1. * arange(len(col)) / (len(col) - 1)
    return col, coly

# example for reading csv file
def ReadFile(id):
    x_axis = []
    y_axis = []
    # id = 39
    empty_col = []
    empty_coly = []
    '''
        if (i % gap == 0):
            value = double(r.strip("\n"))  # timestamp.
            col.append(value)
            empty_col.append(0)
            empty_coly.append(0)
        
    for i, r in enumerate(read):
        # print(i, gap)
        j=int(i*gap)
        if j >= len(read):
            break
        else:
            l = read[j]
            value = double(l.strip("\n"))  # timestamp.
            col.append(value)
    '''
# PMJ_JM_NP
# PMJ_JBCR_NP
# SHJ_JM_NP
# SHJ_JBCR_NP
    task = ['MWAY', 'MPASS', 'NPJ', 'PRJ', 'PMJ_JM_NP', 'PMJ_JBCR_NP', 'SHJ_JM_NP', 'SHJ_JBCR_NP', 
    'SHJ_JM_NP', 'SHJ_JBCR_NP','SHJ_JM_NP', 'SHJ_JBCR_NP','SHJ_JM_NP', 'SHJ_JBCR_NP',
    ]
    if id == 'AR':
        0-4
    else:
        10-14
    gp = [4100]*2 + []*6 + [400]*2 + [300]*2 + [500]*2
    print(gp)
    for i in range(len(task)):
        if (gp[i]==''):
            x_axis.append([0]*500)
            y_axis.append([0]*500)
            continue
        # print(task[i])
        col, coly = CalArray(exp_dir + '/results/timestamps/' + str(task[i]) + str(gp[i]) + '_' + str(id) + '.txt')
        x_axis.append(col)
        y_axis.append(coly)

    return x_axis, y_axis

def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LINE_WIDTH = 8.0
    MARKER_SIZE = 20.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(50, 0.5))
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    idx = 0
    for group in range(len(FIGURE_LABEL)):
        if (idx == 8): #add a space between lazy and eager.
            lines[idx], = ax1.plot(x_values, data,
                                   color=LINE_COLORS[idx], linewidth=0,
                                   marker=MARKERS[idx], markersize=0, label=str(group),
                                   markeredgewidth=0, markeredgecolor='k'
                                   )
        else:
            lines[idx], = ax1.plot(x_values, data,
                                   color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                                   marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(group),
                                   markeredgewidth=1,  markeredgecolor='k'
                                   )

        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=True, handlelength=1.2, handletextpad=0.3, columnspacing=0.5,
                     borderaxespad=-0.2, fancybox=False)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


# draw a line chart
def DrawFigure(xvalues, yvalues, legend_labels, x_label, y_label, x_min, x_max, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(10, 3))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    x_values = xvalues
    y_values = yvalues
    print("mark gap:", ceil(x_max / 6))
    lines = [None] * (len(FIGURE_LABEL))
    # for i in range(len(y_values)):
    for i in range(len(y_values)-1, -1, -1):
        # print(i, len(LINE_COLORS), len(FIGURE_LABEL), len(x_values), len(x_values[i]))
        # print((x_values[i]))
        # print(i, len(LINE_COLORS), len(FIGURE_LABEL), len(x_values), len(x_values[i]),x_values[i][1])
        if (i == 8):
            lines[i] = figure.plot(x_values[i], y_values[i], color='white', \
                                   linewidth=0, marker='None', \
                                   markersize=0, label=FIGURE_LABEL[i],
                                   markevery=100, markeredgewidth=0, markeredgecolor='k'
                                   )
        else:
            lines[i] = figure.plot(x_values[i], y_values[i], color=LINE_COLORS[i], \
                                   linewidth=LINE_WIDTH, marker=MARKERS[i], \
                                   markersize=MARKER_SIZE, label=FIGURE_LABEL[i],
                                   markevery=49, markeredgewidth=1, markeredgecolor='k'
                                   )
        # print('')
        # print(x_values[i][190:200])
        print(x_values[i][10])
    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(lines,
                   FIGURE_LABEL,
                   prop=LEGEND_FP,
                   loc='upper center',
                   ncol=3,
                   #                     mode='expand',
                   bbox_to_anchor=(0.55, 1.5), shadow=False,
                   columnspacing=0.1,
                   frameon=True, borderaxespad=0.0, handlelength=1.5,
                   handletextpad=0.1,
                   labelspacing=0.1)

    plt.xscale('log')
    # plt.xticks(x_values)
    # you may control the limits on your own.
    plt.xlim(left=0)
    # plt.ylim(y_min, y_max)
    plt.grid(axis='y', color='gray')

    figure.yaxis.set_major_formatter(PercentFormatter(1.0))
    # figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_locator(matplotlib.ticker.FixedFormatter(["0.25", "0.5", "0.75", "1"]))
    # figure.xaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
    # figure.xaxis.set_major_locator(pylab.LinearLocator(6))
    figure.xaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_formatter(matplotlib.ticker.ScalarFormatter())
    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')
    # ConvertEpsToPdf(FIGURE_FOLDER + "/" + filename)

if __name__ == "__main__":

    # id = 39
    # try:
    #     opts, args = getopt.getopt(sys.argv[1:], '-i:h', ['test id', 'help'])
    # except getopt.GetoptError:
    #     print('progressive_figure.py -id testid')
    #     sys.exit(2)
    # for opt, opt_value in opts:
    #     if opt in ('-h', '--help'):
    #         print("[*] Help info")
    #         exit()
    #     elif opt == '-i':
    #         print('Test ID:', opt_value)
    #         id = (int)(opt_value)

    # legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
    #                  'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    legend_labels = ['MWAY', 'MPASS','NPJ', 'PRJ', 
                     'PMJ$^{JM}$', 'PMJ$^{JB}$','SHJ$^{JM}$', 'SHJ$^{JB}$', 
                     '', 'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'B-SHJ$^{JM}$', 'B-SHJ$^{JB}$', 
                     'RSHJ$^{JM}$', 'RSHJ$^{JB}$',
                     'B-RSHJ$^{JM}$', 'B-RSHJ$^{JB}$',
                    ]
    # legend_labels = ['', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
    #                  '', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$',
    #                  '', 'S-NPJ', 'S-PRJ', 'S-MWAY', 'S-MPASS',
    #                  '', 'S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$',
    #                  '', 'PROBHASH$^{JM}$', 'PROBHASH$^{JB}$',
    #                  '', 'RSHJ$^{JM}$', 'RSHJ$^{JB}$',
    #                  '', 'PROBALL$^{JM}$', 'PROBALL$^{JB}$',]
                     

    # ts = ceil(getmaxts(id) / 100) * 100
    # print("maximum timestamp:", ts)
    for id in ['AR', 'AD']:
    # for id in [39]:
    # for id in [38,39,41]:
        x_axis, y_axis = ReadFile(id)
        legend = False
        DrawFigure(x_axis, y_axis, legend_labels,
                'elapsed time (msec)', 'cumulative percent', 0, getCount(id),
                1, 0,
                'progressive_sample_app_figure' + str(id),
                legend)

    DrawLegend(legend_labels, 'progressive_legend')

import getopt
import os
import sys

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator
from numpy import double

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 24
LEGEND_FONT_SIZE = 20
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "|", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (["\\", "///", "o", "||", "\\\\", "\\\\", "//////", "//////", ".", "\\\\\\", "\\\\\\"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 0.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME

FIGURE_FOLDER = '/data1/xtra/results/figure'


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


# draw a line chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(9, 3))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    index = np.arange(len(x_values))
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.5
    # draw the bars
    bottom_base = np.zeros(len(y_values[0]))
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i], bottom=bottom_base, edgecolor='black', linewidth=3)
        bottom_base = np.array(y_values[i]) + bottom_base

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(bars, FIGURE_LABEL
                   #                     mode='expand',
                   #                     shadow=False,
                   #                     columnspacing=0.25,
                   #                     labelspacing=-2.2,
                   #                     borderpad=5,
                   #                     bbox_transform=ax.transAxes,
                   #                     frameon=False,
                   #                     columnspacing=5.5,
                   #                     handlelength=2,
                   )
        if allow_legend == True:
            handles, labels = figure.get_legend_handles_labels()
        if allow_legend == True:
            print(handles[::-1], labels[::-1])
            leg = plt.legend(handles[::-1], labels[::-1],
                             loc='center',
                             prop=LEGEND_FP,
                             ncol=2,
                             bbox_to_anchor=(0.5, 1.2),
                             handletextpad=0.1,
                             borderaxespad=0.0,
                             handlelength=1.8,
                             labelspacing=0.3,
                             columnspacing=0.3,
                             )
            leg.get_frame().set_linewidth(2)
            leg.get_frame().set_edgecolor("black")

    # sometimes you may not want to draw legends.
    # if allow_legend == True:
    #     leg=plt.legend(bars,
    #                    FIGURE_LABEL,
    #                    prop=LEGEND_FP,
    #                    loc='right',
    #                    ncol=1,
    #                    #                     mode='expand',
    #                    bbox_to_anchor=(0.45, 1.1), shadow=False,
    #                    columnspacing=0.1,
    #                    frameon=True, borderaxespad=0.0, handlelength=1.5,
    #                    handletextpad=0.1,
    #                    labelspacing=0.1)
    #     leg.get_frame().set_linewidth(2)
    #     leg.get_frame().set_edgecolor("black")

    plt.ylim(0, 100)

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 0.5 * width, x_values)
    plt.xticks(rotation=30)

    # plt.xlim(0,)
    # plt.ylim(0,1)

    plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LinearLocator(6))

    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight', format='pdf')


def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LEGEND_FP = FontProperties(style='normal', size=26)

    bars = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    width = 0.3
    for i in range(len(FIGURE_LABEL)):
        bars[i] = ax1.bar(x_values, data, width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                          linewidth=0.2)

    # LEGEND
    figlegend = pylab.figure(figsize=(11, 0.5))
    figlegend.legend(bars, FIGURE_LABEL, prop=LEGEND_FP, \
                     loc=9,
                     bbox_to_anchor=(0, 0.4, 1, 1),
                     ncol=len(FIGURE_LABEL), mode="expand", shadow=False, \
                     frameon=False, handlelength=1.1, handletextpad=0.2, columnspacing=0.1)

    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


def normalize(y_values):
    y_total_values = np.zeros(len(y_values[0]))

    for i in range(len(y_values)):
        y_total_values += np.array(y_values[i])
    y_norm_values = []

    for i in range(len(y_values)):
        y_norm_values.append(np.array(y_values[i]) / (y_total_values * 1.0))
    return y_norm_values


# example for reading csv file
def ReadFile(id):
    # Creates a list containing w lists, each of h items, all set to 0
    w, h = 8, 4
    y = [[0 for x in range(w+1)] for y in range(h)]
    max_value = 0

    bound = id + 1 * w
    column = {}
    uarch = {}
    j = 0
    for i in range(id, bound, 1):
        print(i)
        f = open('/data1/xtra/results/breakdown/profile_{}.txt'.format(i), "r")
        read = f.readlines()
        for line in read:
            if line.startswith("CPU_CLK_UNHALTED.THREAD"):
                column["CPU_CLK_UNHALTED.THREAD"] = float(line.split(" ")[1])
            elif line.startswith("IDQ_UOPS_NOT_DELIVERED.CORE"):
                column["IDQ_UOPS_NOT_DELIVERED.CORE"] = float(line.split(" ")[1])
            elif line.startswith("UOPS_ISSUED.ANY"):
                column["UOPS_ISSUED.ANY"] = float(line.split(" ")[1])
            elif line.startswith("UOPS_RETIRED.RETIRE_SLOTS"):
                column["UOPS_RETIRED.RETIRE_SLOTS"] = float(line.split(" ")[1])
            elif line.startswith("INT_MISC.RECOVERY_CYCLES"):
                column["INT_MISC.RECOVERY_CYCLES"] = float(line.split(" ")[1])
            elif line.startswith("CYCLE_ACTIVITY.STALLS_MEM_ANY"):
                column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] = float(line.split(" ")[1])
            elif line.startswith("RESOURCE_STALLS.SB"):
                column["RESOURCE_STALLS.SB"] = float(line.split(" ")[1])
            elif line.startswith("UNC_ARB_TRK_OCCUPANCY.ALL"):
                column["UNC_ARB_TRK_OCCUPANCY.ALL"] = float(line.split(" ")[1])
            elif line.startswith("EXE_ACTIVITY.1_PORTS_UTIL"):
                column["EXE_ACTIVITY.1_PORTS_UTIL"] = float(line.split(" ")[1])
            elif line.startswith("EXE_ACTIVITY.2_PORTS_UTIL"):
                column["EXE_ACTIVITY.2_PORTS_UTIL"] = float(line.split(" ")[1])
            elif line.startswith("EXE_ACTIVITY.EXE_BOUND_0_PORTS"):
                column["EXE_ACTIVITY.EXE_BOUND_0_PORTS"] = float(line.split(" ")[1])
            elif line.startswith("EXE_ACTIVITY.BOUND_ON_STORES"):
                column["EXE_ACTIVITY.BOUND_ON_STORES"] = float(line.split(" ")[1])
            elif line.startswith("MEM_LOAD_UOPS_MISC_RETIRED.LLC_MISS"):
                column["MEM_LOAD_UOPS_MISC_RETIRED.LLC_MISS"] = float(line.split(" ")[1])
            elif line.startswith("MEM_LOAD_UOPS_RETIRED.L3_HIT"):
                column["MEM_LOAD_UOPS_RETIRED.L3_HIT"] = float(line.split(" ")[1])
            elif line.startswith("CYCLE_ACTIVITY.STALLS_L2_PENDING"):
                column["CYCLE_ACTIVITY.STALLS_L2_PENDING"] = float(line.split(" ")[1])

        clocks = column["CPU_CLK_UNHALTED.THREAD"]
        slots = 4*clocks
        uarch["Frontend Bound"] = column["IDQ_UOPS_NOT_DELIVERED.CORE"]/slots
        uarch["Bad Speculation"] = (column["UOPS_ISSUED.ANY"] - column["UOPS_RETIRED.RETIRE_SLOTS"] + 4*column["INT_MISC.RECOVERY_CYCLES"])/slots
        uarch["Retiring"] = column["UOPS_RETIRED.RETIRE_SLOTS"]/slots
        uarch["Backend Bound"] = 1 - (uarch["Frontend Bound"] + uarch["Bad Speculation"] + uarch["Retiring"])
        uarch["Memory Bound"] = (column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] + column["RESOURCE_STALLS.SB"])/clocks

        # memory bound
        # Retired_Slots = column["UOPS_RETIRED.RETIRE_SLOTS"]
        # Retiring = Retired_Slots / slots
        # Few_Uops_Executed_Threshold = Retiring * column["EXE_ACTIVITY.2_PORTS_UTIL"]
        # Core_Bound_Cycles = column["EXE_ACTIVITY.EXE_BOUND_0_PORTS"] + column["EXE_ACTIVITY.1_PORTS_UTIL"] + Few_Uops_Executed_Threshold
        # Backend_Bound_Cycles = Core_Bound_Cycles + column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] + column["EXE_ACTIVITY.BOUND_ON_STORES"]
        # Memory_Bound_Fraction = (column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] + column["EXE_ACTIVITY.BOUND_ON_STORES"]) / Backend_Bound_Cycles
        # Memory_bound= Memory_Bound_Fraction * uarch["Backend Bound"]
        # uarch["Memory Bound"] = Memory_bound

        # memory bound 2
        # M = (7 * column["MEM_LOAD_UOPS_MISC_RETIRED.LLC_MISS"]) / (column["MEM_LOAD_UOPS_RETIRED.L3_HIT"] + column["MEM_LOAD_UOPS_MISC_RETIRED.LLC_MISS"])
        # uarch["Memory Bound"] = (M * column["CYCLE_ACTIVITY.STALLS_L2_PENDING"]) / clocks

        uarch["Core Bound"] = uarch["Backend Bound"] - uarch["Memory Bound"]

        print(uarch)

        # y[0][j] = uarch["Frontend Bound"] * 100
        # y[1][j] = uarch["Bad Speculation"] * 100
        # y[2][j] = uarch["Retiring"] * 100
        # y[3][j] = uarch["Memory Bound"] * 100
        # y[4][j] = uarch["Core Bound"] * 100

        y[0][j] = uarch["Frontend Bound"] * 100
        y[1][j] = uarch["Backend Bound"] * 100
        y[2][j] = uarch["Bad Speculation"] * 100
        y[3][j] = uarch["Retiring"] * 100

        j += 1
        if j == 4:
            y[0][j] = 0
            y[1][j] = 0
            y[2][j] = 0
            y[3][j] = 0
            # y[4][j] = 0
            j +=1
        column.clear()
    # reorder value 0,1 and 2,3

    for val in y:
        val[0], val[1], val[2], val[3] = val[2], val[3], val[0], val[1]

    print(y)

    return y, max_value

if __name__ == "__main__":
    id = 400

    x_values = ['NPJ', 'PRJ', 'MWAY', 'MPASS',
            '',
            'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']  # join time is getting from total - others.

    y_values, max_value = ReadFile(id)  # 55
    # break into 4 parts
    legend_labels = ["Frontend Bound", "Backend Bound", "Bad Speculation", "Retiring"]  # , 'others'

    DrawFigure(x_values, y_values, legend_labels,
               '', 'percentage of time',
               'breakdown_ysb', True)


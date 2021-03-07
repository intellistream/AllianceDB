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
matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'

# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


# draw a line chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(12, 3))
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
                             ncol=3,
                             bbox_to_anchor=(0.5, 1.3),
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
    plt.xticks(rotation=20)

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
    w, h = 8, 5
    y = [[0 for x in range(w+1)] for y in range(h)]
    max_value = 0

    bound = id + 1 * w
    column = {}
    uarch = {}
    j = 0
    for i in range(id, bound, 1):
        print(i)
        f_after = open(exp_dir + '/results/breakdown/profile_w_join_{}.txt'.format(i), "r")
        f_before = open(exp_dir + '/results/breakdown/profile_wo_join_{}.txt'.format(i), "r")
        read_after = f_after.readlines()
        read_before = f_before.readlines()
        column["CPU_CLK_UNHALTED.THREAD"] = float(read_after[2].split(",")[0]) - float(read_before[2].split(",")[0])
        column["IDQ_UOPS_NOT_DELIVERED.CORE"] = float(read_after[3].split(",")[0]) - float(read_before[3].split(",")[0])
        column["UOPS_ISSUED.ANY"] = float(read_after[4].split(",")[0]) - float(read_before[4].split(",")[0])
        column["UOPS_RETIRED.RETIRE_SLOTS"] = float(read_after[5].split(",")[0]) - float(read_before[5].split(",")[0])
        column["INT_MISC.RECOVERY_CYCLES"] = float(read_after[6].split(",")[0]) - float(read_before[6].split(",")[0])
        column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] = float(read_after[7].split(",")[0]) - float(read_before[7].split(",")[0])
        column["RESOURCE_STALLS.SB"] = float(read_after[8].split(",")[0]) - float(read_before[8].split(",")[0])

        clocks = column["CPU_CLK_UNHALTED.THREAD"]
        slots = 4*clocks
        uarch["Frontend Bound"] = column["IDQ_UOPS_NOT_DELIVERED.CORE"]/slots
        uarch["Bad Speculation"] = (column["UOPS_ISSUED.ANY"] - column["UOPS_RETIRED.RETIRE_SLOTS"] + 4*column["INT_MISC.RECOVERY_CYCLES"])/slots
        uarch["Retiring"] = column["UOPS_RETIRED.RETIRE_SLOTS"]/slots
        uarch["Backend Bound"] = 1 - (uarch["Frontend Bound"] + uarch["Bad Speculation"] + uarch["Retiring"])
        uarch["Memory Bound"] = (column["CYCLE_ACTIVITY.STALLS_MEM_ANY"] + column["RESOURCE_STALLS.SB"])/clocks
        uarch["Core Bound"] = uarch["Backend Bound"] - uarch["Memory Bound"]

        # print(column)
        print(uarch)

        # y[0][j] = uarch["Frontend Bound"] * 100
        # y[1][j] = uarch["Bad Speculation"] * 100
        # y[2][j] = uarch["Retiring"] * 100
        # y[3][j] = uarch["Memory Bound"] * 100
        # y[4][j] = uarch["Core Bound"] * 100

        y[0][j] = uarch["Frontend Bound"] * 100
        y[1][j] = uarch["Memory Bound"] * 100
        y[2][j] = uarch["Core Bound"] * 100
        y[3][j] = uarch["Bad Speculation"] * 100
        y[4][j] = uarch["Retiring"] * 100

        j += 1
        if j == 4:
            y[0][j] = 0
            y[1][j] = 0
            y[2][j] = 0
            y[3][j] = 0
            y[4][j] = 0
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
    legend_labels = ["Frontend Bound", "Memory Bound", "Core Bound", "Bad Speculation", "Retiring"]  # , 'others'

    DrawFigure(x_values, y_values, legend_labels,
               '', 'percentage of time',
               'breakdown_rovio', True)


import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator, LogLocator
from numpy import double

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 24
LEGEND_FONT_SIZE = 26
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
MARKER_SIZE = 13.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME

FIGURE_FOLDER = '/data1/xtra/results/figure'


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


# draw a line chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
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
    width = 0.4
    # draw the bars
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + i * width + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i], edgecolor='black', linewidth=3)

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 1 * width, x_values)
    # sometimes you may not want to draw legends.

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(bars, FIGURE_LABEL,
                   #                     mode='expand',
                   #                     shadow=False,
                   #                     columnspacing=0.25,
                   #                     labelspacing=-2.2,
                   #                     borderpad=5,
                   #                     bbox_transform=ax.transAxes,
                   frameon=True,
                   #                     columnspacing=5.5,
                   #                     handlelength=2,
                   )
        # if we want to reorder the sequence.
        # handles, labels = figure.get_legend_handles_labels()
        # handles[::-1], labels[::-1]
        leg = plt.legend(
            loc='center',
            prop=LEGEND_FP,
            ncol=2,
            bbox_to_anchor=(0.5, 1.3),
            handletextpad=0.2,
            borderaxespad=0.0,
            handlelength=1.8,
            labelspacing=0.3,
            columnspacing=0.3,
        )
        leg.get_frame().set_linewidth(2)
        leg.get_frame().set_edgecolor("black")

    # plt.xlim(0,)
    # plt.ylim(y_min, y_max)
    # plt.yscale('log')
    plt.ticklabel_format(axis="y", style="sci", scilimits=(0, 0))
    plt.grid(axis='y', color='gray')
    # figure.yaxis.set_major_locator(LinearLocator(6))
    # figure.yaxis.set_major_locator(LogLocator(base=10))

    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')


def ReadFile():
    y_values = []

    col1 = []
    col2 = []

    cnt = 0
    sum = 0
    f = open("/data1/xtra/results/breakdown/SHJ_JM_P_{}.txt".format(133), "r")
    read = f.readlines()

    for x in read:
        value = double(x.strip("\n"))
        sum += value
        if cnt == 1:  # partition
            col1.append(value)
        elif cnt == 2:  # build
            col1.append(value)
        elif cnt == 5:  # probe
            col1.append(value)
        cnt += 1
    col1.append(sum)
    y_values.append(col1)

    cnt = 0
    sum = 0
    f = open("/data1/xtra/results/breakdown/SHJ_JM_NP_{}.txt".format(133), "r")
    read = f.readlines()

    for x in read:
        value = double(x.strip("\n"))
        sum += value
        if cnt == 1:  # partition
            col2.append(value)
        elif cnt == 2:  # build
            col2.append(value)
        elif cnt == 5:  # probe
            col2.append(value)
        cnt += 1
    col2.append(sum)
    y_values.append(col2)
    print(y_values)

    return y_values


if __name__ == "__main__":
    x_values = [
        'partition', 'build', 'probe', 'overall'
    ]
    y_values = []

    y_values = ReadFile()

    legend_labels = ['w/ Partition', 'w/o Partition']

    DrawFigure(x_values, y_values, legend_labels, '', 'cycles per input', 0, 0,
               'breakdown_p_np_study',
               True)
    # DrawLegend(legend_labels, 'profile_legend')

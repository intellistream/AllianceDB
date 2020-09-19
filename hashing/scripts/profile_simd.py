import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 24
LEGEND_FONT_SIZE = 26
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (["", 'o', 's', 'v', "^", "", "h", "v", ">", "x", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#000000', '#332288', '#88CCEE', '#44AA99', '#117733', '#000000', '#17202A', '#DDCC77', '#2E86C1', '#882255', '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (["////", "\\\\", "|||", "o", "---", "\\\\", "\\\\", "//////", "//////", ".", "\\\\\\", "\\\\\\"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 1.0
MARKER_SIZE = 0.0
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
    LINE_WIDTH = 8.0
    MARKER_SIZE = 12.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(12, 0.5))
    idx = 0
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    idx = 0
    for group in range(len(FIGURE_LABEL)):
        lines[idx], = ax1.plot(x_values, data,
                               color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                               marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(group))

        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=False, borderaxespad=0.0, handlelength=2)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


# draw a line chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(8, 3))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    index = np.arange(len(x_values))
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.15
    # draw the bars
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + i * width + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i],
                          edgecolor='black', linewidth=3)

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        leg = plt.legend(bars, FIGURE_LABEL, prop=LEGEND_FP,
                         ncol=3,
                         #                     mode='expand',
                         #                     shadow=False,
                         bbox_to_anchor=(0.5, 1.4),
                         columnspacing=0.25,
                         handletextpad=0.2,
                         #                     bbox_transform=ax.transAxes,
                         #                     frameon=False,
                         #                     columnspacing=5.5,
                         #                     handlelength=2,
                         loc='upper center'
                         )
        leg.get_frame().set_linewidth(2)
        leg.get_frame().set_edgecolor("black")
    #    plt.xticks(rotation=35)

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 2.4 * width, x_values)

    # plt.xlim(0,)
    plt.ylim(y_min, y_max)

    plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LinearLocator(6))

    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')


def GetL1MISS(id):
    file = '/data1/xtra/results/breakdown/profile_{}.txt'.format(id)
    with open(file) as fi:
        for ln in fi:
            if ln.startswith("L2Hit"):
                return float(ln[6:])


def GetL2MISS(id):
    file = '/data1/xtra/results/breakdown/profile_{}.txt'.format(id)
    with open(file) as fi:
        for ln in fi:
            if ln.startswith("L2Misses "):
                return float(ln[9:])


def GetL3MISS(id):
    file = '/data1/xtra/results/breakdown/profile_{}.txt'.format(id)
    with open(file) as fi:
        for ln in fi:
            if ln.startswith("L3Misses "):
                return float(ln[9:])


if __name__ == "__main__":
    x_values = [
        'MWAY', 'MPASS', 'PMJ$^{JM}$', 'PMJ$^{JB}$'
    ]
    y_values = []
    file = '/data1/xtra/results/records/PMJ_JM_NP_{}.txt'.format(104)
    f = open(file, "r")
    read = f.readlines()
    inputs = float(read.pop(0).strip("\n"))  # get number of inputs

    y_values.append([  # L1
        GetL1MISS(102) / inputs,
        GetL1MISS(103) / inputs,
        GetL1MISS(106) / inputs,
        GetL1MISS(107) / inputs,
    ])
    y_values.append([  # L2
        GetL2MISS(102) / inputs,
        GetL2MISS(103) / inputs,
        GetL2MISS(106) / inputs,
        GetL2MISS(107) / inputs,
    ])
    y_values.append([  # L3
        GetL3MISS(102) / inputs,
        GetL3MISS(103) / inputs,
        GetL3MISS(106) / inputs,
        GetL3MISS(107) / inputs,
    ])
    legend_labels = ['L1 miss', 'L2 miss', 'L3 miss']

    DrawFigure(x_values, y_values, legend_labels, '', 'misses per input', 0, GetL3MISS(107) / inputs, 'profile_simd', True)

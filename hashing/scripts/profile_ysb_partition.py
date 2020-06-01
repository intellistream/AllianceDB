import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 24
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", '', "h", "v", ">", "x", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#ABB2B9', '#2E4053', '#8D6E63', '#000000', '', '#CD6155', '#52BE80', '#FFFF00', '#5499C7', '#BB8FCE')
# you may want to change the patterns for different figures
PATTERNS = (["", "", "", "", '', "/", "\\", "||", "-", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
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
    fig = plt.figure(figsize=(10, 4))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    index = np.arange(len(x_values))
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.1
    # draw the bars
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + i * width + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i], edgecolor='black', linewidth=3)

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 3 * width, x_values)

    # plt.xlim(0,)
    plt.ylim(y_min, y_max)

    plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LinearLocator(6))

    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

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
        'L1 miss', 'L2 miss', 'L3 miss'
    ]
    y_values = []
    file = '/data1/xtra/results/records/NPJ_{}.txt'.format(203)
    f = open(file, "r")
    read = f.readlines()
    inputs = float(read.pop(0).strip("\n"))  # get number of inputs

    y_values.append([  # NPJ does not have partition phase.
        0 / inputs,  # L1
        0 / inputs,  # L2
        0 / inputs  # L3
    ])

    ##fill in with results from "profile_205.txt"
    y_values.append([  # PRJ
        GetL1MISS(205) / inputs,
        GetL2MISS(205) / inputs,
        GetL3MISS(205) / inputs
    ])
    y_values.append([  # WAY
        GetL1MISS(201) / inputs,
        GetL2MISS(201) / inputs,
        GetL3MISS(201) / inputs
    ])
    y_values.append([  # MPASS
        GetL1MISS(202) / inputs,
        GetL2MISS(202) / inputs,
        GetL3MISS(202) / inputs
    ])

    y_values.append([  # deliminator
        0 / inputs,  # L1
        0 / inputs,  # L2
        0 / inputs  # L3
    ])

    y_values.append([  # SHJM -- 206
        GetL1MISS(206) / inputs,
        GetL2MISS(206) / inputs,
        GetL3MISS(206) / inputs
    ])
    y_values.append([  # SHJB -- 207
        GetL1MISS(207) / inputs,
        GetL2MISS(207) / inputs,
        GetL3MISS(207) / inputs
    ])
    y_values.append([  # PMJM -- 208
        GetL1MISS(208) / inputs,
        GetL2MISS(208) / inputs,
        GetL3MISS(208) / inputs
    ])
    y_values.append([  # PMJB -- 209
        GetL1MISS(209) / inputs,
        GetL2MISS(209) / inputs,
        GetL3MISS(209) / inputs
    ])

    legend_labels = ['NPJ', 'PRJ', 'MWAY', 'MPASS',
                     '',
                     'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$',
                     'PMJ$^{JB}$']

    DrawFigure(x_values, y_values, legend_labels, '', 'misses per input', 0, 1, 'profile_ysb_partition',
               False)
    # DrawLegend(legend_labels, 'profile_legend')

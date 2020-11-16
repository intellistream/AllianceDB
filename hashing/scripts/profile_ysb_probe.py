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
COLOR_MAP = (
    '#000000', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#000000', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF',
    '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (
    ["", "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\",
     "*"])
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

exp_dir = "/data1/xtra"

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


# draw a line chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(11, 4))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    index = np.arange(len(x_values))
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.05
    # draw the bars
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + i * width + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i], edgecolor='black', linewidth=3)

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 5.5 * width, x_values)

    plt.ticklabel_format(axis="y", style="sci", scilimits=(0, 0), useMathText=True)
    plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LinearLocator(3))
    # plt.grid(axis='y', color='gray')
    # figure.yaxis.set_major_locator(LinearLocator(6))

    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')


def GetL1MISS(id):
    file = exp_dir + '/results/breakdown/profile_{}.txt'.format(id)
    with open(file) as fi:
        for ln in fi:
            if ln.startswith("L2Hit "):
                return float(ln[6:])


def GetL2MISS(id):
    file = exp_dir + '/results/breakdown/profile_{}.txt'.format(id)
    with open(file) as fi:
        for ln in fi:
            if ln.startswith("L2Misses "):
                return float(ln[9:])


def GetL3MISS(id):
    file = exp_dir + '/results/breakdown/profile_{}.txt'.format(id)
    with open(file) as fi:
        for ln in fi:
            if ln.startswith("L3Misses "):
                return float(ln[9:])


if __name__ == "__main__":
    x_values = [
        'L1 miss', 'L2 miss', 'L3 miss'
    ]
    y_values = []
    file = exp_dir + '/results/records/NPJ_{}.txt'.format(40)
    f = open(file, "r")
    read = f.readlines()
    inputs = float(read.pop(0).strip("\n")) / 1000  # get number of inputs (in k)

    y_values.append([  # placeholders
        0 / inputs,  # L1
        0 / inputs,  # L2
        0 / inputs  # L3
    ])

    y_values.append([  # NPJ
        GetL1MISS(211) / inputs,  # L1
        GetL2MISS(211) / inputs,  # L2
        GetL3MISS(211) / inputs  # L3
    ])
    y_values.append([  # PRJ
        max(0, (GetL1MISS(212) - GetL1MISS(206))) / inputs,
        max(0, (GetL2MISS(212) - GetL2MISS(206))) / inputs,
        max(0, (GetL3MISS(212) - GetL2MISS(206))) / inputs
    ])
    y_values.append([  # WAY
        max(0, (GetL1MISS(203) - GetL1MISS(201))) / inputs,
        max(0, (GetL2MISS(203) - GetL2MISS(201))) / inputs,
        max(0, (GetL3MISS(203) - GetL3MISS(201))) / inputs
    ])
    y_values.append([  # MPASS
        max(0, (GetL1MISS(204) - GetL1MISS(202))) / inputs,
        max(0, (GetL2MISS(204) - GetL2MISS(202))) / inputs,
        max(0, (GetL3MISS(204) - GetL3MISS(202))) / inputs
    ])

    y_values.append([  # deliminator
        0,
        0,
        0
    ])

    y_values.append([  # SHJM
        max(0, (GetL1MISS(213) - GetL1MISS(207))) / inputs,
        max(0, (GetL2MISS(213) - GetL2MISS(207))) / inputs,
        max(0, (GetL3MISS(213) - GetL3MISS(207))) / inputs
    ])
    y_values.append([  # SHJB
        max(0, (GetL1MISS(214) - GetL1MISS(208))) / inputs,
        max(0, (GetL2MISS(214) - GetL2MISS(208))) / inputs,
        max(0, (GetL3MISS(214) - GetL3MISS(208))) / inputs
    ])
    y_values.append([  # PMJM
        max(0, (GetL1MISS(215) - GetL1MISS(209))) / inputs,
        max(0, (GetL2MISS(215) - GetL2MISS(209))) / inputs,
        max(0, (GetL3MISS(215) - GetL3MISS(209))) / inputs
    ])
    y_values.append([  # PMJB
        max(0, (GetL1MISS(216) - GetL1MISS(210))) / inputs,
        max(0, (GetL2MISS(216) - GetL2MISS(210))) / inputs,
        max(0, (GetL3MISS(216) - GetL3MISS(210))) / inputs
    ])

    legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
                     'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']

    DrawFigure(x_values, y_values, legend_labels, '', 'misses per k input', 0, 1,
               'profile_ysb_probe',
               False)
    DrawLegend(legend_labels, 'profile_legend')

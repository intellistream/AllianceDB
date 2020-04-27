import itertools as it
import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator
OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 24
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#F15854', '#5DA5DA', '#60BD68', '#B276B2', '#DECF3F', '#F17CB0', '#B2912F', '#FAA43A', '#AFAFAF')
# you may want to change the patterns for different figures
PATTERNS = (["|", "\\", "/", "+", "-", ".", "*", "x", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
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


# draw a bar chart
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
    width = 0.1
    # draw the bars
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + i * width + width / 2,
                          y_values[i], width,
                          hatch=PATTERNS[i],
                          color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i])

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(bars, FIGURE_LABEL,
                   prop=LEGEND_FP,
                   ncol=4,
                   loc='upper center',
                   #                     mode='expand',
                   shadow=False,
                   bbox_to_anchor=(0.45, 1.6),
                   columnspacing=0.1,
                   handletextpad=0.2,
                   #                     bbox_transform=ax.transAxes,
                   #                     frameon=True,
                   #                     columnspacing=5.5,
                   #                     handlelength=2,
                   )

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 2.4 * width, x_values)
    plt.yscale('log')

    plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_locator(LinearLocator(5))
    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')


# example for reading csv file
def ReadFile():
    y = []
    col1 = []
    col2 = []
    col3 = []
    col4 = []
    col5 = []
    col6 = []
    col7 = []
    col8 = []

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/PRJ_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp
        col1.append(x)
    y.append(col1)

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/NPJ_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp
        
        col2.append(x)
    y.append(col2)

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/MPASS_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp

        col3.append(x)
    y.append(col3)

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/MWAY_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp

        col4.append(x)
    y.append(col4)

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/SHJ_JM_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp

        col5.append(x)
    y.append(col5)

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/SHJ_JBCR_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp

        col6.append(x)
    y.append(col6)

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/PMJ_JM_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp

        col7.append(x)
    y.append(col7)

    for id in it.chain(range(5, 10)):
        file = '/data1/xtra/results/latency/PMJ_JBCR_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get last timestamp

        col8.append(x)
    y.append(col8)
    return y


if __name__ == "__main__":
    x_values = [0, 0.2, 0.4, 0.8, 1]

    y_values = ReadFile()

    legend_labels = ['NPJ', 'PRJ', 'MWAY', 'MPASS', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$',
                     'PMJ$^{JB}$']

    DrawFigure(x_values, y_values, legend_labels,
               '$Skew_{ts}$ (zipf)', '$99^{th}$ latency (ms)', 0,
               400, 'latency_figure2', False)

#  DrawLegend(legend_labels, 'factor_legend')

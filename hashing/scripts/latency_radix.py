import getopt
import os
import sys

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 24
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "|", "_"])
# you may want to change the color map for different figures
COLOR_MAP = (['#000000', '#5DA5DA', '#60BD68', '#B276B2', '#DECF3F', '#F17CB0', '#B2912F', '#FAA43A', '#AFAFAF'])
# you may want to change the patterns for different figures
PATTERNS = (["", "\\\\", "//////", "o", "||", "\\\\", "\\\\", "//////", "//////", ".", "\\\\\\", "\\\\\\"])
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


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


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
    figlegend = pylab.figure(figsize=(16, 0.7))
    figlegend.legend(bars, FIGURE_LABEL, prop=LEGEND_FP, \
                     loc=9,
                     bbox_to_anchor=(0, 0.1, 1, 1),
                     ncol=len(FIGURE_LABEL), mode="expand", shadow=False, \
                     frameon=False, handlelength=1.5, handletextpad=0.2, columnspacing=0.1, edgecolor='black',
                     linewidth=3)
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


# draw a bar chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(8, 3.5))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    index = np.arange(len(x_values))
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.2
    # draw the bars
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + i * width + width / 2,
                          y_values[i], width,
                          hatch=PATTERNS[i],
                          color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i], edgecolor='black', linewidth=3)

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        leg = plt.legend(bars, FIGURE_LABEL,
                         prop=LEGEND_FP,
                         ncol=3,
                         loc='upper center',
                         #                     mode='expand',
                         shadow=False,
                         bbox_to_anchor=(0.5, 1.3),
                         columnspacing=0.1,
                         handletextpad=0.2,
                         #                     bbox_transform=ax.transAxes,
                         #                     frameon=True,
                         #                     columnspacing=5.5,
                         #                     handlelength=2,
                         )
        leg.get_frame().set_linewidth(2)
        leg.get_frame().set_edgecolor("black")

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 1 * width, x_values)
    # plt.yscale('log')

    plt.grid(axis='y', color='gray')
    # figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_locator(LinearLocator(5))
    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')


# example for reading csv file
def ReadFile(id):
    y = []
    col1 = []  # 90
    col2 = []  # 95
    col3 = []  # 99
    w = 6
    bound = id + 1 * w
    for i in range(id, bound, 1):
        file = '/data1/xtra/results/latency/PRJ_{}.txt'.format(i)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(int(len(read) * 0.90)).strip("\n"))  # get the 90th timestamp
        col1.append(x)
        x = float(read.pop(int(len(read) * 0.95)).strip("\n"))  # get the 95th timestamp
        col2.append(x)
        x = float(read.pop(int(len(read) * 0.99)).strip("\n"))  # get the 99th timestamp
        col3.append(x)
    y.append(col1)
    y.append(col2)
    y.append(col3)
    return y


if __name__ == "__main__":
    id = 113
    try:
        opts, args = getopt.getopt(sys.argv[1:], '-i:h', ['test id', 'help'])
    except getopt.GetoptError:
        print('breakdown.py -id testid')
        sys.exit(2)
    for opt, opt_value in opts:
        if opt in ('-h', '--help'):
            print("[*] Help info")
            exit()
        elif opt == '-i':
            print('Test ID:', opt_value)
            id = (int)(opt_value)

    x_values = [8, 10, 12, 14, 16, 18]  # number of radix bits.

    y_values = ReadFile(id)

    legend_labels = ['90$^{th}$', '95$^{th}$', '99$^{th}$']

    DrawFigure(x_values, y_values, legend_labels,
               'number of radix bits', 'event latency (ms)', 0,
               400, 'latency_radix_figure', True)

    # DrawLegend(legend_labels, 'latency_radix_legend')

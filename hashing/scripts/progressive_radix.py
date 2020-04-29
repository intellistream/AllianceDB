import getopt
import os
import sys
from math import ceil, floor

import matplotlib
import matplotlib as mpl
from matplotlib.ticker import FuncFormatter, LinearLocator, LogLocator
from numpy import double
from numpy.ma import arange

mpl.use('Agg')

import matplotlib.pyplot as plt
import pylab
from matplotlib.font_manager import FontProperties

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 24
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['^', 'v', '<', ">", "8", "s", "p", "P", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#ABB2B9', '#2E4053', '#8D6E63', '#000000', '#CD6155', '#52BE80', '#FFFF00', '#5499C7', '#BB8FCE')
# you may want to change the patterns for different figures
PATTERNS = (["", "", "", "", "/", "\\", "||", "-", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 13.0
MARKER_FREQUENCY = 1000

mpl.rcParams['ps.useafm'] = True
mpl.rcParams['pdf.use14corefonts'] = True
mpl.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
mpl.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
mpl.rcParams['font.family'] = OPT_FONT_NAME

FIGURE_FOLDER = '/data1/xtra/results/figure'


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")

def getmaxts(id):
    ts = 0
    w = 6
    bound = id + 1 * w
    for i in range(id, bound, 1):
        file = '/data1/xtra/results/timestamps/PRJ_{}.txt'.format(i)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        if (x > ts):
            ts = x

    return ts

def getCount(id):
    file = '/data1/xtra/results/timestamps/PRJ_{}.txt'.format(id)
    f = open(file, "r")
    read = f.readlines()
    return len(read)

# example for reading csv file
def ReadFile(id):
    x_axis = []
    y_axis = []

    w = 6
    bound = id + 1 * w
    for i in range(id, bound, 1):
        col = []
        f = open("/data1/xtra/results/timestamps/PRJ_{}.txt".format(i), "r")
        read = f.readlines()
        for r in read:
            value = double(r.strip("\n"))  # timestamp.
            col.append(value)
        # calculate the proportional values of samples
        coly = 1. * arange(len(col)) / (len(col) - 1)
        x_axis.append(col)
        y_axis.append(coly)

    return x_axis, y_axis


def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LINE_WIDTH = 8.0
    MARKER_SIZE = 15.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(16, 0.3))
    idx = 0
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    idx = 0
    for group in range(len(FIGURE_LABEL)):
        lines[idx], = ax1.plot(x_values, data,
                               color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                               marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(group)
                               ,markeredgewidth=2, markeredgecolor='k'
                               )

        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=False, borderaxespad=-0.3, handlelength=1)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


# draw a line chart
def DrawFigure(xvalues, yvalues, legend_labels, x_label, y_label, x_min, x_max, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(7, 3))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    x_values = xvalues
    y_values = yvalues
    print("mark gap:", ceil(x_max / 6))
    lines = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        lines[i], = figure.plot(x_values[i], y_values[i], color=LINE_COLORS[i], \
                                linewidth=LINE_WIDTH, marker=MARKERS[i], \
                                markersize=MARKER_SIZE, label=FIGURE_LABEL[i],
                                markevery=ceil(x_max / 6),
                                markeredgewidth=2, markeredgecolor='k'
                                )

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        leg=plt.legend(lines,
                   FIGURE_LABEL,
                   prop=LEGEND_FP,
                   loc='right',
                   ncol=1,
                   #                     mode='expand',
                   bbox_to_anchor=(1.4, 0.5), shadow=False,
                   columnspacing=0.1,
                   frameon=True, borderaxespad=0.0, handlelength=1.5,
                   handletextpad=0.1,
                   labelspacing=0.1)
        leg.get_frame().set_linewidth(2)
        leg.get_frame().set_edgecolor("black")

    # plt.yscale('log')
    # plt.xticks(x_values)
    # you may control the limits on your own.
    # plt.xlim(x_min, x_max)
    # plt.ylim(y_min, y_max)

    plt.grid(axis='y', color='gray')

    # figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_locator(matplotlib.ticker.FixedFormatter(["0.25", "0.5", "0.75", "1"]))
    # figure.xaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
    # figure.xaxis.set_major_locator(LinearLocator(5))

    figure.get_xaxis().set_tick_params(direction='in', pad=5)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')
    # ConvertEpsToPdf(FIGURE_FOLDER + "/" + filename)

if __name__ == "__main__":

    id = 113
    try:
        opts, args = getopt.getopt(sys.argv[1:], '-i:h', ['test id', 'help'])
    except getopt.GetoptError:
        print('progressive_figure.py -id testid')
        sys.exit(2)
    for opt, opt_value in opts:
        if opt in ('-h', '--help'):
            print("[*] Help info")
            exit()
        elif opt == '-i':
            print('Test ID:', opt_value)
            id = (int)(opt_value)

    legend_labels = ['$\sharp r$=8', '$\sharp r$=10', '$\sharp r$=12', '$\sharp r$=14', '$\sharp r$=16', '$\sharp r$=18']

    ts = ceil(getmaxts(id) / 100) * 100
    print("maximum timestamp:", ts)
    x_axis, y_axis = ReadFile(id)

    legend = True
    DrawFigure(x_axis, y_axis, legend_labels,
               'elapsed time (msec)', 'cumulative percent', 0, getCount(id),
               1, 0,
               'progressive_radix_figure',
               legend)

    # DrawLegend(legend_labels, 'progressive_radix_legend')
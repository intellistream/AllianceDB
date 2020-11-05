import itertools as it
import os

import matplotlib as mpl
from brokenaxes import brokenaxes

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

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#F15854', '#5DA5DA', '#60BD68', '#B276B2', '#DECF3F', '#F17CB0', '#B2912F', '#FAA43A', '#AFAFAF')
# you may want to change the patterns for different figures
PATTERNS = (["////", "\\\\", "//////", "o", "o", "\\\\", "\\\\", "//////", "//////", ".", "\\\\\\", "\\\\\\"])
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


# example for reading csv file
def ReadFile():
    col1 = []
    col2 = []
    col3 = []
    col4 = []
    col5 = []
    col6 = []

    for id in it.chain(range(1, 5), range(0, 1)):
        file = '/data1/xtra/results/timestamps/PRJ_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col1.append(value)

    for id in it.chain(range(1, 5), range(0, 1)):
        file = '/data1/xtra/results/timestamps/NPJ_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col2.append(value)

    for id in it.chain(range(1, 5), range(0, 1)):
        file = '/data1/xtra/results/timestamps/SHJ_JM_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col3.append(value)

    for id in it.chain(range(1, 5), range(0, 1)):
        file = '/data1/xtra/results/timestamps/SHJ_JBCR_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col4.append(value)

    # for id in it.chain(range(1, 5),  range(0,1)):
    #     file = '/data1/xtra/results/timestamps/PMJ_JM_NP_{}.txt'.format(id)
    #     f = open(file, "r")
    #     read = f.readlines()
    #     x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    #     value = len(read) / x  # get throughput (#items/ms)
    #     col5.append(value)
    #
    # for id in it.chain(range(1, 5),  range(0,1)):
    #     file = '/data1/xtra/results/timestamps/PMJ_JBCR_NP_{}.txt'.format(id)
    #     f = open(file, "r")
    #     read = f.readlines()
    #     x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    #     value = len(read) / x  # get throughput (#items/ms)
    #     col6.append(value)

    return col1, col2, col3, col4, col5, col6


def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LINE_WIDTH = 8.0
    MARKER_SIZE = 12.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(11, 0.3))
    idx = 0
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    idx = 0
    for group in xrange(len(FIGURE_LABEL)):
        lines[idx], = ax1.plot(x_values, data,
                               color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                               marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(group))

        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=False, borderaxespad=-0.3, handlelength=2)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


# draw a line chart
def DrawFigure(xvalues, yvalues, legend_labels, x_label, y_label, x_min, x_max, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(8, 3))
    bax = brokenaxes(xlims=((1, 1000), (5000, 10000)), hspace=.05)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    x_values = xvalues
    y_values = yvalues

    lines = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bax.plot(x_values, y_values[i], label=FIGURE_LABEL[i])
    bax.loglog(x, x)
    # sometimes you may not want to draw legends.
    if allow_legend == True:
        bax.legend(loc="best")

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".eps", bbox_inches='tight', format='eps')
    ConvertEpsToPdf(FIGURE_FOLDER + "/" + filename)


if __name__ == "__main__":

    # legend_labels = ['PRJ', 'NPJ', 'Hash_JM', 'Hash_JB', 'Sort_JM', 'Sort_JB']
    legend_labels = ['PRJ', 'NPJ', 'Hash_JM', 'Hash_JB']
    col0 = []
    for x in 1, 10, 100, 1000, 10000:
        col0.append(x)

    col1, col2, col3, col4, col5, col6 = ReadFile()
    lines = [col1, col2, col3, col4]  # , col5, col6
    DrawFigure(col0, lines, legend_labels,
               'Tuple arrival rate (#items/usec)', 'Throughput (#items/usec)',
               1,
               10000,
               0, 1000,
               'figure1',
               True)
    # DrawLegend(legend_labels, 'interval_legend')

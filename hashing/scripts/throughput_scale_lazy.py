import itertools as it
import os

import matplotlib
import matplotlib as mpl

mpl.use('Agg')

import matplotlib.pyplot as plt
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import MaxNLocator, LinearLocator

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


def getThroughput(id, x):
    if id == 38:
        value = (116941 + 151500) / x  # get throughput (#items/ms)
    elif id == 39:
        value = (51001 + 51001) / x  # get throughput (#items/ms)
    elif id == 40:
        value = (1000 + 40100000) / x  # get throughput (#items/ms)
    elif id == 41:
        value = (1000000 + 1000000) / x  # get throughput (#items/ms)
    else:
        value = (12800000 + 128000000) / x
    return value


def normalize(value):
    norm = []
    base = value[0]

    for i in value:
        norm.append(i / base * 1.0)
    return norm


# example for reading csv file
#     'PRJ (Stock)',
#     'SHJ$^{JM}$ (Stock)',
#     'PRJ (Rovio)',
#     'SHJ$^{JM}$ (Rovio)',
#     'PRJ (YSB)',
#     'SHJ$^{JM}$ (YSB)',
#     'PRJ (DEBS)',
#     'SHJ$^{JM}$ (DEBS)',
def ReadFile():
    y = []
    col1 = []
    col2 = []
    col3 = []
    col4 = []

    for id in it.chain(range(42, 46)):
        file = '/data1/xtra/results/timestamps/MPASS_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        col1.append(getThroughput(id, x))
    y.append(normalize(col1))

    for id in it.chain(range(46, 50)):
        file = '/data1/xtra/results/timestamps/MPASS_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        col2.append(getThroughput(id, x))
    y.append(normalize(col2))

    for id in it.chain(range(50, 54)):
        file = '/data1/xtra/results/timestamps/MPASS_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        col3.append(getThroughput(id, x))
    y.append(normalize(col3))

    for id in it.chain(range(54, 58)):
        file = '/data1/xtra/results/timestamps/MPASS_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        col4.append(getThroughput(id, x))
    y.append(normalize(col4))

    return y


def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LINE_WIDTH = 8.0
    MARKER_SIZE = 20.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(16, 0.4))
    idx = 0
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    idx = 0
    for group in range(len(FIGURE_LABEL)):
        lines[idx], = ax1.plot(x_values, data,
                               color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                               marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(group),
                               markeredgewidth=2, markeredgecolor='k'
                               )

        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=False, borderaxespad=-0.2, handlelength=1.2)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')

# draw a line chart
def DrawFigure(xvalues, yvalues, legend_labels, x_label, y_label, x_min, x_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(10, 3))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    x_values = xvalues
    y_values = yvalues

    lines = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        lines[i], = figure.plot(x_values, y_values[i], color=LINE_COLORS[i], \
                                linewidth=LINE_WIDTH, marker=MARKERS[i], \
                                markersize=MARKER_SIZE, label=FIGURE_LABEL[i],
                                markeredgewidth=2, markeredgecolor='k')

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(lines,
                   FIGURE_LABEL,
                   prop=LEGEND_FP,
                   loc='right',
                   ncol=1,
                   #                     mode='expand',
                   bbox_to_anchor=(1.6, 0.5), shadow=False,
                   columnspacing=0.1,
                   frameon=True, borderaxespad=0.0, handlelength=1.5,
                   handletextpad=0.1,
                   labelspacing=0.1)
    # plt.xscale('log')
    # plt.yscale('log')
    plt.xticks(x_values)
    # you may control the limits on your own.
    plt.xlim(x_min, x_max)
    # plt.ylim(0, 41000)
    plt.ylim(0, 6)
    plt.ticklabel_format(axis="y", style="sci", scilimits=(0, 0))
    plt.grid(axis='y', color='gray')

    figure.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    figure.yaxis.set_major_locator(LinearLocator(3))
    # figure.xaxis.set_major_locator(MaxNLocator(integer=True))

    # figure.get_xaxis().set_tick_params(direction='in', pad=10)
    # figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')


if __name__ == "__main__":
    legend_labels = [
        'Stock',
        'Rovio',
        'YSB',
        'DEBS',
    ]
    y = ReadFile()
    print(y)
    x = [1, 2, 4, 8]

    DrawFigure(x, y, legend_labels,
               'Number of threads', 'Normalized Tpt.',
               0, 10,
               'throughput_scale_lazy',
               False)
    # DrawLegend(legend_labels, 'throughput_scale_legend')

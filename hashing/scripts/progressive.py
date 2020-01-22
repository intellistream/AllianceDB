import getopt
import os
import sys

import matplotlib as mpl

mpl.use('Agg')

import matplotlib.pyplot as plt
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator
from matplotlib.ticker import LogLocator
from pathlib import Path

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 24
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "|", "_"])
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

FIGURE_FOLDER = str(Path.home()) + '/figure'


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


# example for reading csv file
def ReadFile(S):
    col1 = []
    col2 = []
    col3 = []
    col4 = []
    col5 = []
    col6 = []

    f = open("/data1/xtra/results/SHJ_JM_NP_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col1.append(int(x.strip("\n")))
        cnt += 1

    f = open("/data1/xtra/results/SHJ_JBCR_NP_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col2.append(int(x.strip("\n")))
        cnt += 1

    f = open("/data1/xtra/results/PMJ_JM_NP_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col3.append(int(x.strip("\n")))
        cnt += 1

    f = open("/data1/xtra/results/PMJ_JBCR_NP_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col4.append(int(x.strip("\n")))
        cnt += 1

    f = open("/data1/xtra/results/PRJ_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col5.append(int(x.strip("\n")))
        cnt += 1

    f = open("/data1/xtra/results/NPJ_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col6.append(int(x.strip("\n")))
        cnt += 1
    # f = open("/data1/xtra/results/SHJ_HS_NP_timestamps.txt", "r")
    # cnt = 1
    # read = f.readlines()
    # for x in read:
    #     if cnt % S == 0:
    #         col3.append(int(x.strip("\n")))
    #     cnt += 1
    #
    # f = open("/data1/xtra/results/PMJ_HS_NP_timestamps.txt", "r")
    # cnt = 1
    # read = f.readlines()
    # for x in read:
    #     if cnt % S == 0:
    #         col6.append(int(x.strip("\n")))
    #     cnt += 1

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
                                markersize=MARKER_SIZE, label=FIGURE_LABEL[i])

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(lines,
                   FIGURE_LABEL,
                   prop=LEGEND_FP,
                   loc='upper center',
                   ncol=3,
                   #                     mode='expand',
                   bbox_to_anchor=(0.55, 1.5), shadow=False,
                   columnspacing=0.1,
                   frameon=True, borderaxespad=0.0, handlelength=1.5,
                   handletextpad=0.1,
                   labelspacing=0.1)

    plt.yscale('log')
    plt.xticks(x_values)
    # you may control the limits on your own.
    plt.xlim(x_min, x_max)
    plt.ylim(y_min, y_max)

    plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LogLocator(base=10))
    figure.xaxis.set_major_locator(LinearLocator(5))

    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".eps", bbox_inches='tight', format='eps')
    ConvertEpsToPdf(FIGURE_FOLDER + "/" + filename)


if __name__ == "__main__":
    N = 10000
    S = 100
    try:
        opts, args = getopt.getopt(sys.argv[1:], '-h-n:', ['number=', 'help'])
    except getopt.GetoptError:
        print('test.py -n number of join results')
        sys.exit(2)
    for opt, opt_value in opts:
        if opt in ('-h', '--help'):
            print("[*] Help info")
            exit()
        elif opt == '-n':
            print('Number of join results ', opt_value)
            N = (int)(opt_value)
        elif opt == '-s':
            print('Gap of sampling ', opt_value)
            S = (int)(opt_value)

    # 'Hash_JM', 'Hash_JB', 'Hash_HS', 'Sort_JM', 'Sort_JB', 'Sort_HS', 'PRJ'
    legend_labels = ['Hash_JM', 'Hash_JB', 'Sort_JM', 'Sort_JB', 'PRJ', 'NPJ']
    N=(int)(N/S)
    col0 = []
    for x in range(1, N + 1):
        col0.append(x * S)

    col1, col2, col3, col4, col5, col6 = ReadFile(S)
    # print(col1)
    lines = [col1, col2, col3, col4, col5, col6]
    DrawFigure(col0, lines, legend_labels,
               'Number of results', 'time (usec)', 0, N,
               10E6, (2 * 10E8),
               'progressive_results',
               True)
    # DrawLegend(legend_labels, 'interval_legend')

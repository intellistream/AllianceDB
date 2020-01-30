import getopt
import os
import sys
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

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#F15854', '#5DA5DA', '#60BD68', '#B276B2', '#DECF3F', '#F17CB0', '#B2912F', '#FAA43A', '#AFAFAF')
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


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


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
    for group in xrange(len(FIGURE_LABEL)):
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
                          label=FIGURE_LABEL[i])

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(bars, FIGURE_LABEL, prop=LEGEND_FP,
                   ncol=2,
                   #                     mode='expand',
                   #                     shadow=False,
                   bbox_to_anchor=(0.45, 1.6),
                   columnspacing=0.1,
                   handletextpad=0.2,
                   #                     bbox_transform=ax.transAxes,
                   #                     frameon=False,
                   #                     columnspacing=5.5,
                   #                     handlelength=2,
                   loc='upper center'
                   )
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

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".eps", bbox_inches='tight', format='eps')
    ConvertEpsToPdf(FIGURE_FOLDER + "/" + filename)


# example for reading csv file
def ReadFile(N):
    y = []
    col1 = []
    col2 = []
    col3 = []
    col4 = []
    col5 = []
    col6 = []

    f = open("/data1/xtra/results/timestamps/SHJ_JM_NP0.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col1.append(int(x.strip("\n")))
        cnt += 1
    y.append(col1)

    f = open("/data1/xtra/results/SHJ_JBCR_NP_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col2.append(int(x.strip("\n")))
        cnt += 1
    y.append(col2)

    f = open("/data1/xtra/results/PMJ_JM_NP_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col3.append(int(x.strip("\n")))
        cnt += 1
    y.append(col3)

    f = open("/data1/xtra/results/PMJ_JBCR_NP_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col4.append(int(x.strip("\n")))
        cnt += 1
    y.append(col4)

    f = open("/data1/xtra/results/PRJ_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col5.append(int(x.strip("\n")))
        cnt += 1
    y.append(col5)

    f = open("/data1/xtra/results/NPJ_timestamps.txt", "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            col6.append(int(x.strip("\n")))
        cnt += 1
    y.append(col6)
    return y


if __name__ == "__main__":
    N = 10000
    S = 4
    try:
        opts, args = getopt.getopt(sys.argv[1:], '-h-n:-s:', ['sample=', 'number=', 'help'])
    except getopt.GetoptError:
        print('test.py -n number of matches')
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

    N = int(N / S)

    x_values = ['Unique', 'Zipf(0)', 'Zipf(0.2)', 'Zipf(0.4)', 'Zipf(0.6)', 'Zipf(0.8)', 'Zipf(1)']

    y_values = ReadFile(N)

    legend_labels = ['Hash_JM', 'Hash_JB', 'Sort_JM', 'Sort_JB', 'PRJ', 'NPJ']

    DrawFigure(x_values, y_values, legend_labels, '', 'Throughput (events/sec)', 0, 1650, 'skew', True)

#  DrawLegend(legend_labels, 'factor_legend')

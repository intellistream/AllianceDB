import getopt
import os
import sys

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib import gridspec
from matplotlib.font_manager import FontProperties
from numpy import double
from numpy.ma import ceil

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 24
LEGEND_FONT_SIZE = 26
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "|", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#17202A', '#ABB2B9', '#F8F9F9', '#641E16', '#999933', '#2E86C1', '#CC6677', '#882255', '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (["", "\\\\", "//////", "o", "||", "\\\\", "\\\\", "//////", "//////", ".", "\\\\\\", "\\\\\\"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 0.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME

exp_dir = "/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


# draw a line chart
def DrawFigure(x_values, y_values, y_max, legend_labels, x_label, y_label, filename, id, allow_legend):
    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # If we were to simply plot pts, we'd lose most of the interesting
    # details due to the outliers. So let's 'break' or 'cut-out' the y-axis
    # into two portions - use the top (ax) for the outliers, and the bottom
    # (ax2) for the details of the majority of our data
    # f, (ax, ax2) = plt.subplots(2, 1, sharex=True,figsize=(9, 4))
    fig = plt.figure(figsize=(10, 4))
    # ax1 = fig.add_subplot(211)
    # ax2 = fig.add_subplot(212)

    gs = gridspec.GridSpec(2, 1, height_ratios=[5, 1])
    ax1 = plt.subplot(gs[0])
    ax2 = plt.subplot(gs[1])

    # ax1.set_yscale('log')
    # ax2.set_yscale('log')
    # ax = plt.subplot(111)    # The big subplot

    # Turn off axis lines and ticks of the big subplot
    # ax.spines['top'].set_color('none')
    # ax.spines['bottom'].set_color('none')
    # ax.spines['left'].set_color('none')
    # ax.spines['right'].set_color('none')
    # ax.tick_params(labelcolor='w', top=False, bottom=False, left=False, right=False)

    FIGURE_LABEL = legend_labels
    # values in the x_xis
    index = np.arange(len(x_values))
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.5
    # draw the bars
    bottom_base = np.zeros(len(y_values[0]))
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        # plot the same data on both axes
        if (i != 4):
            bars[i] = ax1.bar(index + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                              label=FIGURE_LABEL[i], bottom=bottom_base, edgecolor='black', linewidth=3)
            ax2.bar(index + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                    label=FIGURE_LABEL[i], bottom=bottom_base, edgecolor='black', linewidth=3)
            bottom_base = np.array(y_values[i]) + bottom_base
        else:
            bars[i] = ax1.bar(index + width / 2, y_values[i], 0, hatch='', linewidth=0, fill=False)
            ax2.bar(index + width / 2, y_values[i], 0, hatch='', linewidth=0, fill=False)

    # zoom-in / limit the view to different portions of the data
    ax1.set_ylim(14500, 15500)  # most of the data
    ax2.set_ylim(0, 10)  # waiting only

    # hide the spines between ax and ax2
    ax1.spines['bottom'].set_visible(False)
    ax2.spines['top'].set_visible(False)
    ax1.xaxis.tick_top()
    ax1.tick_params(labeltop=False)  # don't put tick labels at the top
    ax2.xaxis.tick_bottom()
    # This looks pretty good, and was fairly painless, but you can get that
    # cut-out diagonal lines look with just a bit more work. The important
    # thing to know here is that in axes coordinates, which are always
    # between 0-1, spine endpoints are at these locations (0,0), (0,1),
    # (1,0), and (1,1).  Thus, we just need to put the diagonals in the
    # appropriate corners of each of our axes, and so long as we use the
    # right transform and disable clipping.

    d = .015  # how big to make the diagonal lines in axes coordinates
    # arguments to pass to plot, just so we don't keep repeating them
    kwargs = dict(transform=ax1.transAxes, color='k', clip_on=False)
    ax1.plot((-d, +d), (-d, +d), **kwargs)  # top-left diagonal
    ax1.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

    kwargs.update(transform=ax2.transAxes)  # switch to the bottom axes
    ax2.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # bottom-left diagonal
    ax2.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # bottom-right diagonal

    # What's cool about this is that now if we vary the distance between
    # ax and ax2 via f.subplots_adjust(hspace=...) or plt.subplot_tool(),
    # the diagonal lines will move accordingly, and stay right at the tips
    # of the spines they are 'breaking'

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(bars, FIGURE_LABEL, prop=LEGEND_FP,
                   loc='upper center', ncol=len(legend_labels), mode='expand', bbox_to_anchor=(0.45, 1.2), shadow=False,
                   frameon=False, borderaxespad=0.0, handlelength=2, labelspacing=0.2)

    # plt.xlabel(x_label, fontproperties=LABEL_FP)
    # ax1.set_ylabel(y_label, fontproperties=LABEL_FP)
    # Set common labels
    fig.text(0.5, 0.04, x_label, ha='center', fontproperties=LABEL_FP)
    fig.text(0.04, 0.5, y_label, va='center', rotation='vertical', fontproperties=LABEL_FP)
    # ax.set_xlabel(x_label, fontproperties=LABEL_FP)
    # ax.set_ylabel(y_label, fontproperties=LABEL_FP)
    # plt.ticklabel_format(axis='y', style='sci', scilimits=(0,0))
    # ax1.tick_params(axis='y', which='major', pad=-40)
    # ax1.tick_params(axis='x', which='major', pad=-20)
    # ax2.tick_params(axis='y', which='major', pad=40)
    # ax2.tick_params(axis='x', which='major', pad=20)
    # plt.subplots_adjust(left=0.1, bottom=None, right=None, top=None, wspace=None, hspace=None)
    # plt.grid(axis='y', color='gray')
    # ax1.yaxis.set_major_locator(pylab.LinearLocator(3))
    # you may need to tune the xticks position to get the best figure.

    ax1.grid(axis='y', color='gray')
    ax2.grid(axis='y', color='gray')
    plt.xticks(index + 0.5 * width, x_values)
    plt.xticks(rotation=30)
    plt.tight_layout(rect=[0.065, 0, 1, 1])
    plt.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')
    # plt.savefig(FIGURE_FOLDER + "/" + filename + ".eps", bbox_inches='tight', format='eps')
    # ConvertEpsToPdf(FIGURE_FOLDER + "/" + filename)


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
    figlegend = pylab.figure(figsize=(11, 0.5))
    figlegend.legend(bars, FIGURE_LABEL, prop=LEGEND_FP, \
                     loc=9,
                     bbox_to_anchor=(0, 0.4, 1, 1),
                     ncol=len(FIGURE_LABEL), mode="expand", shadow=False, \
                     frameon=False, handlelength=1.1, handletextpad=0.2, columnspacing=0.1)

    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


def normalize(y_values):
    y_total_values = np.zeros(len(y_values[0]))

    for i in range(len(y_values)):
        y_total_values += np.array(y_values[i])
    y_norm_values = []

    for i in range(len(y_values)):
        y_norm_values.append(np.array(y_values[i]) / (y_total_values * 1.0))
    return y_norm_values


# example for reading csv file
def ReadFile(id):
    # Creates a list containing 8 lists, each of 7 items, all set to 0
    w, h = 9, 6
    y = [[0 for x in range(w)] for y in range(h)]
    # print(matches)
    max_value = 0

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/NPJ_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        value = double(x.strip("\n"))
        if (linecnt != 3):  ##skip sort.
            if value > max_value:
                max_value = value
            y[cnt][0] = value
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/PRJ_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        value = double(x.strip("\n"))
        if (linecnt != 3):  ##skip sort.
            if value > max_value:
                max_value = value
            y[cnt][1] = value
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/MWAY_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        value = double(x.strip("\n"))
        if (linecnt != 2):  ##skip build.
            if value > max_value:
                max_value = value
            y[cnt][2] = value
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/MPASS_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        value = double(x.strip("\n"))
        if (linecnt != 2):  ##skip build.
            if value > max_value:
                max_value = value
            y[cnt][3] = value
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/SHJ_JM_P_{}.txt".format(id), "r")
    read = f.readlines()
    for _ in read:
        if (linecnt != 3):  ##skip sort.
            y[cnt][4] = 0  # deliminator
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/SHJ_JM_P_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        if (linecnt != 3):  ##skip sort.
            value = double(x.strip("\n"))
            if value > max_value:
                max_value = value
            y[cnt][5] = value
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/SHJ_JBCR_P_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        if (linecnt != 3):  ##skip sort.
            value = double(x.strip("\n"))
            if value > max_value:
                max_value = value
            y[cnt][6] = value
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/PMJ_JM_P_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        if (linecnt != 2):  ##skip build.
            value = double(x.strip("\n"))
            if value > max_value:
                max_value = value
            y[cnt][7] = value
            cnt += 1
        linecnt += 1

    cnt = 0
    linecnt = 0
    f = open(exp_dir + "/results/breakdown/PMJ_JBCR_P_{}.txt".format(id), "r")
    read = f.readlines()
    for x in read:
        if (linecnt != 2):  ##skip build.
            value = double(x.strip("\n"))
            if value > max_value:
                max_value = value
            y[cnt][8] = value
            cnt += 1
        linecnt += 1
    return y, max_value


if __name__ == "__main__":
    id = 38
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

    x_values = ['NPJ', 'PRJ', 'MWAY', 'MPASS', '',
                'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$',
                'PMJ$^{JB}$']  # join time is getting from total - others.

    y_values, max_value = ReadFile(id)

    # y_norm_values = normalize(y_values)

    # break into 4 parts
    legend_labels = ['wait', 'partition', 'build/sort', 'merge', 'probe', 'others']  #

    DrawFigure(x_values, y_values, double(ceil(max_value / 1000.0)) * 1000, legend_labels, '',
               'cycles per input',
               'breakdown_figure{}'.format(id), id, False)

    #DrawLegend(legend_labels, 'breakdown_legend')

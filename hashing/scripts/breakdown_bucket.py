import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator
from numpy import double

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 24
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
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
matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/home/tangxilin/S-AllianceDB/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


# draw a line chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(12, 3.5))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    index = np.arange(len(x_values))
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.6
    # draw the bars
    bottom_base = np.zeros(len(y_values[0]))
    bars = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + width / 2, y_values[i], width, hatch=PATTERNS[i], color=LINE_COLORS[i],
                          label=FIGURE_LABEL[i], bottom=bottom_base, edgecolor='black', linewidth=3)
        bottom_base = np.array(y_values[i]) + bottom_base

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(bars, FIGURE_LABEL,
                   #                     mode='expand',
                   #                     shadow=False,
                   #                     columnspacing=0.25,
                   #                     labelspacing=-2.2,
                   #                     borderpad=5,
                   #                     bbox_transform=ax.transAxes,
                   frameon=True,
                   #                     columnspacing=5.5,
                   #                     handlelength=2,
                   )
        handles, labels = figure.get_legend_handles_labels()

        leg = plt.legend(handles[::-1], labels[::-1],
                         loc='right',
                         prop=LEGEND_FP,
                         ncol=1,
                         bbox_to_anchor=(1.2, 0.5),
                         handletextpad=0.2,
                         borderaxespad=0.0,
                         handlelength=1.8,
                         labelspacing=0.3,
                         columnspacing=0.3,
                         )
        leg.get_frame().set_linewidth(2)
        leg.get_frame().set_edgecolor("black")

    # you may need to tune the xticks position to get the best figure.
    plt.xticks(index + 0.5 * width, x_values)

    # plt.xlim(0,)
    # plt.ylim(0,1)

    plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LinearLocator(6))

    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    # plt.xticks(rotation=30)

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight', format='pdf')


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
def ReadFile():
    # Creates a list containing w lists, each of h items, all set to 0
    w, h = 5, 2
    y = [[0 for x in range(w)] for y in range(h)]
    j = 0
    id = 108

    bound = id + 1 * w
    for i in range(id, bound, 1):
        cnt = 0
        print(i)
        f = open(exp_dir + "/results/breakdown/NPJ_{}.txt".format(i), "r")
        read = f.readlines()
        others = 0
        for x in read:
            value = double(x.strip("\n"))
            # if cnt == 1:
            # y[0][j] = value
            if cnt == 2:
                y[0][j] = value
            elif cnt == 5:
                y[1][j] = value
            else:
                others += value
            cnt += 1
        j += 1
    print(y)
    return y


if __name__ == "__main__":
    x_values = [
        1, 2, 4, 8, 16
    ]  # different bucket size.

    y_values = ReadFile()  #

    # y_norm_values = normalize(y_values)

    # break into 4 parts
    legend_labels = ['build', 'probe']  # , 'others'

    DrawFigure(x_values, y_values, legend_labels,
               '', 'cycles per input',
               'breakdown_bucket_figure', True)

    # DrawLegend(legend_labels, 'breakdown_radix_legend')

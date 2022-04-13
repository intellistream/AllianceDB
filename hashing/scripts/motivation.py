import itertools as it
import os

import re
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 21
# TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 21
LEGEND_FONT_SIZE = 28
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (["", 'o', 's', 'v', "^", "", "h", "<", ">", "+", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
# COLOR_MAP = ('#FFFFFF', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
#COLOR_MAP = ('#cc0099', '#006699', '#239B56', '#239B56', '#cc00ff', '#cc66ff', '#ccccff', '#F5CBA7', '#239B56', '#009999', '#00cc99', '#00ff99', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499', '#FFFFFF', '#AA4499', '#AA4499', '#FFFFFF', '#AA4499', '#AA4499', '#82E0AA', '#AEB6BF')
COLOR_MAP = ('#545456', '#878789', '#239B56', '#239B56', '#545456', '#878789', '#bbbbbb', '#F5CBA7', '#239B56', '#545456', '#878789', '#bbbbbb', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499', '#FFFFFF', '#AA4499', '#AA4499', '#FFFFFF', '#AA4499', '#AA4499', '#82E0AA', '#AEB6BF')
# you may want to change the patterns for different figures
# PATTERNS = (["", "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
#PATTERNS = [ "/", "/", "//", '', "O", "o", "", '', "-", "O", "o", "", "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "", "o", "O", "", "o", "O", "", "//", "\\"]
PATTERNS = [ "", "", "//", '', "", "", "", '', "-", "", "", "", "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "", "o", "O", "", "o", "O", "", "//", "\\"]
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 15.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = 21
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/home/tangxilin/S-AllianceDB/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'
'''
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
'''
# draw a bar chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(13, 5))
    figure = fig.add_subplot(111)

    # FIGURE_LABEL = legend_labels

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)

    # values in the x_xis
    # index = np.arange(len(x_values))
    index = np.arange(1)
    # the bar width.
    # you may need to tune it to get the best figure.
    width = 0.2
    # draw the bars
    bars = [None] * (len(x_values))
    # for i in range(len(y_values)):
    #     bars[i] = plt.bar(index + i * width + width / 2,
    #                       y_values[i], width,
    #                       hatch=PATTERNS[i],
    #                       color=LINE_COLORS[i],
    #                       label=FIGURE_LABEL[i],edgecolor='black', linewidth=3)
    for i in range(len(y_values)):
        bars[i] = plt.bar(index + i * width + width / 2,
                          y_values[i], width,
                          hatch=PATTERNS[i],
                          color=LINE_COLORS[i],
                          edgecolor='black', linewidth=3)
    '''
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
    '''
    # you may need to tune the xticks position to get the best figure.
    index = np.arange(len(x_values))
    plt.xticks(index * width + width/2, x_values, rotation = 18,)
    # plt.ticklabel_format(axis="y", style="sci", scilimits=(0, 0))
    # plt.grid(axis='y', color='gray')
    # figure.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())

    # you may need to tune the xticks position to get the best figure.
    plt.yscale('log')
    #
    # plt.grid(axis='y', color='gray')
    figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_locator(LinearLocator(5))
    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)
    plt.show()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')

# example for reading csv file
def ReadFile():
    
    col = []
######## non-sample
    hash_non_smp_lz = ['NPO','PRO']
    algo = 'SHJ_JM_NP'
    for ind in ['0', '1']:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + algo + '_profile_0-' + ind + '_39.txt'
        print(file)
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'95th latency: \((.*?)\)', fc)[0]))
    col.append([])
    col.append([])

    for ind in ['2', '3', '4']:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + algo + '_profile_0-' + ind + '_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'95th latency: \((.*?)\)', fc)[0]))
    col.append([])
    col.append([])

    for ind in ['5', '6', '7']:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + algo + '_profile_0-' + ind + '_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'95th latency: \((.*?)\)', fc)[0]))
    
    return col

if __name__ == "__main__":
    # x_values = ["Stock", "Rovio", "YSB", "DEBS"]
    x_values = ['one core', 'eight cores', '','', '$\epsilon=0.9$', '$\epsilon=0.5$', '$\epsilon=0.1$', '', '', '$\epsilon=0.9$', '$\epsilon=0.5$', '$\epsilon=0.1$']

    y_values = ReadFile()

    # legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
    #                  'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
                     'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    print(y_values)
    DrawFigure(x_values, y_values, legend_labels,
              # 'non-sampling' + ' '*18 + 'one core' + ' '*18 + 'eight cores' + ' '*5, 'Latency (ms)', 0,
              'non-sampling' + ' '*24 + 'one core' + ' '*30 + 'eight cores' + ' '*8, 'Latency (ms)', 0,
               400, 'motivation', False)

    # DrawLegend(legend_labels, 'latency_legend')

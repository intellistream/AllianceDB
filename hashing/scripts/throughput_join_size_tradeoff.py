import itertools as it
import os

import numpy as np

import re
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator
from matplotlib.ticker import MultipleLocator

OPT_FONT_NAME = 'Helvetica'
# TICK_FONT_SIZE = 24
TICK_FONT_SIZE = 10
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = [ 'o', 's', 'v', "^", "", "h", "<", ">", "+","", "d", "<","", "|", "+", '','o', 's']
# you may want to change the color map for different figures
# COLOR_MAP = ('#FFFFFF', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
COLOR_MAP = ('#B03A2E','#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499', '#8B6914', '#AA4499', '#AA4499', '#8B6914', '#AA4499', '#AA4499', '#82E0AA', '#AEB6BF')
# you may want to change the patterns for different figures
# PATTERNS = (["", "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
PATTERNS = [ "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "", "////", "\\\\", "//", "o", "", "||", "-", "//", "\\", "", "o", "O", "", "o", "O", "", "//", "\\"]
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 15.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = 10
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/home/tangxilin/S-AllianceDB/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'

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

# draw a bar chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    # fig = plt.figure(figsize=(10, 3))
    fig = plt.figure(figsize=(5, 3))
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
    # bars = [None] * (len(x_values))
    # for i in range(len(y_values)):
    #     bars[i] = plt.bar(index + i * width + width / 2,
    #                       y_values[i], width,
    #                       hatch=PATTERNS[i],
    #                       color=LINE_COLORS[i],
    #                       label=FIGURE_LABEL[i],edgecolor='black', linewidth=3)
    # for i in range(len(y_values)):
    #     bars[i] = plt.bar(index + i * width + width / 2,
    #                       y_values[i], width,
    #                       hatch=PATTERNS[i],
    #                       color=LINE_COLORS[i],
    #                       edgecolor='black', linewidth=3)
    # sometimes you may not want to draw legends.
    # if allow_legend == True:
    #     plt.legend(bars, FIGURE_LABEL,
    #                prop=LEGEND_FP,
    #                ncol=4,
    #                loc='upper center',
    #                #                     mode='expand',
    #                shadow=False,
    #                bbox_to_anchor=(0.45, 1.6),
    #                columnspacing=0.1,
    #                handletextpad=0.2,
    #                #                     bbox_transform=ax.transAxes,
    #                #                     frameon=True,
    #                #                     columnspacing=5.5,
    #                #                     handlelength=2,
    #                )
    # you may need to tune the xticks position to get the best figure.
    # index = np.arange(len(x_values))
    # plt.xticks(index * width + width/2, x_values, rotation = 30)
    # lala = []
    # for i in y_values:
    #     if (isinstance(i, float)):
    #         lala.append(i)
    #     else:
    #         lala.append(0.0)
    # plt.yticks(lala,lala)
    # plt.ticklabel_format(axis="y", style="sci", scilimits=(0, 0))
    # plt.grid(axis='y', color='gray')
    # figure.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())

    for idx, x in enumerate(x_values):
        if idx != 4 and idx != 9 and idx != 12 and idx != 15 :
            plt.scatter(x_values[idx], y_values[idx], c = LINE_COLORS[idx], marker = MARKERS[idx])

    # you may need to tune the xticks position to get the best figure.
    # plt.yscale('log')
    #
    # plt.grid(axis='y', color='gray')
    # figure.yaxis.set_major_locator(LogLocator(base=10, subs=(1.0,1.5,1.7,5.0,) ))
    # figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.yaxis.set_major_locator(MultipleLocator(base=10))
    # figure.xaxis.set_major_locator(LinearLocator(5))
    # figure.get_xaxis().set_tick_params(direction='in', pad=10)
    # figure.get_yaxis().set_tick_params(direction='in', pad=10)

    # plt.yscale('log')
    # plt.xscale('log')
    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)
    # plt.show()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')

# example for reading csv file
def ReadFile_throughput():
    
    col = []
######## non-sample
    hash_non_smp_lz = ['NPO','PRO']
    for str in hash_non_smp_lz:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_0_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))
    sort_non_smp_lz = ['m-pass','m-way']
    for str in sort_non_smp_lz:
        file = exp_dir + '/results/breakdown/Rovio_' + str + '_profile_0_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))
    

    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP', 'PMJ_JM_NP', 'PMJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_0_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))

########### sample

    col.append(0)

    hash_non_smp_lz = ['NPO','PRO']
    for str in hash_non_smp_lz:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_1_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))
    sort_non_smp_lz = ['m-pass','m-way']
    for str in sort_non_smp_lz:
        file = exp_dir + '/results/breakdown/Rovio_' + str + '_profile_1_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))
    

    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP', 'PMJ_JM_NP', 'PMJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_1_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))

########### probhash



    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_2_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))
            
########### sraj



    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_3_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))

########### PROBE_ALL



    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_4_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(2873604*2*1000/float(re.findall(r'99th latency: \((.*?)\)', fc)[0]))

    return col

def ReadFile_join_size():
    
    col = []
######## non-sample
    hash_non_smp_lz = ['NPO','PRO']
    for str in hash_non_smp_lz:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_0_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
    sort_non_smp_lz = ['m-pass','m-way']
    for str in sort_non_smp_lz:
        file = exp_dir + '/results/breakdown/Rovio_' + str + '_profile_0_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
    

    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP', 'PMJ_JM_NP', 'PMJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_0_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))

########### sample

    col.append(0)

    hash_non_smp_lz = ['NPO','PRO']
    for str in hash_non_smp_lz:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_1_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
    sort_non_smp_lz = ['m-pass','m-way']
    for str in sort_non_smp_lz:
        file = exp_dir + '/results/breakdown/Rovio_' + str + '_profile_1_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
    

    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP', 'PMJ_JM_NP', 'PMJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_1_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))

########### probhash



    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_2_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
            
########### sraj



    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_3_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))

########### PROBE_ALL



    col.append(0)

    hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
    for str in hash_non_smp_eg:
        file = exp_dir + '/results/breakdown/ALL_ON_Rovio_' + str + '_profile_4_39.txt'
        with open(file, "r") as f:
            fc = f.read()
            col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
    return col

if __name__ == "__main__":
    # x_values = ["Stock", "Rovio", "YSB", "DEBS"]
    x_values = ['NPJ', 'PRJ', 'MWAY', 'MPASS',
                     '', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$',
                     '', 'S-NPJ', 'S-PRJ', 'S-MWAY', 'S-MPASS',
                     '', 'S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$',
                     '', 'PROBHASH$^{JM}$', 'PROBHASH$^{JB}$',
                     '', 'SRAJ$^{JM}$', 'SRAJ$^{JB}$',
                     '', 'PROBALL$^{JM}$', 'PROBALL$^{JB}$',]

    x_values = ReadFile_throughput()
    y_values = ReadFile_join_size()

    x_values = x_values[10:]
    y_values = y_values[10:]
    print(x_values)

    x_values = np.array(x_values)/np.max(x_values)
    y_values = np.array(y_values)/np.max(y_values)

    # legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
    #                  'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    legend_labels = ['Lazy:', 'S-NPJ', 'S-PRJ', 'S-MWAY', 'S-MPASS',
                     'Eager:', 'S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$',
                     '', 'PROBHASH$^{JM}$', 'PROBHASH$^{JB}$',
                     '', 'SRAJ$^{JM}$', 'SRAJ$^{JB}$',
                     '', 'PROBALL$^{JM}$', 'PROBALL$^{JB}$']
    print(y_values)
    DrawFigure(x_values, y_values, legend_labels,
               'Normalized Tpt. ', 'Normalized Join Size', 0,
               400, 'throughput_join_size_tradeoff', False)

    # DrawLegend(legend_labels, 'latency_legend')

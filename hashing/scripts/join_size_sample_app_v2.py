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
# TICK_FONT_SIZE = 24
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 30
X_LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
X_LABEL_FP = FontProperties(style='normal', size=X_LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (["", 'o', 's', 'v', "^", "", "h", "<", ">", "+", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
# COLOR_MAP = ('#FFFFFF', '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
COLOR_MAP = (
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',

'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',

'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF', '#FFFFFF',

'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF',
)
# you may want to change the patterns for different figures
# PATTERNS = (["", "//", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "//", ".", "|||", "o", "---", "+", "\\\\", "*"])
PATTERNS = [ "//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "", "", "",
]
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 15.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = 24
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42

exp_dir = "/home/tangxilin/S-AllianceDB/data1/xtra"

FIGURE_FOLDER = exp_dir + '/results/figure'

# draw a bar chart
def DrawFigure(x_values, y_values, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(18, 5))
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
    plt.xticks(index * width + width/2, x_values, rotation = 85)
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

    plt.xlabel(x_label, fontproperties=X_LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)
    plt.show()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')

# example for reading csv file
def ReadFile():
    
    # res = []
    col = []
    # for Datastr in [['Stock_', '_38'], ['Rovio_', '_39'], ['YSB_', '_40']]:
    # for Datastr in [['Rovio_', '_39'], ['Stock_', '_38'], ['YSB_', '_40'], ['DEBS_', '_41']]:
    gp_ind = -1
    for Datastr in [['Rovio_', '_39', (2873604*2)], ['DEBS_', '_41', (2e6)], ['Stock_', '_38', (1013800+1034443)]]:
    # for Datastr in [ ['Rovio_', '_39']]:
    ########### sample

        gp_ind = gp_ind + 1
        sort_non_smp_lz = ['m-pass','m-way']
        for algo in sort_non_smp_lz:
            file = exp_dir + '/results/breakdown/' + Datastr[0] + algo + '_profile_' + str(1) + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
        hash_non_smp_lz = ['NPO','PRO']
        for algo in hash_non_smp_lz:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(100) + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
        

        # col.append([])

        hash_non_smp_eg = ['PMJ_JM_NP', 'PMJ_JBCR_NP', 'SHJ_JM_NP', 'SHJ_JBCR_NP']
        for algo in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(100) + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))

    ## PROBEALLL
        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for algo in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(400) + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
                
    ########### sraj

        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for algo in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(300) + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))

    
    ########### sraj probe all
        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for algo in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + algo + '_profile_' + str(500) + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
    ########### probhash


        '''
        col.append([])

        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_2' + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(float(re.findall(r'Results = (.*?). DONE.', fc)[0]))
        '''




        col.append([])
        col.append([])
        col.append([])

        # res.append(col)
    col.pop(-1)
    col.pop(-1)
    col.pop(-1)
    return col

if __name__ == "__main__":
    # x_values = ["EECR", "Rovio", "YSB"]
    x_values = [
                     'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'PS-SHJ$^{JM}$', 'PS-SHJ$^{JB}$', 
                     'SRAJ$^{JM}$', 'SRAJ$^{JB}$',
                     'PS-SRAJ$^{JM}$', 'PS-SRAJ$^{JB}$',
                     '','','',
                     'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'PS-SHJ$^{JM}$', 'PS-SHJ$^{JB}$', 
                     'SRAJ$^{JM}$', 'SRAJ$^{JB}$',
                     'PS-SRAJ$^{JM}$', 'PS-SRAJ$^{JB}$',
                     '','','',
                     'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'PS-SHJ$^{JM}$', 'PS-SHJ$^{JB}$', 
                     'SRAJ$^{JM}$', 'SRAJ$^{JB}$',
                     'PS-SRAJ$^{JM}$', 'PS-SRAJ$^{JB}$',
                    #  '','',
                    #  'MWAY', 'MPASS','NPJ', 'PRJ', 
                    #  'PMJ$^{JM}$', 'PMJ$^{JB}$','SHJ$^{JM}$', 'SHJ$^{JB}$', 
                    #  '', 'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                    #  'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                    #  'PS-SHJ$^{JM}$', 'PS-SHJ$^{JB}$', 
                    #  'SRAJ$^{JM}$', 'SRAJ$^{JB}$',
                    #  'PS-SRAJ$^{JM}$', 'PS-SRAJ$^{JB}$',
                     ]

    y_values = ReadFile()

    # legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
    #                  'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
                     'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    print(y_values)
    DrawFigure(x_values, y_values, legend_labels,
            #    'Rovio' + ' '*50 + 'EECR' + ' '*70 + 'YSB' + ' '*50 + 'DEBS', 'Latency (ms)', 0,
            #    'Rovio' + ' '*100 + 'EECR' + ' '*100 + 'DEBS',
               'Rovio' + ' '*30 + 'DEBS' + ' '*30 + 'EECR',
               'Join Size', 0,
               400, 'join_size_sample_figure_app', False)

    # DrawLegend(legend_labels, 'latency_legend')

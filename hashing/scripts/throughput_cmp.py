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
# '#6ac1c2', '#75c8c9', '#72c2a4', '#72d1ae', '#535ae1', '#6067e7', '#6970e5', '#757be5', '#FFFFFF', 
# '#1c7d7e', '#269091', '#32a0a1', '#46b9ba', '#Ba1697', '#Cb30aa','#Db4cbd', '#Ec79d3', '#888de3', '#9ea2ec', 
# '#2b33c7', '#333bcd', '#3c44d7', '#464edd', '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',

'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#1c7d7e', '#32a0a1',  '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#1c7d7e', '#32a0a1',  '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#1c7d7e', '#32a0a1',  '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',

# , '#1c7d7e', '#32a0a1'

'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#1c7d7e', '#269091', '#32a0a1', '#46b9ba', '#00b300', '#00e600',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff',  '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#1c7d7e', '#269091', '#32a0a1', '#46b9ba', '#72c2a4', '#72d1ae',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff',  '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#6ac1c2', '#75c8c9', '#72c2a4', '#72d1ae', '#535ae1', '#6067e7', '#6970e5', '#757be5', '#FFFFFF', 
'#1c7d7e', '#269091', '#32a0a1', '#46b9ba', '#Ba1697', '#Cb30aa','#Db4cbd', '#Ec79d3', '#888de3', '#9ea2ec', 
'#2b33c7', '#333bcd', '#3c44d7', '#464edd', '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#6ac1c2', '#75c8c9', '#72c2a4', '#72d1ae', '#535ae1', '#6067e7', '#6970e5', '#757be5', '#FFFFFF', 
'#1c7d7e', '#269091', '#32a0a1', '#46b9ba', '#Ba1697', '#Cb30aa','#Db4cbd', '#Ec79d3', '#888de3', '#9ea2ec', 
'#2b33c7', '#333bcd', '#3c44d7', '#464edd', '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',
'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
'#0000b3', '#0000e6', '#1a1aff', '#4d4dff', '#FFFFFF', '#FFFFFF',
)
#'#cc00cc', '#ff00ff', '#ff33ff', '#ff66ff', '#cc0000', '#ff0000', '#ff3333', '#ff6666', '#FFFFFF', 
#'#e68a00', '#ffa31a', '#ffb84d', '#ffcc80', '#00b300', '#00e600', '#1aff1a', '#4dff4d', '#80ff80', '#b3ffb3',
#'#0000b3', '#0000e6', '#1a1aff', '#4d4dff',  '#c2c4c3', '#FFFFFF', '#FFFFFF', '#FFFFFF',

# you may want to change the patterns for different figures
# PATTERNS = (["", "//", "\\\\", "//", "o", "", "||", "-", "//", "\\", "o", "O", "//", ".", "|||", "o", "---", "+", "\\\\", "*"])
PATTERNS = [ "//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o","-", "-", "|", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o","-", "-", "|", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o","-", "-", "|", "", "", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "",
"//", "\\\\", "/", "\\", "//", "\\\\", "/", "\\", "O", "o",
"/", "\\", "O", "o", "|", "", "", "",
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
    fig = plt.figure(figsize=(36, 5))
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
    plt.xticks(index * width + width/2, x_values, rotation = 80)
    # plt.ticklabel_format(axis="y", style="sci", scilimits=(0, 0))
    # plt.grid(axis='y', color='gray')
    # figure.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())

    # you may need to tune the xticks position to get the best figure.
    plt.yscale('log')
    #
    # plt.grid(axis='x', color='gray', ls='--', alpha=0.3)
    plt.grid(axis='y', color='gray', ls='--', alpha=0.3)
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
    for Datastr in [['Rovio_', '_39', (2873604*2)], ['DEBS_', '_41', (2e6)], ['Stock_', '_38', (1013800+1034443)]]:
    # for Datastr in [ ['Rovio_', '_39']]:
    ######## non-sample
        sort_non_smp_lz = ['m-way','m-pass']
        for str in sort_non_smp_lz:
            file = exp_dir + '/results/breakdown/' + Datastr[0] + str + '_profile_0' + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))
        hash_non_smp_lz = ['NPO','PRO']
        for str in hash_non_smp_lz:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_0' + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))

        hash_non_smp_eg = ['PMJ_JM_NP', 'PMJ_JBCR_NP', 'SHJ_JM_NP', 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_0' + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))
                # if str=='SHJ_JM_NP':
                #     print(file)
                #     print(fc)

    ########### sample

        # col.append([])
        col.append([])

        # Grpstr = ['21150', '26150', '24150', '28150', '30150', '32150', ]
        # Grpstr = ['21400', '26450', '24450', '28450', '30450', '32450', ]
        # Grpstr = ['21000', '26000', '24000', '28000', '30000', '32000', ]

        sort_non_smp_lz = ['m-way','m-pass']
        for str in sort_non_smp_lz:
            file = exp_dir + '/results/breakdown/' + Datastr[0] + str + '_profile_' + Grpstr[0] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))
        hash_non_smp_lz = ['NPO','PRO']
        for str in hash_non_smp_lz:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_' + Grpstr[0] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))
        

        # col.append([])

        hash_non_smp_eg = ['PMJ_JM_NP', 'PMJ_JBCR_NP', 'SHJ_JM_NP', 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_' + Grpstr[0] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))

    ## PROBEALLL
        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_' + Grpstr[1] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))
                
    ########### sraj

        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_' + Grpstr[2] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))

    
    ########### sraj probe all
        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_' + Grpstr[3] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))
    ########### probhash


        '''
        col.append([])

        hash_non_smp_eg = ['SHJ_JM_NP', 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_2' + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))
        '''

        hash_non_smp_eg = ['SHJ_JM_NP']#, 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_' + Grpstr[4] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))

    ########### StrApp
        hash_non_smp_eg = ['SHJ_JM_NP']#, 'SHJ_JBCR_NP']
        for str in hash_non_smp_eg:
            file = exp_dir + '/results/breakdown/ALL_ON_' + Datastr[0] + str + '_profile_' + Grpstr[5] + Datastr[1] + '.txt'
            with open(file, "r") as f:
                fc = f.read()
                col.append(Datastr[2]/(float(re.findall(r'99th latency: \((.*?)\)', fc)[0])+1000))

    
        # print(np.array(col[-14:]))
        col.append(np.max(np.array(col[-16:-2])))
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
    
    x_values = ['MWAY', 'MPASS','NPJ', 'PRJ', 
                     'PMJ$^{JM}$', 'PMJ$^{JB}$','SHJ$^{JM}$', 'SHJ$^{JB}$', 
                     '', 'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'B-SHJ$^{JM}$', 'B-SHJ$^{JB}$', 
                     'RSHJ$^{JM}$', 'RSHJ$^{JB}$',
                     'B-RSHJ$^{JM}$', 'B-RSHJ$^{JB}$',
                     'Prior', 'StrApp',
                     'ASSMJoin',
                     '','','',
                     'MWAY', 'MPASS','NPJ', 'PRJ', 
                     'PMJ$^{JM}$', 'PMJ$^{JB}$','SHJ$^{JM}$', 'SHJ$^{JB}$', 
                     '', 'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'B-SHJ$^{JM}$', 'B-SHJ$^{JB}$', 
                     'RSHJ$^{JM}$', 'RSHJ$^{JB}$',
                     'B-RSHJ$^{JM}$', 'B-RSHJ$^{JB}$',
                     'Prior', 'StrApp',
                     'ASSMJoin',
                     '','','',
                     'MWAY', 'MPASS','NPJ', 'PRJ', 
                     'PMJ$^{JM}$', 'PMJ$^{JB}$','SHJ$^{JM}$', 'SHJ$^{JB}$', 
                     '', 'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                     'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                     'B-SHJ$^{JM}$', 'B-SHJ$^{JB}$', 
                     'RSHJ$^{JM}$', 'RSHJ$^{JB}$',
                     'B-RSHJ$^{JM}$', 'B-RSHJ$^{JB}$',
                     'Prior', 'StrApp',
                     'ASSMJoin',
                    #  '','',
                    #  'MWAY', 'MPASS','NPJ', 'PRJ', 
                    #  'PMJ$^{JM}$', 'PMJ$^{JB}$','SHJ$^{JM}$', 'SHJ$^{JB}$', 
                    #  '', 'S-MWAY', 'S-MPASS','S-NPJ', 'S-PRJ', 
                    #  'S-PMJ$^{JM}$', 'S-PMJ$^{JB}$','S-SHJ$^{JM}$', 'S-SHJ$^{JB}$', 
                    #  'B-SHJ$^{JM}$', 'B-SHJ$^{JB}$', 
                    #  'RSHJ$^{JM}$', 'RSHJ$^{JB}$',
                    #  'B-RSHJ$^{JM}$', 'B-RSHJ$^{JB}$',
                     ]
    GrpBase = [21000, 26000, 24000, 28000, 30000, 32000, ]
    GrpstrBuc = []
    for i in range(9):
        GrpstrBuc.append([str(GrpBase[k]+50*i) for k in range(len(GrpBase))])
    # Grpstr = ['21400', '26450', '24450', '28450', '30450', '32450', ]
    GrpstrBuc.append(['21400', '26450', '24450', '28450', '30450', '32450', ])

    # cmp_ind = [ [23, 52, 81, 24, 53, 82], [25, 54, 83, 25, 54, 83] ]
    cmp_ind = [[0,1,2,3,4,5,6,7,6,7,6,7,6,7], [9,10,11,12,13,14,15,16,17,18,19,20,21,22]]
    for j in [0, 1]:
        cmp_ind[j] = cmp_ind[j] + [cmp_ind[j][i]+29 for i in range(len(cmp_ind[j]))] + [cmp_ind[j][i]+29+29 for i in range(len(cmp_ind[j]))]
    res = []
    for Grpstr in GrpstrBuc:
        y_values = ReadFile()
        # print(y_values)
        for i in range(len(cmp_ind[0])):
            # if (y_values[cmp_ind[1][i]])/y_values[cmp_ind[0][i]] > 10:
                # continue
            res.append((y_values[cmp_ind[1][i]])/y_values[cmp_ind[0][i]])
            # print(cmp_ind[0][i], cmp_ind[1][i])
        # sys.exit(0)
    print(res)
    print(np.mean(res))

    # legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
    #                  'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
                     'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']
    print(y_values)
    # DrawFigure(x_values, y_values, legend_labels,
    #         #    'Rovio' + ' '*50 + 'EECR' + ' '*70 + 'YSB' + ' '*50 + 'DEBS', 'Latency (ms)', 0,
    #         #    'Rovio' + ' '*100 + 'EECR' + ' '*100 + 'DEBS',
    #            'Rovio' + ' '*70 + 'DEBS' + ' '*70 + 'EECR',
    #            'Tpt. (#inputs/ms)', 0,
    #            400, 'throughput_sample_figure_app', False)

    # DrawLegend(legend_labels, 'latency_legend')

import itertools as it
import os

import matplotlib
import matplotlib.pyplot as plt
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import MaxNLocator, LinearLocator
from matplotlib import rc


OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 24
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)
MARKERS = (['', '^', 'v', '<', ">", '', "8", "s", "p", "P", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = (
    '#5499C7', '#ABB2B9', '#2E4053', '#8D6E63', '#000000', '#5499C7', '#CD6155', '#52BE80', '#FFFF00', '#5499C7',
    '#BB8FCE')
# you may want to change the patterns for different figures
PATTERNS = (['', "", "", "", "", '', "/", "\\", "||", "-", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 13.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
rc('text.latex', preamble=r'\usepackage[cm]{sfmath}')
rc('font',**{'family':'sans-serif',
             'sans-serif':['Helvetica'],
             'weight' : 'bold',
             'size'   : 22
             }
   )
rc('text', usetex=True)

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
    MARKER_SIZE = 20.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(17, 0.5))
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    for idx in range(len(FIGURE_LABEL)):
        if (idx != 0 or idx != 5):
            lines[idx], = ax1.plot(x_values, data,
                               color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                               marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(idx),
                               markeredgewidth=2, markeredgecolor='k'
                               )
        else:
            lines[idx] = ax1.plot(x_values, y_values, color='white', \
                                   linewidth=0, marker='None', \
                                   markersize=0, label=FIGURE_LABEL[idx],
                                   markevery=0, markeredgewidth=0, markeredgecolor='k'
                                   )

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=False, borderaxespad=-0.2, handlelength=0.8)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')

class ScalarFormatterForceFormat(matplotlib.ticker.ScalarFormatter):
    def _set_format(self):  # Override function that finds format to use.
        self.format = "%1.1f"  # Give format here

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
                   loc='upper center',
                   ncol=3,
                   #                     mode='expand',
                   bbox_to_anchor=(0.55, 1.5), shadow=False,
                   columnspacing=0.1,
                   frameon=True, borderaxespad=0.0, handlelength=1.5,
                   handletextpad=0.1,
                   labelspacing=0.1)
    plt.xscale('log')
    # plt.yscale('log')
    plt.xticks(x_values)
    # you may control the limits on your own.
    plt.xlim(x_min, x_max)
    plt.ylim(0, 41000)
    # plt.ylim(y_min, y_max)
    plt.ticklabel_format(axis="y", style="sci", scilimits=(0,0))
    plt.grid(axis='y', color='gray')

    figure.get_xaxis().set_major_formatter(matplotlib.ticker.ScalarFormatter())
    figure.yaxis.set_major_locator(LinearLocator(3))
    # figure.xaxis.set_major_locator(MaxNLocator(integer=True))

    # figure.get_xaxis().set_tick_params(direction='in', pad=10)
    # figure.get_yaxis().set_tick_params(direction='in', pad=10)

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')


def GetThroughput(file, file2):
    f = open(file, "r")
    f2 = open(file2, "r")
    read = f.readlines()
    read2 = f2.readlines()
    x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
    i = float(read2.pop(0).strip("\n"))  # get number of inputs
    return i / x  # get throughput (#items/ms)


# example for reading csv file
def ReadFile():
    y = []
    col1 = []
    col2 = []
    col3 = []
    col4 = []
    col5 = []
    col6 = []
    col7 = []
    col8 = []
    col9 = []

    for id in it.chain(range(0, 5)):
        col9.append(0)
    y.append(col9)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/PRJ_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/PRJ_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col1.append(value)
    y.append(col1)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/NPJ_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/NPJ_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col2.append(value)
    y.append(col2)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/MPASS_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/MPASS_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col3.append(value)
    y.append(col3)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/MWAY_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/MWAY_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col4.append(value)
    y.append(col4)

    y.append(col9)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/SHJ_JM_NP_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/SHJ_JM_NP_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col5.append(value)
    y.append(col5)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/SHJ_JBCR_NP_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/SHJ_JBCR_NP_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col6.append(value)
    y.append(col6)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/PMJ_JM_NP_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/PMJ_JM_NP_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col7.append(value)
    y.append(col7)

    for id in it.chain(range(0, 5)):
        file = '/data1/xtra/results/timestamps/PMJ_JBCR_NP_{}.txt'.format(id)
        file2 = '/data1/xtra/results/records/PMJ_JBCR_NP_{}.txt'.format(id)
        value = GetThroughput(file, file2)
        col8.append(value)
    y.append(col8)
    return y


if __name__ == "__main__":
    x_values = [1600, 3200, 6400, 12800, 25600]

    y_values = ReadFile()

    legend_labels = ['Lazy:', 'NPJ', 'PRJ', 'MWAY', 'MPASS',
                     'Eager:', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$', 'PMJ$^{JB}$']

    DrawFigure(x_values, y_values, legend_labels,
               r'$v_R$=$v_S$ (inputs/msec)',  'Tpt. (inputs/msec)', 1400,
               30000, 'throughput_figure1', False)
    # print(y_values)
    # DrawLegend(legend_labels, 'throughput_line_legend')

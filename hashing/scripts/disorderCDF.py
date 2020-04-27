import getopt
import os
import sys
from math import ceil

import matplotlib
import matplotlib as mpl
import numpy
from matplotlib.ticker import LinearLocator
from numpy import double

mpl.use('Agg')

import matplotlib.pyplot as plt
import pylab
from matplotlib.font_manager import FontProperties

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 20
LABEL_FONT_SIZE = 22
LEGEND_FONT_SIZE = 24
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = ('#ABB2B9', '#8D6E63', '#616161', '#263238', '#FF0000', '#008000', '#FFFF00', '#0000FF', '#00FFFF')
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

FIGURE_FOLDER = '/data1/xtra/results/figure'


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


# def Convert(data):
#     # res = stats.relfreq(col, numbins=10)
#     # x = res.lowerlimit + np.linspace(0, res.binsize * res.frequency.size, res.frequency.size)
#     # return x
#     # this create the kernel, given an array it will estimate the probability over that values
#     kde = gaussian_kde(data)
#     return kde(dist_space)

# example for reading csv file
def ReadFile(S, id):
    col1 = []
    col2 = []
    col3 = []
    col4 = []
    col5 = []
    col6 = []
    col7 = []
    col8 = []

    cnt1 = 0
    cnt2 = 0
    cnt3 = 0
    cnt4 = 0
    cnt5 = 0
    cnt6 = 0
    cnt7 = 0
    cnt8 = 0
    mingap = 0
    maxgap = 0

    f = open("/data1/xtra/results/gaps/PRJ_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col1.append(value)
            cnt1 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1

    f = open("/data1/xtra/results/gaps/NPJ_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col2.append(value)
            cnt2 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1

    f = open("/data1/xtra/results/gaps/MPASS_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col3.append(value)
            cnt3 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1

    f = open("/data1/xtra/results/gaps/MWAY_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col4.append(value)
            cnt4 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1

    f = open("/data1/xtra/results/gaps/SHJ_JM_NP_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col5.append(value)
            cnt5 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1

    f = open("/data1/xtra/results/gaps/SHJ_JBCR_NP_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col6.append(value)
            cnt6 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1

    f = open("/data1/xtra/results/gaps/PMJ_JM_NP_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col7.append(value)
            cnt7 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1

    f = open("/data1/xtra/results/gaps/PMJ_JBCR_NP_{}.txt".format(id), "r")
    cnt = 1
    read = f.readlines()
    for x in read:
        if cnt % S == 0:
            value = double(x.strip("\n"))
            col8.append(value)
            cnt8 += 1
            if (value > maxgap):
                maxgap = value
            elif (value < mingap):
                mingap = value
        cnt += 1
    minvalue = min(cnt1, cnt2, cnt3, cnt4, cnt5, cnt6, cnt7, cnt8)

    return maxgap, mingap, minvalue, col1, col2, col3, col4, col5, col6, col7, col8


def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LINE_WIDTH = 8.0
    MARKER_SIZE = 12.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(16, 0.3))
    idx = 0
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    idx = 0
    for group in range(len(FIGURE_LABEL)):
        lines[idx], = ax1.plot(x_values, data,
                               color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                               marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(group))

        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=False, borderaxespad=-0.3, handlelength=1)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(FIGURE_FOLDER + '/' + filename + '.pdf')


# draw a line chart
def DrawFigure(xvalues, yvalues, legend_labels, x_label, y_label, x_min, x_max, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(6, 3))
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

    # plt.yscale('log')
    # plt.xticks(x_values)
    # you may control the limits on your own.
    plt.xlim(left=x_min)
    # plt.ylim(y_min, y_max)

    plt.grid(axis='y', color='gray')

    # figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_locator(matplotlib.ticker.FixedFormatter(["0.25", "0.5", "0.75", "1"]))
    # figure.xaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
    # figure.xaxis.set_major_locator(LinearLocator(5))

    # figure.get_xaxis().set_tick_params(direction='in', pad=10)
    # figure.get_yaxis().set_tick_params(direction='in', pad=10)
    figure.xaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
    figure.xaxis.set_major_locator(LinearLocator(5))

    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(FIGURE_FOLDER + "/" + filename + ".pdf", bbox_inches='tight')
    # ConvertEpsToPdf(FIGURE_FOLDER + "/" + filename)


def plot_normal(x_range, A, B, N, mu=0, sigma=1, cdf=False, **kwargs):
    '''
    Plots the normal distribution function for a given x range
    If mu and sigma are not provided, standard normal is plotted
    If cdf=True cumulative distribution is plotted
    Passes any keyword arguments to matplotlib plot function
    '''
    # x = x_range
    # # map = []
    # # for x in x_range:
    # #     map.append(round(float(x) / N, 1))
    # # print(map)
    # import scipy.stats as ss
    # y_pdf = ss.norm.pdf(x, mu, sigma)  # the normal pdf
    # print(y_pdf)
    # return y_pdf
    # dx = 10
    # y = exp(-x ** 2)
    # y /= (dx * y).sum()
    # # if cdf:
    # #     y = ss.norm.cdf(x, mu, sigma)
    # # else:
    # #     y = ss.norm.pdf(x)
    # # print(y)
    # return y
    # hist, bins = np.histogram(x_range, bins=10, normed=True)
    # return hist

    # a = 0
    # b = 1
    map = []
    # for x in x_range:
    #     # linear mapping.
    #     map.append((abs(x) - A) * (b - a) / (B - A) + a)

    for x in x_range:
        map.append(abs(x))

    counts, bins = numpy.histogram(map, bins=5)
    bins = bins[:-1] + (bins[1] - bins[0]) / 2
    probs = counts / float(counts.sum())
    print(probs)

    #
    # from scipy import stats
    # counts, bins = stats.rv_histogram(hist)
    # print(hist_dist.pdf(map))
    # x = res.lowerlimit + np.linspace(0, res.binsize * res.frequency.size, res.frequency.size)
    # return x, res.frequency;
    # hist, bins = np.histogram(map, bins=10)
    # print(hist)
    # print(bins)
    return bins, probs


if __name__ == "__main__":

    id = 0
    try:
        opts, args = getopt.getopt(sys.argv[1:], '-i:h', ['test id', 'help'])
    except getopt.GetoptError:
        print('progressive_figure.py -id testid')
        sys.exit(2)
    for opt, opt_value in opts:
        if opt in ('-h', '--help'):
            print("[*] Help info")
            exit()
        elif opt == '-i':
            print('Test ID:', opt_value)
            id = (int)(opt_value)

    legend_labels = ['PRJ', 'NPJ', 'M-PASS', 'M-WAY', 'SHJ$^M$', 'SHJ$^B$', 'PMJ$^M$', 'PMJ$^B$']
    S = 1  #
    maxgap, mingap, N, col1, col2, col3, col4, col5, col6, col7, col8 = ReadFile(S, id)
    print("Number of points:", N)
    # print(maxgap, mingap)

    x, y1 = plot_normal(col1[(len(col1)) - N:], mingap, maxgap, N)
    x, y2 = plot_normal(col2[(len(col2)) - N:], mingap, maxgap, N)
    x, y3 = plot_normal(col3[(len(col3)) - N:], mingap, maxgap, N)
    x, y4 = plot_normal(col4[(len(col4)) - N:], mingap, maxgap, N)
    x, y5 = plot_normal(col5[(len(col5)) - N:], mingap, maxgap, N)
    x, y6 = plot_normal(col6[(len(col6)) - N:], mingap, maxgap, N)
    x, y7 = plot_normal(col7[(len(col7)) - N:], mingap, maxgap, N)
    x, y8 = plot_normal(col8[(len(col8)) - N:], mingap, maxgap, N)
    lines = [
        y1,
        y2,
        y3,
        y4,
        y5,
        y6,
        y7,
        y8,
    ]
    # lines = [
    #     col1[(len(col1)) - N:],
    #     col2[(len(col2)) - N:],
    #     col3[(len(col3)) - N:],
    #     col4[(len(col4)) - N:],
    #     col5[(len(col5)) - N:],
    #     col6[(len(col6)) - N:],
    #     col7[(len(col7)) - N:],
    #     col8[(len(col8)) - N:],
    # ]

    # col0 = []
    # for x in it.chain(range(-N, 0), range(0, N)):
    #     col0.append(x / N)

    print(x)
    col0 = []
    for num in x:
        col0.append(num / (N * 100))
    print(col0)
    legend = False
    DrawFigure(col0, lines, legend_labels,
               'disorder rate', 'frequency', 0, 0.3,
               1, int(ceil(maxgap / 100.0)) * 100,
               'disordercdf_figure{}'.format(id),
               legend)

    DrawLegend(legend_labels, 'disordercdf_legend')

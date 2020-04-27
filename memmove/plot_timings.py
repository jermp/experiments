#!/bin/python

import sys, json, argparse, math
import numpy as np
import matplotlib.pyplot as plt
plt.style.use('ggplot')
plt.rcParams['axes.facecolor'] = '#f7f7f7'

from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.ticker import FuncFormatter

fig, ax = plt.subplots(figsize = (9,3)) # 4,2.5
plt.margins(0.02)

parser = argparse.ArgumentParser()
parser.add_argument('input_filename', type = str, help = "Input json filename.")
parser.add_argument('output_filename', type = str, help = "Output filename.")
parser.add_argument('operation', type = str, help = "Operation. Either Sum or Update")
parser.add_argument('from_point', type = int, help = "From point of index 'from_point'.")
parser.add_argument('to_point', type = int, help = "To point of index 'to_point'.")
parser.add_argument('types', metavar='types', type = str, nargs='+',
                    help = 'types to plot')

args = parser.parse_args()


ax.set_ylabel('nanosecs/' + args.operation, fontsize = 9)
ax.set_xlabel('n', fontsize = 9)
# ax.set_xscale('log', basex = 2)
# ax.set_yscale('log', basey = 2)


y = {}
min = np.Inf
max = -np.Inf
with open(args.input_filename) as f:
    for line in f:
        parsed_line = json.loads(line)
        type = parsed_line["type"]
        if type in args.types:
            y[type] = parsed_line["timings"][args.from_point:args.to_point]
            m = np.min(y[type])
            if m < min:
                min = m
            mm = np.max(y[type])
            if mm > max:
                max = mm

x = []
for n in range(args.from_point * 10 + 10, args.to_point * 10 + 10, 10):
    x.append(n)

# num_ticks = 8
# spany = (max + 0.01 * (max - min)) / num_ticks
# yticks = []
# for i in range(0, num_ticks + 1):
#     yticks.append(int(i * spany))
# ax.yaxis.set_ticks(yticks)

# xlabels = []
# n = int(math.log2(x[0])) + 1
# max_n = x[len(x)-1]
# val = math.pow(2, n)
# xlabels.append(val)
# while val < max_n:
#     val = math.pow(2, n)
#     xlabels.append(val)
#     n += 1 # 2
# ax.xaxis.set_ticks(xlabels)

ax.tick_params(axis = "x", labelsize = 9)
ax.tick_params(axis = "y", labelsize = 9)
m = 4

gray = '#969696'

# color, marker, alpha, name
info = {
    "std::set":        ["#de2d26",'s', 1, r'std::set', '-'],
    "sorted_vector":   ["#31a354",'o', 1, r'sorted_vector', '-'],
}


lines = []
for i in range(0, len(y)):
    I = info[args.types[i]]
    timings = y[args.types[i]]
    lines.append(ax.plot(x, [t[1] for t in timings], zorder = 1, color = I[0], alpha = I[2], linewidth = 1.1,
        # marker = I[1],
        markersize = m, linestyle = I[4]))


# for i in range(0, len(y)):
#     I = info[args.types[i]]
#     timings = y[args.types[i]]
#     lines.append(ax.fill_between(x, [t[0] for t in timings], [t[2] for t in timings], zorder = 1, color = I[0], alpha = 0.3, interpolate = False, linewidth = 0.1))

ax.legend(
    ([info[args.types[i]][3] for i in range(0,len(args.types))]),
    # ncol=1, bbox_to_anchor=(0, 1),
    # loc='lower left',
    numpoints = 2,
    # loc = 'best',
    loc = 'upper left',
    ncol = 3 if len(args.types) == 3 or len(args.types) == 6 else 2,
    fontsize = 9)

pp = PdfPages(args.output_filename + '.pdf')
pp.savefig(bbox_inches = 'tight')
pp.close()

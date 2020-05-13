#!/bin/python

import sys, json, argparse, math
import numpy as np
import matplotlib.pyplot as plt
plt.style.use('ggplot')
plt.rcParams['axes.facecolor'] = '#f7f7f7'

from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.ticker import FuncFormatter

fig, ax = plt.subplots(figsize = (9,1.5)) # 4,2.5
plt.margins(0.02)

ax.set_ylabel('cache lines', fontsize = 9)
ax.set_xlabel('set number', fontsize = 9)

parser = argparse.ArgumentParser()
parser.add_argument('input_filename', type = str, help = "Input json filename.")
parser.add_argument('type', type = str, help = "Type: 'ft', 'ft_holes', 'st_td', 'st_bu'.")
args = parser.parse_args()


y = []
with open(args.input_filename) as f:
    for line in f:
        values = [int(x) for x in line.split(' ')]
        y.append(values)

x = []
for n in range(0, len(y[0])):
    x.append(n)

xlabels = []
val = 0
while val <= len(y[0]):
    xlabels.append(val)
    val += 8
ax.xaxis.set_ticks(xlabels)

ax.tick_params(axis = "x", labelsize = 9)
ax.tick_params(axis = "y", labelsize = 9)
m = 4

gray = '#969696'

# color, marker, alpha, name
info = {
    "ft":                   ["#de2d26",'s', 1, r'FT', '-'],
    "ft_holes":             ["#31a354",'o', 1, r'FT with holes', '-'],
    "ft_blocked_64":        ["#de2d26",'o', 1, r'FT blocked 64', '-'],
    "ft_blocked_64_holes":  ["#31a354",'o', 1, r'FT blocked 64 with holes', '-'],
    "st_td":                ["#3182bd",'o', 1, r'ST top-down', '-'],
    "st_bu":                ["#fe9929",'o', 1, r'ST bottom-up', '-'],
}

# axes = plt.gca()
# axes.set_ylim([0, 1000])

lw = 6
ax.vlines(x, [0], y[0], color = info[args.type][0], alpha = 1, linewidth = lw)

ax.legend(
    ([info[args.type][3] for i in range(0,len(y))]),
    # ncol=1, bbox_to_anchor=(0, 1),
    # loc='lower left',
    numpoints = 2,
    # loc = 'best',
    loc = 'upper right',
    ncol = len(y),
    fontsize = 9)

# pdf output
# pp = PdfPages(args.type + '.pdf')
# pp.savefig(bbox_inches = 'tight')
# pp.close()

plt.savefig(args.type + '.png', bbox_inches = 'tight')

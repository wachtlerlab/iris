#!/usr/bin/env python

from __future__ import print_function
from __future__ import division

import h5py as h5
import numpy as np
import matplotlib.pyplot as plt
import sys
import csv
import argparse


def main():
    parser = argparse.ArgumentParser(description='CI - Configuration Tool')
    parser.add_argument('--freq', dest='freq', type=float, default=1.0)
    parser.add_argument('data', type=str)
    parser.add_argument('amp', type=float, default=None)
    parser.add_argument('phase', type=float, default=None)
    parser.add_argument('offset', type=float, default=0.66)

    args = parser.parse_args()
    x = []
    y = []
    with open(args.data, 'rb') as csvfile:
        reader = csv.reader(csvfile, delimiter=',', quotechar='#')
        for row in reader:
            x.append(float(row[0]))
            y.append(float(row[1]))

    plt.scatter(x, y,  color='dodgerblue', label='data')
    plt.axhline(y=args.offset, color='black')

    phase = args.phase
    amp = args.amp

    if phase is not None and amp is not None:
        x = np.array(sorted(set(x)))
        plt.hold(True)
        yfit = args.offset + amp * np.sin(args.freq*x - phase)
        plt.plot(x, yfit, '--k')

    plt.show()

if __name__ == '__main__':
    main()
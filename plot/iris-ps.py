#!/usr/bin/env python

from __future__ import print_function
from __future__ import division

import numpy as np
import matplotlib.pyplot as plt
import sys
import csv
import argparse
import yaml
import csv
import StringIO

def main():
    parser = argparse.ArgumentParser(description='CI - Plot ISO fit')
    parser.add_argument('--freq', dest='freq', type=float, default=1.0)
    parser.add_argument('--offset', type=float, default=0.667)
    parser.add_argument('data', type=str)
    parser.add_argument('--iso', type=str, default=None)

    args = parser.parse_args()
    f = open(args.data)
    data = yaml.safe_load(f)
    f.close()

    iso = None
    if args.iso is not None:
        f = open(args.iso)
        iso = yaml.safe_load(f)
        f.close()

    raw = data['isodata']['data']
    print(raw)
    reader = csv.reader(StringIO.StringIO(raw), delimiter=',', quotechar='#')
    x, y = zip(*[(float(r[0]), float(r[1])) for r in reader])
    print(x, y)

    plt.scatter(x, y,  color='dodgerblue', label='data')
    plt.axhline(y=args.offset, color='black')

    if iso is not None:
        x = np.array(sorted(set(x)))
        plt.hold(True)
        freq = 1
        amp = float(iso['isoslant']['dl'])
        phase = float(iso['isoslant']['phi'])
        print(amp, phase, x)
        yfit = args.offset + amp * np.cos(x - phase)
        plt.plot(x, yfit, '--k')

    plt.show()

if __name__ == '__main__':
    main()
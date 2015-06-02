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
import os

def main():
    parser = argparse.ArgumentParser(description='CI - Plot ISO fit')
    parser.add_argument('--freq', dest='freq', type=float, default=1.0)
    parser.add_argument('--offset', type=float, default=0.667)
    parser.add_argument('data', type=str)

    args = parser.parse_args()

    fn = args.data
    if "." in fn:
        fn, ext = os.path.splitext(fn)

    slant_fn = fn + ".isoslant"
    data_fn = fn + ".isodata"

    if not os.path.exists(data_fn):
        sys.stderr.write("No data\n")
        sys.exit(-1)

    f = open(data_fn)
    data = yaml.safe_load(f)
    f.close()

    iso = None
    if os.path.exists(slant_fn):
        f = open(slant_fn)
        iso = yaml.safe_load(f)
        f.close()

    raw = data['isodata']['data']
    print(raw)
    reader = csv.reader(StringIO.StringIO(raw), delimiter=',', quotechar='#')
    next(reader) # ignore the header
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
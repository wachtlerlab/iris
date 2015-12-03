#!/usr/bin/env python
from __future__ import print_function

import numpy as np
import matplotlib.pyplot as plt
import sys
import csv
import argparse
import pandas as pd


def main():
    parser = argparse.ArgumentParser(description='CI - Configuration Tool')
    parser.add_argument('data', type=str)

    args = parser.parse_args()

    filename = args.data if args.data != '-' else '/dev/stdin'
    df = pd.read_csv(filename, skipinitialspace=True)

    x = df['angle']
    for i, c in enumerate(['r', 'g', 'b']):
        data = df[c]
        plt.plot(x, data,  color=c, label=c)

    foo = df.diff()
    print(foo)
    foo['nd'] = foo[["r", "g", "b"]].apply(lambda x: len(set(x)) == 1, axis=1)
    same_rgb = df[foo.nd == True]
    print(same_rgb)
    plt.show()

if __name__ == '__main__':
    main()
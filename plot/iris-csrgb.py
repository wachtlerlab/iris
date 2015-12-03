#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
import sys
import csv
import argparse


def main():
    parser = argparse.ArgumentParser(description='CI - Configuration Tool')
    parser.add_argument('data', type=str)

    args = parser.parse_args()
    x = []
    c = []
    filename = args.data if args.data != '-' else '/dev/stdin'
    with open(filename, 'rb') as csvfile:
        reader = csv.reader(csvfile, delimiter=',', quotechar='#')
        is_header = True
        for row in reader:
            if is_header:
                is_header = False
                continue
            x.append(float(row[0]))
            c.append([float(row[i]) for i in xrange(1, 4)])

    y = np.array(c)
    print(y.shape)
    for i, c in enumerate(['r', 'g', 'b']):
        print i
        data = y[:, i]
        data *= 255
        data = data.astype('u8')
        plt.plot(x, data,  color=c, label=c)

    plt.show()

if __name__ == '__main__':
    main()
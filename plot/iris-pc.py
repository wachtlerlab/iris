#!/usr/bin/env python

from __future__ import print_function
from __future__ import division

import h5py as h5
import numpy as np
import matplotlib.pyplot as plt
import sys
import StringIO
import yaml

def G(azero, a, gamma, x):
    return [azero + a * i**gamma for i in x]

def load_rgb2lms(path):
    f = open(path)
    data = yaml.safe_load(f)
    f.close()

    dkl = data['rgb2lms']['dkl']
    m = [l for l in dkl.split('\n') if len(l) > 0 and not l.startswith('#')]
    allm = np.genfromtxt(StringIO.StringIO(u'\n'.join(m)), dtype='f8', delimiter=',')
    Azero = allm[0, :]
    A = allm[[1,2,3], :]
    gamma = allm[4, :]

    return Azero, A, gamma

def main():

    cac_file = sys.argv[1] + '.cac'
    rgb2lms_file = sys.argv[1] + '.rgb2lms'

    fd = h5.File(cac_file, 'r')
    g = fd['/rgb2sml']
    ds = g['cone-activations']

    spectra = np.array(ds)
    x = np.array(ds.attrs['levels'])

    Azero, A, gamma = load_rgb2lms(rgb2lms_file)

    print(Azero)
    print(A)
    print(gamma)

    lum = x.reshape(spectra.shape)
    print(lum.shape)
    print(spectra.shape)

    cones = ['S', 'M', 'L']
    prims = ['R', 'G', 'B']

    for c, k in [(x,y) for x in range(3) for y in range(3)]:
        plt.subplot(3, 3, (3*c+k)+1)
        plt.scatter(lum[c, k, :], spectra[c, k, :], color='dodgerblue', label='data', s=5)
        plt.hold(True)
        xev = np.arange(0, 255)
        yev = G(Azero[c], A[c, k], gamma[k], xev)
        plt.plot(xev, yev, 'k--', label='fit')
        plt.xlim([0, 255])
        plt.xlabel('%s vs %s' % (cones[c], prims[k]))
        if c == 2 and k == 2:
            plt.legend()

    plt.subplots_adjust(left=0.07, top=0.98, bottom=0.09, right=0.98, hspace=0.35, wspace=0.30)
    plt.show()

if __name__ == '__main__':
    main()
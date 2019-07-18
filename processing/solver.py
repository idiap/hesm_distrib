# License - for Non-Commercial Research and Educational Use Only
# 
# Copyright (c) 2019, Idiap research institute
# 
# All rights reserved.
# 
# Run, copy, study, change, improve and redistribute source and binary forms, with or without modification, are permitted for non-commercial research and educational use only provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# For permission to use for commercial purposes, please contact Idiap's Technology Transfer Office at tto@idiap.ch
#
# Christian.jaques@idiap.ch
# Christian Jaques, Idiap
#
# This module contains the Iteratively Reweighted Least-Squares (IRLS) algorithm used in the 
# paper to minimize Lp norms.
#
#
import sys
import numpy as np
import math
from numpy.linalg import lstsq

def IRLS(A, b, p, delta=1e-6, tolerance=1e-2, eps0=1, delta_eps=100,
         max_iterations=500, verbose=False):
    """ Iteratively Reweighted Least Squares (IRLS) algorithm

    This module implements a modified version of IRLS that converges
    faster and is more robust to local minima, as described in Chartrand and
    Yin's paper : "Iterativery reweighted algorithms for compressive sensing",
    ICASSP 2008.
    It finds the optimum of : ||Ax-b||_p for any p

    Arguments:
        A : the system's matrix
        b : observation vector
        p : the norm to minimize
    Keyword arguments:
        delta : tolerance to stop iterations if the solution is close enough
        eps0 : the initial damping factor (c.f. Chartrand's paper)
        delta_eps : when to update epsilon (by divinding it by 10)
        max_iterations : max number of iterations
        verbose : prints information during computations
    """
    N = A.shape[0]
    diff = sys.maxsize
    w = np.ones(N)  # weigth vector
    iterations = 0
    delta_vec = np.ones((1, N)) * delta  # avoids having weights equal to 0
    # vector to store errors at each iteration
    errors = np.zeros(max_iterations)
    err_old = 0
    # damping factor, from Chartrand and Li's paper
    epsilon = eps0

    while(diff > tolerance and iterations < max_iterations):
        # multiply by weigths
        # avoid matrix multiplication as matrices' sizes may be too big
        A_ls = np.copy(A)
        for i in range(N):
            A_ls[i] *= w[i]
        b_ls = np.multiply(w, b)
        # solve weighted least-square
        x = np.array(lstsq(A_ls, b_ls)[0])
        # compute new weights
        err = np.abs(A_ls.dot(x) - b_ls)
        err_curr = np.sum(err)
        if(len(err.shape) > 1):  # in case b is 2 dimensional
            err = np.mean(err, axis=1)
        w = np.maximum((err**2 + epsilon)**(p / 2. - 1), delta_vec[0])
        # check solution difference with previous iteration
        diff = abs(err_old - err_curr)
        # check if we have to update epsilon
        if((diff) / (err_curr) < math.sqrt(epsilon) / float(delta_eps)):
            epsilon /= 10.0
        err_old = err_curr
        # store error
        errors[iterations] = diff
        iterations += 1
        if(verbose is True):
            if(iterations % 10 == 0):
                print('Iteration ', iterations, ' over ', max_iterations)

    if(verbose is True):
        if(diff < tolerance):
            print('Stopped because diff is lower than tolerance')
        if(iterations > max_iterations):
            print('Max iterations reached, stopped iterating')

    return x, iterations, errors[1:iterations]


if __name__ == "__main__":
    # Generates a noisy line and fits lines to it, with respect
    # to any p-norm.
    from pytoolbox.data import generate_data
    import matplotlib.pyplot as plt

    # Parameters
    # generate data
    N = 30
    x_ordon = np.arange(N)
    s = 2 * x_ordon + 4*np.random.random(N)
    # insert outliers
    s[-1] = 2
    s[-6] = 2

    # build matrices
    A = np.ones((N, 2))
    A[:, 0] = x_ordon
    b = np.hstack(s)

    # iterative re-weighted LS
    p = 1.1  # p-norm
    x1, iterations, errs1 = IRLS(A, b, p, max_iterations=400)
    best_p_sol = x_ordon * x1[0] + x1[1]

    # comparison with l-2 norm, directly with Scipy's solver, equivalent to
    # Matlab's operator "\" (usage would be x = b\A)
    x2 = np.array(lstsq(A, b, rcond=None)[0])
    best_2_sol = x_ordon * x2[0] + x2[1]

    # iterative re-weighted LS
    p2 = 1.5
    x3, iterations, errs3 = IRLS(A, b, p2, max_iterations=400)
    best_3_sol = x_ordon * x3[0] + x3[1]

    # Plot signals
    plt.plot(x_ordon, b, 'r.', label='Original signal')
    plt.plot(x_ordon, best_p_sol, 'b-', label='{0}-norm'.format(p))
    plt.plot(x_ordon, best_2_sol, 'c-', label='l2-norm')
    plt.plot(x_ordon, best_3_sol, 'g-', label='{0}-norm'.format(p2))
    plt.legend(loc=2)
    plt.show()

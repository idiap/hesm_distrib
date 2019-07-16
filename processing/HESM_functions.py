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
# Computational bioimaging group, IDIAP
#
# In this module, we have functions to improve the readability of the notebook
# associated with the method.
#
import numpy as np
from utils import *
import time


def apply_HESM(ims, A, D, lambd=0.001, verbose=True, line_vector=False):
    """ 
    Apply HESM method to perform temporal super-resolution

    """
    Q = np.size(A, 1)
    if(line_vector is False):
        N, nX, nY, i = ims.shape
        D_whole_set = np.tile(D, (nX, nY, N))  # builds an offset volume
        A_whole_set = build_diag_block_matrix(A, N)
        ims_whole_set = restack_images(ims)

    # compute result
    if(verbose):
        start_time = time.time()
        print('Starting computations')

    if(line_vector is False):
        # build other matrices for regularization
        tik = tikhonov2(Q*N)
        tikt = tik.T
        At = A_whole_set.T
        A_with_reg = (At.dot(A_whole_set) + tikt.dot(lambd * tik))
        Ai = np.linalg.pinv(A_with_reg)
        ims_whole_set = np.apply_along_axis(apply_matrix,
                                            2, ims_whole_set-D_whole_set, At)
        f = np.apply_along_axis(apply_matrix, 2, ims_whole_set, Ai)
    else:
        N, Q = ims.shape
        f = np.zeros((N, 3))
        Ai = np.linalg.pinv(A)
        for i in range(N):
            f[i] = Ai.dot(ims[i])

    if(verbose):
        print('Done in ', time.time()-start_time, ' seconds')

    return f


def build_S_matrix(time_functions=[], N_leds=3, N_sensors=3):
    """ Builds the S matrix associated with each light time function

        Watch out, there's no check that there are the correct number
        of time_functions (should be == N_leds) and that they have the
        correct length (should be == Q_steps).
    """
    Q = len(time_functions[0])
    C = N_sensors  # simplifies reading herebelow
    L = N_leds  # simplifies reading herebelow
    S = np.zeros((C, C*L*Q))
    for i in range(N_leds):
        for j in range(N_sensors):
            index = i*C*Q + j*Q
            S[j, index:index+Q] = time_functions[i]

    return S


def build_Gamma_matrix(params=[], N_leds=3, N_sensors=3, Q_steps=3):
    """ Based on parameters calibrated (see calibration procedure), builds the
        right-size Gamma matrix.

        Here it is particularly important to make sure that the parameters are
        saved in the right order. This needs to fit with the time functions
        of the lights, given when building the S matrix.
    """
    # simplifies reading hereunder
    Q = Q_steps
    C = N_sensors
    L = N_leds
    IQ = np.eye(Q)
    Gamma = np.zeros((C*L*Q, Q))
    for i in range(N_leds):
        for j in range(N_sensors):
            index = i*Q*C + j*Q
            Gamma[index:index+Q, 0:Q] = IQ*params[i*C+j]

    return Gamma


def build_diag_block_matrix(A, N):
    """ Copies the A matrix N times to build a block matrix with only "A"'s """
    C = A.shape[0]
    Q = A.shape[1]
    A_big = np.zeros((C*N, Q*N))
    for i in range(N):
        A_big[i*C:i*C+C, i*Q:i*Q+Q] = A

    return A_big


def restack_images(ims):
    """ Stacks images so that all r-g-b layers are stacked onto each other.
        This gives an axis along which we have rgbrgbrgbrgb... values.
    """
    N, nX, nY, Q = ims.shape
    ims_whole_set = np.zeros((nX, nY, Q*N))
    for i in range(N):
        # reorganize images to have rgbrgbrgbrgb,... on one axis
        ims_whole_set[:, :, Q*i:Q*i+Q] = ims[i]

    return ims_whole_set


def score_reconstruction(ims):
    """ This function will score how good a reconstruction is, in order to
        select the right model for the reconstruction
    """
    nX, nY, N = ims.shape
    cost = 0

    for i in range(2, N):
        cost += np.sum(np.abs(ims[..., i] - ims[..., i-1]))

    # normalize score
    cost /= (nX * nY * N)
    return cost




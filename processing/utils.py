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
##
# Christian Jaques, Idiap 
# Christian.jaques@idiap.ch
#
# In this file, we have functions to improve the readability of the notebook
# associated with the method.
#
import os
import numpy as np
import tifffile as tiff
from colour_demosaicing import demosaicing_CFA_Bayer_bilinear
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui
# This is a handler to QtGui app, in order to select ROI by hand
global app
app = QtGui.QApplication([])


def stretch_contrast(image, min_val=0.0, max_val=1.0):
    """ Rescales the greylevels in an image """
    curr_min = np.min(image)
    image -= curr_min  # scale starts at zero
    image += min_val  # scale starts at min_val
    curr_max = np.max(image)
    ratio = float(max_val-min_val) / float(curr_max)
    image_ret = image * ratio + min_val

    return image_ret


def select_roi(image, title):
    """ Select a region of interest with PyQtGraph """
    global app  # created when loading the module
    X = image.shape[0]
    Y = image.shape[1]
    disp_shape = (max(800, X + 100), max(800, Y + 100))
    w = pg.GraphicsWindow(size=disp_shape, border=True)
    w.setWindowTitle('Select ROI ' + title)
    w1 = w.addLayout(row=0, col=0)
    v1 = w1.addViewBox(row=0, col=0)
    img1 = pg.ImageItem(image)
    v1.addItem(img1)
    # add roi to image
    roi = pg.RectROI([20, 20], [X / 3., Y / 3.], pen=(0, 19))
    v1.addItem(roi)

    QtGui.QApplication.instance().exec_()

    return roi


def apply_matrix(x, A):
    """ Function to vectorize computations with numpy, this speeds them up """
    return np.dot(A, x)


def tikhonov2(N):
    """ Builds a 2nd order Tikhonov regularization matrix"""
    vec = np.zeros(N)
    vec[0] = 2
    vec[1] = -1
    vec[-1] = -1
    tik = circular_matrix(vec)

    return tik


def circular_matrix(h):
    """ Builds a circular matrix with input vector h """
    N = np.max(h.shape)
    A = np.zeros((N, N))
    A[np.newaxis, :] = h
    A = np.array(map(np.roll, A[:], np.arange(N)))

    return A


def write_tiff_stack(ims, direct="result_stack_"):
    """ Writes a numpy volume as a stack of tiff files in a folder
        Watch out, time is expected to be the first dimension
    """
    N = ims.shape[0]
    directory_res = direct+"%d"%0
    i = 1
    while os.path.exists(directory_res):
        directory_res = direct+"%d"%i
        i += 1
    os.makedirs(directory_res)
    for i in range(N):
        tiff.imsave(directory_res+ "/im_%d.tif"%i, ims[i])

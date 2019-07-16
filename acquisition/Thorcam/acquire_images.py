# License - for Non-Commercial Research and Educational Use Only
#  
#  Copyright (c) 2019, Idiap research institute
#  
#  All rights reserved.
#  
#  Run, copy, study, change, improve and redistribute source and binary forms, with or without modification, are permitted for non-commercial research and educational use only provided that the following conditions are met:
#  
#  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  
#  For permission to use for commercial purposes, please contact Idiap's Technology Transfer Office at tto@idiap.ch
#
# Christian Jaques (christian.jaques at idiap.ch)
# Computational bioimaging group, Idiap Research Institute
import sys, getopt
import numpy as np
import matplotlib.pyplot as plt
import time as t
from wrapper import *
from colour_demosaicing import demosaicing_CFA_Bayer_bilinear


# Globals
# ROI
x0 = 200
y0 = 100
width = 600
height = 400
use_color = 0


def init_camera(hard_trigg=0, use_color=0, pixel_clock=80, fps=20, exposure_time=10):
    # call to C++ wrapper
    connect_USB_camera(hard_trigg, use_color, pixel_clock, exposure_time, fps)


def acquire_one_frame():
    b = acquire_single_frame()
    plt.imshow(b,interpolation='nearest', cmap='Greys_r')
    plt.show()


def display_stack_serie(stack, interval=50):
    N = stack.shape[2]
    run = True

    def press(event):
        print('pressed ', event.key)
        sys.stdout.flush()
        if event.key == 'q':
            print('Exiting function')
            plt.ioff()
            plt.close('all')
            run = False

    if(run):
        fig = plt.figure()
        ax = fig.add_subplot(111)
        fig.canvas.mpl_connect('key_press_event', press)
        plt.ion()
        plt.show()
        im = demosaicing_CFA_Bayer_bilinear(stack[...,0])
        img = ax.imshow(im, interpolation='nearest', cmap='Greys_r')
    while run:
        for i in range(N):
            if(run):
                plt.title("{0}".format(i))
                im = demosaicing_CFA_Bayer_bilinear(stack[...,i])
                img.set_data(im)
                plt.pause(interval/1000.)


def acquire_multi_frames(nFrames = 15):
    ims = acquire_multiple_frames(nFrames)
    return ims


if __name__ == "__main__":

    # fetch arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ho:v", ["help", "output="])
    except getopt.GetoptError as err:
        # print help information and exit:
        print('No input supplied, using defaults')

	# Camera variables
    # Warning to the pixel clock value, it HAS to be within the supported values by the camera
    # which are not necessarily available. Max is 86, min 1, typical 24, and we use 50 to have ~30 fps
    pixel_clock = 86 # MegaHertz
    fps = 45
    exposure_time = 20 # milliseconds
    file_name = 'images_stack.tif'
    n_frames = 100

    threshold = 0
    interval = 4
    use_color = 0
    flashOn = 0

    hard_trigger = 0
    args_count = len(args)
    if(args_count < 5):
        print('Not all parameters sent as arguments, will use defaults.  -- Not a problem.')
        print('Normal usage : ')
        print('         "python acquire_images.py [n_frames] [fps] [exposure] [file_name] [x0] [y0] [width] [height] [hard_trigger] [use_color] [flashOn]"')
    if (args_count > 0):
        n_frames = args[0]
        if (args_count > 1):
            fps = args[1]
            if(args_count > 2):
                exposure_time = args[2]
                if(args_count > 3):
                    file_name = args[3]
                    if(args_count > 7):
                        x0 = args[4]
                        y0 = args[5]
                        width = args[6]
                        height = args[7]

                    if(args_count > 8):
                        hard_trigger = args[8]
                        if(args_count > 9):
                            use_color = args[9]
                            if(args_count > 10):
                                flashOn = args[10]

                        if (args_count > 11 ):
                            print('------ Bad args ', args)

    # Make sure to cast variables for camera functions
    fps = float(fps)
    exposure_time = float(exposure_time)
    n_frames = int(n_frames)
    x0 = int(x0)
    y0 = int(y0)
    width = int(width)
    height = int(height)
    threshold = int(threshold)
    interval = int(interval)
    use_color = int(use_color)
    hard_trigger = int(hard_trigger)
    flashOn = int(flashOn)

    print('Acquiring ', n_frames, ' images with fps : ', fps, ' and exposure_time : ', exposure_time)
    print('File to save called : ', file_name)
    print('ROI is x0-y0-W-H : ', x0, y0, width, height)
    if(hard_trigger >= 1):
        print('Trigger is set to hardware input')
    else:
        print('Trigger is sent by software')

    if(use_color >= 1):
        print('Acquiring color images ')
    else:
        print('Acquiring BW images')

    print('Flash is set to ', flashOn)

    print('Acquiring ', n_frames, ' images with fps : ', fps, ' and exposure_time : ', exposure_time)
    print('File to save called : ', file_name)
    print('ROI is x0-y0-W-H : ', x0, y0, width, height)
    if(hard_trigger >= 1):
        print('Trigger is set to hardware input')
    else:
        print('Trigger is sent by software')

    if(use_color >= 1):
        print('Acquiring color images ')
    else:
        print('Acquiring BW images')

    # Init camera and call func
    init_camera(hard_trigger, use_color, pixel_clock, fps, exposure_time)
    # set ROI
    set_ROI(x0,y0,width,height)
    # set Flash
    set_Flash(flashOn)
    # Set threshold (for demo, quite useless now)
    if(threshold > 10):
        set_min_difference_threshold_and_interval(threshold, interval)
    # Acquire images
    ims = acquire_multi_frames(n_frames)
    # turn flash off
    set_Flash(0)
    # Close camera
    close_camera()
    ims = np.swapaxes(ims, 0,1)
    # Save images
    np.save(file_name+'.npy', ims)
    print('Saved images')
    # Display
    display_stack_serie(ims,60)

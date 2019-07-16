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
#
# Python script to build the numpy C-API
# Run it with:  python setup.py build_ext --inplace
#
from distutils.core import setup, Extension
import numpy

# define the extension module
wrapper = Extension('wrapper', sources=['wrapper.cpp','camFunctions.cpp'],
                          libraries=['ueye_api'],
                          include_dirs=[numpy.get_include()])

# run the setup'
setup(ext_modules=[wrapper])

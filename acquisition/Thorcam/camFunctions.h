/* 
 License - for Non-Commercial Research and Educational Use Only
  
  Copyright (c) 2019, Idiap research institute
  
  All rights reserved.
  
  Run, copy, study, change, improve and redistribute source and binary forms, with or without modification, are permitted for non-commercial research and educational use only provided that the following conditions are met:
  
  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  For permission to use for commercial purposes, please contact Idiap's Technology Transfer Office at tto@idiap.ch

 Christian Jaques (christian.jaques at idiap.ch)
 Computational bioimaging group, Idiap Research Institute
 
  This code is a python wrapper of ThorLabs DCX serie cameras
  Header file for camera-specific functions.
 
 */
#ifndef CAMFUNCTIONS_H_
#define CAMFUNCTIONS_H_
 // Internal functions declarations 
void GetMaxImageSize(INT *pnSizeX, INT *pnSizeY, HCAM mimu);
void SetCameraParameters(int hardTrigger, int useColor);
void SetCameraPxClkExpFPS(HCAM hCam, int pix_clk, double exp_int, double fps_int);
char * GrabFrame(int*, int*);
char ** GrabSequence(int nFrames, int **idArray);

template<typename T> double ComputeDifference(T * array1, T * array2, int nx, int ny, bool normalize=false)
{
    T val1, val2;
    double diff = 0.0;
    // Normalize difference ?
    long int total_index = nx*ny;
    if (!normalize)
        total_index = 1;

    for(int i=0;i<ny;i++){
        for(int j=0;j<nx;j++){
            val1 = *(array1 + i*nx + j);
            val2 = *(array2 + i*nx + j);
            diff += (double)(abs( val1 - val2)) / (double)total_index;
        }
    }
    
    return diff;
}

// Vars, to store camera, shared across multiple modules and defined in camFunctions.cpp
extern HCAM camera;
extern INT     m_nSizeX;       // width of video 
extern INT     m_nSizeY;       // height of video
extern SENSORINFO sInfo;
extern int BitsPerPixel;

#endif

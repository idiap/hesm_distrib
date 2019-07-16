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
  This module contains camera-specific functions.
 
 */
// Python.h MUST be the first include
#include <Python.h>
#include <numpy/arrayobject.h>
#include <math.h>
#include "uc480.h"
#include "camFunctions.h"

// Globals to this module
double difference_threshold = 0;
int frames_interval =0;
/* Acquire multiple frames from camera */ 
static PyObject* AcquireMultiFrames(PyObject* self, PyObject* args)
{
    int error = 0;
    BitsPerPixel = 8;
    HCAM hCam = camera;
    int nFrames; 
    PyObject      *out_array;    

    // number of frames to acquire
    int ok = PyArg_ParseTuple(args, "i", &nFrames);
    printf("Acquiring %d frames\n",nFrames);

    // Acquire frame sequence
    long int nX = m_nSizeX;
    long int nY = m_nSizeY;
    int * idArray; 

    char ** ppcImgMemArray = GrabSequence(nFrames, &idArray);

    // Get direct pointer on data of array
    char * dataptr = new char[nFrames*nX*nY*BitsPerPixel/8];
    double diff = 0;
    int npy_frm = 0;
    int sameFramesCount = 0;

    // Allocate user memory,  create numpy array
    npy_intp dims[3] = {m_nSizeX, m_nSizeY, nFrames}; 
    int nd = 3;
    out_array = PyArray_SimpleNewFromData(nd, dims, NPY_UINT8, NULL); 
    if (out_array == NULL)
        return NULL;
    NpyIter *out_iter;
    NpyIter_IterNextFunc *out_iternext;

    out_iter = NpyIter_New((PyArrayObject *)out_array, NPY_ITER_READWRITE,
                          NPY_KEEPORDER, NPY_NO_CASTING, NULL);
    if (out_iter == NULL) {
        return NULL;
    }
    char ** out_dataptr = NpyIter_GetDataPtrArray(out_iter);
    out_iternext = NpyIter_GetIterNext(out_iter, NULL);
    if (out_iternext == NULL) { 
        NpyIter_Deallocate(out_iter);
        return NULL;
    }

    // Copy images to user memory
    for (int frm = 0; frm < nFrames; frm++)
    {
        error = is_CopyImageMem (hCam, 
    			ppcImgMemArray[frm], 
    			idArray[frm], 
    			dataptr+npy_frm*nX*nY*BitsPerPixel/8);
        npy_frm ++;
        if (error != IS_SUCCESS) { printf("Error copying data to memory at frame %d\n", frm); }
    }   

    /*  iterate over the array */
    int j=0;
    do {
     **out_dataptr = *(dataptr+j);
    j++;
    } while(out_iternext(out_iter) && j < nX*nX*nFrames); 

    /*  clean up and return the result */
    NpyIter_Deallocate(out_iter);

    /* Release memory */
    error = is_ClearSequence(hCam);
    if (error != IS_SUCCESS) { printf("Error clearing sequence.\n"); }
    for (int frm = 0; frm < nFrames; frm++)
    {
        /* Free memory */
        error = is_FreeImageMem(hCam, ppcImgMemArray[frm], idArray[frm]);
        if (error != IS_SUCCESS) { printf("Error freeing image memory.\n"); }
    }
    /* Release pointer array itself */
    free(ppcImgMemArray);

    /* Return val */
    Py_INCREF(out_array);
    return out_array; 
}

/* Set flash to camera */
static PyObject * SetFlash(PyObject* self, PyObject* args)
{

    HCAM hCam = camera;
    // Parse arguments
    bool flashOn;
    double flashControl;
    int ok = PyArg_ParseTuple(args, "d", &flashControl);
    int error = 0;
    flashOn = (flashControl !=0);

    if (flashOn)
    {
	// Workaround... 
	error = is_SetExternalTrigger(hCam, IS_SET_TRIGGER_SOFTWARE); //OFF);
        if (error != IS_SUCCESS) { printf("Error setting trigger mode in flash settings. -- Error code is : %d\n", error); }

        // Set flash mode to freerun hi active
        int nMode = IO_FLASH_MODE_TRIGGER_HI_ACTIVE; //IO_FLASH_MODE_TRIGGER_HI_ACTIVE;
	error = is_IO(hCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));
        if (error != IS_SUCCESS) { printf("Error setting flash mode. -- Error code is : %d\n", error); }

        // Get minimum flash parameters
        IO_FLASH_PARAMS flashParams;
        error = is_IO( hCam, IS_IO_CMD_FLASH_GET_PARAMS_MIN, (void*)&flashParams, sizeof(flashParams) );
        INT flashDelayMin = flashParams.s32Delay;
        UINT flashDurationMin = flashParams.u32Duration;

        // Get maximum flash parameters
        error = is_IO( hCam, IS_IO_CMD_FLASH_GET_PARAMS_MAX, (void*)&flashParams, sizeof(flashParams) );
        if (error != IS_SUCCESS) { printf("Error getting flash parameters.\n"); }
        INT flashDelayMax = flashParams.s32Delay;
        UINT flashDurationMax = flashParams.u32Duration;


        // Set flash parameters to minimum flash delay, maximum flash duration
        IO_FLASH_PARAMS newFlashParams;
        newFlashParams.s32Delay = 0; //flashDelayMin;
        newFlashParams.u32Duration = 15000; //20*flashDurationMin;
        error = is_IO( hCam, IS_IO_CMD_FLASH_SET_PARAMS, (void*)&newFlashParams, sizeof(newFlashParams) );
        if (error != IS_SUCCESS) { printf("Error setting flash parameters.\n"); }
	printf(" Set values for flash : dur %d and del %d \n",newFlashParams.u32Duration, newFlashParams.s32Delay);

    }
    else
    {
        // Turn off flash
        UINT nMode = IO_FLASH_MODE_OFF;
        error = is_IO( hCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode) );
        if (error != IS_SUCCESS) { printf("Error setting flash mode.\n"); }
    }
    Sleep(500);  

    Py_INCREF(Py_None);
    return Py_None;
}

/* Acquire single frame from camera */ 
static PyObject* AcquireSingleFrame(PyObject* self, PyObject* args)
{

	// =================================================   ACQUIRE IMAGE
	int nx =0;
	int ny =0;
    char * output = GrabFrame(&nx, &ny);

	// =================================================   TRANSFER DATA TO PYTHON
    PyObject      *out_array;
    NpyIter *out_iter;
    NpyIter_IterNextFunc *out_iternext;
    int j=0;
    char ** out_dataptr;

    // Allocate user memory,  create numpy array
    npy_intp dims[2] = {m_nSizeX, m_nSizeY}; 
    int nd = 2;
    out_array = PyArray_SimpleNewFromData(nd, dims, NPY_UINT8, NULL); 
    if (out_array == NULL)
        return NULL;

    out_iter = NpyIter_New((PyArrayObject *)out_array, NPY_ITER_READWRITE,
                          NPY_KEEPORDER, NPY_NO_CASTING, NULL);
    if (out_iter == NULL) {
        goto fail;
    }
    out_dataptr = NpyIter_GetDataPtrArray(out_iter);
    out_iternext = NpyIter_GetIterNext(out_iter, NULL);
    if (out_iternext == NULL) { 
        NpyIter_Deallocate(out_iter);
        goto fail;
    }

	
    /*  iterate over the array */
    do {
     **out_dataptr += output[j]; 
	j++;
    } while(out_iternext(out_iter) && j < nx*ny); 

    /*  clean up, free mem and return the result */
    NpyIter_Deallocate(out_iter);
    free(output);

    // Do NOT incref this variable, as the python function receives this object im = func(), 
    // it has a ref on im already. If the function is called many times (as is the case when going live), 
    // we would increase references on variables that should be freed. --> memory leak
    // Py_INCREF(out_array);

    return out_array;

    /*  in case bad things happen */
    fail:
    	printf("AcquireSingleFrame FAILED ...\n");
        free(output);
        Py_XDECREF(out_array);
        return NULL;
}

/* Function to set camera ROI */
static PyObject* SetROI(PyObject* self, PyObject* args)
{
    if(camera == (HCAM)NULL)
    {
        printf("Cannot set camera ROI while camera is NULL\n");
        return NULL;
    }
    int x0,y0,width,height;
    int ok = PyArg_ParseTuple(args, "iiii", &x0, &y0, &width, &height);
    IS_RECT rectAOI;
    rectAOI.s32X = x0;
    rectAOI.s32Y = y0;
    rectAOI.s32Width = width;
    rectAOI.s32Height = height;
    INT error = is_AOI( camera, IS_AOI_IMAGE_SET_AOI, (void*)&rectAOI, sizeof(rectAOI));
    if(error != IS_SUCCESS) printf("Error while setting image ROI --- error code : %d\n", error);

    m_nSizeX = width;
    m_nSizeY = height;

    Sleep(300);

    Py_INCREF(Py_None);
    return Py_None;
}

/* Function to connect to Thorlabs USB DCX cameras */
static PyObject* ConnectToUSBCamera(PyObject* self, PyObject* args)
{

    int trigger, color, pixelClock;
    double targetFps, expTime;
    int ok = PyArg_ParseTuple(args, "iiidd", &trigger, &color, &pixelClock, &targetFps, &expTime);

    // Vars
    HCAM    m_hG = (HCAM)0;
    int error = 0;

    // init camera
    INT m_Ret = is_InitCamera( &m_hG, NULL ); 

    if( m_Ret == IS_SUCCESS )
    {
        // Set static variable pointing to camera
        camera = m_hG;

        // retrieve original image size
        is_GetSensorInfo( m_hG, &sInfo );
        GetMaxImageSize(&m_nSizeX, &m_nSizeY, m_hG);
        
        // Set camera parameters
        SetCameraParameters(trigger, color);
        SetCameraPxClkExpFPS(m_hG, pixelClock, targetFps, expTime);


	Sleep(500);
        Py_INCREF(Py_None);
        return Py_None;
    }
    else
    {
        printf("COULD NOT CONNECT TO CAMERA ----- ERROR\nMake sure that the camera is correctly wired to computer\n");
        camera = (HCAM)0;
        // here, set exception to let python run-time system what went wrong
        return NULL; // ERROR 
    }
}

// Close properly Thorcam camera
static PyObject* CloseCamera(PyObject* self, PyObject* args)
{
    int error = PyArg_ParseTuple(args, "");

    // close cam
    HCAM hCam = camera;
    error = is_ExitCamera(hCam);
    if(error != IS_SUCCESS)
	    printf("ERROR while closing camera ----- \n");
    Py_INCREF(Py_None);
    return Py_None;
}

/* Function to connect to Thorlabs USB DCX cameras */
static PyObject* SetDifferenceThresholdAndInterval(PyObject* self, PyObject* args)
{

    double threshold;
    int interval;
    int error = PyArg_ParseTuple(args, "di", &threshold, &interval);
    // Shared variable, used by AcquireSequence
    if(threshold > 0.0)
    {
        difference_threshold = threshold;
        frames_interval = interval;
    }
    else
        difference_threshold = 0.0;

    Py_INCREF(Py_None);
    return Py_None;
}

// test function for flash debug
static PyObject * testCam(PyObject * self, PyObject * args)
{
    int error = PyArg_ParseTuple(args, "");

    // 1. connect cam
    HCAM hcam = (HCAM)0;
    error = is_InitCamera( &hcam, NULL ); 
    if(error != IS_SUCCESS) printf("Error while init cam \n");
    printf("Camera connected \n");

    // 2. set minimal params
    error = is_SetColorMode(hcam, IS_CM_MONO8);
    if(error != IS_SUCCESS) printf("Error while setting color\n");
    UINT clk = 36;
    error = is_PixelClock(hcam, IS_PIXELCLOCK_CMD_SET, (void*)&clk, sizeof(clk));
    if(error != IS_SUCCESS) printf("Error while setting pixel clock\n");
    double fps;
    is_SetFrameRate(hcam, 3, &fps);
    double exp = 200.0;
    error = is_Exposure(hcam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&exp, sizeof(exp)); 
    if(error != IS_SUCCESS) printf("Error while setting exposure\n");
    printf("Camera params set\n");

    // 3. set flash
    error = is_SetExternalTrigger(hcam, IS_SET_TRIGGER_SOFTWARE);// IS_SET_TRIGGER_OFF
    if(error != IS_SUCCESS) printf("Error while setting trigger mode\n");
    UINT nMode = IO_FLASH_MODE_TRIGGER_HI_ACTIVE; //IO_FLASH_MODE_FREERUN_HI_ACTIVE; ;
    is_IO(hcam, IS_IO_CMD_FLASH_SET_MODE, (void*)&nMode, sizeof(nMode));
    if(error != IS_SUCCESS) printf("Error while setting flash mode\n");
    // Set flash parameters to minimum flash delay, maximum flash duration
    IO_FLASH_PARAMS newFlashParams;
    newFlashParams.s32Delay = 1000; //flashDelayMin;
    newFlashParams.u32Duration = 20000; //flashDurationMax;
    error = is_IO( hcam, IS_IO_CMD_FLASH_SET_PARAMS, (void*)&newFlashParams, sizeof(newFlashParams) );
    if (error != IS_SUCCESS) { printf("Error setting flash parameters.\n"); }
    printf("Flash set\n");

    // 4. trigger
    printf("Acquire frame ...\n");
    is_FreezeVideo(hcam,IS_WAIT);

    // 5. close cam
    is_ExitCamera(hcam);


    Py_INCREF(Py_None);
    return Py_None;

}


/*  define functions in module */
static PyMethodDef CamMethods[] =
{
     {"acquire_single_frame", AcquireSingleFrame, METH_VARARGS,"Acquires and send to numpy single frame"},
     {"connect_USB_camera", ConnectToUSBCamera, METH_VARARGS,"Connect to USB camera"}, 
     {"close_camera", CloseCamera, METH_VARARGS,"Close properly camera"}, 
     {"acquire_multiple_frames", AcquireMultiFrames, METH_VARARGS,"Acquires multiple frames and sends to numpy array"},
     {"set_ROI", SetROI, METH_VARARGS,"Set camera ROI"},
     {"set_Flash", SetFlash, METH_VARARGS,"Set flash output to camera, so that the camera itself triggers lightings"},
     {"test_cam", testCam, METH_VARARGS,"Bordel de camera de merde"},
     {"set_min_difference_threshold_and_interval", SetDifferenceThresholdAndInterval, METH_VARARGS,"Sets a minimal threshold between two consecutive frames, which aren't copied if their difference is below the threshold"},
     {NULL, NULL, 0, NULL}
};

/* module initialization */
PyMODINIT_FUNC
initwrapper(void)
{
     (void) Py_InitModule("wrapper", CamMethods);
     /* IMPORTANT: this must be called for numpy stuff*/
     import_array();
}

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
#include <stdio.h>
#include "uc480.h"
#include <math.h>
#include <cstring>
#include "camFunctions.h"


HCAM camera; // pointer to camera
INT     m_nSizeX;       // width of video 
INT     m_nSizeY;       // height of video
SENSORINFO sInfo;
int BitsPerPixel;

static INT     m_nColorMode;   // Y8/RGB16/RGB24/REG32
static INT     m_nBitsPerPixel = 8;// number of bits needed store one pixel

static INT     m_lMemoryId;    // grabber memory - buffer ID
static char*   m_pcImageMemory;// grabber memory - pointer to buffer


char ** GrabSequence(int nFrames, int **id_Array)
{
    // Set pointer on camera
    if(camera == NULL)
    {
        printf("Camera is NULL, can't acquire frames without camera correctly initialized\n");
        return NULL;
    }
    HCAM hCam = camera;

    // Get image size
    IS_RECT rectAOI;
    int error = is_AOI(hCam, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
    if (error == IS_SUCCESS)
    {
        m_nSizeX = rectAOI.s32Width;
        m_nSizeY = rectAOI.s32Height;
    }
    
    // Allocate memory for sequence
    char **ppcImgMemArray; ppcImgMemArray = new char*[nFrames];
    int* idArray; idArray = new int[nFrames];
    
    for (int frm = 0; frm < nFrames; frm++)
    {        
        // Allocate memory for a single frame
        error = is_AllocImageMem(hCam,
                            m_nSizeX,
                            m_nSizeY,
                            BitsPerPixel,
                            &ppcImgMemArray[frm],
                            &idArray[frm]);
        if (error != IS_SUCCESS){printf("Error allocating image memory");}
        
        // Add memory to buffer
        error = is_AddToSequence(hCam, ppcImgMemArray[frm], idArray[frm]);
        if (error != IS_SUCCESS){printf("Error adding image memory to sequence");}
        
    }
    *id_Array = idArray;

#ifndef __LINUX__
    // Create event handler to stop acquisition at of end-of-sequence
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    //Enable sequence event, start image capture and wait for event
    is_InitEvent(hCam, hEvent, IS_SET_EVENT_SEQ);
#endif

    is_EnableEvent(hCam, IS_SET_EVENT_SEQ);
    
    // Start capturing video
    error = is_CaptureVideo(hCam, IS_WAIT);
    if (error != IS_SUCCESS) { printf("Error capturing video."); }
    
    // Wait for the acquisition to finish (max 60 seconds)
#ifdef __LINUX__
    if (is_WaitEvent(hCam, IS_SET_EVENT_SEQ, 60000) == IS_SUCCESS)
    {
    }
#else
    DWORD dwRet = WaitForSingleObject(hEvent, 60000); // timeout = 60 seconds
    if (dwRet == WAIT_TIMEOUT)
    {
        /* wait timed out */
    }
    else if (dwRet == WAIT_OBJECT_0)
    {
        /* event signalled */
    }
#endif
    // Stop capturing video 
    error = is_StopLiveVideo(hCam, IS_WAIT);
    if (error != IS_SUCCESS) { printf("Error stoping video capture."); }


    // Uninstall event handling
    is_DisableEvent(hCam, IS_SET_EVENT_FRAME);

#ifndef __LINUX__
    is_ExitEvent(hCam, IS_SET_EVENT_FRAME);
    CloseHandle(hEvent);
#endif

    return ppcImgMemArray;
}

char * GrabFrame(int* nX, int* nY)
{
    HCAM    m_hG = (HCAM)0;
    int error;

    if(camera != (HCAM)0)
        m_hG = camera;
    else
    {
        printf("ERROR -- Trying to set camera parameters while camera is null\n");
        // This tells python there's been an error (but not which one...)
        return NULL;
    }

    // memory initialization
    error = is_AllocImageMem(  
                        m_hG,
                        m_nSizeX,
                        m_nSizeY,
                        m_nBitsPerPixel,
                        &m_pcImageMemory,
                        &m_lMemoryId);
    if(error != IS_SUCCESS) printf("Error allocating camera memory\n");

    // set memory active
    error = is_SetImageMem(m_hG, m_pcImageMemory, m_lMemoryId );   
    if(error != IS_SUCCESS) printf("Error while setting image memory : %d\n", error);

	// Acquire actual image
	const int imSize = m_nBitsPerPixel*m_nSizeX*m_nSizeY;
	char* p_output = new char[imSize];

    //Use freeze video to initiate frame grab
    error = is_FreezeVideo(m_hG, IS_WAIT );
    if (error !=IS_SUCCESS){printf("Error freezing frame in GrabFrame -- Error code is : %d\n",error);}

	// ACTUAL COPY
	error = is_CopyImageMem(m_hG,m_pcImageMemory, m_lMemoryId, p_output);
	if (error !=IS_SUCCESS) {printf("Error after image copy ---- Error code is : %d\n",error);}

	// Set outputs
	*nX = m_nSizeX;
	*nY = m_nSizeY;

    // Free mem
    error = is_FreeImageMem(m_hG, m_pcImageMemory, m_lMemoryId);
    if (error != IS_SUCCESS) { printf("Error freeing image memory."); }

	return p_output;
}


void SetCameraPxClkExpFPS(HCAM hCam, int pix_clk, double exp_int, double fps_int)
{
	int error=0;

    // Set Pixel clock
    UINT Clk = (UINT)pix_clk;
    // Set pixel clock speed
    error = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, &Clk, sizeof(UINT));
    if (error != IS_SUCCESS) { 
        // If setting pixel clock value failed (it has to be in a list of accepted values), read max and 
        // set pxClock to maximum value.
        UINT nRange[3];
        ZeroMemory(nRange, sizeof(nRange));
        error = is_PixelClock (hCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)nRange, sizeof(nRange));
        UINT clkm = nRange[0];
	printf("Clock set : %d \n", clkm);
        error = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, &clkm, sizeof(UINT)); //  &Clk, sizeof(UINT));
    }
    if (error != IS_SUCCESS) { printf("Error setting pixel clock -- Error code is : %d \n", error);}
    // Get current pixel clock
    UINT nPixelClock;
    error = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void*)&nPixelClock, sizeof(nPixelClock));
    printf("Pixel Clock set to : %d MHz\n", nPixelClock);    

    // Set frame rate
    double fps_actual;
    error = is_SetFrameRate (hCam, fps_int, &fps_actual);
    if (error != IS_SUCCESS) { printf("Error setting frame rate"); }
    printf("Frame rate set to : %f fps \n", fps_actual);

    // Set exposure exposure time to input value
    error = is_Exposure (hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&exp_int, sizeof(exp_int));
    if (error != IS_SUCCESS){ printf("Error setting exposure time"); }
    printf("Exposure time set to : %f ms\n", exp_int);

    // Sleep to allow settings to update
    Sleep(10);  

    // Set flash output (usefull for debug)
    int nRet = IS_SUCCESS;
    IO_FLASH_PARAMS flashParams;

    nRet = is_IO(hCam, IS_IO_CMD_FLASH_GET_GPIO_PARAMS_MIN, (void*)&flashParams,
		    sizeof(flashParams));
    if(nRet != IS_SUCCESS)
	    printf("Error ---- Couldn't read Flash parameters from the camera \n");

    nRet = is_IO(hCam, IS_IO_CMD_FLASH_SET_GPIO_PARAMS, (void*)&flashParams,
		    sizeof(flashParams));

    if(nRet != IS_SUCCESS)
	    printf("Error ---- Couldn't set Flash parameters to camera \n");
    

}


void SetCameraParameters(int hardTrigger, int useColor)
{
    int error =0;
    HCAM hCam = (HCAM)0;

    if(camera != (HCAM)0 )
        hCam = camera;
    else
    {
        printf("ERROR -- Trying to set camera parameters while camera is null\n");
        return;
    }

    // setup the color depth 
    if(useColor >= 1)
        m_nColorMode = IS_CM_SENSOR_RAW8;
    else
        m_nColorMode = IS_CM_MONO8;
    is_SetColorMode(hCam, m_nColorMode);
    // is_GetColorDepth(m_hG, &m_nBitsPerPixel, &m_nColorMode); 
    m_nBitsPerPixel = 8; // this should be dealt with by GetColorDepth... 
    error = is_SetDisplayMode(hCam, IS_SET_DM_DIB);
    if (error !=IS_SUCCESS) { printf("Error setting display mode");  }

    int nTriggerMode;
    // Trigger info -- hardware (low to high) or software?
    if(hardTrigger >= 1)
    {
	nTriggerMode = IS_SET_TRIGGER_LO_HI; 
        error = is_SetExternalTrigger(hCam, nTriggerMode );
    }
    else
	nTriggerMode = IS_SET_TRIGGER_SOFTWARE;

    //Disable auto parameters
    double pval1 = 0;
    double pval2 = 0;
    error = is_SetAutoParameter (hCam,  IS_SET_ENABLE_AUTO_GAIN, &pval1, &pval2);
    if (error !=IS_SUCCESS)  { printf("IS_SET_ENABLE_AUTO_GAIN error");  }

     error = is_SetAutoParameter (hCam,  IS_SET_ENABLE_AUTO_SHUTTER, &pval1, &pval2);
    if (error !=IS_SUCCESS) { printf("IS_SET_ENABLE_AUTO_SHUTTER error"); }

    error = is_SetAutoParameter (hCam,  IS_SET_ENABLE_AUTO_FRAMERATE, &pval1, &pval2);
    if (error !=IS_SUCCESS) { printf("IS_SET_ENABLE_AUTO_FRAMERATE error"); }
    
    // Set gain levels for all channels to 0
    error = is_SetHardwareGain (hCam, 0, 0, 0, 0);
    if (error != IS_SUCCESS) { printf("Error setting gain to minimum");}

    // Disable gain boost 
    error = is_SetGainBoost (hCam, IS_SET_GAINBOOST_OFF);
	if (error != IS_SUCCESS) {printf("Error turning off gainboost");}
    
    // Sleep to allow settings to update
    Sleep(10);
}


void GetMaxImageSize(INT *pnSizeX, INT *pnSizeY, HCAM m_hG)
{
    // Check if the camera supports an arbitrary AOI
    INT nAOISupported = 0;
    BOOL bAOISupported = TRUE;
    if (is_ImageFormat(m_hG,
                       IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED, 
                       (void*)&nAOISupported, 
                       sizeof(nAOISupported)) == IS_SUCCESS)
    {
        bAOISupported = (nAOISupported != 0);
    }

    if (bAOISupported)
    {
        // Get maximum image size
	    is_GetSensorInfo (m_hG, &sInfo);
	    *pnSizeX = sInfo.nMaxWidth;
	    *pnSizeY = sInfo.nMaxHeight;
    }
    else
    {
        // Get image size of the current format
        *pnSizeX = 256; //is_SetImageSize(m_hG, IS_GET_IMAGE_SIZE_X, 0);
        *pnSizeY = 256; //is_SetImageSize(m_hG, IS_GET_IMAGE_SIZE_Y, 0);
    }
}

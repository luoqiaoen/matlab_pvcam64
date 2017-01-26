/******************************************************************************/
/* Copyright by Photometrics and QImaging. All rights reserved.               */
/******************************************************************************/
#pragma once
#ifndef PVCAM_HELPER_COLOR_H
#define PVCAM_HELPER_COLOR_H

/* PVCAM */
#include <master.h>
#include <pvcam.h>

/**
Major version number.

This part of version number changes whenever a binary incompatible modification
to library API is done, e.g. changed name or prototype of existing function.

Applications can check this at compile time to use appropriate syntax.
*/
#define PH_COLOR_VERSION_MAJOR 1
/**
Minor version number.

This part of version number changes whenever a binary compatible modification to
library API is done, e.g. added new function or data type.

Applications can check this at compile time to use new functions and/or types.
*/
#define PH_COLOR_VERSION_MINOR 3

/** Debayer algorthms. */
typedef enum PH_COLOR_DEBAYER_ALG
{
    /** @showinitializer */
    PH_COLOR_DEBAYER_ALG_NEAREST = 0,   /**< Nearest neighbour */
    PH_COLOR_DEBAYER_ALG_BILINEAR      /**< Bilinear averaging */
    // TODO: Add bicubic algorithm back
    //PH_COLOR_DEBAYER_ALG_BICUBIC        /**< Bicubic */
}
PH_COLOR_DEBAYER_ALG;

/**
RGB and Debayer output buffer formats.

The endianess of the data will be consistent with the processor used.
For x86 and x64 architecture it is Little Endian.
*/
typedef enum PH_COLOR_RGB_FORMAT
{
    /* RGB formats from 16bit raw sensor */
    PH_COLOR_RGB_FORMAT_RGB48,      /**< 48bpp interleaved
                                        (normal for 16bit sensors) */
    PH_COLOR_RGB_FORMAT_PLANE16,    /**< 48bpp planar, channels in order RGB */

    /* RGB formats from 8bit raw sensor */
    PH_COLOR_RGB_FORMAT_RGB24,      /**< 24bpp RGB (normal for 8bit sensors) */
    PH_COLOR_RGB_FORMAT_BGR24,      /**< 24bpp BGR, common Windows format */
    PH_COLOR_RGB_FORMAT_PLANE8,     /**< 24bpp planar, channels in order RGB */
    PH_COLOR_RGB_FORMAT_ARGB32,     /**< 32bpp aRGB with alpha channel,
                                          format of Mac pixelmap */
    PH_COLOR_RGB_FORMAT_BGRA32,     /**< 32bpp BGRa with alpha channel,
                                          common Windows format */
    PH_COLOR_RGB_FORMAT_RGBA32      /**< 32bpp RGBa with alpha channel */
}
PH_COLOR_RGB_FORMAT;

/**
The context holding camera specific state.

The context allows application to access currently used settings.
If the application changes any value, the library has to be notified by passing
the context to #ph_color_context_apply_changes function. Without calling that
function all other functions will operate on outdated internal copy stored in
@p p.

This approach for instance allows speed up of white balance algorithm with help
of lookup tables.
*/
typedef struct ph_color_context
{
    int32 algorithm;    /**< Debayer algorithm select from enum
                            #PH_COLOR_DEBAYER_ALG. */
    int32 pattern;      /**< Sensor bayer pattern from PVCAM PL_COLOR_MODES enum
                            as reported by the camera. */
    uns16 bitDepth;     /**< Image bit depth from the camera speed table. */
    int32 rgbFormat;    /**< RGB buffer data format from
                            #PH_COLOR_RGB_FORMAT. */

    flt32 redScale;     /**< Red channel scale value for the white balance
                            function #ph_color_white_balance. */
    flt32 greenScale;   /**< Green channel scale value for the white balance
                            function #ph_color_white_balance. */
    flt32 blueScale;    /**< Blue channel scale value for the white balance
                            function #ph_color_white_balance. */

    void* p;            /**< Private context data for internal use. */
}
ph_color_context;

#ifdef PV_C_PLUS_PLUS
extern "C"
{
#endif

/******************************************************************************/
/**
Fills a buffer with the library version as a string.

This function can be called at any time even before the color library has been
initialized. It could fail only in case the NULL pointer is given as parameter.

@param[out] pVerMajor   Major version number, returns at runtime the same as
                        macro #PH_COLOR_VERSION_MAJOR.
@param[out] pVerMinor   Minor version number, returns at runtime the same as
                        macro #PH_COLOR_VERSION_MINOR.
@param[out] pVerBuild   Build version number, provided for completeness.
                        It is always increasing number changed with every
                        modification in library.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_get_lib_version(uns16* pVerMajor, uns16* pVerMinor,
        uns16* pVerBuild);

/******************************************************************************/
/**
Allocate and initialize the context used in the color helper library.

New context must be created for every camera to allow multiple cameras to be
used simultaneously.

The @p pContext value is ignored and replaced by new context. Be sure to release
previous context before calling this function to not have memory leaks.

Default context values are:
- @p algorithm = #PH_COLOR_DEBAYER_ALG_BILINEAR
- @p pattern = COLOR_RGGB
- @p bitDepth = 16
- @p rgbFormat = #PH_COLOR_RGB_FORMAT_RGB48
- @p redScale = 1.0
- @p greenScale = 1.0
- @p blueScale = 1.0

@param[out] pContext    A location where a pointer to newly allocated context
                        will be stored.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_context_create(ph_color_context** pContext);

/******************************************************************************/
/**
Release the context previously created by #ph_color_context_create function.

This function will release memory allocated for context and sets @p pContext to
null.

@param[in,out] pContext     A location where a pointer to already allocated
                            context is stored.
*/
void PV_DECL ph_color_context_release(ph_color_context** pContext);

/******************************************************************************/
/**
Apply changes being done in context structure.

This function will compare context with internal one and will update all
internal data (e.g. lookup tables) stored in context's private member.

All other functions work with internal data and ignore any changes on context
structure done by application. Only this function makes new settings available
for future processing.

@note This function will allow only setting 8 bit RGB format for bit depths from
1 to 8 and 16 bit output format for bit depths from 9 to 16.
See #PH_COLOR_RGB_FORMAT for supported output formats.

@param[in] pContext     A context with updated settings to be applied.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_context_apply_changes(ph_color_context* pContext);

/******************************************************************************/
/**
Debayer a raw image in the input buffer to a RGB image in the output buffer.

The size of the output buffer must be three times larger than the input buffer
(in bytes) for the RGB data.

@note This function will only convert 8 bit raw images to 8bit output formats
and 16 bit raw images to 16 bit output formats. This is enforced by function
#ph_color_context_apply_changes because all functions work with internal copy
of context which is alwyas valid (it changes only when successfully applied).

@param[in]  pContext    A context with current settings.
@param[in]  pInBufRaw   Pointer to raw image from camera.
@param[in]  roi         ROI relative to full frame camera sensor size.
@param[out] pOutBufRgb  Pointer to output buffer being large enough to hold
                        debayered image in output format as set in pContext,
                        e.g. 48bpp RGB image should be 3 times larger than input
                        buffer.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_debayer(const ph_color_context* pContext,
        const void* pInBufRaw, rgn_type roi, void* pOutBufRgb);

/******************************************************************************/
/**
White balance the RGB image in place using the currently set RGB scale values.

This function is normally called after a call to #ph_color_context_apply_changes
which will set the RGB scale values and then for every new every frame.

Currently used RGB scale values can be accessed via context.

@param[in]      pContext    A context with current settings.
@param[in,out]  pBufRgb     RGB image buffer to white balance.
@param[in]      imgWidth    RGB image width in pixels.
@param[in]      imgHeight   RGB image height in pixels.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_white_balance(const ph_color_context* pContext,
        void* pBufRgb, uns16 imgWidth, uns16 imgHeight);

/******************************************************************************/
/**
For the camera specified search optimal exposure time.

The white/grey area ROI is specified as coordinates relative to the sensor full
frame size.

When this function returns successfully, the optimal exposure time is stored in
argument @p pExpTime.

This function uses the current camera settings for speed, gain state, clearing
mode and exposure resolution. It acquires frames using polling and does not
set up any callbacks with PVCAM.

@attention This function uses blocking calls to PVCAM core to acquire images to
    measure the exposure level. Do not call this function while any PVCAM
    functions are in progress or any callbacks have been registered.

@param[in]  pContext    A context with current settings.
@param[in]  hcam        Handle to open camera to measure exposure.
@param[in]  roi         ROI of white/grey area relative to full frame.
@param[out] pBufRaw     A buffer for raw image big enough for @p roi.
@param[out] pBufRgb     A buffer for debayered RGB image big enough for @p roi.
@param[out] pExpTime    Optimal exposure time.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_auto_exposure(const ph_color_context* pContext,
        int16 hcam, rgn_type roi, void* pBufRaw, void* pBufRgb,
        uns32* pExpTime);

/******************************************************************************/
/**
Interrupt the blocking #ph_color_auto_exposure function.

This function can be called multiple times as well as if there is not auto
exposure in progress. In both cases function returns without error.

As the auto exposure is blocking the abort has to be issued from another thread.

Because of #ph_color_auto_exposure function uses polling every a few
milliseconds, this function does not need a camera handle argument. It only sets
an abort flag and exits immediately. The abort flag is periodically tested in
polling loop.

@param[in]  pContext    A context with current settings.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_auto_exposure_abort(const ph_color_context* pContext);

/******************************************************************************/
/**
Calculate RGB scale factors from given debayered image.

The white/grey area ROI is specified as coordinates relative to the sensor full
frame size.

The @p pBufRgb buffer should contain debayered image captured with optimal
exposure time. That time can be received e.g. from #ph_color_auto_exposure
function.

When this function returns successfully, the RGB scale factors are stored in
arguments @p pRedScale, @p pGreenScale and @p pBlueScale respectively.

@param[in]  pContext    A context with current settings.
@param[in]  roi         ROI of white/grey area relative to full frame.
@param[in]  pBufRgb     A buffer with debayered RGB image big enough for @p roi.
@param[out] pRedScale   Scale factor for red channel.
@param[out] pGreenScale Scale factor for green channel.
@param[out] pBlueScale  Scale factor for blue channel.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_auto_white_balance(const ph_color_context* pContext,
        rgn_type roi, const void* pBufRgb, flt32* pRedScale, flt32* pGreenScale,
        flt32* pBlueScale);

/******************************************************************************/
/**
Wrapper for #ph_color_auto_exposure and #ph_color_auto_white_balance functions.

This function internally allocates buffers for raw and RGB images.

Any of the four output arguments (@p pExpTime, @p pRedScale, @p pGreenScale and
@p pBlueScale) can be null if the value is not important.

@param[in]  pContext    A context with current settings.
@param[in]  hcam        Handle to open camera to measure exposure.
@param[in]  roi         ROI of white/grey area relative to full frame.
@param[out] pExpTime    Optimal exposure time.
@param[out] pRedScale   Scale factor for red channel.
@param[out] pGreenScale Scale factor for green channel.
@param[out] pBlueScale  Scale factor for blue channel.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_auto_exposure_and_white_balance(
        const ph_color_context* pContext, int16 hcam, rgn_type roi,
        uns32* pExpTime, flt32* pRedScale, flt32* pGreenScale,
        flt32* pBlueScale);

/******************************************************************************/
/**
Convert image format from one RGB format and/or bit depth to another.

This function can be used to up-scale or down-scale the pixel value based on bit
depth parameters @p inBitDepth and @p outBitDepth. It can also re-order RGB
parts of each pixel so for instance the buffer could be directly passed to
libtiff library to save image as TIFF or to wxImage::SetData to display picture
on the screen.

@param[in]  width           Width of converted image.
@param[in]  height          Height of converted image.
@param[in]  pFromBufRgb     Input buffer with debayered image.
@param[in]  fromRgbFormat   RGB format of the input buffer.
@param[in]  fromBitDepth    Pixel bit depth of the input buffer.
@param[out] pToBufRgb       Output buffer with debayered image.
@param[in]  toRgbFormat     RGB format of the output buffer.
@param[in]  toBitDepth      Pixel bit depth of the output buffer.

@return Returns PV_OK (TRUE) on success, PV_FAIL (FALSE) otherwise.
*/
rs_bool PV_DECL ph_color_convert_format(uns16 width, uns16 height,
        const void* pFromBufRgb, int32 fromRgbFormat, uns16 fromBitDepth,
        void* pToBufRgb, int32 toRgbFormat, uns16 toBitDepth);

#ifdef PV_C_PLUS_PLUS
};
#endif

#endif /* PVCAM_HELPER_COLOR_H */

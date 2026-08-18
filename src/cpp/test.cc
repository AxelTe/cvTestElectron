#include <napi.h>
#include <iostream>
#include <cstdint>
#include <vector>

#include "cvTools.h"

#define GMASKSIZE 5

// Persistent memory allocated in C++ heap
static std::vector<float> gmask32F;
static uint32_t rows;
static uint32_t cols;
static uint32_t size;
static uint32_t nofLevels;

std::vector<imgLevel> imgLevels;

/**
 *
 */
Napi::Value initialize(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    // check input 'info' structure
    if (info.Length() != 2)
    {
        Napi::TypeError::New(env, "expected: (width, height)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    rows = info[0].As<Napi::Number>().Uint32Value();
    cols = info[1].As<Napi::Number>().Uint32Value();
    size = rows * cols;

    // Allocate memory on the C++ heap
    gmask32F.resize(GMASKSIZE, 0);
    cvtInitGMask32F(2.0, gmask32F, GMASKSIZE);
    nofLevels = 3;
    imgLevels.resize(nofLevels);
    cvtInitImgLevels(imgLevels, rows, cols, nofLevels);

    return Napi::String::New(env, "initialize done");
}

/**
 *
 */
Napi::Value process(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    float magMax;
    float gradTh = 5.0;
    uint32_t nofEdges, i, icols, irows;

    // check input 'info' structure
    if (info.Length() < 4 || !info[0].IsBuffer())
    {
        Napi::TypeError::New(env, "expected: (image buffer, width, height, channels)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // extract single arguments from 'info' structure
    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    uint32_t height = info[1].As<Napi::Number>().Uint32Value();
    uint32_t width = info[2].As<Napi::Number>().Uint32Value();
    uint32_t channels = info[3].As<Napi::Number>().Uint32Value();
    if (width != cols || height != rows)
    {
        Napi::TypeError::New(env, "image width/height does not fit initialized cols/rows values")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // pointer to input pixel-array (Zero-Copy!)
    uint8_t *pixels = buffer.Data();
    size_t length = buffer.Length();
    if (length != (size_t)(size * channels))
    {
        Napi::TypeError::New(env, "input buffer length does not fit initialized cols/rows values")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    //
    // start processing ...
    /**/
    cvtRGBA2Grey(pixels, imgLevels[0].greyI_float, size);
    cvtGauss5x5(imgLevels[0].greyI_float, imgLevels[0].tmpI_float, imgLevels[0].gaussI_float, gmask32F, rows, cols);
    magMax = cvtGrad(imgLevels[0].gaussI_float, imgLevels[0].gradI_float, gradTh, rows, cols);
    nofEdges = cvtEdgDetection(imgLevels[0].gradI_float, imgLevels[0].edgL_float, gradTh, rows, cols);
    //
    icols = cols;
    irows = rows;
    for (i = 1; i < nofLevels; i++)
    {
        icols >>= 1;
        irows >>= 1;
        cvtSubSample(imgLevels[i - 1].gaussI_float, imgLevels[i].greyI_float, irows, icols);
        cvtGauss5x5(imgLevels[i].greyI_float, imgLevels[i].tmpI_float, imgLevels[i].gaussI_float, gmask32F, irows, icols);
        magMax = cvtGrad(imgLevels[i].gaussI_float, imgLevels[i].gradI_float, gradTh, irows, icols);
        nofEdges = cvtEdgDetection(imgLevels[i].gradI_float, imgLevels[i].edgL_float, gradTh, irows, icols);
    }

#if 0
    //
    // convert image back for display
    // cvtGrey2RGBA(gfilI32F, pixels, size);
    // cvtMag2RGBA(gradI, pixels, size);
    cvtEdg2RGBA(imgLevels[0].gradI_float, pixels, size);
    icols = cols;
    irows = rows;
    for (i = 1; i < nofLevels; i++)
    {
        icols >>= 1;
        irows >>= 1;
        cvtCopyI32F2RGBA(imgLevels[i].greyI_float, irows, icols, pixels, 0, 0, rows, cols);
    }
#endif
    return Napi::String::New(env, "process done");
}

/**
 *
 */
Napi::Value show(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    uint32_t i, icols, irows, ix0;
    uint32_t height, width, channels, mode;

    // check input 'info' structure
    if (info.Length() < 5 || !info[0].IsBuffer())
    {
        Napi::TypeError::New(env, "expected: (image buffer, width, height, channels, mode)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // extract single arguments from 'info' structure
    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    height = info[1].As<Napi::Number>().Uint32Value();
    width = info[2].As<Napi::Number>().Uint32Value();
    channels = info[3].As<Napi::Number>().Uint32Value();
    mode = info[4].As<Napi::Number>().Uint32Value();
    if (width != cols || height != rows)
    {
        Napi::TypeError::New(env, "image width/height does not fit initialized cols/rows values")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // pointer to input pixel-array (Zero-Copy!)
    uint8_t *pixels = buffer.Data();
    size_t length = buffer.Length();
    if (length != (size_t)(size * channels))
    {
        Napi::TypeError::New(env, "input buffer length does not fit initialized cols/rows values")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    //
    // start processing ...
    switch (mode)
    {
    // pyramid level
    case 12:
        // edges
        icols = cols;
        irows = rows;
        ix0 = 0;
        for (i = 1; i < nofLevels; i++)
        {
            icols >>= 1;
            irows >>= 1;
            cvtCopyI32F2RGBA(imgLevels[i].greyI_float, irows, icols, pixels, ix0, 0, rows, cols);
            cvtEdgPts2RGBA(imgLevels[i].edgL_float, pixels, rows, cols, ix0, 0);
            ix0 += icols;
        }
        break;    
    case 11:
        // gradient magnitude
        icols = cols;
        irows = rows;
        ix0 = 0;
        for (i = 1; i < nofLevels; i++)
        {
            icols >>= 1;
            irows >>= 1;
            cvtMag2floatI(imgLevels[i].gradI_float, imgLevels[i].tmpI_float, icols*irows);
            cvtCopyI32F2RGBA(imgLevels[i].tmpI_float, irows, icols, pixels, ix0, 0, rows, cols);
            ix0 += icols;
        }
        break;
    case 10:
        // grey image
        icols = cols;
        irows = rows;
        ix0 = 0;
        for (i = 1; i < nofLevels; i++)
        {
            icols >>= 1;
            irows >>= 1;
            cvtCopyI32F2RGBA(imgLevels[i].greyI_float, irows, icols, pixels, ix0, 0, rows, cols);
            ix0 += icols;
        }
        break;

    // original level
    case 2:
        // edges
        cvtGrey2RGBA(imgLevels[0].greyI_float, pixels, size);
        cvtEdgPts2RGBA(imgLevels[0].edgL_float, pixels, rows, cols, 0, 0);
        break;
    case 1:
        // gradient magnitude
        cvtMag2RGBA(imgLevels[0].gradI_float, pixels, size);
        break;
    default:
        // grey image
        cvtGrey2RGBA(imgLevels[0].greyI_float, pixels, size);
        break;
    }

    return Napi::String::New(env, "process done");
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "initialize"), Napi::Function::New(env, initialize));
    exports.Set(Napi::String::New(env, "process"), Napi::Function::New(env, process));
    exports.Set(Napi::String::New(env, "show"), Napi::Function::New(env, show));
    return exports;
}

NODE_API_MODULE(my_addon, Init)
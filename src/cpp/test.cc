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
static float gradThLow;
static float gradThHigh;
static uint32_t curvBd;

std::vector<imgLevel> imgLevels;

/**
 *
 */
Napi::Value initialize(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    // check input 'info' structure
    if (info.Length() != 6)
    {
        Napi::TypeError::New(env, "expected: (width, height, nofLevels, gradThLow, gradThHigh, curvBd)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    rows = info[0].As<Napi::Number>().Uint32Value();
    cols = info[1].As<Napi::Number>().Uint32Value();
    nofLevels = info[2].As<Napi::Number>().Uint32Value();
    gradThLow = info[3].As<Napi::Number>().FloatValue();
    gradThHigh = info[4].As<Napi::Number>().FloatValue();
    curvBd = info[5].As<Napi::Number>().Uint32Value();
    size = rows * cols;

    // Allocate memory on the C++ heap
    gmask32F.resize(GMASKSIZE, 0);
    cvtInitGMask32F(2.0, gmask32F, GMASKSIZE);
    // nofLevels = 3;
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
    uint32_t i, icols, irows;
    std::string returnStr;

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
    // start timer
    cvtRGBA2Grey(pixels, imgLevels[0].greyI_float, size);
    cvtGauss5x5(imgLevels[0].greyI_float, imgLevels[0].tmpI_float, imgLevels[0].gaussI_float, gmask32F, rows, cols);
    cvtGrad(imgLevels[0].gaussI_float, imgLevels[0].gradI_float, gradThLow, rows, cols);
    cvtEdgDetection(imgLevels[0].gradI_float, imgLevels[0].edgesL, gradThLow, gradThHigh, rows, cols);
    returnStr += "nofedges: " + std::to_string(imgLevels[0].edgesL.size());
    cvtEdgFilter3x3(imgLevels[0].gradI_float, imgLevels[0].edgesL, rows, cols);
    returnStr += ", " + std::to_string(imgLevels[0].edgesL.size());
    cvtEdgSubPixel(imgLevels[0].gradI_float, imgLevels[0].edgesL, rows, cols);
    cvtLinkEdges(imgLevels[0].edgesL, imgLevels[0].gradI_float, gradThLow, gradThHigh, rows, cols);
    cvtLinkContours(imgLevels[0].edgesL, imgLevels[0].contoursL, gradThLow + 0.5 * (gradThHigh - gradThLow));
    returnStr += ", nofconts: " + std::to_string(imgLevels[0].contoursL.size());
    cvtCurvatureContours(imgLevels[0].edgesL, imgLevels[0].contoursL, curvBd);
    cvtLineDetection(imgLevels[0].edgesL, imgLevels[0].contoursL, imgLevels[0].linesL, curvBd);
    cvtTimerValues(&imgLevels[0].procTime);
    // cvtEdges2Contours(imgLevels[0].edgesL);
    // cvtEdgHistogram(imgLevels[0].edgesL, imgLevels[0].histEdges);
    //
    icols = cols;
    irows = rows;
    for (i = 1; i < nofLevels; i++)
    {
        icols >>= 1;
        irows >>= 1;
        cvtSubSample(imgLevels[i - 1].gaussI_float, imgLevels[i].greyI_float, irows, icols);
        cvtGauss5x5(imgLevels[i].greyI_float, imgLevels[i].tmpI_float, imgLevels[i].gaussI_float, gmask32F, irows, icols);
        cvtGrad(imgLevels[i].gaussI_float, imgLevels[i].gradI_float, gradThLow, irows, icols);
        cvtEdgDetection(imgLevels[i].gradI_float, imgLevels[i].edgesL, gradThLow, gradThHigh, irows, icols);
        cvtEdgFilter3x3(imgLevels[i].gradI_float, imgLevels[i].edgesL, irows, icols);
        cvtEdgSubPixel(imgLevels[i].gradI_float, imgLevels[i].edgesL, irows, icols);
        cvtLinkEdges(imgLevels[i].edgesL, imgLevels[i].gradI_float, gradThLow, gradThHigh, irows, icols);
        cvtLinkContours(imgLevels[i].edgesL, imgLevels[i].contoursL, gradThLow + 0.5 * (gradThHigh - gradThLow));
        cvtCurvatureContours(imgLevels[i].edgesL, imgLevels[i].contoursL, curvBd);
        cvtLineDetection(imgLevels[i].edgesL, imgLevels[i].contoursL, imgLevels[i].linesL, curvBd);
        cvtTimerValues(&imgLevels[i].procTime);
    }

    return Napi::String::New(env, returnStr);
}

/**
 *  Returning a std::vector<uint_32> as a JavaScript Array
 */
Napi::Array getHistEdges(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    uint32_t histSize = imgLevels[0].histEdges.size();

    // Create a new JS Array with pre-allocated length
    Napi::Array jsArray = Napi::Array::New(env, histSize);

    for (size_t i = 0; i < histSize; ++i)
    {
        jsArray.Set(i, Napi::Number::New(env, imgLevels[0].histEdges[i]));
    }

    return jsArray;
}

// Function converting std::vector<DataItem> to Napi::Array
Napi::Value getEdgeData(const Napi::CallbackInfo &info)
{
    int levels = imgLevels.size();
    int nofEdges, nofConts, nofLines;
    Napi::Env env = info.Env();

    Napi::Array result = Napi::Array::New(env, levels);

    for (int l = 0; l < levels; l++)
    {
        nofEdges = imgLevels[l].edgesL.size();
        nofConts = imgLevels[l].contoursL.size();
        nofLines = imgLevels[l].linesL.size();
        Napi::Array edges = Napi::Array::New(env, nofEdges);
        Napi::Array conts = Napi::Array::New(env, nofConts);
        Napi::Array lines = Napi::Array::New(env, nofLines);
        Napi::Array edgHist = Napi::Array::New(env, imgLevels[l].histEdges.size());

        std::vector<edge>::iterator edgPt = imgLevels[l].edgesL.begin();
        for (size_t i = 0; i < nofEdges; ++i, ++edgPt)
        {
            Napi::Object obj = Napi::Object::New(env);
            // Map C++ struct properties to JS Object keys
            obj.Set("xi", Napi::Number::New(env, edgPt->xi));
            obj.Set("yi", Napi::Number::New(env, edgPt->yi));
            obj.Set("xf", Napi::Number::New(env, edgPt->xf));
            obj.Set("yf", Napi::Number::New(env, edgPt->yf));
            obj.Set("mag", Napi::Number::New(env, edgPt->mag));
            obj.Set("dir", Napi::Number::New(env, edgPt->dir));
            obj.Set("p", Napi::Number::New(env, edgPt->p));
            obj.Set("s", Napi::Number::New(env, edgPt->s));
            obj.Set("c", Napi::Number::New(env, edgPt->c));
            obj.Set("n3x3", Napi::Number::New(env, edgPt->n3x3));
            obj.Set("curv", Napi::Number::New(env, edgPt->curv));
            edges[i] = obj;
        }

        std::vector<contour>::iterator cont = imgLevels[l].contoursL.begin();
        for (size_t i = 0; i < nofConts; ++i, ++cont)
        {
            Napi::Object obj = Napi::Object::New(env);
            // Map C++ struct properties to JS Object keys
            obj.Set("start", Napi::Number::New(env, cont->start));
            obj.Set("end", Napi::Number::New(env, cont->end));
            obj.Set("nofEdges", Napi::Number::New(env, cont->nofEdges));
            conts[i] = obj;
        }

        std::vector<line>::iterator linePtr = imgLevels[l].linesL.begin();
        for (size_t i = 0; i < nofLines; ++i, ++linePtr)
        {
            Napi::Object obj = Napi::Object::New(env);
            // Map C++ struct properties to JS Object keys
            obj.Set("start", Napi::Number::New(env, linePtr->start));
            obj.Set("end", Napi::Number::New(env, linePtr->end));
            obj.Set("cont", Napi::Number::New(env, linePtr->cont));
            obj.Set("nofEdges", Napi::Number::New(env, linePtr->nofEdges));
            lines[i] = obj;
        }

        for (size_t i = 0; i < imgLevels[l].histEdges.size(); ++i)
        {
            edgHist.Set(i, Napi::Number::New(env, imgLevels[l].histEdges[i]));
        }

        Napi::Object level = Napi::Object::New(env);

        level.Set("edges", edges);
        level.Set("conts", conts);
        //level.Set("edgHist", edgHist);
        level.Set("lines", lines);

        result.Set(l, level);
    }

    return result;
}

// Function converting std::vector<DataItem> to Napi::Array
Napi::Value getTimerValues(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Array _cvtRGBA2Grey = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtGauss5x5 = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtGrad = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtEdgDetection = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtEdgFilter3x3 = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtEdgSubPixel = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtLinkEdges = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtLinkContours = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtCurvatureContours = Napi::Array::New(env, imgLevels.size());
    Napi::Array _cvtLineDetection = Napi::Array::New(env, imgLevels.size());
    Napi::Array _nofPixels = Napi::Array::New(env, imgLevels.size());
    Napi::Array _nofEdges = Napi::Array::New(env, imgLevels.size());
    Napi::Array _nofConts = Napi::Array::New(env, imgLevels.size());
    Napi::Array _nofLines = Napi::Array::New(env, imgLevels.size());
    for (int i = 0; i < imgLevels.size(); i++)
    {
        _cvtRGBA2Grey.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtRGBA2Grey));
        _cvtGauss5x5.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtGauss5x5));
        _cvtGrad.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtGrad));
        _cvtEdgDetection.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtEdgDetection));
        _cvtEdgFilter3x3.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtEdgFilter3x3));
        _cvtEdgSubPixel.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtEdgSubPixel));
        _cvtLinkEdges.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtLinkEdges));
        _cvtLinkContours.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtLinkContours));
        _cvtCurvatureContours.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtCurvatureContours));
        _cvtLineDetection.Set(i, Napi::Number::New(env, imgLevels[i].procTime.cvtLineDetection));
        //
        _nofPixels.Set(i, Napi::Number::New(env, imgLevels[i].greyI_float.size()));
        _nofEdges.Set(i, Napi::Number::New(env, imgLevels[i].edgesL.size()));
        _nofConts.Set(i, Napi::Number::New(env, imgLevels[i].contoursL.size()));
        _nofLines.Set(i, Napi::Number::New(env, imgLevels[i].linesL.size()));
    }

    Napi::Object result = Napi::Object::New(env);
    result.Set(Napi::String::New(env, "cvtRGBA2Grey"), _cvtRGBA2Grey);
    result.Set(Napi::String::New(env, "cvtGauss5x5"), _cvtGauss5x5);
    result.Set(Napi::String::New(env, "cvtGrad"), _cvtGrad);
    result.Set(Napi::String::New(env, "cvtEdgDetection"), _cvtEdgDetection);
    result.Set(Napi::String::New(env, "cvtEdgFilter3x3"), _cvtEdgFilter3x3);
    result.Set(Napi::String::New(env, "cvtEdgSubPixel"), _cvtEdgSubPixel);
    result.Set(Napi::String::New(env, "cvtLinkEdges"), _cvtLinkEdges);
    result.Set(Napi::String::New(env, "cvtLinkContours"), _cvtLinkContours);
    result.Set(Napi::String::New(env, "cvtCurvatureContours"), _cvtCurvatureContours);
    result.Set(Napi::String::New(env, "cvtLineDetection"), _cvtLineDetection);
    result.Set(Napi::String::New(env, "nofPixels"), _nofPixels);
    result.Set(Napi::String::New(env, "nofEdges"), _nofEdges);
    result.Set(Napi::String::New(env, "nofConts"), _nofConts);
    result.Set(Napi::String::New(env, "nofLines"), _nofLines);

    return result;
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
    case 13:
        // edges
        icols = cols;
        irows = rows;
        ix0 = 0;
        for (i = 1; i < nofLevels; i++)
        {
            icols >>= 1;
            irows >>= 1;
            cvtCopyI32F2RGBA(imgLevels[i].greyI_float, irows, icols, pixels, ix0, 0, rows, cols);
            cvtEdgPts2RGBA(imgLevels[i].edgesL, pixels, rows, cols, ix0, 0);
            ix0 += icols;
        }
        break;
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
            cvtEdgPts2RGBA(imgLevels[i].edgesL, pixels, rows, cols, ix0, 0);
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
            cvtMag2floatI(imgLevels[i].gradI_float, imgLevels[i].tmpI_float, icols * irows);
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
    case 4:
        // edges on mag level
        cvtVal2RGBA(128, pixels, size);
        //cvtMag2RGBA(imgLevels[0].gradI_float, pixels, size);
        cvtEdgCurv2RGBA(imgLevels[0].edgesL, pixels, rows, cols, 0, 0);
        cvtLines2RGBA(imgLevels[0].linesL, imgLevels[0].edgesL, pixels, rows, cols, 0, 0);
        break;
    case 3:
        // edges on mag level
        cvtMag2RGBA(imgLevels[0].gradI_float, pixels, size);
        cvtEdgPts2RGBA(imgLevels[0].edgesL, pixels, rows, cols, 0, 0);
        break;
    case 2:
        // edges on grey level
        cvtGrey2RGBA(imgLevels[0].greyI_float, pixels, size);
        cvtEdgPts2RGBA(imgLevels[0].edgesL, pixels, rows, cols, 0, 0);
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
    exports.Set("getEdgeData", Napi::Function::New(env, getEdgeData));
    exports.Set("getHistEdges", Napi::Function::New(env, getHistEdges));
    exports.Set("getTimerValues", Napi::Function::New(env, getTimerValues));
    exports.Set(Napi::String::New(env, "show"), Napi::Function::New(env, show));
    return exports;
}

NODE_API_MODULE(my_addon, Init)
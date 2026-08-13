#include <napi.h>
#include <iostream>
#include <cstdint>
#include <vector>

#include "cvTools.h"

#define GMASKSIZE 5

// Persistent memory allocated in C++ heap
static std::vector<float> greyI32F;
static std::vector<float> gfilI32F;
static std::vector<float> tmpI32F;
static std::vector<float> gmask32F;
static std::vector<grad32F> gradI;
static std::vector<edgPtF> edges;
static uint32_t rows;
static uint32_t cols;
static uint32_t size;

/**
 * 
 */
Napi::Value initialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // check input 'info' structure
    if (info.Length() != 2 ) {
        Napi::TypeError::New(env, "expected: (width, height)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }
    rows = info[0].As<Napi::Number>().Uint32Value();
    cols = info[1].As<Napi::Number>().Uint32Value();
    size = rows*cols;


    // Allocate memory on the C++ heap
    greyI32F.resize((size), 0);
    gfilI32F.resize((size), 0);
    tmpI32F.resize((size), 0);
    gmask32F.resize(GMASKSIZE,0);
    gradI.resize(size);
   
    
    cvtInitGMask32F(2.0, gmask32F, GMASKSIZE);

    return Napi::String::New(env, "initialize done");
}

/**
 * 
 */
Napi::Value process(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    float magMax;
    float gradTh = 5.0;
    uint32_t nofEdges;

    // check input 'info' structure
    if (info.Length() < 4 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "expected: (image buffer, width, height, channels)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // extract single arguments from 'info' structure
    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    uint32_t height = info[1].As<Napi::Number>().Uint32Value();
    uint32_t width = info[2].As<Napi::Number>().Uint32Value();
    uint32_t channels = info[3].As<Napi::Number>().Uint32Value();
    if( width != cols || height != rows){
        Napi::TypeError::New(env, "image width/height does not fit initialized cols/rows values")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // pointer to input pixel-array (Zero-Copy!)
    uint8_t* pixels = buffer.Data();
    size_t length = buffer.Length(); 
    if( length != (size_t)(size*channels)){
        Napi::TypeError::New(env, "input buffer length does not fit initialized cols/rows values")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // 
    //start processing ...
    cvtRGBA2Grey(pixels, greyI32F, size);
    cvtGauss5x5(greyI32F, tmpI32F, gfilI32F, gmask32F, rows, cols);
    magMax = cvtGrad( gfilI32F, gradI, gradTh, rows, cols);
    nofEdges = cvtEdgDetection(gradI, edges, gradTh, rows, cols);
    
    // 
    //convert image back for display
    //cvtGrey2RGBA(gfilI32F, pixels, size);
    //cvtMag2RGBA(gradI, pixels, size);
    cvtEdg2RGBA(gradI, pixels, size);
    
    return Napi::String::New(env, "process done");
}


Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set( Napi::String::New(env, "initialize"),  Napi::Function::New(env, initialize));
    exports.Set( Napi::String::New(env, "process"),  Napi::Function::New(env, process));
    return exports;
}

NODE_API_MODULE(my_addon, Init)
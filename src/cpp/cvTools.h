#include <iostream>
#include <cstdint>
#include <vector>

struct grad32F {
    float dx;
    float dy;
    float mag;
    float dir;
    float edg;
};

struct edgPtF {
    float x;
    float y;
    float mag;
    float dir;
};

uint32_t cvtEdgDetection(std::vector<grad32F> &gradI, std::vector<edgPtF> &edges, float th, uint32_t rows, uint32_t cols);
void cvtEdg2RGBA(std::vector<grad32F> &gradI32F, uint8_t *outRGBA, uint32_t size);
void cvtGrey2RGBA(std::vector<float>& greyI32F, uint8_t* outRGBA, uint32_t size);
void cvtGauss5x5(std::vector<float>& inI32F, std::vector<float>& tmpI32F, std::vector<float>& outI32F, std::vector<float>& gmask32F, uint32_t rows, uint32_t cols);
float cvtGrad( std::vector<float> &inI32F,std::vector<grad32F> &gradI32F, float th, uint32_t rows, uint32_t cols);
void cvtInitGMask32F(float sigma, std::vector<float>& gmask32F, uint32_t size );
void cvtMag2RGBA(std::vector<grad32F> &gradI32F, uint8_t *outRGBA, uint32_t size);
void cvtRGBA2Grey(uint8_t* inRGBA, std::vector<float>& greyI32F, uint32_t size);
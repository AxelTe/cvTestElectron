#include <iostream>
#include <cstdint>
#include <vector>

/*
struct grad32F
{
    float dx;
    float dy;
    float mag;
    float dir;
    float edg;
};

struct edgPtF
{
    float x;
    float y;
    float mag;
    float dir;
};
*/
//
//
//

struct edg_float
{
    float x;
    float y;
    float mag;
    float dir;
};

struct grad_float
{
    float dx;
    float dy;
    float mag;
    float dir;
    float edg;
};

struct imgLevel
{
    std::vector<float> tmpI_float;
    std::vector<float> greyI_float;
    std::vector<float> gaussI_float;
    std::vector<grad_float> gradI_float;
    std::vector<edg_float> edgL_float;
};


uint32_t cvtEdgDetection(std::vector<grad_float> &gradI, std::vector<edg_float> &edges, float th, uint32_t rows, uint32_t cols);
void cvtEdg2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size);
void cvtGrey2RGBA(std::vector<float> &greyI32F, uint8_t *outRGBA, uint32_t size);
void cvtGauss5x5(std::vector<float> &inI32F, std::vector<float> &tmpI32F, std::vector<float> &outI32F, std::vector<float> &gmask32F, uint32_t rows, uint32_t cols);
float cvtGrad(std::vector<float> &inI32F, std::vector<grad_float> &gradI32F, float th, uint32_t rows, uint32_t cols);
void cvtInitGMask32F(float sigma, std::vector<float> &gmask32F, uint32_t size);
void cvtInitImgLevels(std::vector<imgLevel> &iLevelL, uint32_t rows, uint32_t cols, uint32_t nofLevels);
void cvtMag2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size);
void cvtRGBA2Grey(uint8_t *inRGBA, std::vector<float> &greyI32F, uint32_t size);
void cvtSubSample(std::vector<float> &inI, std::vector<float> &outI, uint32_t rows, uint32_t cols);
void cvtCopyI32F2RGBA(std::vector<float> &inI, uint32_t irows, uint32_t icols,  uint8_t *outRGBA, uint32_t ox0, uint32_t oy0, uint32_t orwos, uint32_t ocols);


#include <iostream>
#include <cstdint>
#include <vector>

struct contour
{
    uint32_t start;
    uint32_t end;
    uint32_t nofEdges;
};

struct edg_float
{
    uint32_t xi;
    uint32_t yi;
    float xf;
    float yf;
    float mag;
    float dir;
    uint32_t p;
    uint32_t s;
    uint32_t c;
};

struct grad_float
{
    float dx;
    float dy;
    float mag;
    float dir;
    uint32_t edg;
};

struct imgLevel
{
    std::vector<float> tmpI_float;
    std::vector<float> greyI_float;
    std::vector<float> gaussI_float;
    std::vector<grad_float> gradI_float;
    std::vector<edg_float> edgL_float;
    std::vector<contour> contoursL;
    std::vector<uint32_t> histEdges;
};


void cvtEdgDetection(std::vector<grad_float> &gradI, std::vector<edg_float> &edges, float thLow, float thHigh, uint32_t rows, uint32_t cols);
void cvtEdgHistogram(std::vector<edg_float> &edges, std::vector<uint32_t> &hist);
void cvtEdg2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size);
void cvtEdgPts2RGBA(std::vector<edg_float> &edges, uint8_t *outRGBA, uint32_t rows, uint32_t cols, uint32_t xoff, uint32_t yoff);
void cvtGrey2RGBA(std::vector<float> &greyI32F, uint8_t *outRGBA, uint32_t size);
void cvtGauss5x5(std::vector<float> &inI32F, std::vector<float> &tmpI32F, std::vector<float> &outI32F, std::vector<float> &gmask32F, uint32_t rows, uint32_t cols);
float cvtGrad(std::vector<float> &inI32F, std::vector<grad_float> &gradI32F, std::vector<uint32_t> &hist, float th, uint32_t rows, uint32_t cols);
void cvtInitGMask32F(float sigma, std::vector<float> &gmask32F, uint32_t size);
void cvtInitImgLevels(std::vector<imgLevel> &iLevelL, uint32_t rows, uint32_t cols, uint32_t nofLevels);
void cvtLinkEdges(std::vector<edg_float> &edges, std::vector<grad_float> &gradI, float thLow, float thHigh, uint32_t rows, uint32_t cols);
void cvtLinkContours(std::vector<edg_float> &edges, std::vector<contour> &conts);
void cvtMag2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size);
void cvtMag2floatI(std::vector<grad_float> &gradI32F, std::vector<float> &out, uint32_t size);
void cvtRGBA2Grey(uint8_t *inRGBA, std::vector<float> &greyI32F, uint32_t size);
void cvtSubSample(std::vector<float> &inI, std::vector<float> &outI, uint32_t rows, uint32_t cols);
void cvtCopyI32F2RGBA(std::vector<float> &inI, uint32_t irows, uint32_t icols,  uint8_t *outRGBA, uint32_t ox0, uint32_t oy0, uint32_t orwos, uint32_t ocols);


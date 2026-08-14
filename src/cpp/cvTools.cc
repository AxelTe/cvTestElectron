#include <iostream>
#include <cstdint>
#include <vector>
#include <math.h>

#include "cvTools.h"

/**
 * Performs edge-thinning via non-local maxima suppression
 * \param mag gradient-magnitude image, type 'cv::Mat, cv_32FC1', value range [0.0 .. 1.0]
 * \param mag gradient-direction image, type 'cv::Mat, cv_32FC1', value range [0.0 .. 360.0]
 * \param edges vector of edge points, type 'edgPoint'
 * \param th threshold, type 'float'
 *
 * \return Number of edges
 *
 * The inputs 'mag' and 'dir' are results a previous gradient calculation.
 * Note: Both images will be thinned during processing.
 */

uint32_t cvtEdgDetection(std::vector<grad_float> &gradI, std::vector<edg_float> &edges, float th, uint32_t rows, uint32_t cols)
{
    float aF, mF1, mF2;
    uint32_t cnt = 0;
    uint32_t c, c0, c1, size;

    // work
    //
    size = rows * cols;
    c0 = 2 * cols + 2;
    c1 = size - (2 * cols) - 2;
    for (c = c0; c < c1; c++)
    {
        if (gradI[c].mag > th)
        {
            aF = gradI[c].dir;
            if (aF >= 180.)
                aF = aF - 180;

            //  1 ->  PI/4
            //  2 ->  PI/2
            //  3 -> 3PI/4
            //  4 ->  PI
            if (aF >= 0. && aF < 45.)
            {
                aF /= 45;
                mF1 = gradI[c - 1].mag + aF * (gradI[c - cols - 1].mag - gradI[c - 1].mag);
                mF2 = gradI[c + 1].mag + aF * (gradI[c + cols + 1].mag - gradI[c + 1].mag);
            }
            else if (aF >= 45. && aF < 90.)
            {
                aF = (aF - 45) / 45;
                mF1 = gradI[c - cols - 1].mag + aF * (gradI[c - cols].mag - gradI[c - cols - 1].mag);
                mF2 = gradI[c + cols + 1].mag + aF * (gradI[c + cols].mag - gradI[c + cols + 1].mag);
            }
            else if (aF >= 90. && aF < 135.)
            {
                aF = (aF - 90) / 45;
                mF1 = gradI[c - cols].mag + aF * (gradI[c - cols + 1].mag - gradI[c - cols].mag);
                mF2 = gradI[c + cols].mag + aF * (gradI[c + cols - 1].mag - gradI[c + cols].mag);
            }
            else if (aF >= 135. && aF <= 180.)
            {
                aF = (aF - 135) / 45;
                mF1 = gradI[c - cols + 1].mag + aF * (gradI[c + 1].mag - gradI[c - cols + 1].mag); // m1_F = p[5] + a_F * (p[2] - p[5]);
                mF2 = gradI[c + cols - 1].mag + aF * (gradI[c - 1].mag - gradI[c + cols - 1].mag);
            }
            else
            {
                std::cout << "hallo " << c << ", " << aF << " ";
            }

            if (gradI[c].mag >= mF1 && gradI[c].mag >= mF2)
            {
                gradI[c].edg = 1.;
                cnt++;
            }
        }
    }

    //
    // edge list
    edges.resize(cnt);
    cnt = 0;
    for (c = c0; c < c1; c++)
    {
        if (gradI[c].edg > 0)
        {
            if (cnt < edges.size())
            {
                edges[cnt].x = c % cols;
                edges[cnt].y = c / cols;
                edges[cnt].mag = gradI[c].mag;
                edges[cnt].dir = gradI[c].dir;
                cnt++;
            }
        }
    }

    return (cnt);
}

/**
 *
 */
void cvtEdg2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size)
{
    uint32_t channels = 4;
    for (uint32_t i = 0, j = 0; i < size; i++)
    {
        if (gradI32F[i].edg > 0)
        {
            outRGBA[j] = 255;
            outRGBA[j + 1] = 0;
            outRGBA[j + 2] = 0;
        }
        j = j + channels;
    }
}

/**
 *
 */
void cvtGrey2RGBA(std::vector<float> &greyI32F, uint8_t *outRGBA, uint32_t size)
{
    float g;
    uint32_t channels = 4;
    for (uint32_t i = 0, j = 0; i < size; i++)
    {
        g = greyI32F[i];
        outRGBA[j] = (uint8_t)g;
        outRGBA[j + 1] = (uint8_t)g;
        outRGBA[j + 2] = (uint8_t)g;
        j = j + channels;
    }
}

/**
 *
 */
void cvtGauss5x5(
    std::vector<float> &inI32F,
    std::vector<float> &tmpI32F,
    std::vector<float> &outI32F,
    std::vector<float> &gmask32F,
    uint32_t rows,
    uint32_t cols)
{
    uint32_t c, size, c0, c1;
    float val;

    size = rows * cols;
    // filter in x-direction
    c0 = 2;
    c1 = size - 2;
    for (c = c0; c < c1; c++)
    {
        val = inI32F[c - 2] * gmask32F[0];
        val += inI32F[c - 1] * gmask32F[1];
        val += inI32F[c] * gmask32F[2];
        val += inI32F[c + 1] * gmask32F[3];
        val += inI32F[c + 2] * gmask32F[4];
        tmpI32F[c] = val;
    }

    // filter in y-direction
    c0 = 2 * cols;
    c1 = size - (2 * cols);
    for (c = c0; c < c1; c++)
    {
        val = tmpI32F[c - (2 * cols)] * gmask32F[0];
        val += tmpI32F[c - cols] * gmask32F[1];
        val += tmpI32F[c] * gmask32F[2];
        val += tmpI32F[c + cols] * gmask32F[3];
        val += tmpI32F[c + (2 * cols)] * gmask32F[4];
        outI32F[c] = val;
    }
}

/**
 *
 * mag [0.0 .. 1.0]
 * dir [0.0 .. 360.0]
 */
float cvtGrad(
    std::vector<float> &inI32F,
    std::vector<grad_float> &gradI32F,
    float th,
    uint32_t rows,
    uint32_t cols)
{
    float dx, dy;
    float mv, dv;
    float pi = atan(1.0) * 4;
    float mmax = 0;
    uint32_t c0, c1, c, size = rows * cols;

    std::fill(gradI32F.begin(), gradI32F.end(), grad32F{0.0f, 0.0f, 0.0f, 0.0f, 0.0f});
    // work
    c0 = cols + 1;
    c1 = size - cols - 2;
    for (c = c0; c < c1; c++)
    {
        dx = 0.5f * (inI32F[c + 1] - inI32F[c - 1]);
        dy = 0.5f * (inI32F[c + cols] - inI32F[c - cols]);
        mv = sqrt(dx * dx + dy * dy);
        if (mv > th)
        {
            dv = 180. * atan2f(dy, dx) / pi;
            dv += 180;
            gradI32F[c] = {dx, dy, mv, dv, 0.};
            mmax = (mmax < mv) ? mv : mmax;
        }
    }
    return (mmax);
}

/**
 *
 */
void cvtInitGMask32F(float sigma, std::vector<float> &gmask32F, uint32_t size)
{
    uint32_t i, radius = size / 2; // radius = 2 for size 5
    float sum = 0.0;
    float x;

    // 1. Calculate unnormalized Gaussian values
    for (i = 0; i < size; i++)
    {
        x = (float)(i - radius); // Maps indices {0, 1, 2, 3, 4} to x = {-2, -1, 0, 1, 2}
        // 1D Gaussian formula: g(x) = exp(-x^2 / (2 * sigma^2))
        gmask32F[i] = expf(-(x * x) / (2.0 * sigma * sigma));
        sum += gmask32F[i];
    }
    // 2. Normalize the mask so the sum of elements equals 1.0
    for (i = 0; i < size; i++)
    {
        gmask32F[i] /= sum;
    }
}

/**
 *
 */
void cvtMag2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size)
{
    float g;
    uint32_t channels = 4;
    for (uint32_t i = 0, j = 0; i < size; i++)
    {
        g = gradI32F[i].mag;
        outRGBA[j] = (uint8_t)g;
        outRGBA[j + 1] = (uint8_t)g;
        outRGBA[j + 2] = (uint8_t)g;
        j = j + channels;
    }
}

/**
 *
 */
void cvtRGBA2Grey(uint8_t *inRGBA, std::vector<float> &greyI32F, uint32_t size)
{
    // input image has got an RGBA structure
    // pixels[0] = Red, pixels[1] = Green, pixels[2] = Blue, pixels[3] = Alpha
    float r, g, b;
    uint32_t channels = 4;
    for (uint32_t i = 0, j = 0; i < size; i++)
    {
        r = (float)inRGBA[j];
        g = (float)inRGBA[j + 1];
        b = (float)inRGBA[j + 2];
        j = j + channels;
        greyI32F[i] = 0.2989 * r + 0.5870 * g + 0.1140 * b;
    }
}

/**
 *
 */
void cvtInitImgLevels(std::vector<imgLevel> &iLevelL, uint32_t rows, uint32_t cols, uint32_t nofLevels)
{
    uint32_t i, size = rows * cols * 2;
    //
    for(i=0; i<nofLevels; i++){
        size = rows * (cols>>i);
        iLevelL[i].tmpI_float.resize(size, 0);
        iLevelL[i].greyI_float.resize(size, 0);
        iLevelL[i].gaussI_float.resize(size, 0);
        iLevelL[i].gradI_float.resize(size);
        iLevelL[i].edgL_float.resize(size);
    }

}

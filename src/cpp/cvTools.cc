#include <iostream>
#include <chrono>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <math.h>
#include <numbers>
#include <unordered_map>
#include <cmath>
#include <cstdint>

#include "cvTools.h"

static const int8_t _lx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
static const int8_t _ly[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int8_t _d180[8] = {4, 5, 6, 7, 0, 1, 2, 3};
static const int8_t _dlink[8][4] =
    {
        {6, 0, 7, 5},
        {0, 6, 7, 1},
        {0, 2, 1, 7},
        {2, 0, 1, 3},
        {2, 4, 3, 1},
        {4, 2, 3, 5},
        {4, 6, 5, 3},
        {6, 4, 5, 7}};
static const int8_t _px[8] = {0, -1, -1, -1, 0, 1, 1, 1};
static const int8_t _py[8] = {1, 1, 0, -1, -1, -1, 0, 1};
static const int8_t _sx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int8_t _sy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

static const int8_t _n3x3[256] = {
    0, 1, 1, 1, 1, 2, 1, 0,
    1, 2, 2, 2, 1, 2, 0, 0,
    1, 2, 2, 2, 2, 3, 2, 0,
    1, 2, 2, 2, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 3, 2, 0,
    2, 3, 3, 3, 2, 3, 0, 0,
    1, 2, 2, 2, 2, 3, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 3, 2, 0,
    2, 3, 3, 3, 2, 3, 0, 0,
    2, 3, 3, 3, 3, 4, 3, 0,
    2, 3, 3, 3, 0, 0, 0, 0,
    1, 2, 2, 2, 2, 3, 2, 0,
    2, 3, 3, 3, 2, 3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 2, 0, 2, 2, 2, 0,
    2, 2, 3, 0, 2, 2, 0, 0,
    2, 2, 3, 0, 3, 3, 3, 0,
    2, 2, 3, 0, 0, 0, 0, 0,
    2, 2, 3, 0, 3, 3, 3, 0,
    3, 3, 4, 0, 3, 3, 0, 0,
    2, 2, 3, 0, 3, 3, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 2, 0, 2, 0, 2, 0,
    2, 0, 3, 0, 2, 0, 0, 0,
    2, 0, 3, 0, 3, 0, 3, 0,
    2, 0, 3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

static float _sqrt2 = sqrtf(2.0);
static float degree2rad = ((float)std::numbers::pi) / 180.;
static float rad2degree = 1. / degree2rad;

static std::vector<float> gmaskCurv;

static processTime _ptime;
std::chrono::time_point<std::chrono::system_clock> _start;

/**
 *
 */
void cvtConts2RGBA(
    std::vector<contour> &conts,
    std::vector<edge> &edges,
    uint8_t *outRGBA,
    float curvTh,
    uint32_t rows,
    uint32_t cols,
    uint32_t xoff,
    uint32_t yoff)
{
    uint32_t channels = 4;
    uint32_t nofConts = conts.size();
    uint32_t xi, yi, p;
    uint16_t val;
    contour cont;
    edge edg;

    for (uint32_t i = 0; i < nofConts; i++)
    {
        cont = conts[i];
        edg = edges[cont.start - 1];
        for (auto i = cont.nofEdges; i > 0; --i)
        {
            xi = edg.xi + xoff;
            yi = edg.yi + yoff;
            p = channels * (yi * cols + xi);
            val = ((uint16_t)edg.mag) + 128;
            val = val > 255 ? 255 : val;
            if( edg.curv >= curvTh){
                outRGBA[p] = (uint8_t)val;
                outRGBA[p + 1] = 0;
                outRGBA[p + 2] = 0;
            } 
            else {
                outRGBA[p] = (uint8_t)val;
                outRGBA[p + 1] = (uint8_t)val;
                outRGBA[p + 2] = (uint8_t)val;
            }
            if (edg.s > 0)
                edg = edges[edg.s - 1];

        }
        if (cont.start == cont.end)
        {
            // closed contours
            xi = edges[cont.start - 1].xi + xoff;
            yi = edges[cont.start - 1].yi + yoff;
            if (xi >= 0 && xi < cols && yi >= 0 && yi < rows)
            {
                p = channels * (yi * cols + xi);
                outRGBA[p] = 0x00;
                outRGBA[p + 1] = 0xff;
                outRGBA[p + 2] = 0xff;
            }
        }
        else
        {
            // open contours
            // start points
            xi = edges[cont.start - 1].xi + xoff;
            yi = edges[cont.start - 1].yi + yoff;
            if (xi >= 0 && xi < cols && yi >= 0 && yi < rows)
            {
                p = channels * (yi * cols + xi);
                outRGBA[p] = 0x00;
                outRGBA[p + 1] = 0xff;
                outRGBA[p + 2] = 0x00;
            }
            // end points
            xi = edges[cont.end - 1].xi + xoff;
            yi = edges[cont.end - 1].yi + yoff;
            if (xi >= 0 && xi < cols && yi >= 0 && yi < rows)
            {
                p = channels * (yi * cols + xi);
                outRGBA[p] = 0x00;
                outRGBA[p + 1] = 0x00;
                outRGBA[p + 2] = 0xff;
            }
        }
    }
}

#if 0

void cvtCurvatureContours(
    std::vector<edge> &edges,
    std::vector<contour> &conts,
    uint32_t curvBd)
{
    int numConts = conts.size();
    contour cont;

    cvtTimerStart();

    for (int cIdx = 0; cIdx < numConts; ++cIdx)
    {
        cont = conts[cIdx];
        int nofEdges = cont.nofEdges;
        edge *edgS, *edgP, *edg = &edges[cont.start-1];
        for(auto i=0; i<nofEdges; ++i){
            // forward
            edgS = edg;
            int x0 = edgS->xi;
            int y0 = edgS->yi;
            int dx, dy;
            float arcL = 0.0f;
            for(auto k=0; k<curvBd; ++k){
                if( edgS->s > 0 ){
                    edgS = &edges[edgS->s -1];
                    dx = abs((int) (edgS->xi - x0));
                    dy = abs((int) (edgS->yi - y0));
                    arcL = (dy>0 && dx>0) ? (arcL+1.414214) : (arcL+1.0f);
                    x0 = edgS->xi;
                    y0 = edgS->yi;
                }
            }
            //backward
            edgP = edg;
            x0 = edgP->xi;
            y0 = edgP->yi;
            for(auto k=0; k<curvBd; ++k){
                if( edgP->p > 0 ){
                    edgP = &edges[edgP->p -1];
                    dx = abs((int) (edgP->xi - x0));
                    dy = abs((int) (edgP->yi - y0));
                    arcL = (dy>0 && dx>0) ? (arcL+1.414214) : (arcL+1.0f);
                    x0 = edgP->xi;
                    y0 = edgP->yi;
                }
            }
            if(arcL > 0){
                edg->curv = fabs(edgS->dir - edgP->dir);
                edg->curv = std::min(edg->curv, 360.0f-edg->curv);
                edg->curv /= arcL;
            }
            if(edg->s > 0) edg = &edges[edg->s - 1];
        }
    }
    _ptime.cvtCurvatureContours = cvtTimerEnd();
}

#else

void cvtCurvatureContours(
    std::vector<edge> &edges,
    std::vector<contour> &conts,
    uint32_t curvBd)
{
    int numConts = conts.size();
    contour cont;

    cvtTimerStart();

    for (int cIdx = 0; cIdx < numConts; ++cIdx)
    {
        cont = conts[cIdx];
        int nofEdges = cont.nofEdges;
        edge *edgS, *edgP, *edg = &edges[cont.start-1];
        for(auto i=0; i<nofEdges; ++i){
            // forward
            edgS = edg;
            float x0 = edgS->xf;
            float y0 = edgS->yf;
            float dx, dy;
            float arcL = 0.0f;
            for(auto k=0; k<curvBd; ++k){
                if( edgS->s > 0 ){
                    edgS = &edges[edgS->s -1];
                    dx = edgS->xf - x0;
                    dy = edgS->yf - y0;
                    arcL += sqrt( dx*dx + dy*dy );
                    x0 = edgS->xf;
                    y0 = edgS->yf;
                }
            }
            //backward
            edgP = edg;
            x0 = edgP->xf;
            y0 = edgP->yf;
            for(auto k=0; k<curvBd; ++k){
                if( edgP->p > 0 ){
                    edgP = &edges[edgP->p -1];
                    dx = edgP->xf - x0;
                    dy = edgP->yf - y0;
                    arcL += sqrt( dx*dx + dy*dy );
                    x0 = edgP->xf;
                    y0 = edgP->yf;
                }
            }
            if(arcL > 0){
                edg->curv = fabs(edgS->dir - edgP->dir);
                edg->curv = std::min(edg->curv, 360.0f-edg->curv);
                edg->curv /= arcL;
            }
            if(edg->s > 0) edg = &edges[edg->s - 1];
        }
    }
    _ptime.cvtCurvatureContours = cvtTimerEnd();
}
#endif

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
void cvtEdgDetection(
    std::vector<grad_float> &gradI,
    std::vector<edge> &edges,
    float thLow,
    uint32_t rows,
    uint32_t cols)
{
    cvtTimerStart();

    grad_float *__restrict grad = gradI.data();
    const int iCols = static_cast<int>(cols);
    constexpr float INV_45 = 1.0f / 45.0f;

    // Pre-allocate typical edge capacity (~5-10% of image) to avoid reallocations
    edges.clear();
    edges.reserve((rows * cols) / 10);

    // Neighbor offset lookup tables for the 4 gradient angle sectors (0..3)
    // Sector 0: [0, 45), Sector 1: [45, 90), Sector 2: [90, 135), Sector 3: [135, 180]
    const int off1A[4] = {-1, -iCols - 1, -iCols, -iCols + 1};
    const int off1B[4] = {-iCols - 1, -iCols, -iCols + 1, 1};
    const int off2A[4] = {1, iCols + 1, iCols, iCols - 1};
    const int off2B[4] = {iCols + 1, iCols, iCols - 1, -1};

    uint32_t cnt = 0;

    // Explicit (y, x) loops eliminate `c % cols` and `c / cols` divisions
    for (uint32_t y = 2; y < rows - 2; ++y)
    {
        uint32_t c = y * cols + 2;
        for (uint32_t x = 2; x < cols - 2; ++x, ++c)
        {
            const float mF = grad[c].mag;
            if (mF <= thLow)
                continue;

            float dF = grad[c].dir;
            float aF = (dF >= 180.0f) ? (dF - 180.0f) : dF;

            // Map [0, 180] directly to sector index 0..3
            int sector = static_cast<int>(aF * INV_45);
            if (sector > 3)
                sector = 3; // Clamp edge case where aF == 180.0f

            // Fractional interpolation weight within the 45-degree sector
            float frac = (aF - (sector * 45.0f)) * INV_45;

            const float m1a = grad[c + off1A[sector]].mag;
            const float m1b = grad[c + off1B[sector]].mag;
            const float mF1 = m1a + frac * (m1b - m1a);

            const float m2a = grad[c + off2A[sector]].mag;
            const float m2b = grad[c + off2B[sector]].mag;
            const float mF2 = m2a + frac * (m2b - m2a);

            if (mF >= mF1 && mF >= mF2)
            {
                ++cnt;
                grad[c].edg = cnt;

                // Construct edge directly in-place (eliminates 2nd pass + std::fill)
                edges.emplace_back(edge{x, y, 0.0f, 0.0f, mF, dF, -1.0f, 0, 0, 0, 0});
            }
        }
    }

    _ptime.cvtEdgDetection = cvtTimerEnd();
}

/**
 *
 */
void cvtEdgFilter3x3(
    std::vector<grad_float> &gradI,
    std::vector<edge> &edges,
    uint32_t rows,
    uint32_t cols)

{
    uint32_t j, pc, c, cnt, val;
    std::vector<edge> tedges;
    uint32_t __ly[8];

    cvtTimerStart();

    for (j = 0; j < 8; j++)
        __ly[j] = cols * ((uint32_t)_ly[j]);

    tedges.resize(edges.size());
    cnt = 0;
    //
    // n3x3
    for (const auto &edg : edges)
    {
        val = 0;
        pc = (edg.yi * cols) + edg.xi;
        for (j = 0; j < 8; j++)
        {
            c = pc + __ly[j] + _lx[j];
            if (gradI[c].edg > 0)
                val += (1 << j);
        }
        if (_n3x3[val] == 0)
        {
            gradI[pc].edg = 0;
        }
        else
        {
            // edg.n3x3 = (uint32_t)_n3x3[val];
            tedges[cnt] = edg;
            cnt++;
            gradI[pc].edg = cnt;
        }
    }
    tedges.resize(cnt);
    edges = std::move(tedges);

    _ptime.cvtEdgFilter3x3 = cvtTimerEnd();
}

/**
 *
 */
void cvtEdgSubPixel(
    std::vector<grad_float> &gradI,
    std::vector<edge> &edges,
    uint32_t rows,
    uint32_t cols)

{
    int u, v, j, c;
    float xf, yf, mF;

    cvtTimerStart();

    for (auto &edg : edges)
    {
        mF = edg.mag;
        u = edg.xi;
        v = edg.yi;
        xf = mF * ((float)u);
        yf = mF * ((float)v);
        for (j = 0; j < 8; j++)
        {
            c = (v + _ly[j]) * cols + (u + _lx[j]);
            if (gradI[c].edg > 0)
            {
                mF += gradI[c].mag;
                xf += gradI[c].mag * ((float)u + _lx[j]);
                yf += gradI[c].mag * ((float)v + _ly[j]);
            }
        }
        edg.xf = xf / mF;
        edg.yf = yf / mF;
    }
    _ptime.cvtEdgSubPixel = cvtTimerEnd();
}

/**
 *
 */
void cvtLineDetection(
    std::vector<edge> &edges,
    std::vector<contour> &contours,
    std::vector<line> &lines,
    uint32_t curvBd,
    float curvTh,
    float lineTh
)
{
   
    cvtTimerStart();

    //
    // loop over contours
    uint32_t nofConts = contours.size();
    uint32_t nofEdges = edges.size();
    lines.clear();
    lines.reserve(nofEdges);
    for (auto i=0; i<(nofConts); i++) 
    {
        contour *cont = &contours[i];
        if (cont->nofEdges > 2)
        {
            uint32_t edgCnt = 0;
            uint32_t edgId = cont->start;
            uint32_t tstart, tend;
            
            for (auto j = 0; j < cont->nofEdges; j++)
            {
                edge edg = edges[edgId-1];
                if (edg.curv >= 0 && edg.curv < curvTh)
                {
                    if (edgCnt == 0)
                    {
                        tstart = edgId;
                    }
                    tend = edgId;
                    edgCnt++;
                }
                else
                {
                    if (edgCnt > curvBd)
                    {
                        lines.emplace_back(line{tstart, tend, (uint32_t) (i+1), edgCnt});
                    }
                    edgCnt = 0;
                }
                if (edg.s > 0) edgId = edg.s;
                else break;
            }
            if (edgCnt > curvBd && tstart != tend)
            {
                lines.emplace_back(line{tstart, tend, (uint32_t) (i+1), edgCnt});
            }
        }
    }

    // prove straight lines
#if 1
    // Precompute squared threshold to avoid sqrt() inside the loop
    const int nofLines = lines.size();
    std::vector<line> tlines;
    tlines.reserve( nofLines );
    int cnt = 0;
    float cross;
        
    for (auto l=0; l<nofLines; ++l)
    {
        line *lineP = &lines[l];
        edge* edgS = &edges[lineP->start - 1];
        edge* edgE = &edges[lineP->end - 1];

        const float dx = edgE->xf - edgS->xf;
        const float dy = edgE->yf - edgS->yf;
        const float len_sq = dx * dx + dy * dy;

        // Guard against closed loops or degenerate zero-length segments
        if (len_sq < 0.001f) {
            continue;
        }

        const float threshold_sq = (lineTh * lineTh) * len_sq;
        const int nofEdges = lineP->nofEdges;
        edge *edg = edgS;
        bool valid = true;
        for (auto i = 1; i < nofEdges; ++i) {
            if(edg->s>0){
                edg = &edges[edg->s-1];
                const float vx = edg->xf - edgS->xf;
                const float vy = edg->yf - edgS->yf;

                // 1. Perpendicular distance check (2D cross product squared)
                cross = vx * dy - vy * dx;
                if ((cross * cross) > threshold_sq) {
                    valid = false;
                    continue;
                }
            }
        }
        if(valid){
            tlines.emplace_back(*lineP);
            cnt++;
        }
    }
    lines = tlines;
#endif

#if 0
    // expand
    for (lineP = lines.begin(); lineP != lines.end(); lineP++)
    {
        tstart = edges.begin() + (lineP->start - 1);
        tend = edges.begin() + (lineP->end - 1);
        dx = tend->xf - tstart->xf;
        dy = tend->yf - tstart->yf;
        norm = sqrtf(powf(dx, 2) + powf(dy, 2));
        if (norm > 0.0001)
        {
            // start
            ept = tstart;
            edgCnt = 0;
            while (ept->p > 0)
            {
                ept = edges.begin() + (ept->p - 1);
                dis = fabsf((dx * (tstart->yf - ept->yf)) - (dy * (tstart->xf - ept->xf))) / norm;
                if (dis < 0.4)
                {
                    tstart = ept;
                    edgCnt++;
                }
                else
                {
                    break;
                }
            }
            // end
            ept = tend;
            while (ept->s > 0)
            {
                ept = edges.begin() + (ept->s - 1);
                dis = fabsf((dx * (tstart->yf - ept->yf)) - (dy * (tstart->xf - ept->xf))) / norm;
                if (dis < 0.4)
                {
                    tend = ept;
                    edgCnt++;
                }
                else
                {
                    break;
                }
            }
        }
        lineP->start = std::distance(edges.begin(), tstart) + 1;
        lineP->end = std::distance(edges.begin(), tend) + 1;
        lineP->nofEdges += edgCnt;
    }
#endif

    _ptime.cvtLineDetection = cvtTimerEnd();
}

/**
 *
 */
void cvtLinkEdges(
    std::vector<edge> &edges,
    std::vector<grad_float> &gradI,
    float th,
    uint32_t rows,
    uint32_t cols)
{

    uint32_t i, j, nofEdges = edges.size();
    float a1;
    uint32_t x, y, xp, yp, c, pid;
    uint32_t di, dj, predecessor;

    cvtTimerStart();

    for (i = 0; i < nofEdges; i++)
    {
        // look for a start point
        // if (edges[i].mag >= thHigh && edges[i].s == 0)
        if (edges[i].s == 0)
        {
            a1 = edges[i].dir; // [0 ... 360]
            di = (uint32_t)floorf(a1 / 45.0f);
            di = di % 8;
            x = edges[i].xi;
            y = edges[i].yi;
            predecessor = i + 1;

            // look for neighbors
            for (j = 0; j < 4; j++)
            {
                dj = (uint32_t)_dlink[di][j];
                xp = x + ((uint32_t)_lx[dj]);
                yp = y + ((uint32_t)_ly[dj]);
                c = yp * cols + xp;
                if (gradI[c].edg > 0 && gradI[c].mag >= th)
                {
                    pid = gradI[c].edg - 1;
                    if (edges[pid].p == 0 && (edges[pid].s - 1) != i)
                    {
                        edges[predecessor - 1].s = (pid + 1);
                        edges[pid].p = predecessor;
                        break;
                    }
                }
            }
        }
    }

    _ptime.cvtLinkEdges = cvtTimerEnd();
}

/**
 *
 */
void cvtLinkContours(
    std::vector<edge> &edges,
    std::vector<contour> &conts,
    float th)
{

    uint32_t i, i0, i1, nofEdges = edges.size();
    uint32_t cnt = 0, tcnt = 0;
    float mag;

    cvtTimerStart();

    conts.resize(nofEdges);
    //std::fill(conts.begin(), conts.end(), contour{0, 0, 0});

    for (i = 0; i < nofEdges; i++)
    {
        if (edges[i].c == 0)
        {
            i0 = i;
            // look for a start point
            while (edges[i0].p > 0)
            {
                i0 = edges[i0].p - 1;
                if (i0 == i)
                    break;
            }

            conts[cnt].start = (i0 + 1);
            edges[i0].c = (cnt + 1);
            i1 = i0;
            conts[cnt].end = (i1 + 1);
            conts[cnt].nofEdges = 1;
            mag = edges[i0].mag;
            while (edges[i1].s > 0)
            {
                i1 = edges[i1].s - 1;
                edges[i1].c = (cnt + 1);
                conts[cnt].end = (i1 + 1);
                conts[cnt].nofEdges++;
                mag += edges[i1].mag;
                if (i1 == i0)
                    break;
            }
            // contours should have at least three points ...
            if (conts[cnt].nofEdges > 2)
            {
                // ... and the mean gradient magnitude should be greater
                // than the gradient threshold
                mag /= (float)(conts[cnt].nofEdges);
                if (mag > th)
                    cnt++;
                else
                    tcnt++;
            }
        }
    }
    conts.resize(cnt);
    _ptime.cvtLinkContours = cvtTimerEnd();
}

void cvtEdgHistogram(
    std::vector<edge> &edges,
    std::vector<uint32_t> &hist)
{
    uint32_t nofEdges = edges.size();
    float val;

    for (uint32_t i = 0; i < 256; i++)
        hist[i] = 0;

    for (uint32_t i = 0; i < nofEdges; i++)
    {
        val = edges[i].mag;
        val = val > 255. ? 255. : val;
        hist[(uint8_t)val]++;
    }
}

/**
 *
 */
void cvtEdgLinkHistogram(
    std::vector<grad_float> &gradI,
    std::vector<edge> &edges,
    std::vector<uint32_t> &hist,
    uint32_t rows,
    uint32_t cols)
{
    uint32_t nofEdges = edges.size();
    uint32_t xi, yi, i, xj, yj, j;
    uint8_t val;

    // reset histogram
    for (uint32_t i = 0; i < 256; i++)
        hist[i] = 0;

    // loop over edge points
    for (i = 0; i < nofEdges; i++)
    {
        xi = edges[i].xi;
        yi = edges[i].yi;
        val = 0;
        for (j = 0; j < 8; j++)
        {
            xj = xi + _lx[j];
            yj = yi + _ly[j];
            if (gradI[(yj * cols) + xj].edg > 0)
                val += (1 << j);
        }
        edges[i].n3x3 = val;
        hist[val]++;
    }
}

/**
 *
 */
void cvtEdgCurv2RGBA(
    std::vector<edge> &edges,
    uint8_t *outRGBA,
    uint32_t rows,
    uint32_t cols,
    uint32_t xoff,
    uint32_t yoff)
{
    float cur;
    uint8_t val, r, g, b;
    uint32_t channels = 4;
    uint32_t nofEdgs = edges.size();
    uint32_t xi, yi, p;
    for (uint32_t i = 0; i < nofEdgs; i++)
    {
        xi = edges[i].xi;
        yi = edges[i].yi;
        xi = xi + xoff;
        yi = yi + yoff;
        if (xi >= 0 && xi < cols && yi >= 0 && yi < rows)
        {
            //
            cur = edges[i].curv;
            if (cur < 0.00001f)
            {
                r = g = b = 0;
            }
            else
            {
                val = (uint8_t)(cur * 50.0f);
                val = val > 255 ? 255 : val;
                if (val >= 128)
                {
                    r = 255;
                    g = 255 - (2 * (val - 128));
                    b = 0x00;
                }
                else
                {
                    r = 2 * val;
                    g = 255;
                    b = 0x00;
                }
            }

            p = yi * (channels * cols);
            p = p + (channels * xi);
            outRGBA[p] = r;
            outRGBA[p + 1] = g;
            outRGBA[p + 2] = b;
        }
    }
}

/**
 *
 */
void cvtEdgPts2RGBA(
    std::vector<edge> &edges,
    uint8_t *outRGBA,
    uint8_t mode,
    uint32_t rows,
    uint32_t cols,
    uint32_t xoff,
    uint32_t yoff)
{
    float dir, h, f;
    int sector;
    uint8_t q, t, r, g, b;
    uint32_t channels = 4;
    uint32_t nofEdgs = edges.size();
    uint32_t xi, yi, p;
    for (uint32_t i = 0; i < nofEdgs; i++)
    {
        xi = edges[i].xi;
        yi = edges[i].yi;
        xi = xi + xoff;
        yi = yi + yoff;
        if (xi >= 0 && xi < cols && yi >= 0 && yi < rows)
        {
            switch (mode)
            {
            case 1:
                // Normalize angle to [0.0, 360.0) range
                dir = fmodf(edges[i].dir, 360.0f);
                // Scale angle to 0.0 - 6.0 range (representing 6 color segments)
                h = dir / 60.0f;
                sector = (int)h;
                f = h - (float)sector; // Fractional part

                q = (uint8_t)((1.0f - f) * 255.0f);
                t = (uint8_t)(f * 255.0f);

                switch (sector)
                {
                case 0: // Red to Yellow (0° - 60°)
                    r = 255;
                    g = t;
                    b = 0;
                    break;
                case 1: // Yellow to Green (60° - 120°)
                    r = q;
                    g = 255;
                    b = 0;
                    break;
                case 2: // Green to Cyan (120° - 180°)
                    r = 0;
                    g = 255;
                    b = t;
                    break;
                case 3: // Cyan to Blue (180° - 240°)
                    r = 0;
                    g = q;
                    b = 255;
                    break;
                case 4: // Blue to Magenta (240° - 300°)
                    r = t;
                    g = 0;
                    b = 255;
                    break;
                default: // Magenta to Red (300° - 360°)
                    r = 255;
                    g = 0;
                    b = q;
                    break;
                }
                p = yi * (channels * cols);
                p = p + (channels * xi);
                outRGBA[p] = r;
                outRGBA[p + 1] = g;
                outRGBA[p + 2] = b;
                break;
            default:
                p = channels * (yi * cols + xi);
                outRGBA[p] = 0;
                outRGBA[p + 1] = 0;
                outRGBA[p + 2] = 0;
                break;
            }
        }
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
    uint32_t cols2 = 2 * cols;
    float val;

    cvtTimerStart();

    // Inform compiler pointers do not overlap (alias)
    const float *__restrict in = inI32F.data();
    float *__restrict tmp = tmpI32F.data();
    float *__restrict out = outI32F.data();

    const float g0 = gmask32F[0];
    const float g1 = gmask32F[1];
    const float g2 = gmask32F[2];
    const float g3 = gmask32F[3];
    const float g4 = gmask32F[4];

    size = rows * cols;
    // filter in x-direction
    c0 = 2;
    c1 = size - 2;
#pragma omp simd
    for (c = c0; c < c1; c++)
    {
        tmp[c] = in[c - 2] * g0 +
                 in[c - 1] * g1 +
                 in[c] * g2 +
                 in[c + 1] * g3 +
                 in[c + 2] * g4;
    }

    // filter in y-direction
    c0 = 2 * cols;
    c1 = size - (2 * cols);
#pragma omp simd
    for (c = c0; c < c1; c++)
    {
        out[c] = tmp[c - cols2] * g0 +
                 tmp[c - cols] * g1 +
                 tmp[c] * g2 +
                 tmp[c + cols] * g3 +
                 tmp[c + cols2] * g4;
    }

    _ptime.cvtGauss5x5 = cvtTimerEnd();
}

/**
 *
 * mag [0.0 .. 1.0]
 * dir [0.0 .. 360.0]
 */
void cvtGrad(
    std::vector<float> &inI32F,
    std::vector<grad_float> &gradI32F,
    uint32_t rows,
    uint32_t cols)
{
    float RAD_TO_DEG = 57.295779513f; // 180. / ((float)(atan(1.0) * 4.));;
    int x, y, yc;
    int colsm3 = cols - 3;
    int rowsm3 = rows - 3;

    cvtTimerStart();

    // work
    yc = 2 * cols;
    for (y = 3; y < rowsm3; ++y)
    {
        yc += cols;
        const float *__restrict ind = inI32F.data() + yc - cols;
        const float *__restrict inl = inI32F.data() + yc - 1;
        const float *__restrict inr = inI32F.data() + yc + 1;
        const float *__restrict inu = inI32F.data() + yc + cols;
        grad_float *__restrict out = gradI32F.data() + yc;

#pragma omp simd
        for (x = 3; x < colsm3; x++)
        {
            float dx = 0.5f * (inr[x] - inl[x]);
            float dy = 0.5f * (inu[x] - ind[x]);
            float mv = dx * dx + dy * dy;
            float dv = 0;
            if (mv > 4)
            {
                mv = std::sqrt(mv);
                // atan2f between [-180, 180]
                dv = (std::atan2(dy, dx) * RAD_TO_DEG) + 180.0f;
                // grad dir values [0, 360]
            }
            out[x] = {mv, dv, 0};
        }
    }
    _ptime.cvtGrad = cvtTimerEnd();
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
void cvtLines2RGBA(
    std::vector<line> &lines,
    std::vector<edge> &edges,
    uint8_t *outRGBA,
    uint32_t rows,
    uint32_t cols,
    uint32_t xoff,
    uint32_t yoff)
{
    uint8_t val, r, g, b;
    uint32_t channels = 4;
    uint32_t nofLines = lines.size();
    uint32_t c, nofEdges = 0;
    uint32_t xi, yi, p;
    edge edgS, edgE;

    for (auto i = 0; i < nofLines; i++)
    {
        nofEdges = lines[i].nofEdges;
        if(nofEdges == 0) continue;
        edgS = edges[lines[i].start - 1];
        c = 0;
        for (c = 0; c < nofEdges; c++)
        {
            xi = edgS.xi + xoff;
            yi = edgS.yi + yoff;
            if (xi >= 0 && xi < cols && yi >= 0 && yi < rows)
            {
                p = yi * (channels * cols) + (channels * xi);
                outRGBA[p] = 0xFF;
                outRGBA[p + 1] = 0x00;
                outRGBA[p + 2] = 0xFF;
            }
            if (edgS.s > 0)
                edgS = edges[edgS.s - 1];
        }
    }
}

/**
 *
 */
void cvtMag2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size)
{
    float g;
    uint8_t gi;
    uint32_t channels = 4;
    for (uint32_t i = 0, j = 0; i < size; i++)
    {
        g = 2 * gradI32F[i].mag;
        gi = (uint8_t)(g > 255.0 ? 255 : g);
        outRGBA[j] = (uint8_t)g;
        outRGBA[j + 1] = (uint8_t)g;
        outRGBA[j + 2] = (uint8_t)g;
        j = j + channels;
    }
}

/**
 *
 */
void cvtMag2floatI(std::vector<grad_float> &gradI32F, std::vector<float> &out, uint32_t size)
{
    float g;
    for (uint32_t i = 0; i < size; i++)
    {
        g = gradI32F[i].mag;
        out[i] = g;
    }
}

/**
 *
 */
void cvtRGBA2Grey(uint8_t *inRGBA, std::vector<float> &greyI32F, uint32_t size)
{
    cvtTimerStart();

    const uint8_t *__restrict in = inRGBA;
    float *__restrict out = greyI32F.data();

    // Standard ITU-R BT.601 luminance coefficients
    constexpr float Wr = 0.2989f;
    constexpr float Wg = 0.5870f;
    constexpr float Wb = 0.1140f;

#pragma omp parallel for schedule(static)
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t idx = i * 4;
        out[i] = static_cast<float>(in[idx]) * Wr +
                 static_cast<float>(in[idx + 1]) * Wg +
                 static_cast<float>(in[idx + 2]) * Wb;
    }

    _ptime.cvtRGBA2Grey = cvtTimerEnd();
}

void cvtVal2RGBA(uint8_t val, uint8_t *outRGBA, uint32_t size)
{
    cvtTimerStart();

    uint8_t *__restrict out = outRGBA;

#pragma omp parallel for schedule(static)
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t idx = i * 4;
        out[idx] = val;
        out[idx + 1] = val;
        out[idx + 2] = val;
    }
}

/**
 *
 */
void cvtInitImgLevels(std::vector<imgLevel> &iLevelL, uint32_t rows, uint32_t cols, uint32_t nofLevels)
{
    uint32_t i, size = rows * cols * 2;

    //
    for (i = 0; i < nofLevels; i++)
    {
        size = rows * (cols >> i);
        iLevelL[i].tmpI_float.resize(size, 0);
        iLevelL[i].greyI_float.resize(size, 0);
        iLevelL[i].gaussI_float.resize(size, 0);
        iLevelL[i].gradI_float.resize(size);
        iLevelL[i].edgesL.resize(size);
        iLevelL[i].contoursL.resize(size);
        iLevelL[i].histEdges.resize(256);
    }
}

/**
 *
 */
void cvtSubSample(std::vector<float> &inI, std::vector<float> &outI, uint32_t rows, uint32_t cols)
{
    uint32_t i, size = rows * cols;
    uint32_t i2, rows2, cols2, size2;
    float tv;

    if (size > outI.size())
        return;
    cols2 = cols * 2;
    rows2 = rows * 2;
    size2 = rows2 * cols2;
    if (size2 > inI.size())
        return;

    for (i = 0; i < size; i++)
    {
        if ((i % cols) == 0)
        {
            i2 = (i / cols) * 2 * cols2;
        }
        tv = inI[i2] + inI[i2 + 1];
        tv = tv + inI[i2 + cols2] + inI[i2 + cols2 + 1];
        tv = tv / 4;

        outI[i] = tv;

        i2 += 2;
    }
}

void cvtCopyI32F2RGBA(
    std::vector<float> &inI,
    uint32_t irows, uint32_t icols,
    uint8_t *outRGBA,
    uint32_t ox0,
    uint32_t oy0,
    uint32_t orows,
    uint32_t ocols)
{
    uint32_t in_i, in_j, size = irows * icols;
    uint32_t out_i, out_j;
    uint8_t in_v;

    for (in_i = 0; in_i < size; in_i++)
    {
        if ((in_i % icols) == 0)
        {
            out_i = 4 * ((in_i / icols) * ocols + ox0);
        }
        in_v = (uint8_t)inI[in_i];
        outRGBA[out_i] = in_v;
        outRGBA[out_i + 1] = in_v;
        outRGBA[out_i + 2] = in_v;
        out_i += 4;
    }
}

/**
 *
 */
void cvtTimerStart()
{
    _start = std::chrono::system_clock::now();
}

double cvtTimerEnd()
{
    std::chrono::time_point<std::chrono::system_clock> end;
    end = std::chrono::system_clock::now();

    std::chrono::duration<double, std::milli> duration = end - _start;

    return (duration.count());
}

void cvtTimerValues(processTime *pTime)
{
    pTime->cvtGauss5x5 = _ptime.cvtGauss5x5;
    pTime->cvtRGBA2Grey = _ptime.cvtRGBA2Grey;
    pTime->cvtGrad = _ptime.cvtGrad;
    pTime->cvtEdgDetection = _ptime.cvtEdgDetection;
    pTime->cvtEdgFilter3x3 = _ptime.cvtEdgFilter3x3;
    pTime->cvtEdgSubPixel = _ptime.cvtEdgSubPixel;
    pTime->cvtLinkEdges = _ptime.cvtLinkEdges;
    pTime->cvtLinkContours = _ptime.cvtLinkContours;
    pTime->cvtCurvatureContours = _ptime.cvtCurvatureContours;
    pTime->cvtLineDetection = _ptime.cvtLineDetection;
}

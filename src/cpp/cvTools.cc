#include <iostream>
#include <chrono>
#include <cstdint>
#include <vector>
#include <math.h>
#include <numbers>

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

static processTime pTime;
std::chrono::time_point<std::chrono::system_clock> _start;

void cvtCurvatureContours(
    std::vector<edge> &edges,
    std::vector<contour> &conts,
    uint32_t curvBd)
{
    int cnt, c, i, isize, bd, bsize;
    float dx, dy, norm;
    std::vector<edge>::iterator edg0, pt0, pt1, ptc;
    std::vector<contour>::iterator cont;

    edg0 = edges.begin();

    // loop over contours
    for (cont = conts.begin(); cont != conts.end(); cont++)
    {

        bd = curvBd;
        bsize = 2 * curvBd + 1;
        if (cont->nofEdges < (uint32_t) bsize)
        {
            bd = (int)(cont->nofEdges > 1);
            bsize = 2 * bd + 1;
        }
        // closed ?
        if (cont->start == cont->end)
        {
             // closed contour
            // start point
            pt0 = edg0 + (cont->start - 1);
            for (i = bd; i >= 0; i--)
            {
                if (pt0->p > 0)
                    pt0 = edg0 + (pt0->p - 1);
            }
            // end point
            pt1 = edg0 + (cont->start - 1);
            for (i = bd; i >= 0; i--)
            {
                if (pt1->s > 0)
                    pt1 = edg0 + (pt1->s - 1);
            }
            // center point
            ptc = edg0 + (cont->start - 1);
            //
            cnt = cont->nofEdges;
        }
        else
        {
            // open contour
            /// start point
            pt0 = edg0 + (cont->start - 1);
            // end point
            pt1 = edg0 + (cont->start - 1);
            for (i = bsize; i > 0; i--)
            {
                if (pt1->s > 0)
                    pt1 = edg0 + (pt1->s - 1);
            }
            // center point
            ptc = edg0 + (cont->start - 1);
            for (i = bd; i > 0; i--)
            {
                if (ptc->s > 0)
                    ptc = edg0 + (ptc->s - 1);
            }
            //
            cnt = cont->nofEdges - (2 * bd);
        }
        //
        // calculate curvature
        for (i = 0; i < cnt; i++)
        {
            if (pt1 != ptc && pt0 != ptc)
            {
                dx = pt1->xf - pt0->xf;
                dy = pt1->yf - pt0->yf;
                norm = sqrtf(powf(dx, 2) + powf(dy, 2));
                if (norm > 0.0001)
                {
                    ptc->curv = fabsf((dx * (pt0->yf - ptc->yf)) - (dy * (pt0->xf - ptc->xf))) / norm;
                }
            }
            if (pt0->s > 0)
                pt0 = edg0 + (pt0->s - 1);
            if (pt1->s > 0)
                pt1 = edg0 + (pt1->s - 1);
            if (ptc->s > 0)
                ptc = edg0 + (ptc->s - 1);
        }
    }
}

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
    float thHigh,
    uint32_t rows,
    uint32_t cols)
{
    float aF, mF, mF1, mF2, xf, yf;
    float dxf, dyf;
    uint32_t cnt = 0, val;
    uint32_t i, j, c, c0, c1, size;
    uint32_t u, v;

    // work
    //
    size = rows * cols;
    c0 = 2 * cols + 2;
    c1 = size - (2 * cols) - 2;
    for (c = c0; c < c1; c++)
    {
        mF = gradI[c].mag;
        if (mF > thLow)
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

            if (mF >= mF1 && mF >= mF2)
            {
                gradI[c].edg = 1;
                //
                cnt++;
            }
        }
    }

    //
    // edge list
    edges.resize(cnt);
    std::fill(edges.begin(), edges.end(), edge{0, 0, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0, 0, 0, 0});
    cnt = 0;
    for (c = c0; c < c1; c++)
    {
        if (gradI[c].edg > 0)
        {
            if (cnt < edges.size())
            {
                edges[cnt].xi = (uint32_t)(c % cols);
                edges[cnt].yi = (uint32_t)(c / cols);
                edges[cnt].mag = gradI[c].mag;
                edges[cnt].dir = gradI[c].dir;
                cnt++;
                gradI[c].edg = cnt;
            }
        }
    }
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
    uint32_t j, c, cnt, val;
    //
    // n3x3
    cnt = 0;
    for (std::vector<edge>::iterator pt = edges.begin(); pt != edges.end();)
    {
        val = 0;
        for (j = 0; j < 8; j++)
        {
            c = (pt->yi + _ly[j]) * cols + (pt->xi + _lx[j]);
            if (gradI[c].edg > 0)
                val += (1 << j);
        }
        if (_n3x3[val] == 0)
        {
            gradI[(pt->yi * cols) + pt->xi].edg = 0;
            pt = edges.erase(pt);
        }
        else
        {
            pt->n3x3 = (uint32_t)_n3x3[val];
            cnt++;
            gradI[(pt->yi * cols) + pt->xi].edg = cnt;
            pt++;
        }
    }
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
    int cnt, u, v, j, c;
    float xf, yf, mF;

    for (std::vector<edge>::iterator pt = edges.begin(); pt != edges.end(); pt++)
    {
        cnt = 1;
        mF = pt->mag;
        // dxf = cosf(edges[i].dir * degree2rad);
        // dyf = sinf(edges[i].dir * degree2rad);
        u = pt->xi;
        v = pt->yi;
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
                cnt++;
            }
        }
        pt->xf = xf / mF;
        pt->yf = yf / mF;
    }
}

/**
 *
 */
void cvtLineDetection(
    std::vector<edge> &edges,
    std::vector<contour> &contours,
    std::vector<line> &lines,
    uint32_t curvBd)
{

    std::vector<contour>::iterator cont;
    std::vector<edge>::iterator ept, tstart, tend;
    std::vector<line>::iterator lineP;
    int i, edgCnt, lineCnt;
    float dx, dy, norm, dis;
    bool valid;

    //
    lines.resize(edges.size());
    std::fill(lines.begin(), lines.end(), line{0, 0, 0, 0});
    lineP = lines.begin();

    // loop over contours
    for (cont = contours.begin(); cont != contours.end(); cont++)
    {
        if (cont->nofEdges > (2 * curvBd + 1))
        {
            edgCnt = 0;
            ept = edges.begin() + cont->start - 1;
            for (int i = 0; i < cont->nofEdges; i++)
            {
                if (ept->curv >= 0 && ept->curv < 0.9)
                {
                    if (edgCnt == 0)
                    {
                        tstart = ept;
                    }
                    tend = ept;
                    edgCnt++;
                }
                else
                {
                    if (edgCnt > 10)
                    {
                        lineP->start = std::distance(edges.begin(), tstart) + 1;
                        lineP->end = std::distance(edges.begin(), tend) + 1;
                        lineP->cont = std::distance(contours.begin(), cont) + 1;
                        lineP->nofEdges = edgCnt;
                        lineP++;
                    }
                    edgCnt = 0;
                }
                if (ept->s > 0)
                    ept = edges.begin() + (ept->s - 1);
                else
                    break;
            }
            if (edgCnt > 10)
            {
                lineP->start = std::distance(edges.begin(), tstart) + 1;
                lineP->end = std::distance(edges.begin(), tend) + 1;
                lineP->cont = std::distance(contours.begin(), cont) + 1;
                lineP->nofEdges = edgCnt;
                lineP++;
            }
        }
    }

    lineCnt = std::distance(lines.begin(), lineP);
    lines.resize(lineCnt);

#if 1
    // prove straight lines
    for (lineP = lines.begin(); lineP != lines.end();)
    {
        tstart = edges.begin() + (lineP->start - 1);
        tend = edges.begin() + (lineP->end - 1);
        dx = tend->xf - tstart->xf;
        dy = tend->yf - tstart->yf;
        norm = sqrtf(powf(dx, 2) + powf(dy, 2));
        valid = true;
        if (norm > 0.0001)
        {
            ept = tstart;
            for (i = 0; i < lineP->nofEdges; i++)
            {
                if (ept->s > 0) ept = edges.begin() + (ept->s - 1);
                dis = fabsf((dx * (tstart->yf - ept->yf)) - (dy * (tstart->xf - ept->xf))) / norm;
                if (dis > 1.0)
                {
                    valid = false;
                    break;
                }
            }
        }
        else
        {
            valid = false;
        }
        if (valid)
            lineP++;
        else
            lineP = lines.erase(lineP);
    }
#endif

#if 1
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
            while( ept->p > 0){
                ept = edges.begin() + (ept->p - 1);
                dis = fabsf((dx * (tstart->yf - ept->yf)) - (dy * (tstart->xf - ept->xf))) / norm;
                if( dis < 0.4){
                    tstart = ept;
                    edgCnt++;
                } else{
                    break;
                }
            }
            // end
            ept = tend;
            while( ept->s > 0){
                ept = edges.begin() + (ept->s - 1);
                dis = fabsf((dx * (tstart->yf - ept->yf)) - (dy * (tstart->xf - ept->xf))) / norm;
                if( dis < 0.4){
                    tend = ept;
                    edgCnt++;
                } else{
                    break;
                }
            }
        }
        lineP->start = std::distance(edges.begin(), tstart) + 1;
        lineP->end =  std::distance(edges.begin(), tend) + 1;
        lineP->nofEdges += edgCnt;

    }
#endif

}

/**
 *
 */
void cvtLinkEdges(
    std::vector<edge> &edges,
    std::vector<grad_float> &gradI,
    float thLow,
    float thHigh,
    uint32_t rows,
    uint32_t cols)
{

    uint32_t i, j, nofEdges = edges.size();
    float dr, dq, a1, a2;
    uint32_t x, y, xp, yp, c, pid;
    uint32_t di, dj, predecessor, cnt = 0;

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
                if (gradI[c].edg > 0 && gradI[c].mag >= thLow)
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
}

/**
 *
 */
void cvtLinkContours(
    std::vector<edge> &edges,
    std::vector<contour> &conts,
    float th)
{

    uint32_t i, j, i0, i1, nofEdges = edges.size();
    uint32_t cnt = 0, nofConts;
    int xi1, yi1, xj0, yj0;
    float mag;

    conts.resize(nofEdges);
    std::fill(conts.begin(), conts.end(), contour{0, 0, 0});

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
            if (conts[cnt].nofEdges > 2)
            {
                mag /= (float)(conts[cnt].nofEdges);
                if (mag > th)
                    cnt++;
            }
        }
    }
    conts.resize(cnt);
}

/**
 *
 */
void cvtContoursCombine(
    std::vector<edge> &edges,
    std::vector<contour> &conts)
{
    uint32_t cnt, i, j, i0;
    int xi, yi, xj, yj;

    // contours divided?
    cnt = 0;
    for (i = 0; i < conts.size(); i++)
    {
        if (conts[i].start != conts[i].end)
        {
            // 1
            xi = (int)edges[conts[i].end - 1].xi;
            yi = (int)edges[conts[i].end - 1].yi;
            for (j = i + 1; j < conts.size(); j++)
            {
                if (conts[j].nofEdges > 0 && (conts[j].start != conts[j].end))
                {
                    xj = (int)edges[conts[j].start - 1].xi;
                    yj = (int)edges[conts[j].start - 1].yi;
                    if (abs(xi - xj) <= 1 && abs(yi - yj) <= 1)
                    {
                        // change contour i
                        edges[conts[i].end - 1].s = conts[j].start;
                        edges[conts[j].start - 1].p = conts[i].end;
                        i0 = conts[i].end;
                        while (edges[i0 - 1].s > 0)
                        {
                            i0 = edges[i0 - 1].s;
                            edges[i0 - 1].c = (i + 1);
                            if (i0 == conts[i].start)
                                break;
                        }
                        /*
                         */
                        conts[i].end = conts[j].end;
                        conts[i].nofEdges += conts[j].nofEdges;
                        conts[j].start = 0;
                        conts[j].end = 0;
                        conts[j].nofEdges = 0;
                        cnt++;
                        break;
                    }
                }
            }
            // 2
            xi = (int)edges[conts[i].start - 1].xi;
            yi = (int)edges[conts[i].start - 1].yi;
            for (j = i + 1; j < conts.size(); j++)
            {
                if (conts[j].nofEdges > 0 && (conts[j].start != conts[j].end))
                {
                    xj = (int)edges[conts[j].end - 1].xi;
                    yj = (int)edges[conts[j].end - 1].yi;
                    if (abs(xi - xj) <= 1 && abs(yi - yj) <= 1)
                    {
                        // change contour i
                        edges[conts[i].start - 1].p = conts[j].end;
                        edges[conts[j].end - 1].s = conts[i].start;
                        i0 = conts[i].start;
                        while (edges[i0 - 1].p > 0)
                        {
                            i0 = edges[i0 - 1].p;
                            edges[i0 - 1].c = (i + 1);
                            if (i0 == conts[i].end)
                                break;
                        }
                        /*
                         */
                        conts[i].start = conts[j].start;
                        conts[i].nofEdges += conts[j].nofEdges;
                        conts[j].start = 0;
                        conts[j].end = 0;
                        conts[j].nofEdges = 0;
                        cnt++;
                        break;
                    }
                }
            }
        }
    }

    std::vector<contour>::iterator cpt = conts.begin();
    while (cpt != conts.end())
    {
        if (cpt->nofEdges == 0)
        {
            cpt = conts.erase(cpt);
        }
        else
        {
            cpt = cpt + 1;
        }
    }
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
void cvtEdgPts2RGBA(std::vector<edge> &edges, uint8_t *outRGBA, uint32_t rows, uint32_t cols, uint32_t xoff, uint32_t yoff)
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
            // Normalize angle to [0.0, 360.0) range
            dir = fmodf(edges[i].dir, 360.0f);
            if (dir < 0.0f)
            {
                dir += 360.0f;
            }

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
    std::vector<uint32_t> &hist,
    float th,
    uint32_t rows,
    uint32_t cols)
{
    float dx, dy;
    float mv, dv;
    float pi = (float)(atan(1.0) * 4.);
    float mmax = 0;
    uint32_t c0, c;
    uint32_t x, y;

    // for (uint32_t i = 0; i < 256; i++)  hist[i] = 0;
    std::fill(hist.begin(), hist.end(), 0);

    std::fill(gradI32F.begin(), gradI32F.end(), grad_float{0.0f, 0.0f, 0.0f, 0.0f, 0});
    // work
    for (y = 3; y < (rows - 3); y++)
    {
        c0 = y * cols;
        for (x = 3; x < (cols - 3); x++)
        {
            c = c0 + x;
            dx = 0.5f * (inI32F[c + 1] - inI32F[c - 1]);
            dy = 0.5f * (inI32F[c + cols] - inI32F[c - cols]);
            mv = sqrt(dx * dx + dy * dy);
            if (mv > th)
            {
                // atan2f between [-180, 180]
                dv = 180. * atan2f(dy, dx) / pi;
                dv += 180;
                // grad dir values [0, 360]
                gradI32F[c] = {dx, dy, mv, dv, 0};
                mmax = (mmax < mv) ? mv : mmax;
                //
                mv = mv > 255. ? 255. : mv;
                hist[(uint8_t)mv]++;
            }
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
    // input image has got an RGBA structure
    // pixels[0] = Red, pixels[1] = Green, pixels[2] = Blue, pixels[3] = Alpha
    float r, g, b;
    uint32_t channels = 4;

    cvtTimerStart();

    for (uint32_t i = 0, j = 0; i < size; i++)
    {
        r = (float)inRGBA[j];
        g = (float)inRGBA[j + 1];
        b = (float)inRGBA[j + 2];
        j = j + channels;
        greyI32F[i] = 0.2989 * r + 0.5870 * g + 0.1140 * b;
    }

    pTime.cvtRGBA2Grey = cvtTimerEnd(); 
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
        iLevelL[i].linesL.resize(size);
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
void cvtTimerStart(){
    _start = std::chrono::system_clock::now();
}

std::chrono::duration<double> cvtTimerEnd(){
    std::chrono::time_point<std::chrono::system_clock> end;
    end  = std::chrono::system_clock::now();

    std::chrono::duration<double, std::milli> duration = end - _start;

    return(duration);
}

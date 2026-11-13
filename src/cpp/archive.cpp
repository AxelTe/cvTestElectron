void cvtEdgLinkHistogram(std::vector<grad_float> &gradI,std::vector<edge> &edges, std::vector<uint32_t> &hist, uint32_t rows, uint32_t cols);
void cvtLinkEdges(std::vector<edge> &edges, std::vector<grad_float> &gradI, uint32_t rows, uint32_t cols);
void cvtLinkPts2RGBA(std::vector<edge> &edges, uint8_t *outRGBA, uint32_t rows, uint32_t cols, uint32_t xoff, uint32_t yoff);
void cvtEdges2Contours(std::vector<edge> &edges);
void cvtContoursCombine(std::vector<edge> &edges, std::vector<contour> &conts);
void cvtEdg2RGBA(std::vector<grad_float> &gradI32F, uint8_t *outRGBA, uint32_t size);



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
    uint32_t xi,yi,i,xj,yj,j;
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
        for(j=0; j<8; j++){
            xj = xi + _lx[j];
            yj = yi + _ly[j];
            if( gradI[(yj*cols)+xj].edg>0) val += (1<<j);
        }
        hist[val]++;
    }
}

/**
 *
 */
void cvtLinkEdges(
    std::vector<edge> &edges,
    std::vector<grad_float> &gradI,
    uint32_t rows,
    uint32_t cols)
{

    uint32_t i, nofEdges = edges.size();
    float dr, dq;
    uint32_t x, y, xp, yp, pid;
    uint8_t dri;

    for (i = 0; i < nofEdges; i++)
    {
        dr = roundf(edges[i].dir / 45.0f);
        dq = edges[i].dir - (dr * 45.0f);
        x = edges[i].xi;
        y = edges[i].yi;

        // predecessor
        dri = (uint8_t) dr;
        dri = (dri > 7) ? (dri-8) : dri;
        xp = x + ((uint32_t)_px[dri]);
        yp = y + ((uint32_t)_py[dri]);
        if (xp >= 0 && xp < cols && yp >= 0 && yp < rows)
        {
            pid = gradI[(yp * cols) + xp].edg;
            if(pid == 0){
                if(dq < 0.){
                    dri = dri-1;
                    if(dri == -1) dri = 7; 
                } else  {
                    dri = dri+1;
                    if(dri == 8) dri = 0; 
                }
                xp = x + ((uint32_t)_px[dri]);
                yp = y + ((uint32_t)_py[dri]);
                if (xp >= 0 && xp < cols && yp >= 0 && yp < rows)
                {
                    pid = gradI[(yp * cols) + xp].edg;
                }
            }
            edges[i].p = pid;
        }
        else
        {
            edges[i].p = 0;
        }
        // sucessor
        dri = (uint8_t) dr;
        dri = (dri > 7) ? (dri-8) : dri;
        xp = x + ((uint32_t)_sx[dri]);
        yp = y + ((uint32_t)_sy[dri]);
        if (xp >= 0 && xp < cols && yp >= 0 && yp < rows)
        {
            pid = gradI[(yp * cols) + xp].edg;
            if(pid == 0){
                if(dq > 0){
                    dri = dri-1;
                    if(dri == -1) dri = 7; 
                } else  {
                    dri = dri+1;
                    if(dri == 8) dri = 0; 
                }
                xp = x + ((uint32_t)_sx[dri]);
                yp = y + ((uint32_t)_sy[dri]);
                if (xp >= 0 && xp < cols && yp >= 0 && yp < rows)
                {
                    pid = gradI[(yp * cols) + xp].edg;
                }
            }
            edges[i].s = pid;
        }
        else
        {
            edges[i].s = 0;
        }
    }
}

/**
 *
 */
void cvtLinkPts2RGBA(std::vector<edge> &edges, uint8_t *outRGBA, uint32_t rows, uint32_t cols, uint32_t xoff, uint32_t yoff)
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
            if ((edges[i].s > 0) && (edges[i].p > 0))
            {
                r = 255;
                g = 0;
                b = 0;
            }
            if ((edges[i].s == 0) && (edges[i].p > 0))
            {
                r = 0;
                g = 255;
                b = 0;
            }
            if ((edges[i].s > 0) && (edges[i].p == 0))
            {
                r = 0;
                g = 0;
                b = 255;
            }
            if ((edges[i].s == 0) && (edges[i].p == 0))
            {
                r = 0;
                g = 255;
                b = 255;
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
void cvtEdges2Contours(std::vector<edge> &edges)
{

    uint32_t i, j;
    uint32_t nofEdges = edges.size();
    float xif, yif;
    float xjf, yjf;
    float dis;
    uint32_t cnt = 0, lastPt, contCnt = 0;

    for (i = 0; i < nofEdges; i++)
    {
        if (edges[i].p == 0)
        {
            xif = edges[i].xf;
            yif = edges[i].yf;
            // increment contour counter
            contCnt++;
            // mark edgepoint as processed
            edges[i].c = contCnt;
            edges[i].p = 0;
            lastPt = i;
            //
            for (j = (i + 1); j < nofEdges; j++)
            {
                if (edges[j].p == 0)
                {
                    xjf = edges[j].xf;
                    yjf = edges[j].yf;
                    dis = sqrtf(powf((xif - xjf), 2) + powf((yif - yjf), 2));
                    if (dis <= 1.9)
                    { // _sqrt2){
                        edges[j].c = contCnt;
                        edges[j].p = lastPt;
                        edges[lastPt].s = j;
                        xif = xjf;
                        yif = yjf;
                        lastPt = j;
                    }
                }
            }
        }
    }
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
    std::vector<contour>::iterator icont, jcont;

    cvtTimerStart();

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

    conts.erase(
        std::remove_if(conts.begin(), conts.end(), [](const contour& c) {
            return c.nofEdges == 0;
        }),
        conts.end()
    );
    
    _ptime.cvtContoursCombine = cvtTimerEnd(); 

}


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

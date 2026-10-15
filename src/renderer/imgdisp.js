const toHex = (component) => {
    // Clamp to [0, 255], round to nearest integer, and convert to 2-digit hex
    const clamped = Math.max(0, Math.min(255, Math.round(component)));
    return clamped.toString(16).padStart(2, '0');
  };

/**
 *
 * @emits imgDispError Customevent 
 * @emits loadFromRootEvent Customevent
 */
class imgdisp {
    #t0 = 0;
    #colors8 = ["#ff0000", "#ffff00", "#ff00ff","#00ff00","#00ffff","#E69F00","#0000ff", "#CC79A7"];
    /**
     * 
     * @param {object} cv DOM-id of canvas object
     * @param {number} wc maximal width for canvas 
     * @param {number} hc maximal height for canvas 
     */
    constructor(cv) {
        // canvas object
        this.cv = cv;
        this.rows = 0;
        this.cols = 0;
        this.start = 0;
        this.end = 0;
        this.id = 0;
        this.dispQueueTimer = null;
        this.imgD = null;
        this.edgeData = null;
        this.config = null;

        const ctx = this.cv.getContext('2d');
        ctx.font = '30px Arial';
        ctx.fillStyle = '#2b2b2b';      // Text color
        ctx.textAlign = 'left';        // Options: left, right, center, start, end
        ctx.textBaseline = 'bottom';     // Options: top, hanging, middle, alphabetic, ideographic, bottom
    }

    /**
     * 
     * @param {Uint8Array} img 
     */
    dispImg(imgU8Array, rows, cols, pTimes) {
        const ctx = this.cv.getContext('2d');
        let clampedArr = new Uint8ClampedArray(imgU8Array); //.data);
        this.imgD = new ImageData(clampedArr, cols, rows);
        ctx.putImageData(this.imgD, 0, 0);

        //
        let tstr = "Proc. time: ";
        tstr = tstr + pTimes[0].min + ".." + pTimes[0].val + ".." + pTimes[0].max;
        tstr = tstr + ", " + pTimes[1].min + ".." + pTimes[1].val + ".." + pTimes[1].max;
        tstr = tstr + ", " + (Date.now() - this.#t0);
        ctx.fillText(tstr, 2, this.rows - 2);
        //
        if ((this.end - this.start) <= this.id) {
            clearInterval(this.dispQueueTimer);
            this.dispQueueTimer = null
        } else {
            this.id++;
        }
    }

    /**
     * 
     * @param {*} edgPts 
     */
    contours() {
        const ctx = this.cv.getContext('2d');
        let radius = 1.5;
        // 
        let fillStyle = ctx.fillStyle;
        let strokeStyle = ctx.strokeStyle;
        //
        // no of contours
        let edges = this.edgeData.edges;
        let conts = this.edgeData.conts;
        let cnt = 0, pt;
        for( let c=0; c<conts.length; c++){
            if (conts[c].nofEdges > 1) {
                //console.log(edges[pt-1]);
                radius = 1.5;
                /* */
                
                ctx.beginPath();
                ctx.strokeStyle = this.#colors8[cnt % 8];
                pt = conts[c].start;
                ctx.moveTo(edges[pt-1].xi, edges[pt-1].yi);
                while( edges[pt-1].s > 0){
                    pt =  edges[pt-1].s;
                    ctx.lineTo(edges[pt-1].xi, edges[pt-1].yi);
                    if(pt === conts[c].end) break;
                }
                ctx.stroke();                
                //
                if (conts[c].start === conts[c].end) {
                    // start points
                    ctx.fillStyle = "#FF0000"
                    pt = conts[c].start;
                    ctx.beginPath();
                    //ctx.moveTo(edges[pt-1].xi + radius, edges[pt-1].yi);
                    ctx.arc(edges[pt - 1].xi, edges[pt - 1].yi, radius, 0, Math.PI * 2);
                    ctx.fill();
                } else {
                    // start points
                    ctx.fillStyle = "#00FF00"
                    pt = conts[c].start;
                    ctx.beginPath();
                    //ctx.moveTo(edges[pt-1].xi + radius, edges[pt-1].yi);
                    ctx.arc(edges[pt - 1].xi, edges[pt - 1].yi, radius, 0, Math.PI * 2);
                    ctx.fill();
                    //
                    // end points
                    pt = conts[c].end;
                    ctx.fillStyle = "#0000FF"
                    ctx.beginPath();
                    //ctx.moveTo(edges[pt-1].xi + radius, edges[pt-1].yi);
                    ctx.arc(edges[pt - 1].xi, edges[pt - 1].yi, radius, 0, Math.PI * 2);
                    ctx.fill();
                }
                //
                cnt++;
            }
        }
        console.log("imgDisp->contours: nof conts: ", cnt);
        //
        ctx.fillStyle = fillStyle;
        ctx.strokeStyle = strokeStyle;
    }

    /**
     * 
     * @param {*} edgPts 
     */
    edges() {
        const ctx = this.cv.getContext('2d');
        let radius = 1.5;
        // 
        let fillStyle = ctx.fillStyle;
        let strokeStyle = ctx.strokeStyle;
        //
        // line
        ctx.strokeStyle = "#0000FF";
        let edges = this.edgeData.edges;
        let conts = this.edgeData.conts;
        let i, i0, i1;
        for( i=0; i<conts.length; i++){
            if (conts[i].nofEdges > 1) {
                i0 = conts[i].start;
                i1 = conts[i].end;
                ctx.moveTo(edges[i0-1].xi, edges[i0-1].yi);
        
                while( edges[i0-1].s > 0){
                    i0 = edges[i0-1].s;
                    ctx.lineTo(edges[i0-1].xi, edges[i0-1].yi);
                    if( i1 === i0 ) break;
                }
            }
        }
        ctx.stroke();
        //
        ctx.fillStyle = "#00FF00"
        ctx.beginPath();     
        for( i=0; i<conts.length; i++){
            if (conts[i].nofEdges > 1) {
                i0 = conts[i].start;
                ctx.moveTo(edges[i0-1].xi, edges[i0-1].yi);
                ctx.arc(edges[i0-1].xi, edges[i0-1].yi, radius, 0, Math.PI * 2);
            }
        }
        ctx.fill();

        ctx.fillStyle = "#FF0000"
        ctx.beginPath();     
        for( i=0; i<conts.length; i++){
            if (conts[i].nofEdges > 1) {
                i0 = conts[i].end;
                ctx.moveTo(edges[i0-1].xi, edges[i0-1].yi);
                ctx.arc(edges[i0-1].xi, edges[i0-1].yi, radius, 0, Math.PI * 2);
            }
        }
        ctx.fill();
        //
        ctx.fillStyle = fillStyle;
        ctx.strokeStyle = strokeStyle;
    }

    directions() {
        const ctx = this.cv.getContext('2d');
        const imgdata = this.imgD.data.slice();
        let rows = this.rows;
        let cols = this.cols;
        let id;

        // 
        let xi, yi, r, g, b;
        let h, sector, f, q, t;
        //
        let edges = this.edgeData.edges;
        let conts = this.edgeData.conts;
        let i, i0, i1;
        for( i=0; i<conts.length; i++){
            if (conts[i].nofEdges > 1) {
                i0 = conts[i].start;
                i1 = conts[i].end;
        
                while (edges[i0 - 1].s > 0) {
                    xi = edges[i0 - 1].xi;
                    yi = edges[i0 - 1].yi;
                    id = yi * (4 * cols) + (4 * xi);

                    h = edges[i0-1].dir / 60.;
                    sector = Math.floor(h);
                    f = h - sector; // Fractional part

                    q = Math.round((1.0 - f) * 255.0);
                    t = Math.round(f * 255.0);

                    switch (sector) {
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

                    imgdata[id] = r;
                    imgdata[id + 1] = g;
                    imgdata[id + 2] = b;

                    i0 = edges[i0 - 1].s;
                    if (i1 === i0) break;
                }
            }
        }

        let imgD = new ImageData(imgdata, cols, rows);
        ctx.putImageData(imgD, 0, 0);

    }
    /**
     * 
     * @param {string} err error message 
     */
    error(err) {
        let imgDispErrEvent = new CustomEvent('imgDispError', {
            bubbles: true,
            detail: { msg: err }
        });
        this.cv.dispatchEvent(imgDispErrEvent);
    }

    loadImgFrommRoot() {
        console.log("loadImgFrommRoot");
        this.#t0 = Date.now();
        let loadFromRootEvent = new CustomEvent('loadFromRoot', {
            bubbles: true,
            //detail: { msg: {id: this.id, cols: this.wd, rows: this.hd} } 
            detail: { msg: { id: this.id, cols: this.cols, rows: this.rows } }
        });
        this.cv.dispatchEvent(loadFromRootEvent);
    }

    seqInfo(info) {
        if (info.hasOwnProperty("start")) this.start = info.start;
        else this.error("seqInfo: missing property: start");
        if (info.hasOwnProperty("end")) this.end = info.end;
        else this.error("seqInfo: missing property: end");
        if (info.hasOwnProperty("rows")) this.rows = info.rows;
        else this.error("seqInfo: missing property: rows");
        if (info.hasOwnProperty("rows")) this.cols = info.cols;
        else this.error("seqInfo: missing property: cols");

        // reset img id
        this.id = 0;

        this.setCVSize2(this.cols, this.rows);
    }

    setCVSize2(iw, ih) {
        this.cv.width = this.cols;
        this.cv.height = this.rows;
    }


    setCVSize(iw, ih) {
        if (this.wc >= iw && this.hc >= ih) {
            console.log("case 1");
            this.wd = iw;
            this.hd = ih;
        }
        if (this.wc >= iw && this.hc < ih) {
            console.log("case 2");
            this.hd = this.hc;
            this.wd = iw * this.hc / ih;
        }
        if (this.wc < iw && this.hc >= ih) {
            console.log("case 3");
            this.wd = this.wc;
            this.hd = ih * this.wc / iw;
        }
        if (this.wc < iw && this.hc < ih) {
            if (ih < iw) {
                console.log("case 4");
                this.wd = this.wc;
                this.hd = ih * this.wc / iw;
            } else {
                console.log("case 5");
                this.hd = this.hc;
                this.wd = iw * this.hc / ih;
            }
        }
        this.wd = Math.floor(this.wd);
        this.hd = Math.floor(this.hd);
        this.xd0 = (this.wc - this.wd) / 2;
        this.yd0 = (this.hc - this.hd) / 2;
        this.cv.style.top = this.yd0 + "px";
        this.cv.style.left = this.xd0 + "px";
        this.cv.style.width = this.wd;
        this.cv.style.height = this.hd;
        this.cv.width = this.wd;
        this.cv.height = this.hd;
        console.log("client:", this.wc, this.hc, "cv:", this.wd, this.hd, "img: ", iw, ih);
    }

    startDisplayQueue() {
        if (this.end > this.start) {
            if (this.dispQueueTimer !== null) {
                clearInterval(this.dispQueueTimer);
                this.dispQueueTimer = null;
            }
            this.dispQueueTimer = setInterval(() => {
                this.loadImgFrommRoot();
            }, 400);
        } else {
            this.error("startDisplayQueue: no correct start and end values to start display queue");
        }
    }

    stopDisplayQueue() {
        if (this.dispQueueTimer !== null) {
            clearInterval(this.dispQueueTimer);
            this.dispQueueTimer = null;
        }
    }
}
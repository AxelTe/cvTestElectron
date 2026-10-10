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
    contours(edgeData) {
        this.edgeData = edgeData;
        const ctx = this.cv.getContext('2d');
        let radius = 1.5;
        // 
        let fillStyle = ctx.fillStyle;
        let strokeStyle = ctx.strokeStyle;
        //
        // no of contours
        let edges = edgeData.edges;
        let conts = edgeData.conts;
        let cnt = 0, pt;
        for( let c=0; c<conts.length; c++){
            if (conts[c].nofEdges > 1) {
                console.log(conts[c]);
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
    edges(edgeData) {
        this.edgeData = edgeData;
        const ctx = this.cv.getContext('2d');
        let radius = 1.0;
        // 
        let fillStyle = ctx.fillStyle;
        let strokeStyle = ctx.strokeStyle;
        //
        // line
        ctx.strokeStyle = "#0000FF"
        ctx.beginPath();
        let edgPts = this.edgeData.edges;
        for (let i = 0; i < edgPts.length; i++) {
            if (edgPts[i].s > 0) {
                ctx.moveTo(edgPts[i].xi, edgPts[i].yi);
                ctx.lineTo(edgPts[edgPts[i].s-1].xi, edgPts[edgPts[i].s-1].yi);
            }
        }
        ctx.stroke();
        //
        //
        radius = 1.5;
        // start points
        ctx.fillStyle = "#00FF00"
        ctx.beginPath();
        for (let i = 0; i < edgPts.length; i++) {
            if (edgPts[i].p == 0 && edgPts[i].s > 0) {
                ctx.moveTo(edgPts[i].xi + radius, edgPts[i].yi);
                ctx.arc(edgPts[i].xi, edgPts[i].yi, radius, 0, Math.PI * 2);
            }
        }
        ctx.fill();
        //
        // end points
        ctx.fillStyle = "#FF0000"
        ctx.beginPath();
        for (let i = 0; i < edgPts.length; i++) {
            if (edgPts[i].p > 0 && edgPts[i].s == 0) {
                ctx.moveTo(edgPts[i].xi + radius, edgPts[i].yi);
                ctx.arc(edgPts[i].xi, edgPts[i].yi, radius, 0, Math.PI * 2);
            }
        }
        ctx.fill();
        //
        ctx.fillStyle = fillStyle;
        ctx.strokeStyle = strokeStyle;
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
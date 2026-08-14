/**
 *
 * @emits imgDispError Customevent 
 * @emits loadFromRootEvent Customevent
 */
class imgdisp {
    #t0 = 0;
    /**
     * 
     * @param {object} cv DOM-id of canvas object
     * @param {number} wc maximal width for canvas 
     * @param {number} hc maximal height for canvas 
     */
    constructor(cv, wc, hc){
        // canvas object
        this.cv = cv;
        this.wc = wc;
        this.hc = hc;
        this.wd = this.cv.width;
        this.hd = this.cv.height;
        this.xd0 = (this.wc-this.wd)/2;
        this.yd0 = (this.hc-this.hd)/2;
        this.cv.style.top = this.yd0+"px";
        this.cv.style.left = this.xd0+"px";
        this.rows = this.hd;
        this.cols = this.wd;
        this.start = 0;
        this.end = 0;
        this.id = 0;
        this.dispQueueTimer = null;
        
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
    dispImg( imgU8Array , pTimes){
        const ctx = this.cv.getContext('2d');
        const clampedArray = new Uint8ClampedArray(imgU8Array); //.data);
        const imageData = new ImageData(clampedArray, this.cols, this.rows);
        ctx.putImageData(imageData, 0, 0);
        //
        let tstr = "Proc. time: ";
        tstr = tstr + pTimes[0].min + ".."+pTimes[0].val+".."+pTimes[0].max;
        tstr = tstr + ", "+pTimes[1].min + ".."+pTimes[1].val+".."+pTimes[1].max;
        tstr = tstr + ", "+(Date.now() - this.#t0);
        ctx.fillText(tstr, 2, this.hd -2 );
        //
        if( (this.end-this.start)<=this.id){
            clearInterval(this.dispQueueTimer);
            this.dispQueueTimer = null
        } else {
            this.id++;
        }
    }


    /**
     * 
     * @param {string} err error message 
     */
    error( err ){
        let imgDispErrEvent = new CustomEvent('imgDispError',{ 
            bubbles: true, 
            detail: { msg: err } 
        });
        this.cv.dispatchEvent(imgDispErrEvent);
    }

    loadImgFrommRoot(){
        console.log("loadImgFrommRoot");
        this.#t0 = Date.now();
        let loadFromRootEvent = new CustomEvent('loadFromRoot',{ 
            bubbles: true, 
            //detail: { msg: {id: this.id, cols: this.wd, rows: this.hd} } 
            detail: { msg: {id: this.id, cols: this.cols, rows: this.rows} } 
        });
        this.cv.dispatchEvent(loadFromRootEvent);
    }

    seqInfo( info){
        if( info.hasOwnProperty("start")) this.start = info.start;
        else  this.error( "seqInfo: missing property: start" );
        if( info.hasOwnProperty("end")) this.end = info.end;
        else  this.error( "seqInfo: missing property: end" );
        if( info.hasOwnProperty("rows")) this.rows = info.rows;
        else  this.error( "seqInfo: missing property: rows" );
        if( info.hasOwnProperty("rows")) this.cols = info.cols;
        else  this.error( "seqInfo: missing property: cols" );

        // reset img id
        this.id = 0;

        this.setCVSize(this.cols, this.rows);
    }

    setCVSize(iw,ih){
        if(this.wc >= iw && this.hc >= ih){
            console.log("case 1");
            this.wd = iw;
            this.hd = ih;
        }
        if(this.wc >= iw && this.hc < ih){
            console.log("case 2");
            this.hd = this.hc;
            this.wd = iw * this.hc/ih;
        }
        if(this.wc < iw && this.hc >= ih){
            console.log("case 3");
            this.wd = this.wc;
            this.hd = ih * this.wc/iw;
        }
        if(this.wc < iw && this.hc < ih){
            if( ih < iw){
                console.log("case 4");
                this.wd = this.wc;
                this.hd = ih * this.wc/iw;
            } else {
                console.log("case 5");
                this.hd = this.hc;
                this.wd = iw * this.hc/ih;
            }
        }
        this.wd = Math.floor(this.wd);
        this.hd = Math.floor(this.hd);
        this.xd0 = (this.wc - this.wd)/2;
        this.yd0 = (this.hc - this.hd)/2;
        this.cv.style.top = this.yd0+"px";
        this.cv.style.left = this.xd0+"px";
        this.cv.style.width = this.wd;
        this.cv.style.height = this.hd;
        this.cv.width = this.wd;
        this.cv.height = this.hd;
        console.log("client:", this.wc, this.hc, "cv:", this.wd, this.hd, "img: ", iw, ih);
    }

    startDisplayQueue(){
        if(this.end>this.start){
            if( this.dispQueueTimer !== null){
                clearInterval(this.dispQueueTimer);
                this.dispQueueTimer = null;
            }
            this.dispQueueTimer = setInterval(() => {
                this.loadImgFrommRoot();
             }, 400);
        } else {
            this.error( "startDisplayQueue: no correct start and end values to start display queue" );
        }
    }

    stopDisplayQueue(){
        if( this.dispQueueTimer !== null){
            clearInterval(this.dispQueueTimer);
            this.dispQueueTimer = null;
        }
    }
}
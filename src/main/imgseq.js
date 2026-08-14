const fs = require('node:fs');
const path = require('node:path');
const bmp = require('@vingle/bmp-js');
const sharp = require('sharp');
const EventEmitter = require('node:events');
const { toUnicode } = require('node:punycode');

//const addonPath = path.join(__dirname, '../../build/Release/my_addon.node');
const addonPath = path.join(__dirname, '../../build/Debug/my_addon.node');
const myAddon = require(addonPath);

/**
 * 
 */
class imgseq extends EventEmitter {
    #imgD = undefined;
    #imgInfo = undefined;

    constructor() {
        super();
        this.path = "";
        this.seqname = "";
        this.last = -1;
        this.seqinfo = {};
        this.#imgD = undefined;
        this.#imgInfo = undefined;
        this.rows = 0;
        this.cols = 0;

        this.id = -1;
    }

    /**
     * 
     * @param {*} config 
     * @param {string} config.path "path name"
     * @param {string} config.seqname "name of seq. info file inside path"
     * @param {number} config.last "id of last processed image"
     * @param {number} config.imgArrLen "size of internal image array"
     * @returns {void}
     */
    start(config) {
        let valid = true;
        if (config.hasOwnProperty("path")) this.path = config.path;
        else valid = false;
        if (config.hasOwnProperty("seqname")) this.seqname = config.seqname;
        else valid = false;
        if (config.hasOwnProperty("last")) this.last = config.last;
        else valid = false;
        if (valid === false) {
            this.emit("imgseqError", "no correct sequence definition in config data");
            return;
        }

        let fname = this.path + "\\" + this.seqname;
        console.log("imgseq.start:", fname);

        fs.readFile(fname, (err, data) => {
            if (err) {
                this.emit("imgseqError", "Error: " + err.message);
                console.log("Error:", err.message);
                return;
            }
            // 2. Daten verarbeiten
            try {
                this.seqinfo = JSON.parse(data);
            } catch (error) {
                this.emit("imgseqError", "Error during JSON.parse():" + error);
                console.log("Error during JSON.parse():", error);
                return
            }
            console.log("imgseq.start: ", this.seqinfo);
            
            this.rows = this.seqinfo.rows;
            this.cols = this.seqinfo.cols;
            try {

                const result = myAddon.initialize(this.rows, this.cols);
                console.log("imgseq.loadImg()> myAddon.initialize() returned: ", result);
                
            } catch (error) {
                console.log("imgseq.loadImg()> Error: ", error);
            }

            this.emit("imgseqInfo", { seqname: this.seqname, seqinfo: this.seqinfo, ptime: 0 });
        });
    }

    /**
     * 
     * @fires imgseq#imgseqInfo
     * @fires imgseq#imgseqError
     * @returns {void}
     */
    loadImg(id, cv_rows, cv_cols) {

        let t0 = Date.now();
        let tid = this.seqinfo.start + id;
        if (tid > this.seqinfo.end) {
            this.emit("imgseqError", "id > seqinfo.end: " + tid + ", " + this.seqinfo.end);
            return
        }
        let fname = this.path + "\\" + this.seqinfo.prefix + tid + "." + this.seqinfo.suffix;
        console.log("imgseq.loadImg()> ", fname);
        switch (this.seqinfo.suffix) {
            case "jpg":
                sharp(fname)
                    //.resize(cv_cols, cv_rows, {
                    //    kernel: sharp.kernel.nearest,
                    //    fit: 'fill',
                    //})
                    .ensureAlpha() // Garantiert 4 Kanäle (Rot, Grün, Blau, Alpha)
                    .raw()
                    .toBuffer({ resolveWithObject: true })
                    .then(({ data, info }) => {
                        this.#imgD = data;
                        this.#imgInfo = info;
                        this.emit("imgseqInfo", { loadImg: true, info: info, img: data, ptime: Date.now() - t0 });
                    })
                    .catch(err => {
                        this.emit("imgseqError", err);
                        return;
                    })
                break;
            case "bmp":
                const bmpBuffer = fs.readFileSync(fname);
                const bitmap = bmp.decode(bmpBuffer, true);
                sharp(bitmap.data, {
                    raw: {
                        width: bitmap.width,
                        height: bitmap.height,
                        channels: 4, // BMP decoded as RGBA uses 4 channels
                    },
                })
                    .resize(cv_cols, cv_rows, {
                        kernel: sharp.kernel.nearest,
                        fit: 'fill',
                    })
                    .ensureAlpha() // Garantiert 4 Kanäle (Rot, Grün, Blau, Alpha)
                    .raw()
                    .toBuffer({ resolveWithObject: true })
                    .then(({ data, info }) => {
                        this.#imgD = data;
                        this.#imgInfo = info;
                        this.emit("imgseqInfo", { loadImg: true, info: info, img: data, ptime: Date.now() - t0 });
                    })
                    .catch(err => {
                        this.emit("imgseqError", err);
                        return;
                    })
                break;
        }
    }

    /**
     * 
     */
    processImg() {
        console.log("imgseq.processImg> ");
        let t0 = Date.now();
        try {
            const result = myAddon.process(
                this.#imgD,
                this.#imgInfo.height,
                this.#imgInfo.width,
                this.#imgInfo.channels
            );
            console.log("imgseq.processImg> myAddon.process() retruned:", result);
            this.emit("imgseqInfo", { processImg: true, ptime: Date.now() - t0 });
        } catch (error) {
            console.log("imgseq.processImg> Error: ", error);
        }
    }

}

// Nur was an module.exports hängen wird, ist außen sichtbar
module.exports = new imgseq();
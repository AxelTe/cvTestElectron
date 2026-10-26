/**
 *
 */
class mouseOnCanvas {
    #container = undefined;
    #cvin = undefined;
    #cvout = undefined;
    #histCV = undefined;
    #chart = undefined;
    #magCtx = undefined;
    #zoomLevel = 5.0;
    #magSize = 150.0;
    #sourceSize = this.#magSize / this.#zoomLevel;
    #contourBtn = undefined;
    #magnifierBtn = undefined;
    #gradDirBtns = [];
    #gradDirFields = [];
    #handleMouseMove = null;
    #backBtn = undefined;
    #gradBtn = undefined;
    #gradInfoEl = undefined;
    #mouseX = 0;
    #mouseY = 0;


    constructor(container, cvin, cvout, gradInfo, histogram, imgDisp) {
        // canvas object
        this.#container = container;
        this.#cvin = cvin;
        this.#cvout = cvout;
        this.#magCtx = this.#cvout.getContext('2d');
        this.#gradInfoEl = gradInfo;
        this.#histCV = histogram;
        this.imgDisp = imgDisp;

        this.#container.addEventListener('mouseleave', () => {
            this.#cvout.style.display = 'none';
        });

        this.#container.addEventListener('contextmenu', (event) => this.onContextMenu(this, event));
    }


    displayGradientInfo(data, x0, y0) {
        const rect = this.#cvin.getBoundingClientRect();
        this.#gradInfoEl.style.top = rect.top+"px";
        this.#gradInfoEl.style.left = rect.x+"px";
        this.#gradInfoEl.style.display = "block";
        //this.#gradInfoEl.style.top = this.#mouseY + "px";
        //this.#gradInfoEl.style.left = this.#mouseX + "px";

        let elt, elc, bd = 10, r, c, x, y;
        let fieldNames = ["id", "x", "y", "mag", "dir", "c", "s", "p","n3x3", "curv"];
        let edges = this.imgDisp.edgeData.edges;
        //
        while (this.#gradInfoEl.firstChild) {
            this.#gradInfoEl.removeChild(this.#gradInfoEl.lastChild);
        }

        elt = this.#gradInfoEl.appendChild(document.createElement('div'));
        elt.classList.add("grad-grid-table");
        elt.style.gridTemplateColumns = "repeat(" + (2 * bd + 2) + ", 1fr)";

        // top row
        elc = elt.appendChild(document.createElement('div'));
        elc.classList.add("grad-cell");
        elc.classList.add("grad-cell-up");
        elc.innerText = "dir";
        for (c = (1 - bd); c < bd; c++) {
            x = x0 + c;
            elc = elt.appendChild(document.createElement('div'));
            elc.classList.add("grad-cell");
            elc.classList.add("grad-cell-up");
            elc.innerText = x;
        }
        elc = elt.appendChild(document.createElement('div'));
        elc.classList.add("grad-cell");
        elc.classList.add("grad-cell-up");
        elc.innerText = "";
        elc = elt.appendChild(document.createElement('div'));
        elc.classList.add("grad-cell");
        elc.classList.add("grad-cell-up");
        elc.innerText = "edgPt";

        for (r = (1 - bd); r < bd; r++) {
            y = y0 + r;
            elc = elt.appendChild(document.createElement('div'));
            elc.classList.add("grad-cell");
            elc.classList.add("grad-cell-up");
            elc.innerText = y;
        
            for (c = (1 - bd); c < bd; c++) {
                x = x0 + c;
                elc = elt.appendChild(document.createElement('div'));
                elc.classList.add("grad-cell");
                if( r === 0 && c === 0 ) elc.classList.add("grad-cell-up");
                const edgPt = edges.find(edgPt => edgPt.xi === x && edgPt.yi === y);
                if (edgPt === undefined) { elc.innerText = "0"; }
                else {
                    elc.id = x+","+y;
                    elc.innerText = edgPt.dir.toFixed(1);
                    elc.addEventListener("click", (event) => this.onGradDirCell(this, event) );
                    this.#gradDirBtns.push(elc);
                }
            }
            elc = elt.appendChild(document.createElement('div'));
            elc.classList.add("grad-cell");
            elc.classList.add("grad-cell-up");
            if( r+bd-1 < 10) elc.innerText = fieldNames[r+bd-1];
            //
            elc = elt.appendChild(document.createElement('div'));
            elc.classList.add("grad-cell");
            elc.innerText = "tbd";
            this.#gradDirFields.push(elc);

            //
        }

        console.log(data.length);
    }

    onGradDirCell(self, ev){
        console.log("onGradDirCell", ev.target.id)
        let td = ev.target.id.split(",");
        if(td.length !== 2) return;
        let x = parseInt(td[0]);
        let y = parseInt(td[1]);
        let edges = self.imgDisp.edgeData.edges;
        const edgPt = edges.find(edgPt => edgPt.xi === x && edgPt.yi === y);
        const id = edges.findIndex(edgPt => edgPt.xi === x && edgPt.yi === y);
        if (edgPt === undefined) return;
        this.#gradDirFields[0].innerText = id+1;
        this.#gradDirFields[1].innerText = edgPt.xi;
        this.#gradDirFields[2].innerText = edgPt.yi;
        this.#gradDirFields[3].innerText = edgPt.mag.toFixed(1);
        this.#gradDirFields[4].innerText = edgPt.dir.toFixed(1);
        this.#gradDirFields[5].innerText = edgPt.c;
        this.#gradDirFields[6].innerText = edgPt.s;
        this.#gradDirFields[7].innerText = edgPt.p;
        this.#gradDirFields[8].innerText = edgPt.n3x3;
        this.#gradDirFields[9].innerText = edgPt.curv.toFixed(2);
        //console.log( edgPt );
                
                
    }
    /**
     * 
     * @param {*} self 
     * @param {*} e 
     */
    onContextMenu(self, e) {
        //
        this.#gradInfoEl.style.display = "none";
        this.#histCV.style.display = 'none';
        // Mausposition relativ zum Canvas berechnen
        const rect = self.#container.getBoundingClientRect();
        self.#mouseX = e.clientX - rect.left;
        self.#mouseY = e.clientY - rect.top;
        console.log("onContextMenu", self.#mouseX, self.#mouseY);
        let el = document.getElementById("mouseCtx");
        el.style.display = "flex";
        el.style.top = self.#mouseY + "px";
        el.style.left = self.#mouseX + "px";
        //
        this.#container.removeEventListener('mousemove', this.#handleMouseMove);
        this.#handleMouseMove = null;
        //
        for(let i=0; i<this.#gradDirBtns.length; i++){
            this.#gradDirBtns[i].removeEventListener("click", this.onGradDirCell);
        }
        this.#gradDirBtns = [];
        this.#gradDirFields = [];
        //
        self.#contourBtn = el.appendChild(document.createElement('button'));
        self.#contourBtn.innerText = "edges - contours - direction";
        self.#contourBtn.addEventListener("click", (event) => self.onContourBtn(self, event));
        //
        self.#magnifierBtn = el.appendChild(document.createElement('button'));
        self.#magnifierBtn.innerText = "magnify";
        self.#magnifierBtn.addEventListener("click", (event) => self.onMagnifierBtn(self, event));
        //
        self.#gradBtn = el.appendChild(document.createElement('button'));
        self.#gradBtn.innerText = "gradient";
        self.#gradBtn.addEventListener("click", (event) => self.onGradBtn(self.#mouseX, self.#mouseY, self, event));
        //
        self.#contourBtn = el.appendChild(document.createElement('button'));
        self.#contourBtn.innerText = "dir-histogram";
        self.#contourBtn.addEventListener("click", (event) => self.onDirHistBtn(self, event));
        //
        self.#contourBtn = el.appendChild(document.createElement('button'));
        self.#contourBtn.innerText = "edge-histogram";
        //self.#contourBtn.addEventListener("click", (event) => self.onEdgeHistBtn(self, event));
        self.#contourBtn.addEventListener("click", (event) => self.onCurvHistBtn(self, event));
        //
        self.#backBtn = el.appendChild(document.createElement('button'));
        self.#backBtn.innerText = "back";
        self.#backBtn.addEventListener("click", (event) => self.offContextMenu(self, event));
    }

    onContourBtn(self, event) {
        console.log("onContourBtn");
        self.#handleMouseMove = (event) => this.onMouseMove(this, event);

        self.imgDisp.config.edgeMode = (self.imgDisp.config.edgeMode + 1) % 4;
        switch (self.imgDisp.config.edgeMode) {
            case 3:
                self.imgDisp.lines();
                break;
            case 2:
                self.imgDisp.directions();
                break;
            case 1:
                self.imgDisp.curvature();
                break;
            default:
                self.imgDisp.edges();
                break;
        }
        //this.#container.addEventListener('mousemove', self.#handleMouseMove);
        this.offContextMenu(self, event);
    }


    onMagnifierBtn(self, event) {
        console.log("onMagnifierBtn");
        self.#handleMouseMove = (event) => this.onMouseMove(this, event);
        this.#container.addEventListener('mousemove', self.#handleMouseMove);
        this.offContextMenu(self, event);
    }

    onGradBtn(x, y, self, event) {
        console.log("onGradBtn");
        let x0 = Math.round(x * (self.#cvin.width / self.#cvin.clientWidth));
        let y0 = Math.round(y * (self.#cvin.height / self.#cvin.clientHeight));
        self.displayGradientInfo(self.imgDisp.edgeData, x0, y0);
        this.offContextMenu(self, event);
    }

    onCurvHistBtn(self, event) {
        console.log("onCurvHistBtn");
        //
        let edges = self.imgDisp.edgeData.edges;
        let conts = self.imgDisp.edgeData.conts;
        let i, i0, i1, curv, cnt=0;
        let hist = new Array(1000);
        let labels = new Array(1000);
        //
        hist.fill(0);
        for (i = 0; i < 1000; i++) labels[i] = 1*i/1000;
        for( i=0; i<conts.length; i++){
            if (conts[i].nofEdges > 1) {
                i0 = conts[i].start;
                i1 = conts[i].end;
                curv = Math.floor(1000*edges[i0-1].curv);
                curv = curv >= 1000 ? 999 : curv;
                hist[curv]++;
                while( edges[i0-1].s > 0){
                    i0 = edges[i0-1].s;
                     curv = Math.floor(1000*edges[i0-1].curv);
                    curv = curv >= 1000 ? 999 : curv;
                    hist[curv]++;
                    if( i1 === i0 ) break;
                }
            }
        }
        cnt = 0;
        for (i = 0; i < 1000; i++) cnt += hist[i];
        if(cnt>0) for (i = 0; i < 1000; i++) hist[i] = hist[i]/cnt;
        
        //
        const rect = this.#cvin.getBoundingClientRect();
        this.#histCV.style.top = rect.top+"px";
        let xoff = (rect.width - this.#histCV.width)/2;
        xoff = xoff > 0 ? xoff : 0;
        this.#histCV.style.left = (rect.x + xoff)+"px";
        this.#histCV.style.display = 'block';

        let datasets = [
            {
                data: hist,
                fill: '1',
                backgroundColor: 'rgba(0,149,237, 0.2)',
                borderColor: 'rgb(0,160,237, 0.3)',
                pointBackgroundColor: 'rgba(0,149,237, 0.0)',
                pointBorderColor: 'rgba(0,149,237, 0.0)',
                pointHoverBackgroundColor: '#fff',
                pointHoverBorderColor: 'rgb(255, 99, 132)'
            }
        ]
        let data = { labels: labels, datasets: datasets };

        let chartConfig = {
            type: 'line',
            data: data,
            options: {  plugins: {  legend: {  display: false  } },
                        elements: { line: { borderWidth: 3 } },
                        responsive: false,
                        maintainAspectRatio: false,
            }
        };
        if(self.#chart !== undefined){
            self.#chart.destroy();
        }
        self.#chart = new Chart(self.#histCV.getContext("2d"), chartConfig);
        //
        this.offContextMenu(self, event);
    }

    onDirHistBtn(self, event) {
        console.log("onDirHistBtn");
        //
        let edges = self.imgDisp.edgeData.edges;
        let conts = self.imgDisp.edgeData.conts;
        let i, i0, i1, dir, cnt=0;
        let hist = new Array(360);
        let labels = new Array(360);
        //
        hist.fill(0);
        for (i = 0; i < 360; i++) labels[i] = i;
        for( i=0; i<conts.length; i++){
            if (conts[i].nofEdges > 1) {
                i0 = conts[i].start;
                i1 = conts[i].end;
                dir = Math.floor(edges[i0-1].dir);
                hist[dir]++;
                while( edges[i0-1].s > 0){
                    i0 = edges[i0-1].s;
                    dir = Math.floor(edges[i0-1].dir);
                    hist[dir]++;
                    if( i1 === i0 ) break;
                }
            }
        }
        cnt = 0;
        for (i = 0; i < 360; i++) cnt += hist[i];
        if(cnt>0) for (i = 0; i < 360; i++) hist[i] = hist[i]/cnt;
        
        //
        const rect = this.#cvin.getBoundingClientRect();
        this.#histCV.style.top = rect.top+"px";
        let xoff = (rect.width - this.#histCV.width)/2;
        xoff = xoff > 0 ? xoff : 0;
        this.#histCV.style.left = (rect.x + xoff)+"px";
        this.#histCV.style.display = 'block';

        let datasets = [
            {
                data: hist,
                fill: '1',
                backgroundColor: 'rgba(0,149,237, 0.2)',
                borderColor: 'rgb(0,160,237, 0.3)',
                pointBackgroundColor: 'rgba(0,149,237, 0.0)',
                pointBorderColor: 'rgba(0,149,237, 0.0)',
                pointHoverBackgroundColor: '#fff',
                pointHoverBorderColor: 'rgb(255, 99, 132)'
            }
        ]
        let data = { labels: labels, datasets: datasets };

        let chartConfig = {
            type: 'line',
            data: data,
            options: {  plugins: {  legend: {  display: false  } },
                        elements: { line: { borderWidth: 3 } },
                        responsive: false,
                        maintainAspectRatio: false,
            }
        };
        if(self.#chart !== undefined){
            self.#chart.destroy();
        }
        self.#chart = new Chart(self.#histCV.getContext("2d"), chartConfig);
        //
        this.offContextMenu(self, event);
    }

    /**
     * 
     * @param {*} self 
     * @param {*} event 
     */
    onEdgeHistBtn(self, event) {
        console.log("onDirHistBtn");
        //
        let i, i0, i1, dir, cnt=0;
        let hist = self.imgDisp.edgeData.edgHist;
        let labels = new Array(hist.length);
        //
        for (i = 0; i < labels.length; i++) labels[i] = i;
        //
        const rect = this.#cvin.getBoundingClientRect();
        this.#histCV.style.top = rect.top+"px";
        let xoff = (rect.width - this.#histCV.width)/2;
        xoff = xoff > 0 ? xoff : 0;
        this.#histCV.style.left = (rect.x + xoff)+"px";
        this.#histCV.style.display = 'block';

        let datasets = [
            {
                data: hist,
                fill: '1',
                backgroundColor: 'rgba(0,149,237, 0.2)',
                borderColor: 'rgb(0,160,237, 0.3)',
                pointBackgroundColor: 'rgba(0,149,237, 0.0)',
                pointBorderColor: 'rgba(0,149,237, 0.0)',
                pointHoverBackgroundColor: '#fff',
                pointHoverBorderColor: 'rgb(255, 99, 132)'
            }
        ]
        let data = { labels: labels, datasets: datasets };

        let chartConfig = {
            type: 'line',
            data: data,
            options: {  plugins: {  legend: {  display: false  } },
                        elements: { line: { borderWidth: 3 } },
                        responsive: false,
                        maintainAspectRatio: false,
            }
        };
        if(self.#chart !== undefined){
            self.#chart.destroy();
        }
        self.#chart = new Chart(self.#histCV.getContext("2d"), chartConfig);
        //
        this.offContextMenu(self, event);
    }


    offContextMenu(self, e) {
        console.log("offContextMenu");
        self.#magnifierBtn = null;
        self.#backBtn = null;
        let el = document.getElementById("mouseCtx");
        el.replaceChildren();
        el.style.display = "none";
    }

    /**
     * 
     * @param {*} self 
     * @param {*} e 
     */
    async onMouseMove(self, e) {
        const rect = self.#container.getBoundingClientRect();

        // Mausposition relativ zum Canvas berechnen
        const mouseX = e.clientX - rect.left;
        const mouseY = e.clientY - rect.top;

        // Prüfen, ob die Maus innerhalb des Canvas liegt
        if (mouseX >= 0 && mouseX <= self.#cvin.width && mouseY >= 0 && mouseY <= self.#cvin.height) {
            self.#cvout.style.display = 'block';

            // Position der Lupe zentriert über den Mauszeiger legen
            self.#cvout.style.left = `${e.clientX - self.#magSize / 2}px`;
            self.#cvout.style.top = `${e.clientY - self.#magSize / 2}px`;

            // Quellbereich berechnen, der in der Lupe angezeigt werden soll
            let sourceX = (parseFloat(mouseX * self.#cvin.width) / parseFloat(self.#cvin.clientWidth)) - (0.5 * self.#sourceSize);
            let sourceY = (parseFloat(mouseY * self.#cvin.height) / parseFloat(self.#cvin.clientHeight)) - (0.5 * self.#sourceSize);

            // console.log("---")
            //console.log(self.#cvin.width, self.#cvin.height, self.#cvin.clientWidth, self.#cvin.clientHeight)
            //console.log(e.clientX, e.clientY, mouseX, mouseY, sourceX, sourceY);

            // Lupen-Canvas leeren und Ausschnitt skaliert hineinzeichnen
            const bitmap = await createImageBitmap(self.imgDisp.imgD, sourceX, sourceY, self.#sourceSize, self.#sourceSize);
            self.#magCtx.clearRect(0, 0, self.#magSize, self.#magSize);
            self.#magCtx.drawImage(bitmap, 0, 0, self.#magSize, self.#magSize);
            //
            if (self.imgDisp.edgeData !== null) {
                let xf, yf;
                let radius = 1.0;
                let fillStyle = self.#magCtx.fillStyle;
                let strokeStyle = self.#magCtx.strokeStyle;
                let edges = self.imgDisp.edgeData.edges;
                //
                //
                self.#magCtx.strokeStyle = "#0000FF"
                self.#magCtx.beginPath();
                for (let i = 0; i < edges.length; i++) {
                    if (edges[i].s > 0) {
                        xf = edges[i].xf - sourceX + 1.5; // constant 1.5 ???
                        yf = edges[i].yf - sourceY + 1.5; // constant 1.5 ???
                        xf = (xf * self.#zoomLevel);
                        yf = (yf * self.#zoomLevel);
                        let distance = Math.sqrt(Math.pow(xf, 2) + Math.pow(yf, 2));
                        if (distance < self.#magSize) {
                            self.#magCtx.moveTo((xf), (yf));
                            xf = edges[edges[i].s - 1].xf - sourceX + 1.5; // constant 1.5 ???
                            yf = edges[edges[i].s - 1].yf - sourceY + 1.5; // constant 1.5 ???
                            xf = (xf * self.#zoomLevel);
                            yf = (yf * self.#zoomLevel);
                            self.#magCtx.lineTo((xf), (yf));
                        }
                    }
                }
                self.#magCtx.stroke();

                //
                radius = 1.0;
                for (let i = 0; i < edges.length; i++) {
                    if (edges[i].s > 0) {
                        xf = edges[i].xi - sourceX + 1.5; // constant 1.5 ???
                        yf = edges[i].yi - sourceY + 1.5; // constant 1.5 ???
                        xf = (xf * self.#zoomLevel);
                        yf = (yf * self.#zoomLevel);
                        let distance = Math.sqrt(Math.pow(xf, 2) + Math.pow(yf, 2));
                        if (distance < self.#magSize) {
                            let val = parseInt(100.0*edges[i].curv);
                            val = val > 255 ? 255 : val;
                            self.#magCtx.beginPath();
                            self.#magCtx.fillStyle = "#"+toHex(val)+toHex(255-val)+"00";
                            self.#magCtx.arc((xf), (yf), radius, 0, Math.PI * 2);
                            self.#magCtx.fill();
                        }
                    }
                }
                //

                //
                radius = 1.5;
                self.#magCtx.fillStyle = "#FF0000"
                self.#magCtx.beginPath();
                for (let i = 0; i < edges.length; i++) {
                    if (edges[i].s == 0 && edges[i].p > 0) {
                        xf = edges[i].xi - sourceX + 1.5; // constant 1.5 ???
                        yf = edges[i].yi - sourceY + 1.5; // constant 1.5 ???
                        xf = (xf * self.#zoomLevel);
                        yf = (yf * self.#zoomLevel);
                        let distance = Math.sqrt(Math.pow(xf, 2) + Math.pow(yf, 2));
                        if (distance < self.#magSize) {
                            self.#magCtx.moveTo((xf), (yf));
                            self.#magCtx.arc((xf), (yf), radius, 0, Math.PI * 2);
                        }
                    }
                }
                self.#magCtx.fill();
                //
                self.#magCtx.fillStyle = "#00FF00"
                self.#magCtx.beginPath();
                for (let i = 0; i < edges.length; i++) {
                    if (edges[i].p == 0 && edges[i].s > 0) {
                        xf = edges[i].xi - sourceX + 1.5; // constant 1.5 ???
                        yf = edges[i].yi - sourceY + 1.5; // constant 1.5 ???
                        xf = (xf * self.#zoomLevel);
                        yf = (yf * self.#zoomLevel);
                        let distance = Math.sqrt(Math.pow(xf, 2) + Math.pow(yf, 2));
                        if (distance < self.#magSize) {
                            self.#magCtx.moveTo((xf), (yf));
                            self.#magCtx.arc((xf), (yf), radius, 0, Math.PI * 2);
                        }
                    }
                }
                self.#magCtx.fill();
                //
                self.#magCtx.fillStyle = fillStyle;
                self.#magCtx.strokeStyle = strokeStyle;
            }
        } else {
            self.#cvout.style.display = 'none';
        }
    };
}
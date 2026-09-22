/**
 *
 */
class mouseOnCanvas {
    #container = undefined;
    #cvin = undefined;
    #cvout = undefined;
    #magCtx = undefined;
    #zoomLevel = 5.0;
    #magSize = 150;
    #sourceSize = this.#magSize / this.#zoomLevel;
    #magnifierBtn = undefined;
    #handleMouseMove = null;
    #backBtn = undefined;
    #gradBtn = undefined;
    #gradInfoEl = undefined;
    #mouseX = 0;
    #mouseY = 0;


    constructor(container, cvin, cvout, gradInfo) {
        // canvas object
        this.#container = container;
        this.#cvin = cvin;
        this.#cvout = cvout;
        this.#magCtx = this.#cvout.getContext('2d');
        this.#gradInfoEl = gradInfo;


        this.#container.addEventListener('mouseleave', () => {
            this.#cvout.style.display = 'none';
        });


        this.#container.addEventListener('contextmenu', (event) => this.onContextMenu(this, event));
    }

    displayGradientInfo(data, x0, y0) {
        this.#gradInfoEl.style.display = "inline";
        this.#gradInfoEl.style.top = this.#mouseY + "px";
        this.#gradInfoEl.style.left = this.#mouseX + "px";

        let nofR = Math.round(Math.sqrt(data.length / 2));
        let elt, elc, r, c;

        //
        while (this.#gradInfoEl.firstChild) {
            this.#gradInfoEl.removeChild(this.#gradInfoEl.lastChild);
        }

        elt = this.#gradInfoEl.appendChild(document.createElement('div'));
        elt.classList.add("grad-grid-table");

        for (r = 0; r < nofR; r++) {
            for (c = 0; c < nofR; c++) {
                elc = elt.appendChild(document.createElement('div'));
                elc.classList.add("grad-cell");
                elc.innerText = data[r * (2 * nofR) + (2 * c)].toFixed(1);
            }
            elc = elt.appendChild(document.createElement('div'));
            elc.classList.add("grad-cell");

            for (c = 0; c < nofR; c++) {
                elc = elt.appendChild(document.createElement('div'));
                elc.classList.add("grad-cell");
                elc.innerText = data[r * (2 * nofR) + (2 * c + 1)].toFixed(1);
            }
        }

        console.log(data.length);
    }

    /**
     * 
     * @param {*} self 
     * @param {*} e 
     */
    onContextMenu(self, e) {
        //
        this.#gradInfoEl.style.display = "none";
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
        self.#magnifierBtn = el.appendChild(document.createElement('button'));
        self.#magnifierBtn.innerText = "magnify";
        self.#magnifierBtn.addEventListener("click", (event) => self.onMagnifierBtn(self, event));
        //
        self.#gradBtn = el.appendChild(document.createElement('button'));
        self.#gradBtn.innerText = "gradient";
        self.#gradBtn.addEventListener("click", (event) => self.onGradBtn(self.#mouseX, self.#mouseY, self, event));
        //
        self.#backBtn = el.appendChild(document.createElement('button'));
        self.#backBtn.innerText = "back";
        self.#backBtn.addEventListener("click", (event) => self.offContextMenu(self, event));

    }

    onMagnifierBtn(self, event) {
        console.log("onMagnifierBtn");
        self.#handleMouseMove = (event) => this.onMouseMove(this, event);
        this.#container.addEventListener('mousemove', self.#handleMouseMove);
        this.offContextMenu(self, event);
    }

    onGradBtn(x, y, self, event) {
        console.log("onGradBtn");
        let x0 = x * (self.#cvin.width / self.#cvin.clientWidth);
        let y0 = y * (self.#cvin.height / self.#cvin.clientHeight);
        let requestGradientInfoEvent = new CustomEvent('requestGradientInfo', {
            bubbles: true,
            detail: { msg: { x0: x0, y0: y0 } }
        });
        self.#cvin.dispatchEvent(requestGradientInfoEvent);
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
    onMouseMove(self, e) {
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
            const sourceX = (mouseX * (self.#cvin.width / self.#cvin.clientWidth)) - self.#sourceSize / 2;
            const sourceY = (mouseY * (self.#cvin.height / self.#cvin.clientHeight)) - self.#sourceSize / 2;

            // console.log("---")
            // console.log(self.#cvin.width, self.#cvin.height, self.#cvin.clientWidth, self.#cvin.clientHeight)
            // console.log(e.clientX, e.clientY, mouseX, mouseY, sourceX, sourceY);

            // Lupen-Canvas leeren und Ausschnitt skaliert hineinzeichnen
            self.#magCtx.clearRect(0, 0, self.#magSize, self.#magSize);
            self.#magCtx.drawImage(
                self.#cvin,
                sourceX, sourceY, self.#sourceSize, self.#sourceSize, // Quell-Ausschnitt
                0, 0, self.#magSize, self.#magSize                       // Ziel-Position & Größe
            );
        } else {
            self.#cvout.style.display = 'none';
        }
    };
}
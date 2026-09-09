/**
 *
 */
class mouseOnCanvas {
    #container = undefined;
    #cvin = undefined;
    #cvout = undefined;
    #magCtx = undefined;
    #zoomLevel = 2.5;
    #magSize = 150;
    #sourceSize = this.#magSize / this.#zoomLevel;


    constructor(container, cvin, cvout) {
        // canvas object
        this.#container = container;
        this.#cvin = cvin;
        this.#cvout = cvout;
        this.#magCtx = this.#cvout.getContext('2d');

        this.#container.addEventListener('mouseleave', () => {
            this.#cvout.style.display = 'none';
        });

        this.#container.addEventListener('mousemove', (event) => this.onMouseMove(this, event));

        
    }

    
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
            const sourceX = (mouseX * (self.#cvin.width/self.#cvin.clientWidth)) - self.#sourceSize / 2;
            const sourceY = (mouseY * (self.#cvin.height/self.#cvin.clientHeight)) - self.#sourceSize / 2;

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
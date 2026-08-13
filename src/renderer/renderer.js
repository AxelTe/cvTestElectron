// 1. Target the root element (<html>)
const root = document.documentElement;
// 2. Get the computed styles for that element
const styles = getComputedStyle(root);
// 3. Extract the specific variable value
const color1 = styles.getPropertyValue('--color1').trim();
const color2 = styles.getPropertyValue('--color2').trim();
const color3 = styles.getPropertyValue('--color3').trim();
const color4 = styles.getPropertyValue('--color4').trim();

const infoId = document.getElementById('info')
let cv = document.getElementById('incv');
let tid = document.getElementById('mainFrame')
let imgDisp = new imgdisp(cv, tid.clientWidth, tid.clientHeight);
let ptimes = new Array(10);
for(let i=0; i<ptimes.length; i++){
  ptimes[i] = {min: 1000, max:0, val: 0};
}

//
// button function
//---------------------------------------------
//
let inputBtn = document.getElementById('InputBtn');
let runBtn = document.getElementById('runBtn');
let lastBtn = document.getElementById('lastBtn');
let nextBtn = document.getElementById('nextBtn');
let run = 0;

inputBtn.addEventListener('click', async () => {
  // Ruft die Funktion aus dem Preload-Skript auf
  const paths = await window.electronAPI.selectFiles();
  console.log('Ausgewählte Dateien:', paths);
});

runBtn.addEventListener('click', async () => {
  // Ruft die Funktion aus dem Preload-Skript auf
  if (run === 0) {
    runBtn.style.backgroundColor = color3;
    runBtn.style.color = color1;
    runBtn.innerText = "\u23F8";
    run = 2;
    imgDisp.startDisplayQueue();
  } else {
    if (run === 2) {
      runBtn.style.backgroundColor = color4;
      runBtn.style.color = color1;
      runBtn.innerText = "\u25B6";
      run = 0;
      imgDisp.stopDisplayQueue();
    }
  }
});

lastBtn.addEventListener('click', async () => {
  // Ruft die Funktion aus dem Preload-Skript auf
  if (run === 2) {
    runBtn.style.backgroundColor = color4;
    runBtn.style.color = color1;
    runBtn.innerText = "\u25B6";
    run = 0;
    imgDisp.stopDisplayQueue();
  }
  imgDisp.id = imgDisp.id-2;
  if(imgDisp.id<0) imgDisp = 0;
  imgDisp.loadImgFrommRoot();
});

nextBtn.addEventListener('click', async () => {
  // Ruft die Funktion aus dem Preload-Skript auf
  if (run === 2) {
    runBtn.style.backgroundColor = color4;
    runBtn.style.color = color1;
    runBtn.innerText = "\u25B6";
    run = 0;
    imgDisp.stopDisplayQueue();
  }
  imgDisp.loadImgFrommRoot();
});

resetBtn.addEventListener('click', async () => {
  // Ruft die Funktion aus dem Preload-Skript auf
  if (run === 2) {
    runBtn.style.backgroundColor = color4;
    runBtn.style.color = color1;
    runBtn.innerText = "\u25B6";
    run = 0;
    imgDisp.stopDisplayQueue();
  }
  imgDisp.id = 0;
  imgDisp.loadImgFrommRoot();
});
//
//
//---------------------------------------------
//

window.electronAPI.onFehler((meldung) => {
  alert("Achtung: " + meldung);
  // Oder zeige einen schicken Toast-Banner
});

window.electronAPI.onInfo((msg) => {
  if (msg.hasOwnProperty("seqinfo")) {
    let el = document.getElementById("DispBar");
    el.min = msg.seqinfo.start;
    el.max = msg.seqinfo.end;
    imgDisp.seqInfo(msg.seqinfo);
  }

  if (msg.hasOwnProperty("seqname")) {
    const el = document.getElementById("footCenter");
    el.innerText = msg.seqname;
  }

  if (msg.hasOwnProperty("img")) {
    let el = document.getElementById("DispBar");
    el.value = imgDisp.id;
    ptimes[0].min =  ptimes[0].min > msg.ptime ? msg.ptime : ptimes[0].min;
    ptimes[0].max =  ptimes[0].max < msg.ptime ? msg.ptime : ptimes[0].max;
    ptimes[0].val = msg.ptime;
    el = document.getElementById("footLeft");
    el.innerText = ptimes[0].min+".."+ptimes[0].val+".."+ptimes[0].max;
    imgDisp.dispImg(msg.img);
  }

});

/**
 * 
 */
cv = document.getElementById('incv');

cv.addEventListener('imgDispError', (e) => {
  alert('imgDispError:' + " " + e.detail.msg);
});

cv.addEventListener('loadFromRoot', (e) => {
  // console.log('loadFromRoot'+" "+e.detail.msg);
  window.electronAPI.loadFromRoot(e.detail.msg);
});
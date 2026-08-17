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

//
//
//---------------------------------------------
//

window.electronAPI.onPyramidInfo((msg) => {
  if (msg.hasOwnProperty("seqinfo")) {
    console.log("seqInfo");
    imgDisp.seqInfo(msg.seqinfo);
  }

  if (msg.hasOwnProperty("img")) {
    console.log("img");
    let img = msg.img;
    let rows = msg.rows;
    let cols = msg.cols;
    let channels = msg.channels;   
    imgDisp.dispImg(img, rows, cols, []);
  }
});

/**
 * 
 */



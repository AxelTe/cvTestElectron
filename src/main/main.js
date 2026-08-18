const { app, Menu, BrowserWindow, ipcMain, shell } = require('electron')
const { dialog } = require('electron');
const path = require('node:path');
const fs = require('node:fs');
require('log-timestamp');

let configFile = path.join(__dirname, '../main/config.json');
console.log(configFile);
let config = JSON.parse(fs.readFileSync(configFile));
let imgseq = require('../main/imgseq.js');
let mainWin = undefined;
let imgPyramidWin = undefined;


//
// 
const createWindow = () => {
  mainWin = new BrowserWindow({
    width: 800,
    height: 600,
    icon: path.join(__dirname, '../renderer/logo1.png'),
    webPreferences: {
      preload: path.join(__dirname, '../preload/preload.js')
    }
  });
  //win.maximize();
  mainWin.loadFile('src/renderer/index.html')

  mainWin.webContents.on('did-finish-load', () => {
    console.log("main browser window ready");
    imgseq.start(config.imgseq);
  });

  mainWin.on("close", () => {
    console.log("main browser window closes");
    if( imgPyramidWin !== undefined){
      imgPyramidWin.close();
      imgPyramidWin = undefined;
    }
  });
}


//
// 
const createPyramidWin = () => {
  /* */
  if (imgPyramidWin !== undefined) {
    imgPyramidWin.focus();
    return;
  }
  imgPyramidWin = new BrowserWindow({
    //width: 800,
    //height: 600,
    icon: path.join(__dirname, '../renderer/logo1.png'),
    webPreferences: {
      preload: path.join(__dirname, '../preload/preload.js')
    }
  });
  //win.maximize();
  imgPyramidWin.loadFile('src/renderer/indexPyramidWin.html');

  imgPyramidWin.on("close", () => {
    console.log("imgPyramidWin browser window closes");
    imgPyramidWin = undefined;
  });

}

// 
//
//
app.whenReady().then(() => {
  console.log("on: whenReady");
  console.log(`=== Main Process PID: ${process.pid} ===`);
  console.log(__dirname);

  // Menü registrieren
  /*
  const menu = Menu.buildFromTemplate(template);
  Menu.setApplicationMenu(menu);
  */

  ipcMain.handle('ping', () => {
    console.log("on: ping");
    return ({
      version: app.getVersion(),
      pont: 'pong'
    })
  })

  createWindow();
  createPyramidWin();

})


app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit()
})

//
// Listener für die Dateiauswahl
ipcMain.handle('dialog:openFile', async () => {
  const { canceled, filePaths } = await dialog.showOpenDialog({
    properties: ['openFile', 'multiSelections'], // Erlaubt Dateien & Mehrfachauswahl
    filters: [
      { name: 'json', extensions: ['json'] }
    ]
  });

  if (canceled) {
    return [];
  } else {
    let tstr = filePaths[0];
    let tstr1 = tstr.split("\\");
    if (tstr1.length > 0) {
      tstr = tstr1[0];
      for (let i = 1; i < (tstr1.length - 1); i++) {
        tstr += "\\" + tstr1[i];
      }
      config.imgseq.path = tstr;
      config.imgseq.seqname = tstr1[tstr1.length - 1];
      imgseq.start(config.imgseq);
      try {
        fs.writeFileSync(configFile, JSON.stringify(config, null, 2));
      } catch (error) {
        sendErrorToUI("can not write " + configFile);
      }
    }
    return filePaths; // Gibt die Pfade an das Frontend zurück
  }
});


//
//
// root - renderer communication
//-------------------------------
//
/**
 * 
 * @param {string} meldung 
 */
function sendErrorToUI(msg) {
  if (mainWin !== undefined) {
    mainWin.webContents.send('globaler-fehler', msg);
  }
}

/**
 * 
 * @param {string} meldung 
 */
function sendInfoToMainGUI(msg) {
  // Wir schicken es an das aktive Fenster
  if (mainWin !== undefined) {
    mainWin.webContents.send('info', msg);
  }
}

function sendInfoToPyramidImg(msg) {
  // Wir schicken es an das aktive Fenster
  console.log("sendInfoToPyramidImg")
  if (imgPyramidWin !== undefined) {
    imgPyramidWin.webContents.send('pyramidInfo', msg);
  }
}

// Empfängt den Wert vom Renderer
ipcMain.on('loadFromRoot', (event, data) => {
  //console.log("loadFromRoot", data);
  imgseq.loadImg(data.id, data.rows, data.cols, data.id)
});


//
// 
// listeners on imgseq events
//---------------------------
//

imgseq.on("imgseqInfo", function (msg) {
  //@@@ console.log("imgseq.on()>", msg)
  if (msg.hasOwnProperty("loadTime")) {
    sendInfoToMainGUI(msg);
    imgseq.processImg();
  }
  if (msg.hasOwnProperty("processTime")) {
    sendInfoToMainGUI(msg);
    imgseq.showImg(2);
    imgseq.showImg(12);
  }
  if (msg.hasOwnProperty("disp")) {
    if(msg.disp === "main") sendInfoToMainGUI(msg);
    if(msg.disp === "pyramid") sendInfoToPyramidImg(msg);
  }
  if (msg.hasOwnProperty("seqname")) {
    sendInfoToMainGUI(msg);
    sendInfoToPyramidImg(msg);
    if(msg.hasOwnProperty("seqinfo")){
      console.log("imgseq.on()>", msg)
      mainWin.setSize(msg.seqinfo.cols, Math.round(msg.seqinfo.rows*1.1), true);
      if( imgPyramidWin != undefined) imgPyramidWin.setSize(msg.seqinfo.cols, Math.round(msg.seqinfo.rows*1.1), true);
    }
  }

});

imgseq.on("imgseqError", function (msg) {
  sendErrorToUI(`Kritischer Systemfehler: ${msg}`);
});

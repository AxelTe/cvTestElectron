const { app, Menu, BrowserWindow, ipcMain, shell } = require('electron')
const { dialog } = require('electron');
const path = require('node:path');
const fs = require('node:fs');
require('log-timestamp');

let configFile = path.join(__dirname, '../main/config.json');
console.log(configFile);
let config = JSON.parse( fs.readFileSync(configFile));
let imgseq = require('../main/imgseq.js');

const template = [
  // ... andere Menüpunkte wie File, Edit, etc.
  { role: 'fileMenu' },
  { role: 'editMenu' },
  { role: 'viewMenu' },
  { role: 'windowMenu' },
  {
    role: 'help', // Sorgt auf macOS für die richtige Position
    submenu: [
      {
        label: 'About',
        click: async () => {
          // Hier rufen wir die Versionsinfos ab
          const versionInfo = `
            App Version: ${app.getVersion()}
            Electron: ${process.versions.electron}
            Chrome: ${process.versions.chrome}
            Node.js: ${process.versions.node}
          `;

          dialog.showMessageBox({
            type: 'info',
            title: 'version:',
            detail: versionInfo,
            buttons: ['OK']
          });
        }
      },
      { type: 'separator' },
      {
        label: 'Website besuchen',
        click: async () => {
          await shell.openExternal('https://deine-website.de');
        }
      }
    ]
  }
];



//
// 
const createWindow = () => {
  const win = new BrowserWindow({
    width: 800,
    height: 600,
    icon: path.join(__dirname, '../renderer/logo1.png'),
    webPreferences: {
      preload: path.join(__dirname, '../preload/preload.js')
    }
  });
  //win.maximize();
  win.loadFile('src/renderer/index.html')

  win.webContents.on('did-finish-load', () => {
    console.log("browser window ready");
    imgseq.start(config.imgseq);

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
  const menu = Menu.buildFromTemplate(template);
  Menu.setApplicationMenu(menu);

  ipcMain.handle('ping', () => {
    console.log("on: ping");
    return ({
      version: app.getVersion(),
      pont: 'pong'
    })
  })

  createWindow()


  app.on('activate', () => {
    console.log("on: activate");
    //  if (BrowserWindow.getAllWindows().length === 0) {
    //    createWindow()
    //  }
  })

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
        sendErrorToUI("can not write "+configFile);
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
function sendErrorToUI(meldung) {
  // Wir schicken es an das aktive Fenster
  const win = BrowserWindow.getFocusedWindow();
  if (win) {
    win.webContents.send('globaler-fehler', meldung);
  }
}

/**
 * 
 * @param {string} meldung 
 */
function sendInfoToUI(meldung) {
  // Wir schicken es an das aktive Fenster
  const win = BrowserWindow.getFocusedWindow();
  if (win) {
    win.webContents.send('info', meldung);
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
    if (msg.hasOwnProperty("loadImg")) {
      imgseq.processImg();
      sendInfoToUI(msg);
    }
    if (msg.hasOwnProperty("processImg")) {

    }
    if (msg.hasOwnProperty("seqname")) {
      sendInfoToUI(msg);
    }

  });

  imgseq.on("imgseqError", function (msg) {
    sendErrorToUI(`Kritischer Systemfehler: ${msg}`);
  });

const { contextBridge, ipcRenderer } = require('electron')

console.log("hello preload");
    
contextBridge.exposeInMainWorld('versions', {
  node: () => process.versions.node,
  chrome: () => process.versions.chrome,
  electron: () => process.versions.electron,
  ping: () => ipcRenderer.invoke('ping')
  // we can also expose variables, not just functions
});

contextBridge.exposeInMainWorld('electronAPI', {
  selectFiles: () => ipcRenderer.invoke('dialog:openFile'),
  onFehler: (callback) => ipcRenderer.on('globaler-fehler', (event, msg) => callback(msg)),
  onInfo: (callback) => ipcRenderer.on('info', (event, msg) => callback(msg)),
  loadFromRoot: (msg) => ipcRenderer.send('loadFromRoot', msg)
});


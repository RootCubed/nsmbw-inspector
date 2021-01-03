const electron = require("electron");

function createWindow() {
    let win = new electron.BrowserWindow({
        width: 1600,
        height: 900,
        webPreferences: {
            nodeIntegration: true
        }
    });
    win.loadFile("index.html");
}

electron.app.whenReady().then(createWindow);

electron.app.on("window-all-closed", () => {
    electron.app.quit();
});
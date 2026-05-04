const { app, BrowserWindow, Menu } = require('electron');
const path = require('path');

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 800,
    minHeight: 600,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'electron-preload.js')
    },
    icon: path.join(__dirname, 'assets/icon.ico'), // опционально
    title: 'Пекарня - Система управления',
    backgroundColor: '#fdf8ed'
  });

  // Загружаем index.html
  mainWindow.loadFile('index.html');

  // Убираем стандартное меню (или кастомизируй)
  Menu.setApplicationMenu(null);

  // Открываем DevTools в разработке (закомментируй при сборке)
  // mainWindow.webContents.openDevTools();

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});
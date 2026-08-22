const { app, BrowserWindow, ipcMain, globalShortcut } = require('electron');
const path = require('path');
const fs = require('fs');

// 한글 윈도우 계정명 경로 캐시 접근 오류 방지: 프로젝트 로컬 .userData 디렉터리 사용
const userDataPath = path.resolve(__dirname, '../.userData');
if (!fs.existsSync(userDataPath)) {
  fs.mkdirSync(userDataPath, { recursive: true });
}
app.setPath('userData', userDataPath);
app.setPath('appData', userDataPath);

let mainWindow = null;

function createWindow() {
  const preloadScriptPath = path.resolve(__dirname, 'preload.js');
  console.log('>>> [Main Process] Loading Preload script from:', preloadScriptPath);

  mainWindow = new BrowserWindow({
    width: 1920,
    height: 1040,
    minWidth: 1280,
    minHeight: 768,
    frame: false,             // OS 기본 타이틀바 및 테두리 제거 (Frameless)
    backgroundColor: '#0b0f19',
    show: false,              // 준비 완료 시 표출
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      sandbox: false,
      preload: preloadScriptPath
    }
  });

  // index.html 로드
  mainWindow.loadFile(path.resolve(__dirname, '../index.html'));

  // 팝업 창(window.open) 생성 시에도 동일한 Frameless 및 다크 테마 적용
  mainWindow.webContents.setWindowOpenHandler(({ url }) => {
    console.log('>>> [Main Process] Opening Frameless Popup Window for:', url);
    return {
      action: 'allow',
      overrideBrowserWindowOptions: {
        frame: false,
        autoHideMenuBar: true,
        backgroundColor: '#0b0f19',
        webPreferences: {
          nodeIntegration: false,
          contextIsolation: true,
          sandbox: false,
          preload: preloadScriptPath
        }
      }
    };
  });

  // 부드러운 로딩 표출
  mainWindow.once('ready-to-show', () => {
    mainWindow.show();
    console.log('>>> [Electron Main] Frameless MainWindow is now visible.');
  });


  // 최대화 상태 변화 알림
  mainWindow.on('maximize', () => {
    if (mainWindow && mainWindow.webContents) {
      mainWindow.webContents.send('window-maximize-changed', true);
    }
  });
  mainWindow.on('unmaximize', () => {
    if (mainWindow && mainWindow.webContents) {
      mainWindow.webContents.send('window-maximize-changed', false);
    }
  });

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

// IPC 통신 핸들러 등록 (멀티 윈도우 / 팝업 지원)
ipcMain.on('window-minimize', (event) => {
  const win = BrowserWindow.fromWebContents(event.sender) || mainWindow;
  console.log('>>> [IPC Received] window-minimize on window id:', win ? win.id : 'unknown');
  if (win) win.minimize();
});

ipcMain.on('window-maximize', (event) => {
  const win = BrowserWindow.fromWebContents(event.sender) || mainWindow;
  console.log('>>> [IPC Received] window-maximize on window id:', win ? win.id : 'unknown');
  if (win) {
    if (win.isMaximized()) {
      win.unmaximize();
    } else {
      win.maximize();
    }
  }
});

ipcMain.on('window-close', (event) => {
  const win = BrowserWindow.fromWebContents(event.sender) || mainWindow;
  console.log('>>> [IPC Received] window-close on window id:', win ? win.id : 'unknown');
  if (win) win.close();
});

ipcMain.handle('window-is-maximized', (event) => {
  const win = BrowserWindow.fromWebContents(event.sender) || mainWindow;
  return win ? win.isMaximized() : false;
});


// 앱 생명주기 관리
app.whenReady().then(() => {
  createWindow();

  // F12 개발자 도구 토글 단축키 등록
  globalShortcut.register('F12', () => {
    if (mainWindow) {
      mainWindow.webContents.toggleDevTools();
    }
  });

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on('will-quit', () => {
  globalShortcut.unregisterAll();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});


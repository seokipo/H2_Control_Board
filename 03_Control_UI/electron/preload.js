const { contextBridge, ipcRenderer, webFrame } = require('electron');

// 1. contextBridge를 통한 전역 API 노출
contextBridge.exposeInMainWorld('electronAPI', {
  isElectron: true,
  minimizeWindow: () => ipcRenderer.send('window-minimize'),
  maximizeWindow: () => ipcRenderer.send('window-maximize'),
  closeWindow: () => ipcRenderer.send('window-close'),
  isMaximized: () => ipcRenderer.invoke('window-is-maximized'),
  onMaximizeChange: (callback) => {
    ipcRenderer.on('window-maximize-changed', (_event, isMax) => callback(isMax));
  },
  getZoomFactor: () => webFrame.getZoomFactor(),
  setZoomFactor: (factor) => webFrame.setZoomFactor(factor)
});

// 2. DOMContentLoaded 시점에 버튼 엘리먼트 직접 하드 바인딩 및 Ctrl + 마우스 휠 줌 기능 등록
window.addEventListener('DOMContentLoaded', () => {
  // 창 제어 버튼 바인딩
  const minBtn = document.getElementById('win-min-btn');
  const maxBtn = document.getElementById('win-max-btn');
  const closeBtn = document.getElementById('win-close-btn');

  if (minBtn) {
    minBtn.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      ipcRenderer.send('window-minimize');
    });
  }

  if (maxBtn) {
    maxBtn.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      ipcRenderer.send('window-maximize');
    });
  }

  if (closeBtn) {
    closeBtn.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      ipcRenderer.send('window-close');
    });
  }

  console.log('>>> [Preload] Window controls & zoom system initialized.');
});

// 3. 브라우저와 동일한 Ctrl + 마우스 휠 줌(Zoom In / Out) 및 키보드 단축키 지원
let currentZoomFactor = 1.0;

window.addEventListener('wheel', (e) => {
  if (e.ctrlKey) {
    e.preventDefault();
    if (e.deltaY < 0) {
      // 휠 위로: 확대 (최대 250%)
      currentZoomFactor = Math.min(2.5, currentZoomFactor + 0.05);
    } else if (e.deltaY > 0) {
      // 휠 아래로: 축소 (최소 50%)
      currentZoomFactor = Math.max(0.5, currentZoomFactor - 0.05);
    }
    webFrame.setZoomFactor(currentZoomFactor);
  }
}, { passive: false });

window.addEventListener('keydown', (e) => {
  if (e.ctrlKey) {
    if (e.key === '=' || e.key === '+') {
      e.preventDefault();
      currentZoomFactor = Math.min(2.5, currentZoomFactor + 0.1);
      webFrame.setZoomFactor(currentZoomFactor);
    } else if (e.key === '-' || e.key === '_') {
      e.preventDefault();
      currentZoomFactor = Math.max(0.5, currentZoomFactor - 0.1);
      webFrame.setZoomFactor(currentZoomFactor);
    } else if (e.key === '0') {
      e.preventDefault();
      currentZoomFactor = 1.0;
      webFrame.setZoomFactor(1.0);
    }
  }
});




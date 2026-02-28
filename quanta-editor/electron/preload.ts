import { contextBridge, ipcRenderer } from 'electron';

contextBridge.exposeInMainWorld('electronAPI', {
    openFile: () => ipcRenderer.invoke('dialog:openFile'),
    openDirectory: () => ipcRenderer.invoke('dialog:openDirectory'),
    readDirectory: (dirPath: string) => ipcRenderer.invoke('fs:readDirectory', dirPath),
    readFile: (filePath: string) => ipcRenderer.invoke('fs:readFile', filePath),
    saveFileAs: (content: string) => ipcRenderer.invoke('dialog:saveFileAs', content),
    saveFile: (filePath: string, content: string) => ipcRenderer.invoke('fs:saveFile', filePath, content),
    executeCompiler: (filePath: string) => ipcRenderer.invoke('exec:quanta', filePath),
    aiGenerate: (prompt: string, apiKey: string) => ipcRenderer.invoke('ai:generate', prompt, apiKey),
    fetchLeetcode: (titleSlug: string) => ipcRenderer.invoke('api:fetchLeetcode', titleSlug),

    // Terminal APIs
    terminalInput: (data: string) => ipcRenderer.send('terminal:input', data),
    resizeTerminal: (cols: number, rows: number) => ipcRenderer.send('terminal:resize', cols, rows),
    onTerminalData: (callback: (data: string) => void) => {
        ipcRenderer.on('terminal:data', (_, data: string) => callback(data));
    }
});

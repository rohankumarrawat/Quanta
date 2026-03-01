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
    aiTranslate: (code: string, targetLang: string, apiKey: string, fnName?: string) => ipcRenderer.invoke('ai:translate', code, targetLang, apiKey, fnName),
    fetchLeetcode: (titleSlug: string) => ipcRenderer.invoke('api:fetchLeetcode', titleSlug),
    submitLeetcode: (slug: string, questionId: string, lang: string, code: string, sessionCookie: string, csrfToken: string) => ipcRenderer.invoke('api:submitLeetcode', slug, questionId, lang, code, sessionCookie, csrfToken),
    checkSubmission: (submissionId: string, sessionCookie: string, csrfToken: string) => ipcRenderer.invoke('api:checkSubmission', submissionId, sessionCookie, csrfToken),
    pushToGithub: (titleSlug: string, code: string) => ipcRenderer.invoke('api:pushToGithub', titleSlug, code),

    // Terminal APIs
    terminalInput: (data: string) => ipcRenderer.send('terminal:input', data),
    resizeTerminal: (cols: number, rows: number) => ipcRenderer.send('terminal:resize', cols, rows),
    onTerminalData: (callback: (data: string) => void) => {
        ipcRenderer.on('terminal:data', (_, data: string) => callback(data));
    }
});

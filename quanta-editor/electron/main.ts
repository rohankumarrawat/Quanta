import { app, BrowserWindow, ipcMain, dialog } from 'electron';
import * as path from 'path';
import * as fs from 'fs';
import * as os from 'os';
import * as pty from 'node-pty';
import { exec } from 'child_process';

let mainWindow: BrowserWindow | null = null;
const isDev = process.env.NODE_ENV === 'development';

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1400,
        height: 900,
        minWidth: 900,
        minHeight: 600,
        // FIX: show: true so the window is ALWAYS visible on launch
        // Previously 'show: false' was hiding it when ready-to-show never fired
        show: true,
        backgroundColor: '#1e1e1e',
        // NOTE: titleBarOverlay is Windows-only and caused crashes on some setups
        // Removed icon path (no icon file exists yet in public/)
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            preload: path.join(__dirname, 'preload.js')
        },
    });

    if (isDev) {
        mainWindow.loadURL('http://localhost:5173');
        mainWindow.webContents.openDevTools();
    } else {
        // When packaged by electron-builder, __dirname is inside the asar:
        //   app.asar/dist-electron/main.js
        // So ../dist/index.html correctly points to app.asar/dist/index.html
        const indexHtml = path.join(__dirname, '../dist/index.html');
        mainWindow.loadFile(indexHtml).catch((err) => {
            // If loading fails, log the error and show a dialog so the user
            // knows something went wrong instead of seeing a blank screen
            console.error('Failed to load index.html:', err);
            dialog.showErrorBox(
                'Quanta Studio - Load Error',
                `Failed to load the editor UI.\nPath tried: ${indexHtml}\nError: ${err.message}`
            );
        });
    }

    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

app.whenReady().then(() => {
    createWindow();

    app.on('activate', () => {
        // macOS: re-create window when dock icon is clicked
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    });
});

// ─── PTY TERMINAL INTEGRATION ───────────────────────────────────────────────
const shell = os.platform() === 'win32' ? 'powershell.exe' : process.env.SHELL || 'bash';
let ptyProcess: pty.IPty | null = null;

function initPty() {
    ptyProcess = pty.spawn(shell, [], {
        name: 'xterm-color',
        cols: 80,
        rows: 24,
        cwd: process.env.HOME,
        env: process.env as any
    });

    ptyProcess.onData((data) => {
        if (mainWindow) {
            mainWindow.webContents.send('terminal:data', data);
        }
    });
}

// Initialize PTY immediately
initPty();

ipcMain.on('terminal:input', (_, data) => {
    if (ptyProcess) {
        ptyProcess.write(data);
    }
});

ipcMain.on('terminal:resize', (_, cols: number, rows: number) => {
    if (ptyProcess) {
        ptyProcess.resize(cols, rows);
    }
});

app.on('window-all-closed', () => {
    // macOS: keep app running until CMD+Q
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

// ─── IPC: Open File ──────────────────────────────────────────────────────────
ipcMain.handle('dialog:openFile', async () => {
    if (!mainWindow) return null;
    const { canceled, filePaths } = await dialog.showOpenDialog(mainWindow, {
        properties: ['openFile'],
        filters: [
            { name: 'Quanta Files', extensions: ['qnt', 'quanta'] },
            { name: 'All Files', extensions: ['*'] }
        ]
    });
    if (canceled || filePaths.length === 0) return null;

    const filePath = filePaths[0];
    const content = fs.readFileSync(filePath, 'utf-8');
    return { filePath, content, fileName: path.basename(filePath) };
});

// ─── IPC: Open Directory & File System ───────────────────────────────────────
ipcMain.handle('dialog:openDirectory', async () => {
    if (!mainWindow) return null;
    const { canceled, filePaths } = await dialog.showOpenDialog(mainWindow, {
        properties: ['openDirectory']
    });
    if (canceled || filePaths.length === 0) return null;

    const dirPath = filePaths[0];

    // Automatically cd the terminal to the new directory
    if (ptyProcess) {
        // We write the command followed by a return/enter character '\r'
        ptyProcess.write(`cd "${dirPath}"\r`);
        if (os.platform() !== 'win32') {
            ptyProcess.write('clear\r');
        } else {
            ptyProcess.write('cls\r');
        }
    }

    return dirPath; // Returns absolute path
});

interface FileNode {
    name: string;
    path: string;
    isDirectory: boolean;
    children?: FileNode[];
}

function buildFileTree(dirPath: string): FileNode[] {
    const result: FileNode[] = [];
    try {
        const items = fs.readdirSync(dirPath);
        for (const item of items) {
            if (item === 'node_modules' || item === '.git' || item === '.DS_Store') continue;

            const fullPath = path.join(dirPath, item);
            const stat = fs.statSync(fullPath);
            if (stat.isDirectory()) {
                result.push({
                    name: item,
                    path: fullPath,
                    isDirectory: true,
                    children: buildFileTree(fullPath)
                });
            } else {
                result.push({
                    name: item,
                    path: fullPath,
                    isDirectory: false
                });
            }
        }
    } catch (e) {
        console.error("Error reading dir", e);
    }
    result.sort((a, b) => {
        if (a.isDirectory === b.isDirectory) {
            return a.name.localeCompare(b.name);
        }
        return a.isDirectory ? -1 : 1;
    });
    return result;
}

ipcMain.handle('fs:readDirectory', async (_, dirPath: string) => {
    return buildFileTree(dirPath);
});

ipcMain.handle('fs:readFile', async (_, filePath: string) => {
    try {
        return fs.readFileSync(filePath, 'utf-8');
    } catch (e) {
        return null;
    }
});

// ─── IPC: Save File ───────────────────────────────────────────────────────────
ipcMain.handle('fs:saveFile', async (_, filePath: string, content: string) => {
    fs.writeFileSync(filePath, content, 'utf-8');
    return true;
});

ipcMain.handle('dialog:saveFileAs', async (_, content: string) => {
    if (!mainWindow) return null;
    const { canceled, filePath } = await dialog.showSaveDialog(mainWindow, {
        filters: [
            { name: 'Quanta Files', extensions: ['qnt', 'quanta'] }
        ]
    });
    if (canceled || !filePath) return null;

    fs.writeFileSync(filePath, content, 'utf-8');
    return { filePath, fileName: path.basename(filePath) };
});

// ─── IPC: Run Quanta Compiler ─────────────────────────────────────────────────
ipcMain.handle('exec:quanta', async (_, filePath: string) => {
    return new Promise((resolve) => {
        const home = process.env.HOME || '';
        const resourcesPath = process.resourcesPath || '';  // set by Electron when packaged
        const candidates = [
            // 1. Bundled inside the .app / installed package (highest priority)
            path.join(resourcesPath, 'quanta'),       // macOS/Linux: Contents/Resources/quanta
            path.join(resourcesPath, 'quanta.exe'),   // Windows NSIS: resources\quanta.exe
            // 2. Windows installer: quanta.exe one level above Quanta Studio folder
            path.join(path.dirname(process.execPath), '..', 'quanta.exe'),
            path.join(path.dirname(process.execPath), '..', 'quanta'),
            // 3. macOS global installs
            '/opt/homebrew/bin/quanta',
            '/usr/local/bin/quanta',
            // 4. Local dev build
            path.join(home, 'Desktop', 'Quanta', 'build', 'quanta'),
            path.join(home, 'Quanta', 'build', 'quanta'),
        ];
        let compilerPath = 'quanta';
        for (const c of candidates) {
            try { if (fs.existsSync(c)) { compilerPath = `"${path.resolve(c)}"`; break; } } catch { }
        }
        const command = `${compilerPath} "${filePath}"`;
        const env = {
            ...process.env,
            PATH: ['/opt/homebrew/bin', '/opt/homebrew/sbin', '/usr/local/bin', '/usr/bin', '/bin', process.env.PATH || ''].join(':')
        };
        exec(command, { env, cwd: path.dirname(filePath), timeout: 15000 }, (error, stdout, stderr) => {
            resolve({ error: error ? error.message : null, stdout, stderr });
        });
    });
});

// ─── IPC: AI Code Generation (Gemini) ─────────────────────────────────────────
import { GoogleGenAI } from '@google/genai';

ipcMain.handle('ai:generate', async (_, prompt: string, apiKey: string) => {
    try {
        if (!apiKey) {
            return { error: 'No Gemini API Key provided. Please configure it in settings.' };
        }

        // Find the bundled syntax.qnt file containing all rules of Quanta
        const resourcesPath = process.resourcesPath || __dirname;
        const candidates = [
            path.join(resourcesPath, 'syntax.qnt'),                 // Prod Mac/Win
            path.join(__dirname, '..', 'resources', 'syntax.qnt'),  // Dev environment
        ];

        let syntaxContext = "Quanta is a C-like programming language."; // Fallback
        for (const c of candidates) {
            if (fs.existsSync(c)) {
                syntaxContext = fs.readFileSync(c, 'utf-8');
                break;
            }
        }

        const ai = new GoogleGenAI({ apiKey: apiKey });

        const systemPrompt = `You are an expert AI code generator exclusively for the Quanta programming language. 
1. DO NOT WRAP CODE IN MARKDOWN BLOCKS (like \`\`\`quanta). Return raw text.
2. Under no circumstances should you generate code in C, C++, Python, or Rust.
3. CRITICAL LIMITATION: Quanta DOES NOT use C-style comments. INLINE AND BLOCK COMMENTS MUST EXCLUSIVELY USE \`@\` OR \`'''\`. ABSOLUTELY NEVER USE \`//\` OR \`/*\`. If you use \`//\`, the compiler will crash.
4. CRITICAL LIMITATION: DO NOT WRAP THE CODE IN A \`main()\` FUNCTION unless the user explicitly asks for it. Quanta natively supports top-level execution, so just write the raw logic or variables directly.
5. Read the following master syntax file carefully to understand how Quanta operates.

MASTER QUANTA SYNTAX:
${syntaxContext}

USER REQUEST:
${prompt}

Output ONLY valid, functional Quanta source code designed to run perfectly.`;

        const response = await ai.models.generateContent({
            model: 'gemini-2.5-flash',
            contents: systemPrompt,
        });

        // Strip markdown backticks if Gemini ignores the system prompt instructions
        let rawCode = response.text || '';
        if (rawCode.startsWith('```')) {
            const lines = rawCode.split('\n');
            if (lines.length > 2) {
                rawCode = lines.slice(1, -1).join('\n');
            }
        }

        return { code: rawCode };

    } catch (error: any) {
        return { error: error.message || 'Unknown error occurred during code generation' };
    }
});

// ─── IPC: LeetCode Integration (GraphQL) ──────────────────────────────────────
ipcMain.handle('api:fetchLeetcode', async (_, titleSlug: string) => {
    try {
        const query = `
            query questionData($titleSlug: String!) {
                question(titleSlug: $titleSlug) {
                    questionId
                    questionFrontendId
                    title
                    titleSlug
                    content
                    difficulty
                    exampleTestcases
                    topicTags {
                        name
                        slug
                    }
                    hints
                    sampleTestCase
                    metaData
                }
            }
        `;

        const response = await fetch('https://leetcode.com/graphql', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Referer': 'https://leetcode.com/',
                'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
            },
            body: JSON.stringify({
                query: query,
                variables: { titleSlug: titleSlug }
            })
        });

        if (!response.ok) {
            throw new Error(`LeetCode API returned ${response.status} ${response.statusText}`);
        }

        const data: any = await response.json();

        if (data.errors) {
            return { error: data.errors[0]?.message || 'GraphQL Error' };
        }

        if (!data.data || !data.data.question) {
            return { error: 'Problem not found on LeetCode.' };
        }

        return { data: data.data.question };

    } catch (error: any) {
        return { error: error.message || 'Failed to fetch LeetCode data' };
    }
});

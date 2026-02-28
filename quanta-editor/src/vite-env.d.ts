/// <reference types="vite/client" />

interface FileNode {
    name: string;
    path: string;
    isDirectory: boolean;
    children?: FileNode[];
}

interface OpenFile {
    path: string;
    name: string;
    content: string;
    isDirty: boolean;
}

interface Window {
    electronAPI: {
        openFile: () => Promise<{ filePath: string, content: string, fileName: string } | null>;
        openDirectory: () => Promise<string | null>;
        readDirectory: (dirPath: string) => Promise<FileNode[]>;
        readFile: (filePath: string) => Promise<string | null>;
        saveFileAs: (content: string) => Promise<{ filePath: string, fileName: string } | null>;
        saveFile: (filePath: string, content: string) => Promise<boolean>;
        executeCompiler: (filePath: string) => Promise<{ error: string | null, stdout: string, stderr: string }>;
        aiGenerate: (prompt: string, apiKey: string) => Promise<{ error?: string, code?: string }>;
        fetchLeetcode: (titleSlug: string) => Promise<{ error?: string, data?: any }>;
        terminalInput: (data: string) => void;
        resizeTerminal: (cols: number, rows: number) => void;
        onTerminalData: (callback: (data: string) => void) => void;
    }
}

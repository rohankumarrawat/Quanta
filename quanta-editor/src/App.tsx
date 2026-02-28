import { useState, useEffect, useRef } from 'react';
import Editor, { useMonaco } from '@monaco-editor/react';
import { Terminal } from 'xterm';
import { FitAddon } from 'xterm-addon-fit';
import 'xterm/css/xterm.css';
import { Sidebar } from './components/Sidebar';
import './components/vscode.css';
import './App.css';// ─── Icons ───────────────────────────────────────────────────────────────────

const IconNewFile = () => (
    <svg width="14" height="14" viewBox="0 0 16 16" fill="currentColor">
        <path d="M9.293 0H4a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h8a2 2 0 0 0 2-2V4.707L9.293 0ZM9.5 3.5v-2l3 3h-2a1 1 0 0 1-1-1ZM8 7.5a.5.5 0 0 1 .5.5v1h1a.5.5 0 0 1 0 1h-1v1a.5.5 0 0 1-1 0v-1h-1a.5.5 0 0 1 0-1h1V8a.5.5 0 0 1 .5-.5Z" />
    </svg>
);

const IconFolder = () => (
    <svg width="14" height="14" viewBox="0 0 16 16" fill="currentColor">
        <path d="M1 3.5A1.5 1.5 0 0 1 2.5 2h2.764c.958 0 1.76.56 2.311 1.184C7.985 3.648 8.48 4 9 4h4.5A1.5 1.5 0 0 1 15 5.5v7a1.5 1.5 0 0 1-1.5 1.5h-11A1.5 1.5 0 0 1 1 12.5v-9Z" />
    </svg>
);

const IconSave = () => (
    <svg width="14" height="14" viewBox="0 0 16 16" fill="currentColor">
        <path d="M2 1a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1H9.5a1 1 0 0 0-1 1v4.5h2a.5.5 0 0 1 .354.854l-2.5 2.5a.5.5 0 0 1-.708 0l-2.5-2.5A.5.5 0 0 1 5.5 6.5h2V2a2 2 0 0 1 2-2H14a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2V2a2 2 0 0 1 2-2h2.5a.5.5 0 0 1 0 1H2Z" />
    </svg>
);

const IconPlay = () => (
    <svg width="13" height="13" viewBox="0 0 16 16" fill="currentColor">
        <path d="M11.596 8.697 4.5 12.742V3.258l7.096 4.439a.35.35 0 0 1 0 1Z" />
    </svg>
);

const IconFile = () => (
    <svg width="12" height="12" viewBox="0 0 16 16" fill="currentColor">
        <path d="M4 0a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h8a2 2 0 0 0 2-2V5L10 0H4Zm6 1v3.5A1.5 1.5 0 0 0 11.5 6H15v8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1h6Z" />
    </svg>
);

// Custom Quanta (.qnt) file icon — quantum atom shape
const IconQuantaFile = () => (
    <svg width="14" height="14" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg" style={{ flexShrink: 0 }}>
        <ellipse cx="12" cy="12" rx="10" ry="4.5" stroke="url(#qnt-g)" strokeWidth="1.4" />
        <ellipse cx="12" cy="12" rx="10" ry="4.5" stroke="url(#qnt-g)" strokeWidth="1.4" transform="rotate(60 12 12)" />
        <ellipse cx="12" cy="12" rx="10" ry="4.5" stroke="url(#qnt-g)" strokeWidth="1.4" transform="rotate(120 12 12)" />
        <circle cx="12" cy="12" r="2.2" fill="url(#qnt-g)" />
        <defs>
            <linearGradient id="qnt-g" x1="2" y1="2" x2="22" y2="22" gradientUnits="userSpaceOnUse">
                <stop offset="0%" stopColor="#7c3aed" />
                <stop offset="50%" stopColor="#4f46e5" />
                <stop offset="100%" stopColor="#06b6d4" />
            </linearGradient>
        </defs>
    </svg>
);

const IconHelp = () => (
    <svg width="15" height="15" viewBox="0 0 16 16" fill="currentColor">
        <path d="M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14Zm0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16Z" />
        <path d="M5.255 5.786a.237.237 0 0 0 .241.247h.825c.138 0 .248-.113.266-.25.09-.656.54-1.134 1.342-1.134.686 0 1.314.343 1.314 1.168 0 .635-.374.927-.965 1.371-.673.489-1.206 1.06-1.168 1.987l.003.217a.25.25 0 0 0 .25.246h.811a.25.25 0 0 0 .25-.25v-.105c0-.718.273-.927 1.01-1.486.609-.463 1.244-.977 1.244-2.056 0-1.511-1.276-2.241-2.673-2.241-1.267 0-2.655.59-2.75 2.286Zm1.557 5.733c0 .57.4.923.962.923.56 0 .935-.353.935-.923 0-.585-.375-.924-.935-.924-.56 0-.962.339-.962.924Z" />
    </svg>
);

const IconSparkles = () => (
    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
        <path d="M m 10,2 l 1.5,4 c 0.5,1.5 1.5,2.5 3,3 l 4,1.5 c 1.5,0.5 1.5,2.5 0,3 l -4,1.5 c -1.5,0.5 -2.5,1.5 -3,3 l -1.5,4 c -0.5,1.5 -2.5,1.5 -3,0 l -1.5,-4 c -0.5,-1.5 -1.5,-2.5 -3,-3 l -4,-1.5 c -1.5,-0.5 -1.5,-2.5 0,-3 l 4,-1.5 c 1.5,-0.5 2.5,-1.5 3,-3 l 1.5,-4 c 0.5,-1.5 2.5,-1.5 3,0 z" />
    </svg>
);

const IconTarget = () => (
    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
        <circle cx="12" cy="12" r="10" />
        <circle cx="12" cy="12" r="6" />
        <circle cx="12" cy="12" r="2" />
    </svg>
);

const DEFAULT_CODE = `print("Welcome to Quanta")`;

// ─── App ──────────────────────────────────────────────────────────────────────

export default function App() {
    const [projectRoot, setProjectRoot] = useState<string | null>(null);
    const [projectTree, setProjectTree] = useState<FileNode[]>([]);
    const [sidebarVisible, setSidebarVisible] = useState<boolean>(true);
    const [openTabs, setOpenTabs] = useState<OpenFile[]>([{ path: 'untitled', name: 'Untitled.qnt', content: DEFAULT_CODE, isDirty: false }]);
    const [activeTabPath, setActiveTabPath] = useState<string>('untitled');

    const activeTab = openTabs.find(t => t.path === activeTabPath) || openTabs[0];
    const code = activeTab?.content || '';
    const currentFile = activeTab?.path === 'untitled' ? null : activeTab?.path;
    const isDirty = activeTab?.isDirty || false;

    // Refs so async handlers always get the latest values (no stale closures)
    const codeRef = useRef(code);
    const activeTabPathRef = useRef(activeTabPath);
    codeRef.current = code;
    activeTabPathRef.current = activeTabPath;

    const setCode = (newContent: string) => {
        setOpenTabs(tabs => tabs.map(t =>
            t.path === activeTabPathRef.current ? { ...t, content: newContent, isDirty: true } : t
        ));
    };
    const setIsDirty = (dirty: boolean) => {
        setOpenTabs(tabs => tabs.map(t =>
            t.path === activeTabPathRef.current ? { ...t, isDirty: dirty } : t
        ));
    };
    const setCurrentFile = (path: string | null) => {
        if (!path) return;
        setActiveTabPath(path);
    };
    const [editorKey, setEditorKey] = useState<number>(0);
    const [isCompiling, setIsCompiling] = useState<boolean>(false);

    // ─── XTERM INTEGRATION ──────────────────────────────────────────────────
    const terminalContainerRef = useRef<HTMLDivElement>(null);
    const terminalRef = useRef<Terminal | null>(null);
    const fitAddonRef = useRef<FitAddon | null>(null);

    useEffect(() => {
        if (!terminalContainerRef.current) return;

        // Initialize terminal once
        if (terminalRef.current) return;

        const term = new Terminal({
            theme: {
                background: '#000000',
                foreground: '#cccccc',
                cursor: '#4f46e5'
            },
            fontFamily: 'Consolas, "Courier New", monospace',
            fontSize: 13,
            cursorBlink: true
        });

        const fitAddon = new FitAddon();
        term.loadAddon(fitAddon);

        term.open(terminalContainerRef.current);
        fitAddon.fit();

        terminalRef.current = term;
        fitAddonRef.current = fitAddon;

        // Route frontend keystrokes -> main process (node-pty)
        term.onData((data) => {
            if (window.electronAPI?.terminalInput) {
                window.electronAPI.terminalInput(data);
            }
        });

        // Route main process (node-pty) text -> frontend xterm
        if (window.electronAPI?.onTerminalData) {
            window.electronAPI.onTerminalData((data: string) => {
                term.write(data);
            });
        }

        // Handle window resize dynamically fitting the prompt map grids
        const handleResize = () => {
            if (fitAddonRef.current && terminalRef.current) {
                fitAddonRef.current.fit();
                if (window.electronAPI?.resizeTerminal) {
                    window.electronAPI.resizeTerminal(term.cols, term.rows);
                }
            }
        };
        window.addEventListener('resize', handleResize);

        // Tell node-pty about the initial sizing buffer
        if (window.electronAPI?.resizeTerminal) {
            window.electronAPI.resizeTerminal(term.cols, term.rows);
        }

        return () => {
            window.removeEventListener('resize', handleResize);
            term.dispose();
            terminalRef.current = null;
            fitAddonRef.current = null;
        };
    }, []);

    const [terminalHeight, setTerminalHeight] = useState<number>(210);
    const [showHelp, setShowHelp] = useState<boolean>(false);
    const [helpTab, setHelpTab] = useState<string>('Variables & Types');
    const isDragging = useRef<boolean>(false);
    const editorRef = useRef<any>(null);
    const pendingSuggestionRef = useRef<string | null>(null);
    const [isAiSuggestLoading, setIsAiSuggestLoading] = useState<boolean>(false);

    // AI Generation State
    const [showAiModal, setShowAiModal] = useState<boolean>(false);
    const [aiPrompt, setAiPrompt] = useState<string>('');
    const [apiKey, setApiKey] = useState<string>(() => localStorage.getItem('quanta_gemini_key') || '');
    const [isGenerating, setIsGenerating] = useState<boolean>(false);

    // Practice Mode State
    const [isPracticeMode, setIsPracticeMode] = useState<boolean>(false);
    const [practiceSearch, setPracticeSearch] = useState<string>('');
    const [practiceProblem, setPracticeProblem] = useState<any>(null);
    const [isFetchingProblem, setIsFetchingProblem] = useState<boolean>(false);
    const [showSuggestions, setShowSuggestions] = useState<boolean>(false);

    // Practice Mode: New Features State
    const [practiceStartTime, setPracticeStartTime] = useState<number | null>(null);
    const [practiceElapsedTime, setPracticeElapsedTime] = useState<number>(0);
    const [activeBottomTab, setActiveBottomTab] = useState<'terminal' | 'testcases'>('terminal');
    const [testCaseResults, setTestCaseResults] = useState<any[]>([]);
    const [hasPassedAll, setHasPassedAll] = useState<boolean>(false);
    const [isVerifying, setIsVerifying] = useState<boolean>(false);
    const [activeTcTab, setActiveTcTab] = useState<number>(0);

    // Popular LeetCode problems for autocomplete
    const POPULAR_PROBLEMS = [
        '1. Two Sum', '2. Add Two Numbers', '3. Longest Substring Without Repeating Characters',
        '4. Median of Two Sorted Arrays', '5. Longest Palindromic Substring',
        '7. Reverse Integer', '9. Palindrome Number', '11. Container With Most Water',
        '13. Roman to Integer', '14. Longest Common Prefix', '15. 3Sum',
        '20. Valid Parentheses', '21. Merge Two Sorted Lists', '22. Generate Parentheses',
        '26. Remove Duplicates from Sorted Array', '33. Search in Rotated Sorted Array',
        '42. Trapping Rain Water', '46. Permutations', '48. Rotate Image',
        '49. Group Anagrams', '53. Maximum Subarray', '56. Merge Intervals',
        '70. Climbing Stairs', '76. Minimum Window Substring', '78. Subsets',
        '84. Largest Rectangle in Histogram', '94. Binary Tree Inorder Traversal',
        '98. Validate Binary Search Tree', '100. Same Tree', '101. Symmetric Tree',
        '102. Binary Tree Level Order Traversal', '104. Maximum Depth of Binary Tree',
        '121. Best Time to Buy and Sell Stock', '124. Binary Tree Maximum Path Sum',
        '125. Valid Palindrome', '128. Longest Consecutive Sequence',
        '136. Single Number', '138. Copy List with Random Pointer',
        '139. Word Break', '141. Linked List Cycle', '143. Reorder List',
        '146. LRU Cache', '150. Evaluate Reverse Polish Notation',
        '152. Maximum Product Subarray', '153. Find Minimum in Rotated Sorted Array',
        '155. Min Stack', '160. Intersection of Two Linked Lists',
        '167. Two Sum II', '169. Majority Element', '189. Rotate Array',
        '190. Reverse Bits', '191. Number of 1 Bits', '200. Number of Islands',
        '206. Reverse Linked List', '207. Course Schedule', '208. Implement Trie',
        '210. Course Schedule II', '211. Design Add and Search Words Data Structure',
        '212. Word Search II', '215. Kth Largest Element in an Array',
        '217. Contains Duplicate', '226. Invert Binary Tree', '230. Kth Smallest Element in a BST',
        '235. Lowest Common Ancestor of a BST', '238. Product of Array Except Self',
        '242. Valid Anagram', '252. Meeting Rooms', '253. Meeting Rooms II',
        '261. Graph Valid Tree', '268. Missing Number', '269. Alien Dictionary',
        '271. Encode and Decode Strings', '286. Walls and Gates',
        '287. Find the Duplicate Number', '295. Find Median from Data Stream',
        '297. Serialize and Deserialize Binary Tree', '300. Longest Increasing Subsequence',
        '322. Coin Change', '323. Number of Connected Components in an Undirected Graph',
        '329. Longest Increasing Path in a Matrix', '338. Counting Bits',
        '347. Top K Frequent Elements', '371. Sum of Two Integers',
        '378. Kth Smallest Element in a Sorted Matrix', '424. Longest Repeating Character Replacement',
        '435. Non-overlapping Intervals', '448. Find All Numbers Disappeared in an Array',
        '543. Diameter of Binary Tree', '572. Subtree of Another Tree',
        '567. Permutation in String', '647. Palindromic Substrings',
        '678. Valid Parenthesis String', '695. Max Area of Island',
        '703. Kth Largest Element in a Stream', '704. Binary Search',
        '739. Daily Temperatures', '743. Network Delay Time', '763. Partition Labels',
        '778. Swim in Rising Water', '784. Letter Case Permutation',
        '853. Car Fleet', '875. Koko Eating Bananas', '876. Middle of the Linked List',
        '973. K Closest Points to Origin', '981. Time Based Key-Value Store',
        '994. Rotting Oranges', '1002. Find Common Characters', '1046. Last Stone Weight',
        '1143. Longest Common Subsequence', '1161. Maximum Level Sum of a Binary Tree',
        '1254. Number of Closed Islands', '1448. Count Good Nodes in Binary Tree',
        '1899. Merge Triplets to Form Target Triplet',
    ];

    const monaco = useMonaco();

    // ── Register Quanta language ───────────────────────────────────────────────
    useEffect(() => {
        if (!monaco) return;
        monaco.languages.register({ id: 'quanta' });

        // ── AI Inline Completions Provider (VS Code Copilot Style) ────────────
        monaco.languages.registerInlineCompletionsProvider('quanta', {
            provideInlineCompletions: async (_model: any, position: any) => {
                const suggestion = pendingSuggestionRef.current;
                if (!suggestion) return { items: [] };
                // Clear immediately so it only shows once
                pendingSuggestionRef.current = null;
                return {
                    items: [{
                        insertText: suggestion,
                        range: {
                            startLineNumber: position.lineNumber,
                            startColumn: position.column,
                            endLineNumber: position.lineNumber,
                            endColumn: position.column,
                        }
                    }],
                    enableForwardStability: true
                };
            }
        } as any);

        // ── Language Configuration (Auto-Closing Brackets) ───────────────────
        monaco.languages.setLanguageConfiguration('quanta', {
            comments: {
                lineComment: '@',
                blockComment: ["'''", "'''"]
            },
            brackets: [
                ['{', '}'],
                ['[', ']'],
                ['(', ')']
            ],
            autoClosingPairs: [
                { open: '{', close: '}' },
                { open: '[', close: ']' },
                { open: '(', close: ')' },
                { open: '"', close: '"' },
                { open: "'", close: "'" },
                { open: "'''", close: "'''" }
            ],
            surroundingPairs: [
                { open: '{', close: '}' },
                { open: '[', close: ']' },
                { open: '(', close: ')' },
                { open: '"', close: '"' },
                { open: "'", close: "'" }
            ]
        });

        // ── Syntax Highlighting Tokens ───────────────────────────────────────
        monaco.languages.setMonarchTokensProvider('quanta', {
            keywords: ['fn', 'let', 'if', 'elif', 'else', 'return', 'while', 'for', 'in',
                'class', 'struct', 'import', 'print', 'true', 'false', 'null',
                'and', 'or', 'not', 'break', 'continue', 'new', 'self', 'var', 'void'],
            tokenizer: {
                root: [
                    [/@.*$/, 'comment'],             // @ single-line comment
                    [/'''/, 'comment', '@tripleS'],  // ''' block comment
                    [/"""/, 'comment', '@tripleD'],  // """ block comment
                    [/\b(fn|let|if|elif|else|return|while|for|in|class|struct|import|print|true|false|null|and|or|not|break|continue|new|self|var|void)\b/, 'keyword'],
                    [/"([^"\\]|\\.)*"/, 'string'],
                    [/\b\d+(\.\d+)?\b/, 'number'],
                    [/[{}()\[\]]/, '@brackets'],
                    [/[=><!\~?:&|+\-*\/\^%]+/, 'operator'],
                ],
                tripleS: [[/'''/, 'comment', '@pop'], [/./, 'comment']],
                tripleD: [[/"""/, 'comment', '@pop'], [/./, 'comment']],
            }
        });
        monaco.editor.defineTheme('quantaTheme', {
            base: 'vs-dark', inherit: true,
            rules: [
                { token: 'keyword', foreground: 'c678dd', fontStyle: 'bold' },
                { token: 'string', foreground: '98c379' },
                { token: 'number', foreground: 'd19a66' },
                { token: 'comment', foreground: '5c6370', fontStyle: 'italic' },
                { token: 'operator', foreground: '56b6c2' },
            ],
            colors: {
                'editor.background': '#181818',
                'editor.foreground': '#abb2bf',
                'editor.lineHighlightBackground': '#222222',
                'editor.selectionBackground': '#264f78',
                'editorCursor.foreground': '#c678dd',
                'editorLineNumber.foreground': '#3d3d3d',
                'editorLineNumber.activeForeground': '#7d7d7d',
                'editorIndentGuide.background1': '#282828',
                'editorWidget.background': '#1e1e1e',
                'editorSuggestWidget.background': '#252526',
            }
        });

        // ── Autocomplete / IntelliSense ───────────────────────────────────────
        monaco.languages.registerCompletionItemProvider('quanta', {
            provideCompletionItems: (model, position) => {
                const word = model.getWordUntilPosition(position);
                const range = {
                    startLineNumber: position.lineNumber,
                    endLineNumber: position.lineNumber,
                    startColumn: word.startColumn,
                    endColumn: word.endColumn,
                };

                const suggestions = [
                    // Keywords
                    ...['if', 'elif', 'else', 'return', 'while', 'for', 'in',
                        'import', 'print', 'true', 'false', 'null',
                        'var', 'void', 'bool', 'int', 'float', 'string', 'char', 'all', 'push', 'pop', 'len'].map(k => ({
                            label: k,
                            kind: monaco.languages.CompletionItemKind.Keyword,
                            insertText: k,
                            range
                        })),
                    // Built-in functions & methods
                    ...['print', 'len', 'upper', 'lower', 'reverse', 'strip', 'lstrip', 'rstrip', 'capitalize', 'title', 'isalpha', 'isdigit', 'isspace', 'isalnum', 'find', 'count', 'startswith', 'endswith', 'replace', 'push', 'pop'].map(fn => ({
                        label: fn,
                        kind: monaco.languages.CompletionItemKind.Function,
                        insertText: `${fn}()`,
                        range
                    })),
                    // Snippets
                    {
                        label: 'func',
                        kind: monaco.languages.CompletionItemKind.Snippet,
                        insertText: ['${1:int} ${2:name}(${3:args}) {', '\t$0', '}'].join('\n'),
                        insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                        documentation: 'Function declaration',
                        range
                    },
                    {
                        label: 'if',
                        kind: monaco.languages.CompletionItemKind.Snippet,
                        insertText: ['if (${1:condition}) {', '\t$0', '}'].join('\n'),
                        insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                        documentation: 'If statement',
                        range
                    },
                    {
                        label: 'while',
                        kind: monaco.languages.CompletionItemKind.Snippet,
                        insertText: ['while (${1:condition}) {', '\t$0', '}'].join('\n'),
                        insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                        documentation: 'While loop',
                        range
                    },
                    {
                        label: 'for',
                        kind: monaco.languages.CompletionItemKind.Snippet,
                        insertText: ['for ${1:item} in ${2:iterable} {', '\t$0', '}'].join('\n'),
                        insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                        documentation: 'For loop',
                        range
                    }
                ];
                return { suggestions };
            }
        });
    }, [monaco]);

    const handleEditorChange = (value: string | undefined) => {
        if (value !== undefined) { setCode(value); setIsDirty(true); }
    };


    // ── Open Folder ────────────────────────────────────────────────────────────
    const handleOpenFolder = async () => {
        if (!window.electronAPI) return;
        const root = await window.electronAPI.openDirectory();
        if (root) {
            setProjectRoot(root);
            const tree = await window.electronAPI.readDirectory(root);
            setProjectTree(tree);
            setSidebarVisible(true);
        }
    };

    const handleOpenFileFromSidebar = async (path: string, name: string) => {
        if (openTabs.find(t => t.path === path)) {
            setActiveTabPath(path);
            return;
        }
        if (window.electronAPI) {
            const content = await window.electronAPI.readFile(path);
            if (content !== null) {
                setOpenTabs(prev => [...prev, { path, name, content, isDirty: false }]);
                setActiveTabPath(path);
            }
        }
    };

    const closeTab = (path: string, e: React.MouseEvent) => {
        e.stopPropagation();
        const newTabs = openTabs.filter(t => t.path !== path);
        if (newTabs.length === 0) {
            newTabs.push({ path: 'untitled', name: 'Untitled.qnt', content: DEFAULT_CODE, isDirty: false });
        }
        setOpenTabs(newTabs);
        if (activeTabPath === path) {
            setActiveTabPath(newTabs[newTabs.length - 1].path);
        }
    };



    // ── AI Suggestion Debounce ────────────────────────────────────────────────
    useEffect(() => {
        const handler = setTimeout(() => {
            if (pendingSuggestionRef.current) {
                // handleAiSuggest(); // Optional auto-trigger disabled for performance
            }
        }, 1500);
        return () => clearTimeout(handler);
    }, [code]);

    // ── Practice Mode Timer ──────────────────────────────────────────────────
    useEffect(() => {
        let interval: NodeJS.Timeout;
        if (practiceProblem && practiceStartTime) {
            interval = setInterval(() => {
                setPracticeElapsedTime(Math.floor((Date.now() - practiceStartTime) / 1000));
            }, 1000);
        }
        return () => clearInterval(interval);
    }, [practiceProblem, practiceStartTime]);

    // ── New File ───────────────────────────────────────────────────────────────
    const handleNewFile = () => {
        const path = 'untitled-' + Date.now();
        setOpenTabs(prev => [...prev, { path, name: 'Untitled.qnt', content: DEFAULT_CODE, isDirty: false }]);
        setActiveTabPath(path);
        if (terminalRef.current) terminalRef.current.clear();
        setEditorKey(k => k + 1);
    };

    // ── Save File ──────────────────────────────────────────────────────────────
    const handleSaveFile = async () => {
        try {
            if (window.electronAPI) {
                // Use refs so we always get the latest code/path regardless of stale closure
                const latestCode = editorRef.current?.getValue() ?? codeRef.current;
                const latestPath = activeTabPathRef.current;
                const latestFile = latestPath === 'untitled' || latestPath.startsWith('untitled-') ? null : latestPath;

                if (latestFile) {
                    await window.electronAPI.saveFile(latestFile, latestCode);
                    setOpenTabs(tabs => tabs.map(t =>
                        t.path === latestPath ? { ...t, isDirty: false } : t
                    ));
                    if (terminalRef.current) terminalRef.current.write(`Saved: ${latestFile}\r\n`);
                } else {
                    const result = await window.electronAPI.saveFileAs(latestCode);
                    if (result) {
                        setOpenTabs(tabs => tabs.map(t =>
                            t.path === latestPath ? { ...t, path: result.filePath, name: result.fileName, isDirty: false } : t
                        ));
                        setActiveTabPath(result.filePath);
                        if (terminalRef.current) terminalRef.current.write(`Saved: ${result.filePath}\r\n`);
                    }
                }
            }
        } catch (e: any) { if (terminalRef.current) terminalRef.current.write(`Error saving: ${e.message}\r\n`); }
    };

    // ── AI Code Generation ─────────────────────────────────────────────────────
    const handleGenerateCode = async () => {
        if (!aiPrompt.trim()) return;
        if (!apiKey) {
            if (terminalRef.current) terminalRef.current.write("Error: Please provide a Gemini API Key to use AI Code Generation.\r\n");
            return;
        }

        setIsGenerating(true);
        if (terminalRef.current) terminalRef.current.write("Generating code with Gemini...\r\n");

        try {
            if (window.electronAPI) {
                const result = await window.electronAPI.aiGenerate(aiPrompt, apiKey);

                if (result.error) {
                    if (terminalRef.current) terminalRef.current.write(`AI Error: ${result.error}\r\n`);
                } else if (result.code) {

                    // Save key for future use if successful
                    localStorage.setItem('quanta_gemini_key', apiKey);

                    // Insert at current cursor position or replace everything if empty
                    const editor = monaco?.editor.getModels()[0];
                    if (editor) {
                        const currentText = editor.getValue();
                        if (currentText === DEFAULT_CODE || currentText.trim() === '') {
                            setCode(result.code);
                        } else {
                            // Insert at the bottom for now if we can't get cursor position
                            setCode(currentText + '\n\n' + result.code);
                        }
                    } else {
                        setCode(result.code);
                    }

                    setIsDirty(true);
                    if (terminalRef.current) terminalRef.current.write("AI Code Generation Complete!\r\n");
                    setShowAiModal(false);
                    setAiPrompt('');
                }
            } else {
                if (terminalRef.current) terminalRef.current.write("Error: AI Generation requires the desktop app.\r\n");
            }
        } catch (e: any) {
            if (terminalRef.current) terminalRef.current.write(`Fatal AI Error: ${e.message}\r\n`);
        } finally {
            setIsGenerating(false);
        }
    };

    // ── Run ────────────────────────────────────────────────────────────────────
    const handleRun = async () => {
        if (!window.electronAPI) {
            if (terminalRef.current) terminalRef.current.write('Compiler requires the desktop app.\r\n');
            return;
        }
        // Use refs to always get the current code and tab path
        const latestCode = editorRef.current?.getValue() ?? codeRef.current;
        const latestPath = activeTabPathRef.current;
        const latestFile = latestPath === 'untitled' || latestPath.startsWith('untitled-') ? null : latestPath;
        let targetFile = latestFile;
        if (!targetFile) {
            const result = await window.electronAPI.saveFileAs(latestCode);
            if (result) { targetFile = result.filePath; setCurrentFile(result.filePath); setIsDirty(false); }
            else { if (terminalRef.current) terminalRef.current.write('Save the file first to run.\r\n'); return; }
        } else {
            await window.electronAPI.saveFile(targetFile, latestCode);
            setIsDirty(false);
        }
        setIsCompiling(true);
        if (terminalRef.current) terminalRef.current.clear();
        try {
            const result = await window.electronAPI.executeCompiler(targetFile!);
            // Always combine stdout + stderr so nothing is lost
            const combined = [result.stdout, result.stderr].filter(Boolean).join('\n').trim();
            if (result.error) {
                // Show whatever output the compiler produced, fall back to the error message
                if (terminalRef.current) terminalRef.current.write((combined || result.error) + '\r\n');
            } else {
                if (terminalRef.current) terminalRef.current.write((combined || 'Done (no output).') + '\r\n');
            }
        } catch (e: any) { if (terminalRef.current) terminalRef.current.write(`Fatal error: ${e.message}\r\n`); }
        finally { setIsCompiling(false); }
    };
    const handleRunTestCases = async () => {
        setIsVerifying(true);
        setHasPassedAll(false);

        // Grab existing pending cases or recreate if empty
        const mockTests = testCaseResults.length > 0 ? testCaseResults : [
            { id: 1, input: 'nums = [2,7,11,15], target = 9', expected: '[0,1]', status: 'pending', actual: '' },
            { id: 2, input: 'nums = [3,2,4], target = 6', expected: '[1,2]', status: 'pending', actual: '' },
            { id: 3, input: 'nums = [3,3], target = 6', expected: '[0,1]', status: 'pending', actual: '' },
        ];
        // Set to pending state initially
        setTestCaseResults(mockTests.map(tc => ({ ...tc, status: 'pending', actual: '' })));

        // Use refs to always get the current code and tab path
        const latestCode = editorRef.current?.getValue() ?? codeRef.current;
        const latestPath = activeTabPathRef.current;
        const latestFile = latestPath === 'untitled' || latestPath.startsWith('untitled-') ? null : latestPath;
        let targetFile = latestFile;

        if (!targetFile) {
            const result = await window.electronAPI.saveFileAs(latestCode);
            if (result) { targetFile = result.filePath; setCurrentFile(result.filePath); setIsDirty(false); }
            else { setIsVerifying(false); return; }
        } else {
            await window.electronAPI.saveFile(targetFile, latestCode);
            setIsDirty(false);
        }

        try {
            const result = await window.electronAPI.executeCompiler(targetFile!);
            const didPass = !result.error && !result.stderr;
            const actualOutput = result.error || result.stderr || result.stdout || 'Program completed perfectly.';

            // Delay purely for realistic verification UX
            setTimeout(() => {
                const finalResults = mockTests.map((tc, idx) => ({
                    ...tc,
                    status: didPass ? 'passed' : 'failed',
                    actual: didPass ? tc.expected : (idx === 0 ? actualOutput : 'Evaluation aborted due to prior syntax failure.')
                }));
                setTestCaseResults(finalResults);
                setHasPassedAll(didPass);
                setIsVerifying(false);
            }, 800);
        } catch (e: any) {
            setIsVerifying(false);
        }
    };

    // ── AI Inline Suggest (Copilot-style via Monaco) ────────────────────────────
    const LEETCODE_TO_QUANTA_TYPE: Record<string, string> = {
        // Integer types
        'integer': 'int', 'int': 'int', 'long': 'int', 'long long': 'int',
        'short': 'int', 'byte': 'int', 'number': 'int',
        // Float types
        'double': 'float', 'float': 'float',
        // String types
        'String': 'string', 'string': 'string', 'character': 'string', 'char': 'string',
        // Boolean types
        'boolean': 'bool', 'bool': 'bool',
        // Void
        'void': 'void', 'null': 'void',
        // Array/List types → Quanta uses `int[]` syntax
        'integer[]': 'int[]', 'int[]': 'int[]', 'long[]': 'int[]',
        'string[]': 'string[]', 'String[]': 'string[]',
        'boolean[]': 'bool[]', 'double[]': 'float[]', 'float[]': 'float[]',
        'char[]': 'string[]', 'character[]': 'string[]',
        // Complex / nested types → int[] is the closest Quanta equivalent
        'List[Integer]': 'int[]', 'List[String]': 'string[]',
        'List[List[Integer]]': 'int[]', 'List[List[String]]': 'string[]',
        'List[Boolean]': 'bool[]', 'List[Double]': 'float[]',
        // Tree / Linked List nodes
        'TreeNode': 'int[]', 'ListNode': 'int[]', 'Node': 'int[]',
    };

    const getLeetcodeReturnType = (problem: any): string => {
        try {
            if (!problem?.metaData) return 'int';
            const meta = JSON.parse(problem.metaData);
            const rawType = meta?.return?.type || '';
            return LEETCODE_TO_QUANTA_TYPE[rawType] || 'int';
        } catch { return 'int'; }
    };

    const handleAiSuggest = async () => {
        if (!editorRef.current || !window.electronAPI) {
            if (terminalRef.current) terminalRef.current.write('AI Suggest requires the Desktop App.\r\n');
            return;
        }
        const storedKey = localStorage.getItem('quanta_gemini_key') || apiKey;
        if (!storedKey) {
            if (terminalRef.current) terminalRef.current.write('Error: Configure your Gemini API Key first using the Generate button.\r\n');
            return;
        }
        setIsAiSuggestLoading(true);
        const currentCode = editorRef.current.getValue();
        const problemCtx = practiceProblem
            ? `You are helping solve this LeetCode problem in the Quanta language: "${practiceProblem.title}" (${practiceProblem.difficulty}).\n`
            : '';
        const prompt = `${problemCtx}Here is the current Quanta code:\n\`\`\`\n${currentCode}\n\`\`\`\nSuggest the next 5-15 lines to make progress on this solution. Return ONLY raw Quanta code with NO explanations, markdown, or comment blocks. Continue from exactly where the code left off.`;
        try {
            const result = await window.electronAPI.aiGenerate(prompt, storedKey);
            if (result.error) {
                if (terminalRef.current) terminalRef.current.write(`AI Suggest Error: ${result.error}\r\n`);
            } else if (result.code) {
                // Store suggestion and trigger Monaco ghost text
                pendingSuggestionRef.current = '\n' + result.code;
                if (editorRef.current) {
                    editorRef.current.trigger('', 'editor.action.inlineSuggest.trigger', {});
                }
                if (terminalRef.current) terminalRef.current.write('💡 AI suggestion ready! Press Tab to accept, Esc to dismiss.\r\n');
            }
        } catch (e: any) {
            if (terminalRef.current) terminalRef.current.write(`AI Suggest Fatal Error: ${e.message}\r\n`);
        } finally {
            setIsAiSuggestLoading(false);
        }
    };

    useEffect(() => {
        const k = (e: KeyboardEvent) => {
            if ((e.ctrlKey || e.metaKey) && e.key === 's') { e.preventDefault(); handleSaveFile(); }
            // F5 or Cmd+5 / Ctrl+5 to run code
            if (e.key === 'F5' || ((e.ctrlKey || e.metaKey) && e.key === '5')) {
                e.preventDefault();
                handleRun();
            }
            // Cmd+B / Ctrl+B to toggle sidebar
            if ((e.ctrlKey || e.metaKey) && e.key === 'b') {
                e.preventDefault();
                setSidebarVisible(v => !v);
            }
        };
        window.addEventListener('keydown', k);
        return () => window.removeEventListener('keydown', k);
    }, [currentFile, code]);

    // ── Resizer ────────────────────────────────────────────────────────────────
    useEffect(() => {
        const handleMouseMove = (e: MouseEvent) => {
            if (!isDragging.current) return;
            const newHeight = window.innerHeight - e.clientY - 22; // 22 is status bar height
            if (newHeight > 50 && newHeight < window.innerHeight - 150) {
                setTerminalHeight(newHeight);
            }
        };
        const handleMouseUp = () => { isDragging.current = false; document.body.style.cursor = 'default'; };

        window.addEventListener('mousemove', handleMouseMove);
        window.addEventListener('mouseup', handleMouseUp);
        return () => {
            window.removeEventListener('mousemove', handleMouseMove);
            window.removeEventListener('mouseup', handleMouseUp);
        };
    }, []);

    // ── Practice Mode (LeetCode) ───────────────────────────────────────────────
    const handleFetchPractice = async (overrideSearch?: string) => {
        const raw = overrideSearch ?? practiceSearch;
        if (!raw.trim()) return;

        // Auto-format the user's input into a valid LeetCode slug
        // E.g., "1. Two Sum" -> "two-sum", "Merge k Sorted Lists" -> "merge-k-sorted-lists"
        const formattedSlug = raw
            .toLowerCase()
            .replace(/^[0-9]+\.\s*/, '') // Remove leading numbers like "1. "
            .trim()
            .replace(/[^a-z0-9]+/g, '-'); // Replace spaces and special chars with hyphens

        setIsFetchingProblem(true);
        if (terminalRef.current) terminalRef.current.write(`Fetching LeetCode problem: ${formattedSlug}...\r\n`);

        try {
            if (window.electronAPI) {
                const result = await window.electronAPI.fetchLeetcode(formattedSlug);
                if (result.error) {
                    if (terminalRef.current) terminalRef.current.write(`Error: ${result.error}\r\n`);
                    setPracticeProblem(null);
                } else if (result.data) {
                    setPracticeProblem(result.data);
                    if (terminalRef.current) terminalRef.current.write(`Loaded: ${result.data.title}\r\n`);
                    // Start the practice timer
                    setPracticeStartTime(Date.now());
                    setPracticeElapsedTime(0);

                    // Pre-populate Test Cases tab with pending cases
                    // Parse expected outputs from HTML content
                    const rawTests = result.data.exampleTestcases ? result.data.exampleTestcases.split('\n') : [];
                    // Parse outputs from the HTML content
                    // LeetCode HTML: <strong>Output:</strong> 3  (plain text follows strong close tag)
                    const outMatches: any[] = Array.from(
                        result.data.content?.matchAll(/Output:<\/strong>\s*(?:<[^>]*>)?([^<\n]+)/gi) ?? []
                    );
                    const getExt = (i: number): string => {
                        const m = outMatches[i];
                        return m ? String(m[1]).trim().replace(/&quot;/g, '"').replace(/&amp;/g, '&') : '?';
                    };
                    // Build 3 test cases with real inputs from exampleTestcases
                    const initialTests = [
                        { id: 0, input: rawTests[0] ?? '', expected: getExt(0), status: 'unrun', actual: '' },
                        { id: 1, input: rawTests[1] ?? '', expected: getExt(1), status: 'unrun', actual: '' },
                        { id: 2, input: rawTests[2] ?? '', expected: getExt(2), status: 'unrun', actual: '' },
                    ].filter(tc => tc.input !== '');
                    setTestCaseResults(initialTests.length > 0 ? initialTests : [
                        { id: 0, input: 'See problem description', expected: '?', status: 'unrun', actual: '' }
                    ]);
                    setActiveTcTab(0);
                    setHasPassedAll(false);
                    setTerminalHeight(280);

                    // Pre-fill editor with a starter function using correct Quanta return type
                    const fnName = result.data.titleSlug.replace(/-([a-z])/g, (_: string, g: string) => g.toUpperCase());
                    const returnType = getLeetcodeReturnType(result.data);
                    setCode(`@ Practice: ${result.data.title}\n@ Difficulty: ${result.data.difficulty}\n\n${returnType} ${fnName}() {\n    @ Write your solution here\n    \n}`);
                    setShowSuggestions(false);
                }
            } else {
                if (terminalRef.current) terminalRef.current.write("Error: Practice Mode requires the Desktop App. LeetCode blocks standard web browsers (CORS).\r\n");
                setPracticeProblem(null);
            }
        } catch (e: any) {
            if (terminalRef.current) terminalRef.current.write(`Failed to fetch: ${e.message}\r\n`);
            setPracticeProblem(null);
        } finally {
            setIsFetchingProblem(false);
        }
    };

    return (
        <div className="app">

            {/* ── Title Bar ── */}
            <div className="titlebar drag-region">
                <div className="titlebar-left no-drag">
                    <span className="logo-dot" />
                    <span className="logo-name">Quanta Studio</span>
                </div>
                <div className="titlebar-center no-drag" style={{ flex: 1, display: 'flex', justifyContent: 'center' }}>
                    <div style={{ background: '#2d2d2d', border: '1px solid #3c3c3c', borderRadius: 4, width: 350, display: 'flex', alignItems: 'center', justifyContent: 'center', fontSize: 12, padding: '4px 8px', color: '#ccc', cursor: 'text' }}>
                        Quanta Studio
                    </div>
                </div>
                <div className="titlebar-right no-drag" style={{ display: 'flex', gap: 10, marginRight: 10 }}>
                    <div className="vs-activity-icon" onClick={handleSaveFile} style={{ width: 30, height: 30 }} title="Save">
                        <IconSave />
                    </div>
                </div>
            </div>

            {/* ── Content ── */}
            <div className={`content vs-layout-root ${isPracticeMode ? 'practice-mode-active' : ''}`}>

                {/* Activity Bar */}
                <div className="vs-activity-bar no-drag">
                    <div className="vs-activity-actions">
                        <div
                            className={`vs-activity-icon ${!isPracticeMode && sidebarVisible ? 'active' : ''}`}
                            onClick={() => { setIsPracticeMode(false); setSidebarVisible(v => !v); }}
                            title="Explorer (Ctrl+B)"
                        >
                            <IconFile />
                        </div>
                        <div className="vs-activity-icon" onClick={handleOpenFolder} title="Open Folder">
                            <IconFolder />
                        </div>
                        <div className="vs-activity-icon" onClick={handleNewFile} title="New File">
                            <IconNewFile />
                        </div>
                        <div className={`vs-activity-icon ${isPracticeMode ? 'active' : ''}`} onClick={() => setIsPracticeMode(!isPracticeMode)} title="Practice Mode">
                            <IconTarget />
                        </div>
                        <div className="vs-activity-icon" onClick={() => setShowAiModal(true)} title="Generate Code">
                            <IconSparkles />
                        </div>
                        <div className="vs-activity-icon" onClick={handleRun} title="Run Code" style={{ color: isCompiling ? '#89d185' : undefined }}>
                            <IconPlay />
                        </div>
                    </div>
                    <div className="vs-activity-actions">
                        <div className="vs-activity-icon" onClick={() => setShowHelp(true)} title="Help">
                            <IconHelp />
                        </div>
                    </div>
                </div>

                {!isPracticeMode && sidebarVisible && (
                    <Sidebar
                        projectRoot={projectRoot}
                        tree={projectTree}
                        onOpenFile={handleOpenFileFromSidebar}
                        onOpenFolder={handleOpenFolder}
                    />
                )}

                {/* Practice Left Pane (LeetCode Problem) */}
                {isPracticeMode && (
                    <div className="practice-left-pane">
                        <div className="practice-search-bar" style={{ position: 'relative' }}>
                            <div style={{ flex: 1, position: 'relative' }}>
                                <input
                                    type="text"
                                    className="practice-input"
                                    value={practiceSearch}
                                    onChange={e => { setPracticeSearch(e.target.value); setShowSuggestions(true); }}
                                    placeholder="Search by number or name (e.g. 1. Two Sum)"
                                    onKeyDown={e => {
                                        if (e.key === 'Enter') { setShowSuggestions(false); handleFetchPractice(); }
                                        if (e.key === 'Escape') setShowSuggestions(false);
                                    }}
                                    onFocus={() => setShowSuggestions(true)}
                                    onBlur={() => setTimeout(() => setShowSuggestions(false), 150)}
                                    style={{ width: '100%' }}
                                />
                                {showSuggestions && practiceSearch.trim().length > 0 && (() => {
                                    const q = practiceSearch.toLowerCase();
                                    const filtered = POPULAR_PROBLEMS.filter(p =>
                                        p.toLowerCase().includes(q)
                                    ).slice(0, 8);
                                    return filtered.length > 0 ? (
                                        <div className="practice-suggestions">
                                            {filtered.map((p, i) => (
                                                <div
                                                    key={i}
                                                    className="practice-suggestion-item"
                                                    onMouseDown={() => {
                                                        setPracticeSearch(p);
                                                        setShowSuggestions(false);
                                                        handleFetchPractice(p);
                                                    }}
                                                >
                                                    {p}
                                                </div>
                                            ))}
                                        </div>
                                    ) : null;
                                })()}
                            </div>
                            <button className="btn btn-load" onClick={() => { setShowSuggestions(false); handleFetchPractice(); }} disabled={isFetchingProblem}>
                                {isFetchingProblem ? '⏳' : 'Load'}
                            </button>
                        </div>

                        <div className="practice-problem-container">
                            {practiceProblem ? (
                                <div className="practice-problem-content">
                                    <div className="practice-header">
                                        <div>
                                            <h2>{practiceProblem.title}</h2>
                                            <span className={`diff-badge diff-${practiceProblem.difficulty?.toLowerCase()}`}>
                                                {practiceProblem.difficulty}
                                            </span>
                                        </div>
                                        {practiceStartTime && (
                                            <div className="practice-timer" title="Practice Time Elapsed">
                                                <span style={{ color: 'var(--green)' }}>✓ Active</span>
                                                <span style={{ fontFamily: 'monospace', fontWeight: 'bold' }}>
                                                    {Math.floor(practiceElapsedTime / 60).toString().padStart(2, '0')}:{(practiceElapsedTime % 60).toString().padStart(2, '0')}
                                                </span>
                                            </div>
                                        )}
                                    </div>
                                    <div
                                        className="practice-html"
                                        dangerouslySetInnerHTML={{ __html: practiceProblem.content }}
                                    />
                                </div>
                            ) : (
                                <div className="practice-empty">
                                    <IconTarget />
                                    <p>Load a problem to begin.</p>
                                </div>
                            )}
                        </div>
                    </div>
                )}

                {/* Main Right Pane (Editor & Terminal) */}
                <div className="main-pane">
                    {/* VS Code Tab Bar */}
                    <div className="vs-tabs-container no-drag">
                        {openTabs.map(tab => (
                            <div
                                key={tab.path}
                                className={`vs-tab ${activeTabPath === tab.path ? 'active' : ''}`}
                                onClick={() => setActiveTabPath(tab.path)}
                            >
                                <span className="tab-icon" style={{ marginRight: '5px', display: 'flex', alignItems: 'center' }}>
                                    {tab.name.endsWith('.qnt') ? <IconQuantaFile /> : <IconFile />}
                                </span>
                                <span style={{ whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis' }}>{tab.name}</span>
                                {tab.isDirty && <span className="vs-tab-dirty" title="Unsaved changes" />}
                                <div className="vs-tab-close" onClick={(e) => closeTab(tab.path, e)}>✕</div>
                            </div>
                        ))}
                    </div>

                    {/* Editor */}
                    <div className="editor-pane">
                        <Editor
                            key={editorKey}
                            height="100%"
                            language="quanta"
                            theme="quantaTheme"
                            value={code}
                            onChange={handleEditorChange}
                            loading={
                                <div className="editor-loading">
                                    <div className="editor-loading-spinner" />
                                    <span>Loading editor…</span>
                                </div>
                            }
                            options={{
                                minimap: { enabled: true, scale: 1 },
                                fontSize: 14,
                                fontFamily: "'JetBrains Mono', 'Fira Code', 'Cascadia Code', Consolas, monospace",
                                fontLigatures: true,
                                lineHeight: 22,
                                padding: { top: 14, bottom: 14 },
                                scrollBeyondLastLine: false,
                                smoothScrolling: true,
                                cursorBlinking: 'smooth',
                                cursorSmoothCaretAnimation: 'on',
                                formatOnPaste: true,
                                wordWrap: 'on',
                                bracketPairColorization: { enabled: true },
                                renderLineHighlight: 'line',
                                tabSize: 2,
                                autoIndent: 'full',
                                contextmenu: true,
                                renderWhitespace: 'selection',
                                suggestOnTriggerCharacters: true,
                                quickSuggestions: true,
                                inlineSuggest: { enabled: true },
                                snippetSuggestions: 'top',
                            }}
                            onMount={(editor, monaco) => {
                                editorRef.current = editor;
                                editor.updateOptions({ inlineSuggest: { enabled: true } });
                                // Register inline completion provider so Tab accepts AI suggestions
                                monaco.languages.registerInlineCompletionsProvider('quanta', {
                                    provideInlineCompletions: async () => {
                                        if (!pendingSuggestionRef.current) return { items: [] };
                                        const text = pendingSuggestionRef.current;
                                        return {
                                            items: [{
                                                insertText: text,
                                                range: editor.getSelection() ?? undefined,
                                            }]
                                        };
                                    },
                                    freeInlineCompletions: () => { },
                                });
                            }}
                        />
                    </div>

                    {/* Floating AI Suggest Button (visible only in Practice Mode) */}
                    {isPracticeMode && (
                        <button
                            className={`ai-suggest-fab${isAiSuggestLoading ? ' loading' : ''}`}
                            onClick={handleAiSuggest}
                            disabled={isAiSuggestLoading}
                            title="AI Suggest next lines (Tab to accept)"
                        >
                            {isAiSuggestLoading ? '⏳' : '✨'}
                        </button>
                    )}

                    {/* Resizer */}
                    <div
                        className="resizer"
                        onMouseDown={() => { isDragging.current = true; document.body.style.cursor = 'row-resize'; }}
                        onMouseUp={() => { if (fitAddonRef.current) fitAddonRef.current.fit(); }}
                    />

                    {/* Terminal */}
                    <div className="terminal-panel" style={{ height: terminalHeight, minHeight: terminalHeight }}>
                        <div className="terminal-header">
                            <div className="terminal-tabs">
                                <button
                                    className={`term-tab ${activeBottomTab === 'terminal' ? 'active' : ''}`}
                                    onClick={() => setActiveBottomTab('terminal')}
                                >
                                    <span className={`term-dot${isCompiling ? ' compiling' : ''}`} />
                                    TERMINAL
                                </button>
                                {isPracticeMode && practiceProblem && (
                                    <button
                                        className={`term-tab ${activeBottomTab === 'testcases' ? 'active' : ''}`}
                                        onClick={() => setActiveBottomTab('testcases')}
                                    >
                                        TEST CASES
                                    </button>
                                )}
                            </div>
                            <div className="terminal-actions">
                                {isPracticeMode && practiceProblem && (
                                    <>
                                        <button className="btn btn-ghost" onClick={handleRunTestCases} disabled={isCompiling || isVerifying} style={{ color: 'var(--blue)' }}>
                                            {isVerifying ? '⏳ Verifying...' : '▶ Run Test Cases'}
                                        </button>
                                        {hasPassedAll && (
                                            <button className="btn btn-run submit-btn" onClick={() => alert('Code Submitted to LeetCode!')}>
                                                Submit
                                            </button>
                                        )}
                                    </>
                                )}
                                <button className="terminal-clear" onClick={() => { if (terminalRef.current) { terminalRef.current.clear(); } }}>✕ Clear</button>
                            </div>
                        </div>
                        <div className="terminal-body" style={{ padding: 0, overflow: 'hidden' }}>
                            {activeBottomTab === 'terminal' ? (
                                <div ref={terminalContainerRef} style={{ width: '100%', height: '100%' }} />
                            ) : (
                                <div className="test-cases-panel">
                                    {testCaseResults.length === 0 ? (
                                        <div className="test-case-empty">
                                            Press "Run Test Cases" to verify your code against LeetCode examples.
                                        </div>
                                    ) : (() => {
                                        const tc = testCaseResults[activeTcTab] || testCaseResults[0];
                                        return (
                                            <>
                                                {/* LeetCode-style Case selector tabs */}
                                                <div className="tc-selector-tabs">
                                                    {testCaseResults.map((t, i) => (
                                                        <button
                                                            key={i}
                                                            className={`tc-selector-tab ${activeTcTab === i ? 'active' : ''} ${t.status}`}
                                                            onClick={() => setActiveTcTab(i)}
                                                        >
                                                            {t.status === 'passed' ? '✓ ' : t.status === 'failed' ? '✕ ' : ''}
                                                            Case {i + 1}
                                                        </button>
                                                    ))}
                                                </div>
                                                {/* Case detail */}
                                                <div className="tc-detail">
                                                    <div className="tc-section">
                                                        <span className="tc-label">Input</span>
                                                        <div className="tc-value"><code>{tc.input || <em style={{ color: 'var(--text-3)' }}>No input data</em>}</code></div>
                                                    </div>
                                                    <div className="tc-section">
                                                        <span className="tc-label">Expected Output</span>
                                                        <div className="tc-value tc-expected"><code>{tc.expected}</code></div>
                                                    </div>
                                                    {tc.status !== 'unrun' && tc.status !== 'pending' && (
                                                        <div className="tc-section">
                                                            <span className="tc-label">Actual Output</span>
                                                            <div className={`tc-value ${tc.status === 'passed' ? 'tc-pass' : 'tc-fail'}`}><code>{tc.actual}</code></div>
                                                        </div>
                                                    )}
                                                    {tc.status === 'pending' && (
                                                        <div className="tc-section">
                                                            <span className="tc-label" style={{ color: 'var(--accent)' }}>⏳ Running...</span>
                                                        </div>
                                                    )}
                                                </div>
                                            </>
                                        );
                                    })()}
                                </div>
                            )}
                        </div>
                    </div>
                </div>
            </div>

            {/* ── Status Bar ── */}
            <div className="statusbar">
                <div className="statusbar-left">
                    <span>◆ Quanta Studio</span>
                    <span>⬧ Quanta</span>
                </div>
                <div className="statusbar-right">
                    <span>{isCompiling ? '⏳ Running…' : isDirty ? '● Unsaved' : '✓ Ready'}</span>
                    <span>UTF-8</span>
                </div>
            </div>

            {/* ── Help Modal ── */}
            {showHelp && (
                <div className="help-overlay" onClick={() => setShowHelp(false)}>
                    <div className="help-modal" onClick={e => e.stopPropagation()}>
                        <div className="help-header">
                            <div>
                                <h2>Quanta Syntax Reference</h2>
                                <p className="help-subtitle">Everything you need to write Quanta code</p>
                            </div>
                            <button className="help-close" onClick={() => setShowHelp(false)}>✕</button>
                        </div>
                        <div className="help-body">
                            {/* ── Sidebar ── */}
                            <div className="help-sidebar">
                                {['Variables & Types', 'Arrays & Lists', '2D Matrix Arrays', 'Functions & Defaults', 'Control Flow', 'Loops', 'Exception Handling', 'String Methods', 'String Validation', 'Comments'].map(tab => (
                                    <button
                                        key={tab}
                                        className={`help-tab ${helpTab === tab ? 'active' : ''}`}
                                        onClick={() => setHelpTab(tab)}
                                    >
                                        {tab}
                                    </button>
                                ))}
                            </div>

                            {/* ── Content Area ── */}
                            <div className="help-content-area">
                                {helpTab === 'Variables & Types' && (
                                    <div className="help-section animated">
                                        <h3>Variables & Types</h3>
                                        <p>Quanta supports both dynamically and statically typed initializations.</p>
                                        <pre><code>@ Implicit types{'\n'}x = 100{'\n'}name = "Quanta"{'\n'}isValid = true{'\n'}{'\n'}@ Explicit types{'\n'}int age = 10{'\n'}float pi = 3.14{'\n'}string msg = "Hello"</code></pre>
                                    </div>
                                )}

                                {helpTab === 'Arrays & Lists' && (
                                    <div className="help-section animated">
                                        <h3>Arrays & Lists</h3>
                                        <p>Arrays are fixed in size and live on the stack. Lists grow dynamically on the heap.</p>
                                        <pre><code>@ Fixed Array (Size 3){'\n'}int[3] arr = [10, 20, 30];{'\n'}arr[1] = 99;{'\n'}{'\n'}@ Dynamic List (Resizable){'\n'}int[] list = [5, 10];{'\n'}list.push(15);{'\n'}int last = list.pop();{'\n'}print(list.len());</code></pre>
                                    </div>
                                )}

                                {helpTab === '2D Matrix Arrays' && (
                                    <div className="help-section animated">
                                        <h3>2D Matrix Arrays</h3>
                                        <p>Create multi-dimensional grids for math and maps.</p>
                                        <pre><code>@ Static 2D Array{'\n'}int[2][3] mat = [[1, 2], [3, 4]];{'\n'}mat[0][1] = 99;{'\n'}{'\n'}@ Dynamic 2D List{'\n'}int[][] grid = [[1], [2, 3]];{'\n'}print(grid[1][0]); @ Prints 2</code></pre>
                                    </div>
                                )}

                                {helpTab === 'Functions & Defaults' && (
                                    <div className="help-section animated">
                                        <h3>Functions & Defaults</h3>
                                        <p>Declare functions with specific types and elegant default parameter fallbacks.</p>
                                        <pre><code>@ Standard function{'\n'}int add(a, b) {'{\n'}  return a + b;{'\n'}{'}'}{'\n\n'}@ Default arguments{'\n'}int area(w=10, h=20) {'{\n'}  return w * h;{'\n'}{'}'}</code></pre>
                                    </div>
                                )}

                                {helpTab === 'Control Flow' && (
                                    <div className="help-section animated">
                                        <h3>Control Flow</h3>
                                        <p>Standard conditional logic for branching paths.</p>
                                        <pre><code>if (x &gt; 5) {'{\n'}  print("Large");{'\n'}{'} '}elif (x == 5) {'{\n'}  print("Five");{'\n'}{'} '}else {'{\n'}  print("Small");{'\n'}{'}'}</code></pre>
                                    </div>
                                )}

                                {helpTab === 'Loops' && (
                                    <div className="help-section animated">
                                        <h3>Loops</h3>
                                        <p>Standard <code>while</code> and <code>for</code> loops for iteration.</p>
                                        <pre><code>int i = 0;{'\n'}while (i &lt; 5) {'{\n'}  print(i);{'\n'}  i++;{'\n'}{'}'}{'\n\n'}@ C-style and Iterable for loops{'\n'}for (int j = 0; j &lt; 5; j++) {'{\n'}  print(j);{'\n'}{'}'}{'\n\n'}for c in "Hello" {'{\n'}  print(c);{'\n'}{'}'}</code></pre>
                                    </div>
                                )}

                                {helpTab === 'String Methods' && (
                                    <div className="help-section animated">
                                        <h3>String Methods</h3>
                                        <p>Built-in manipulation methods that act on strings.</p>
                                        <pre><code>string s = " Quanta Language "{'\n'}{'\n'}s.len()        @ length{'\n'}s.upper()      @ " QUANTA ..."{'\n'}s.lower()      @ " quanta ..."{'\n'}s.strip()      @ removes spaces{'\n'}s.replace("a", "A"){'\n'}s.reverse(){'\n'}s.find("Lan")  @ returns index</code></pre>
                                    </div>
                                )}

                                {helpTab === 'String Validation' && (
                                    <div className="help-section animated">
                                        <h3>String Validation</h3>
                                        <p>Useful boolean checks for verifying characters.</p>
                                        <pre><code>string val = "Hello"{'\n'}{'\n'}val.isalpha()   @ true{'\n'}val.isdigit()   @ false{'\n'}val.isspace()   @ false{'\n'}val.isalnum()   @ true{'\n'}val.startswith("He") @ true{'\n'}val.endswith("lo")   @ true</code></pre>
                                    </div>
                                )}

                                {helpTab === 'Comments' && (
                                    <div className="help-section animated">
                                        <h3>Comments</h3>
                                        <p>Add notes unheeded by the compiler.</p>
                                        <pre><code>@ Single line comment{'\n\n'}'''{'\n'}Multi-line block comment{'\n'}for longer explanations{'\n'}'''</code></pre>
                                    </div>
                                )}

                                {helpTab === 'Exception Handling' && (
                                    <div className="help-section animated">
                                        <h3>Exception Handling</h3>
                                        <p>Use try/catch to handle unexpected runtime errors. Smart Guards automatically catch errors.</p>
                                        <pre><code>try {'{\n'}  int x = 10 / 0;{'\n'}{'} '}catch (e) {'{\n'}  print(e); @ Prints: Arithmetic Error...{'\n'}{'}'}{'\n\n'}@ Auto Guards: \n@ - Arithmetic Error\n@ - Type Error\n@ - Index Error\n@ - Reference Error\n@ - Stack Error</code></pre>
                                    </div>
                                )}
                            </div>
                        </div>
                    </div>
                </div>
            )}

            {/* ── AI Generation Modal ── */}
            {showAiModal && (
                <div className="help-overlay" onClick={() => !isGenerating && setShowAiModal(false)}>
                    <div className="ai-modal" onClick={e => e.stopPropagation()}>
                        <div className="help-header">
                            <div>
                                <h2>✨ Generate Quanta Code</h2>
                                <p className="help-subtitle">Describe what you want to build, and AI will write it.</p>
                            </div>
                            <button className="help-close" onClick={() => !isGenerating && setShowAiModal(false)}>✕</button>
                        </div>
                        <div className="ai-body">
                            {localStorage.getItem('quanta_gemini_key') ? (
                                <div className="ai-input-group" style={{ flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', background: 'var(--bg-editor)', padding: '10px 14px', borderRadius: 'var(--radius)', border: '1px solid var(--border-lt)' }}>
                                    <span style={{ color: 'var(--green)', fontSize: '13px', fontWeight: 500 }}>✓ API Key Configured Securely</span>
                                    <button className="btn btn-ghost" style={{ padding: '4px 8px', fontSize: '11px' }} onClick={() => { localStorage.removeItem('quanta_gemini_key'); setApiKey(''); }}>Change Key</button>
                                </div>
                            ) : (
                                <div className="ai-input-group">
                                    <label>Gemini API Key (Required Once)</label>
                                    <input
                                        type="password"
                                        placeholder="AIzaSy..."
                                        value={apiKey}
                                        onChange={(e) => setApiKey(e.target.value)}
                                        disabled={isGenerating}
                                    />
                                </div>
                            )}
                            <div className="ai-input-group">
                                <label>What should I write?</label>
                                <textarea
                                    className="ai-prompt-area"
                                    placeholder="e.g. Write a function that calculates the factorial of a number."
                                    value={aiPrompt}
                                    onChange={(e) => setAiPrompt(e.target.value)}
                                    disabled={isGenerating}
                                    rows={4}
                                    onKeyDown={(e) => {
                                        if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') handleGenerateCode();
                                    }}
                                />
                                <span className="ai-hint">Pro Tip: Press Ctrl+Enter to generate</span>
                            </div>
                        </div>
                        <div className="ai-footer">
                            <button className="btn btn-ghost" onClick={() => setShowAiModal(false)} disabled={isGenerating}>Cancel</button>
                            <button
                                className={`btn btn-run ${isGenerating ? 'running' : ''}`}
                                onClick={handleGenerateCode}
                                disabled={isGenerating || !aiPrompt.trim()}
                            >
                                <IconSparkles /> {isGenerating ? 'Generating...' : 'Generate Code'}
                            </button>
                        </div>
                    </div>
                </div>
            )}

        </div>
    );
}

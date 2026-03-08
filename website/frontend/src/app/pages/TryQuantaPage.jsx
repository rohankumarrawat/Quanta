import { useState, useRef } from "react";
import { Play, RotateCcw, Copy, Check } from "lucide-react";

const EXAMPLES = [
    {
        label: "Hello, Quanta!",
        code: `@ My first Quanta program
print("Hello, Quanta!")
print("Welcome to the language!")`,
    },
    {
        label: "Variables & Types",
        code: `@ Implicit type inference
name = "Quanta"
score = 100
pi = 3.14159
flag = true

print("Name:", name)
print("Score:", score)
print("Pi:", pi)

@ Explicit integers
int8 tiny = 42
int32 medium = 50000
print("tiny:", tiny)
print("medium:", medium)`,
    },
    {
        label: "Control Flow",
        code: `score = 75

if (score > 90) {
    print("Grade A")
} elif (score > 70) {
    print("Grade B")
} elif (score > 50) {
    print("Grade C")
} else {
    print("Fail")
}

@ Truthiness
active = true
if (active) {
    print("System is active")
}`,
    },
    {
        label: "while & for Loops",
        code: `@ while loop
count = 0
while (count < 5) {
    print("Iteration:", count)
    count++
}

@ for loop
for (i = 0; i < 4; i++) {
    print("Step:", i)
}

@ Accumulator with while
sum = 0
x = 1
while (x <= 5) {
    sum = sum + x
    x++
}
print("Sum 1 to 5:", sum)`,
    },
    {
        label: "String Operations",
        code: `@ Dynamic string
name = "  quanta language  "
print(strip(name))
print(upper(name))
print(capitalize(name))
print(len(name))

@ String functions
msg = "Hello, Quanta!"
print(find(msg, "Quanta"))
print(replace(msg, "Quanta", "World"))
print(startswith(msg, "Hello"))
print(endswith(msg, "!"))`,
    },
    {
        label: "String Indexing & Slicing",
        code: `word = "Quanta"
print("Length:", len(word))
print("First:", word[0])
print("Last:", word[-1])
print("Slice:", word[0:3])
print("Reverse slice:", word[-1])

@ Loop over string
loop i in word {
    print(word[i])
}`,
    },
    {
        label: "Functions",
        code: `@ Function with explicit types
int add(int a, int b) {
    return a + b
}

@ Default arguments
int getArea(width = 10, height = 20) {
    return width * height
}

@ String function
string greet(string name) {
    return "Hello, " + name + "!"
}

print(add(5, 3))
print(getArea())
print(getArea(width=5))
print(greet("Rohan"))`,
    },
    {
        label: "Dual String System",
        code: `@ Dynamic heap string
string dynamic = "Hello Quanta"
print(dynamic)
print(upper(dynamic))

@ Static stack string — for embedded systems
@ string[16] device = "Sensor_A1"
@ string[8]  buf = "SuperLong"  -- stored as "SuperLo"

@ Concatenation
first = "Hello"
second = "World"
combined = first + ", " + second + "!"
print(combined)

@ var keyword (same as implicit)
var msg = "Quanta"
print(reverse(msg))`,
    },
];

// Simulate Quanta code execution client-side
function runQuantaCode(code) {
    const lines = code.split("\n");
    const output = [];
    const variables = {};

    function resolveValue(raw) {
        raw = raw.trim();
        // String literal
        if (raw.startsWith('"') && raw.endsWith('"')) return raw.slice(1, -1);
        // Number
        if (!isNaN(raw) && raw !== "") return Number(raw);
        // Boolean
        if (raw === "true") return true;
        if (raw === "false") return false;
        // Variable lookup
        if (variables[raw] !== undefined) return variables[raw];
        return raw;
    }

    function applyStringFn(fn, args) {
        const s = String(args[0]);
        if (fn === "upper" || fn === "upper") return s.toUpperCase();
        if (fn === "lower") return s.toLowerCase();
        if (fn === "strip") return s.trim();
        if (fn === "lstrip") return s.trimStart();
        if (fn === "rstrip") return s.trimEnd();
        if (fn === "len") return s.length;
        if (fn === "reverse") return s.split("").reverse().join("");
        if (fn === "capitalize") return s.charAt(0).toUpperCase() + s.slice(1).toLowerCase();
        if (fn === "title") return s.replace(/\b\w/g, c => c.toUpperCase());
        if (fn === "isupper") return /^[A-Z\s\W]*$/.test(s) && /[A-Z]/.test(s) ? 1 : 0;
        if (fn === "islower") return /^[a-z\s\W]*$/.test(s) && /[a-z]/.test(s) ? 1 : 0;
        if (fn === "isalpha") return /^[a-zA-Z]+$/.test(s) ? 1 : 0;
        if (fn === "isdigit") return /^\d+$/.test(s) ? 1 : 0;
        if (fn === "isspace") return /^\s+$/.test(s) ? 1 : 0;
        if (fn === "isalnum") return /^[a-zA-Z0-9]+$/.test(s) ? 1 : 0;
        if (fn === "find") return s.indexOf(String(args[1]));
        if (fn === "count") { const sub = String(args[1]); let c = 0, idx = 0; while ((idx = s.indexOf(sub, idx)) !== -1) { c++; idx += sub.length; } return c; }
        if (fn === "startswith") return s.startsWith(String(args[1])) ? 1 : 0;
        if (fn === "endswith") return s.endsWith(String(args[1])) ? 1 : 0;
        if (fn === "replace") return s.split(String(args[1])).join(String(args[2]));
        if (fn === "type") return typeof args[0] === "number" ? (Number.isInteger(args[0]) ? "int" : "float") : "string";
        if (fn === "bytesize") return typeof args[0] === "number" ? 8 : String(args[0]).length;
        return null;
    }

    function evalExpr(expr) {
        expr = expr.trim();
        // String function call
        const fnMatch = expr.match(/^(\w+)\((.*)?\)$/s);
        if (fnMatch) {
            const fn = fnMatch[1];
            const rawArgs = fnMatch[2] ? splitArgs(fnMatch[2]) : [];
            const args = rawArgs.map(a => resolveValue(a.trim()));
            const result = applyStringFn(fn, args);
            if (result !== null) return result;
        }
        // String indexing
        const idxMatch = expr.match(/^(\w+)\[(-?\d+)\]$/);
        if (idxMatch) {
            const s = String(resolveValue(idxMatch[1]));
            let idx = parseInt(idxMatch[2]);
            if (idx < 0) idx = s.length + idx;
            return s[idx] || "";
        }
        // String slicing
        const sliceMatch = expr.match(/^(\w+)\[(-?\d*):(-?\d*)\]$/);
        if (sliceMatch) {
            const s = String(resolveValue(sliceMatch[1]));
            let start = sliceMatch[2] !== "" ? parseInt(sliceMatch[2]) : 0;
            let end = sliceMatch[3] !== "" ? parseInt(sliceMatch[3]) : s.length;
            if (start < 0) start = s.length + start;
            if (end < 0) end = s.length + end;
            return s.slice(start, end);
        }
        // Arithmetic / concatenation
        // Simple addition of strings or numbers
        if (expr.includes(" + ")) {
            const parts = expr.split(" + ");
            const vals = parts.map(p => resolveValue(p.trim()));
            if (vals.some(v => typeof v === "string")) return vals.map(String).join("");
            return vals.reduce((a, b) => a + b, 0);
        }
        if (expr.includes(" - ")) {
            const parts = expr.split(" - ");
            return parts.map(p => resolveValue(p.trim())).reduce((a, b) => a - b);
        }
        if (expr.includes(" * ")) {
            const parts = expr.split(" * ");
            return parts.map(p => resolveValue(p.trim())).reduce((a, b) => a * b);
        }
        if (expr.includes(" / ")) {
            const [a, b] = expr.split(" / ").map(p => resolveValue(p.trim()));
            return Math.trunc(a / b);
        }
        if (expr.includes(" % ")) {
            const [a, b] = expr.split(" % ").map(p => resolveValue(p.trim()));
            return a % b;
        }
        return resolveValue(expr);
    }

    function splitArgs(s) {
        const args = []; let depth = 0; let cur = "";
        for (const ch of s) {
            if (ch === "(" || ch === "[") depth++;
            else if (ch === ")" || ch === "]") depth--;
            if (ch === "," && depth === 0) { args.push(cur.trim()); cur = ""; }
            else cur += ch;
        }
        if (cur.trim()) args.push(cur.trim());
        return args;
    }

    let i = 0;
    const loopStack = [];

    while (i < lines.length) {
        const raw = lines[i].trim();
        // Comment or blank or opening brace — skip (but NOT closing brace, which is handled by loopStack)
        if (raw.startsWith("@") || raw === "" || raw === "{") { i++; continue; }

        // Loop end
        if (raw === "}" && loopStack.length > 0) {
            const { condFn, startLine } = loopStack[loopStack.length - 1];
            if (condFn()) { i = startLine + 1; }
            else { loopStack.pop(); i++; }
            continue;
        }

        // Print statement
        const printMatch = raw.match(/^print\((.+)\)$/s);
        if (printMatch) {
            const args = splitArgs(printMatch[1]);
            const vals = args.map(a => evalExpr(a.trim()));
            output.push(vals.join(" "));
            i++; continue;
        }

        // while (condition) { — and loop (condition) { for backwards compat
        const whileCond = raw.match(/^(?:while|loop)\s*\((.+)\)\s*\{/);
        if (whileCond) {
            const condStr = whileCond[1];
            const condFn = () => {
                const opMatch = condStr.match(/(.+?)\s*(==|!=|<=|>=|<|>)\s*(.+)/);
                if (!opMatch) { const val = evalExpr(condStr.trim()); return !!val && val !== 0 && val !== "false"; }
                const a = evalExpr(opMatch[1].trim()), b = evalExpr(opMatch[3].trim()), op = opMatch[2];
                if (op === "==") return a == b; if (op === "!=") return a != b;
                if (op === "<") return a < b; if (op === ">") return a > b;
                if (op === "<=") return a <= b; if (op === ">=") return a >= b;
                return false;
            };
            if (condFn()) { loopStack.push({ condFn, startLine: i }); i++; }
            else { let depth = 1; i++; while (i < lines.length && depth > 0) { const l = lines[i].trim(); if (l.endsWith("{")) depth++; if (l === "}") depth--; i++; } }
            continue;
        }

        // for (init; condition; iter) {
        const forLoop = raw.match(/^for\s*\(\s*(.+?);\s*(.+?);\s*(.+?)\s*\)\s*\{/);
        if (forLoop) {
            const [, initExpr, condExpr, iterExpr] = forLoop;
            // Execute init
            const initAssign = initExpr.match(/^(\w+)\s*=\s*(.+)$/);
            if (initAssign) variables[initAssign[1]] = evalExpr(initAssign[2]);
            const condFn = () => {
                const opMatch = condExpr.trim().match(/(.+?)\s*(==|!=|<=|>=|<|>)\s*(.+)/);
                if (!opMatch) return false;
                const a = evalExpr(opMatch[1].trim()), b = evalExpr(opMatch[3].trim()), op = opMatch[2];
                if (op === "==") return a == b; if (op === "!=") return a != b;
                if (op === "<") return a < b; if (op === ">") return a > b;
                if (op === "<=") return a <= b; if (op === ">=") return a >= b;
                return false;
            };
            // Collect body
            const bodyStart = i + 1;
            let bd = 1, bi = bodyStart;
            while (bi < lines.length && bd > 0) { const l = lines[bi].trim(); if (l.endsWith("{")) bd++; if (l === "}") bd--; bi++; }
            const bodyEnd = bi - 1;
            let iters = 0;
            while (condFn() && iters++ < 10000) {
                for (let k = bodyStart; k < bodyEnd; k++) {
                    const bl = lines[k].trim();
                    if (!bl || bl.startsWith("@")) continue;
                    const pm = bl.match(/^print\((.+)\)$/s);
                    if (pm) { const args = splitArgs(pm[1]); output.push(args.map(a2 => evalExpr(a2.trim())).join(" ")); continue; }
                    if (bl.endsWith("++")) { const v = bl.slice(0, -2).trim(); variables[v] = (Number(resolveValue(v)) || 0) + 1; continue; }
                    if (bl.endsWith("--")) { const v = bl.slice(0, -2).trim(); variables[v] = (Number(resolveValue(v)) || 0) - 1; continue; }
                    const am = bl.match(/^(\w+)\s*=\s*(.+)$/);
                    if (am) variables[am[1]] = evalExpr(am[2]);
                }
                // Execute iter expression
                if (iterExpr.trim().endsWith("++")) { const v = iterExpr.trim().slice(0, -2).trim(); variables[v] = (Number(resolveValue(v)) || 0) + 1; }
                else if (iterExpr.trim().endsWith("--")) { const v = iterExpr.trim().slice(0, -2).trim(); variables[v] = (Number(resolveValue(v)) || 0) - 1; }
            }
            i = bi;
            continue;
        }

        // loop i in string {
        const loopIn = raw.match(/^loop\s+(\w+)\s+in\s+(\w+)\s*\{/);
        if (loopIn) {
            const indexVar = loopIn[1], strVar = loopIn[2];
            const strVal = String(resolveValue(strVar));
            const bodyLines = [];
            let j = i + 1, depth = 1;
            while (j < lines.length && depth > 0) {
                const l = lines[j].trim();
                if (l.endsWith("{")) depth++;
                if (l === "}") { depth--; if (depth === 0) break; }
                bodyLines.push(lines[j]);
                j++;
            }
            for (let k = 0; k < strVal.length; k++) {
                variables[indexVar] = k;
                for (const bl of bodyLines) {
                    const blr = bl.trim();
                    const pm = blr.match(/^print\((.+)\)$/s);
                    if (pm) { const args = splitArgs(pm[1]); const vals = args.map(a => evalExpr(a.trim())); output.push(vals.join(" ")); }
                }
            }
            delete variables[indexVar];
            i = j + 1;
            continue;
        }

        // if / elif / else
        const ifMatch = raw.match(/^(if|elif)\s*\((.+)\)\s*\{/);
        if (ifMatch) {
            const condStr = ifMatch[2];
            const opMatch = condStr.match(/(.+?)\s*(==|!=|<=|>=|<|>)\s*(.+)/);
            let result = false;
            if (opMatch) {
                const a = evalExpr(opMatch[1].trim()), b = evalExpr(opMatch[3].trim()), op = opMatch[2];
                if (op === "==") result = a == b; if (op === "!=") result = a != b;
                if (op === "<") result = a < b; if (op === ">") result = a > b;
                if (op === "<=") result = a <= b; if (op === ">=") result = a >= b;
            } else {
                const val = evalExpr(condStr.trim());
                result = !!val && val !== 0 && val !== "false";
            }
            if (result) { i++; continue; }
            else { let depth = 1; i++; while (i < lines.length && depth > 0) { const l = lines[i].trim(); if (l.endsWith("{")) depth++; if (l === "}") depth--; i++; } continue; }
        }

        if (raw === "} else {") { i++; continue; }
        if (raw === "else {") { i++; continue; }

        // Increment / Decrement
        if (raw.endsWith("++")) { const v = raw.slice(0, -2).trim(); variables[v] = (Number(resolveValue(v)) || 0) + 1; i++; continue; }
        if (raw.endsWith("--")) { const v = raw.slice(0, -2).trim(); variables[v] = (Number(resolveValue(v)) || 0) - 1; i++; continue; }

        // Assignment
        const assignMatch = raw.match(/^(?:var|int\d*|float\d*|string(?:\[\d+\])?|bool|char)?\s*(\w+)\s*=\s*(.+)$/);
        if (assignMatch) {
            const varName = assignMatch[1].trim();
            if (!["if", "else", "elif", "loop", "while", "for", "print", "return", "void", "import"].includes(varName)) {
                variables[varName] = evalExpr(assignMatch[2].trim());
                i++; continue;
            }
        }
        i++;
    }

    return output.length > 0 ? output.join("\n") : "(No output)";
}

export function TryQuantaPage() {
    const [code, setCode] = useState(EXAMPLES[0].code);
    const [output, setOutput] = useState("");
    const [isRunning, setIsRunning] = useState(false);
    const [selectedExample, setSelectedExample] = useState(0);
    const [copied, setCopied] = useState(false);
    const [hasRun, setHasRun] = useState(false);
    const textareaRef = useRef(null);

    const handleRun = () => {
        setIsRunning(true);
        setHasRun(true);
        setTimeout(() => {
            try {
                const result = runQuantaCode(code);
                setOutput(result);
            } catch (e) {
                setOutput("Runtime error: " + e.message);
            }
            setIsRunning(false);
        }, 300);
    };

    const handleReset = () => {
        setCode(EXAMPLES[selectedExample].code);
        setOutput("");
        setHasRun(false);
    };

    const handleCopy = () => {
        navigator.clipboard.writeText(code);
        setCopied(true);
        setTimeout(() => setCopied(false), 2000);
    };

    const handleExampleChange = (index) => {
        setSelectedExample(index);
        setCode(EXAMPLES[index].code);
        setOutput("");
        setHasRun(false);
    };

    const handleTabKey = (e) => {
        if (e.key === "Tab") {
            e.preventDefault();
            const start = e.target.selectionStart;
            const end = e.target.selectionEnd;
            const newCode = code.substring(0, start) + "    " + code.substring(end);
            setCode(newCode);
            setTimeout(() => {
                if (textareaRef.current) {
                    textareaRef.current.selectionStart = start + 4;
                    textareaRef.current.selectionEnd = start + 4;
                }
            }, 0);
        }
    };

    return (
        <div className="space-y-6">
            {/* Header */}
            <div className="space-y-3">
                <h1 className="text-4xl font-bold">
                    Try{" "}
                    <span className="bg-gradient-to-r from-[#22d3ee] to-[#a855f7] bg-clip-text text-transparent">
                        Quanta
                    </span>
                    {" "}Online
                </h1>
                <p className="text-muted-foreground text-lg">
                    Write, run, and explore Quanta code directly in your browser. Select an example or write your own.
                </p>
            </div>

            {/* Example Selector */}
            <div className="flex flex-wrap items-center gap-2">
                <span className="text-sm text-muted-foreground font-medium">Examples:</span>
                <div className="flex flex-wrap gap-2">
                    {EXAMPLES.map((ex, index) => (
                        <button
                            key={index}
                            onClick={() => handleExampleChange(index)}
                            className={`px-3 py-1.5 rounded-lg text-xs font-medium transition-all ${selectedExample === index
                                ? "bg-gradient-to-r from-[#22d3ee]/20 to-[#a855f7]/20 text-[#22d3ee] border border-[#22d3ee]/30"
                                : "text-muted-foreground hover:text-foreground border border-transparent hover:border-border hover:bg-card"
                                }`}
                        >
                            {ex.label}
                        </button>
                    ))}
                </div>
            </div>

            {/* Editor + Output */}
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
                {/* Editor Panel */}
                <div className="flex flex-col rounded-xl border border-border overflow-hidden">
                    <div className="flex items-center justify-between px-4 py-2.5 bg-muted/30 border-b border-border">
                        <div className="flex items-center gap-2">
                            <div className="w-3 h-3 rounded-full bg-red-500/70" />
                            <div className="w-3 h-3 rounded-full bg-yellow-500/70" />
                            <div className="w-3 h-3 rounded-full bg-green-500/70" />
                            <span className="ml-2 text-xs text-muted-foreground font-mono">editor.qnt</span>
                        </div>
                        <div className="flex items-center gap-2">
                            <button
                                onClick={handleCopy}
                                className="flex items-center gap-1.5 px-2.5 py-1 rounded text-xs text-muted-foreground hover:text-foreground hover:bg-muted transition-all"
                            >
                                {copied ? <Check className="w-3.5 h-3.5 text-green-400" /> : <Copy className="w-3.5 h-3.5" />}
                                {copied ? "Copied!" : "Copy"}
                            </button>
                            <button
                                onClick={handleReset}
                                className="flex items-center gap-1.5 px-2.5 py-1 rounded text-xs text-muted-foreground hover:text-foreground hover:bg-muted transition-all"
                            >
                                <RotateCcw className="w-3.5 h-3.5" />
                                Reset
                            </button>
                        </div>
                    </div>
                    <textarea
                        ref={textareaRef}
                        value={code}
                        onChange={(e) => setCode(e.target.value)}
                        onKeyDown={handleTabKey}
                        className="flex-1 bg-background text-foreground font-mono text-sm p-5 resize-none outline-none min-h-[420px] leading-relaxed"
                        spellCheck={false}
                        style={{ tabSize: 4 }}
                    />
                    <div className="flex items-center justify-between px-4 py-3 bg-muted/20 border-t border-border">
                        <span className="text-xs text-muted-foreground font-mono">
                            {code.split("\n").length} lines
                        </span>
                        <button
                            onClick={handleRun}
                            disabled={isRunning}
                            className="flex items-center gap-2 px-5 py-2 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium text-sm hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all hover:scale-105 disabled:opacity-60 disabled:cursor-not-allowed disabled:hover:scale-100"
                        >
                            <Play className="w-4 h-4" />
                            {isRunning ? "Running..." : "Run →"}
                        </button>
                    </div>
                </div>

                {/* Output Panel */}
                <div className="flex flex-col rounded-xl border border-border overflow-hidden">
                    <div className="flex items-center gap-2 px-4 py-2.5 bg-muted/30 border-b border-border">
                        <img src="/logo.jpeg" alt="Quanta" className="w-4 h-4 rounded object-cover" />
                        <span className="text-xs text-muted-foreground font-mono">output</span>
                        {hasRun && (
                            <span className="ml-auto text-xs text-green-400 font-mono">● executed</span>
                        )}
                    </div>
                    <div className="flex-1 bg-black/40 p-5 font-mono text-sm leading-relaxed min-h-[420px] overflow-auto">
                        {!hasRun ? (
                            <div className="flex flex-col items-center justify-center h-full text-center gap-3 text-muted-foreground">
                                <Play className="w-8 h-8 opacity-30" />
                                <p className="text-sm">Click <strong className="text-foreground">Run →</strong> to execute your Quanta code</p>
                            </div>
                        ) : isRunning ? (
                            <div className="flex items-center gap-2 text-[#22d3ee]">
                                <div className="w-2 h-2 rounded-full bg-[#22d3ee] animate-pulse" />
                                <span>Running...</span>
                            </div>
                        ) : (
                            <pre className="text-green-400 whitespace-pre-wrap">{output}</pre>
                        )}
                    </div>
                    <div className="px-4 py-3 bg-muted/20 border-t border-border">
                        <p className="text-xs text-muted-foreground">
                            🛈 This is a browser-based Quanta simulator. For full compilation,{" "}
                            <a href="/quickstart" className="text-[#22d3ee] hover:underline">install Quanta</a> on your machine.
                        </p>
                    </div>
                </div>
            </div>

            {/* Quick Reference */}
            <div className="bg-card border border-border rounded-xl p-6">
                <h3 className="text-lg font-semibold mb-4">Quick Reference</h3>
                <div className="grid grid-cols-1 md:grid-cols-3 gap-6 text-sm">
                    <div>
                        <p className="text-[#22d3ee] font-medium mb-2">Variables</p>
                        <pre className="text-muted-foreground font-mono text-xs leading-relaxed">{`x = 100           @ implicit
int8 y = 42       @ explicit
var z = "text"    @ var keyword`}</pre>
                    </div>
                    <div>
                        <p className="text-[#22d3ee] font-medium mb-2">Loops & Control</p>
                        <pre className="text-muted-foreground font-mono text-xs leading-relaxed">{`while (i < 5) { }
for (i=0; i<n; i++) { }
loop i in str { }
if (x > 0) { }`}</pre>
                    </div>
                    <div>
                        <p className="text-[#22d3ee] font-medium mb-2">String Functions</p>
                        <pre className="text-muted-foreground font-mono text-xs leading-relaxed">{`upper(s)  lower(s)
strip(s)  reverse(s)
find(s, sub)
replace(s, old, new)`}</pre>
                    </div>
                </div>
            </div>
        </div>
    );
}

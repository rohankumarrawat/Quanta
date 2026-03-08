import { Copy, Check } from "lucide-react";
import { useState } from "react";

export function CodeBlock({ code, language = "bash" }) {
    const [copied, setCopied] = useState(false);

    const handleCopy = () => {
        navigator.clipboard.writeText(code);
        setCopied(true);
        setTimeout(() => setCopied(false), 2000);
    };

    return (
        <div className="relative group">
            <div className="absolute top-4 right-4 z-10">
                <button
                    onClick={handleCopy}
                    className="p-2 rounded-lg bg-muted/50 backdrop-blur-sm text-muted-foreground hover:text-foreground hover:bg-muted transition-all opacity-0 group-hover:opacity-100"
                >
                    {copied ? <Check className="w-4 h-4" /> : <Copy className="w-4 h-4" />}
                </button>
            </div>
            <div className="bg-card border border-border rounded-xl p-6 overflow-x-auto">
                <div className="flex items-center gap-2 mb-4">
                    <div className="flex gap-1.5">
                        <div className="w-3 h-3 rounded-full bg-red-500/80" />
                        <div className="w-3 h-3 rounded-full bg-yellow-500/80" />
                        <div className="w-3 h-3 rounded-full bg-green-500/80" />
                    </div>
                    <span className="text-xs text-muted-foreground ml-2">{language}</span>
                </div>
                <pre className="text-sm text-foreground font-mono leading-relaxed">
                    <code>{code}</code>
                </pre>
            </div>
        </div>
    );
}

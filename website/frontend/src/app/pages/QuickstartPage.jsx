import { LinkCard } from "../components/LinkCard";
import { Download, CheckCircle2, FileText } from "lucide-react";
import { CodeBlock } from "../components/CodeBlock";

export function QuickstartPage() {
    return (
        <div className="space-y-12">
            {/* Header */}
            <div className="space-y-4">
                <div className="flex items-center gap-3 text-muted-foreground text-sm">
                    <span>Documentation</span>
                    <span>/</span>
                    <span>Quickstart Guide</span>
                </div>
                <h1 className="text-5xl font-bold">Quickstart Guide</h1>
                <p className="text-xl text-muted-foreground leading-relaxed max-w-3xl">
                    Get up and running with Quanta in minutes. Install the software on your laptop
                    and run your first <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">.qnt</code> program.
                </p>
            </div>

            {/* Installation Section */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#a855f7]/20 to-[#22d3ee]/20 flex items-center justify-center">
                        <Download className="w-4 h-4 text-[#a855f7]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Install Quanta</h2>
                </div>

                <p className="text-muted-foreground mb-4">
                    To get started, simply download the installer for your operating system below and run it on your laptop.
                    The setup wizard will automatically configure your Quanta environment.
                </p>

                <div className="grid gap-4">
                    <LinkCard title="Download for macOS (Apple Silicon)" href="/downloads/Quanta Studio-1.0.0-arm64.dmg" download="Quanta Studio-1.0.0-arm64.dmg" />
                    <LinkCard title="Download for macOS (Intel)" href="/downloads/Quanta Studio-1.0.0-arm64.dmg" download="Quanta Studio-1.0.0-arm64.dmg" />

                    <div className="bg-orange-500/10 border border-orange-500/20 rounded-xl p-4 mt-1 mb-2">
                        <h4 className="text-orange-400 font-medium text-sm mb-1">macOS Installation Note</h4>
                        <p className="text-xs text-muted-foreground leading-relaxed mb-3">
                            After downloading, macOS may mistakenly say the app is "damaged" because it is an unsigned open-source binary. To fix this, open Terminal and run the following command to bypass Gatekeeper:
                        </p>
                        <code className="block bg-black/50 p-2 rounded text-xs text-orange-300 font-mono select-all">
                            xattr -cr "/Applications/Quanta Studio.app"
                        </code>
                    </div>

                    <LinkCard title="Download for Windows" href="/downloads/Quanta_Installer_v1.0.exe" download="Quanta_Installer_v1.0.exe" />
                    <LinkCard title="Download for Linux" href="/downloads/Quanta Studio-1.0.0.AppImage" download="Quanta Studio-1.0.0.AppImage" />
                </div>
            </div>

            {/* Your First Program */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#22d3ee]/20 to-[#a855f7]/20 flex items-center justify-center">
                        <FileText className="w-4 h-4 text-[#22d3ee]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Your First Program</h2>
                </div>

                <p className="text-muted-foreground">
                    Create a new file named <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">hello.qnt</code> and write the following:
                </p>

                <CodeBlock
                    code={`@ My first Quanta program
print("Hello, Quanta!")`}
                    language="quanta"
                />

                <p className="text-muted-foreground">
                    The <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">@</code> symbol is used for comments — anything after it on the same line is ignored by the compiler.
                </p>
            </div>

            {/* Verify Installation */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#22d3ee]/20 to-[#a855f7]/20 flex items-center justify-center">
                        <CheckCircle2 className="w-4 h-4 text-[#22d3ee]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Run Your Program</h2>
                </div>

                <p className="text-muted-foreground">
                    After installation, open a terminal and run your program using the Quanta compiler:
                </p>

                <CodeBlock
                    code={`quanta hello.qnt`}
                    language="bash"
                />

                <div className="bg-muted/30 border border-border rounded-xl p-5">
                    <p className="text-sm text-muted-foreground mb-2 font-medium">Expected Output:</p>
                    <pre className="font-mono text-sm text-green-400">Hello, Quanta!</pre>
                </div>

                <p className="text-muted-foreground">
                    That's it! You've successfully run your first Quanta program.
                </p>
            </div>

            {/* Quick Syntax Overview */}
            <div className="space-y-6">
                <h2 className="text-2xl font-semibold">Quick Syntax Overview</h2>
                <p className="text-muted-foreground">Here's a taste of what Quanta looks like:</p>

                <CodeBlock
                    code={`@ Variables — Quanta infers types automatically
name = "Quanta"
score = 100
pi = 3.14

@ Explicit types for embedded / precision control
int8 tiny = 42
string[16] device = "Sensor_A1"

@ Control flow
if (score > 90) {
    print("Excellent!")
} elif (score > 70) {
    print("Good job")
} else {
    print("Keep going!")
}

@ while loop
count = 0
while (count < 5) {
    print("Iteration:", count)
    count++
}`}
                    language="quanta"
                />
            </div>

            {/* Next Steps */}
            <div className="bg-gradient-to-r from-[#22d3ee]/10 to-[#a855f7]/10 border border-[#22d3ee]/20 rounded-xl p-8">
                <h3 className="text-xl font-semibold mb-3">Next Steps</h3>
                <p className="text-muted-foreground mb-6">
                    Now that Quanta is installed, explore the core syntax or try writing code directly in your browser.
                </p>
                <div className="flex gap-4">
                    <a
                        href="/syntax"
                        className="inline-flex items-center px-5 py-2.5 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all"
                    >
                        Learn Core Syntax →
                    </a>
                    <a
                        href="/try"
                        className="inline-flex items-center px-5 py-2.5 rounded-lg border border-border text-foreground font-medium hover:bg-card transition-all"
                    >
                        Try Quanta Online ⚡
                    </a>
                </div>
            </div>
        </div>
    );
}

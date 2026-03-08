import { InfoCard } from "../components/InfoCard";
import { Badge } from "../components/Badge";
import { Zap, Shield, Cpu, Globe, FileText, Sparkles, Megaphone } from "lucide-react";

export function IntroductionPage() {
    const latestUpdates = [
        {
            icon: FileText,
            type: "Release Note",
            title: "Quanta v1.0 – LLVM Backend",
            description: "Full LLVM-based compilation pipeline, delivering native-speed executables on desktop and embedded targets.",
            date: "March 1, 2026",
            accentColor: "from-[#22d3ee]",
        },
        {
            icon: Sparkles,
            type: "New Feature",
            title: "Dual String System",
            description: "Dynamic heap strings and static stack-allocated string[N] buffers — pick the right memory model for your target device.",
            date: "February 24, 2026",
            accentColor: "from-[#a855f7]",
        },
        {
            icon: Megaphone,
            type: "Announcement",
            title: "Community Forum Now Live",
            description: "Join discussions, ask questions, and share Quanta projects with the growing developer community.",
            date: "February 15, 2026",
            accentColor: "from-[#22d3ee]",
        },
    ];

    return (
        <div className="space-y-12">
            {/* Hero Section */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <Badge variant="primary">v1.0 Stable</Badge>
                    <Badge variant="secondary">Open Source</Badge>
                </div>

                <h1 className="text-4xl md:text-6xl font-bold leading-tight">
                    Welcome to{" "}
                    <span className="bg-gradient-to-r from-[#22d3ee] to-[#a855f7] bg-clip-text text-transparent">
                        Quanta
                    </span>
                </h1>

                <p className="text-xl text-muted-foreground leading-relaxed max-w-3xl">
                    A modern, memory-safe programming language designed to bridge the gap between
                    high-level scripting and low-level systems programming. Write clean, readable
                    code that compiles to blazing-fast native executables — perfect for both
                    desktop software and constrained embedded devices.
                </p>

                <div className="flex flex-col sm:flex-row gap-4">
                    <a
                        href="/quickstart"
                        className="inline-flex items-center justify-center px-6 py-3 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all hover:scale-105"
                    >
                        Get Started →
                    </a>
                    <a
                        href="/try"
                        className="inline-flex items-center justify-center px-6 py-3 rounded-lg border border-[#22d3ee]/30 text-[#22d3ee] font-medium hover:bg-[#22d3ee]/10 transition-all"
                    >
                        Try Quanta Online ⚡
                    </a>
                </div>
            </div>

            {/* Hello World Snippet */}
            <div className="bg-card border border-border rounded-xl overflow-hidden">
                <div className="flex items-center gap-2 px-5 py-3 border-b border-border bg-muted/30">
                    <div className="w-3 h-3 rounded-full bg-red-500/70" />
                    <div className="w-3 h-3 rounded-full bg-yellow-500/70" />
                    <div className="w-3 h-3 rounded-full bg-green-500/70" />
                    <span className="ml-3 text-xs text-muted-foreground font-mono">hello.qnt</span>
                </div>
                <pre className="p-6 text-sm font-mono overflow-x-auto leading-relaxed">
                    <span className="text-muted-foreground">{"@ My first Quanta program\n"}</span>
                    <span className="text-[#22d3ee]">{"name"}</span>
                    <span className="text-foreground">{" = "}</span>
                    <span className="text-[#a855f7]">{"\"Quanta\"\n"}</span>
                    <span className="text-[#22d3ee]">{"count"}</span>
                    <span className="text-foreground">{" = 0\n\n"}</span>
                    <span className="text-[#22d3ee]">{"while"}</span>
                    <span className="text-foreground">{"("}</span>
                    <span className="text-[#22d3ee]">{"count"}</span>
                    <span className="text-foreground">{" < 3) {\n"}</span>
                    <span className="text-foreground">{"    print("}</span>
                    <span className="text-[#a855f7]">{"\"Hello from\""}</span>
                    <span className="text-foreground">{", "}</span>
                    <span className="text-[#22d3ee]">{"name"}</span>
                    <span className="text-foreground">{")\n"}</span>
                    <span className="text-foreground">{"    "}</span>
                    <span className="text-[#22d3ee]">{"count"}</span>
                    <span className="text-foreground">{"++\n}\n"}</span>
                </pre>
            </div>

            {/* Latest Updates Section */}
            <div className="space-y-6">
                <h2 className="text-3xl font-bold">Latest Updates</h2>
                <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
                    {latestUpdates.map((update, index) => {
                        const Icon = update.icon;
                        return (
                            <div
                                key={index}
                                className="group bg-card border border-border rounded-xl p-6 hover:border-[#22d3ee]/30 transition-all cursor-pointer"
                            >
                                <div className={`w-12 h-12 rounded-lg bg-gradient-to-br ${update.accentColor} to-[#a855f7]/20 flex items-center justify-center mb-4`}>
                                    <Icon className="w-6 h-6 text-[#22d3ee]" />
                                </div>
                                <div className="text-xs font-medium text-[#22d3ee] mb-2">
                                    {update.type}
                                </div>
                                <h3 className="text-lg font-semibold mb-2 group-hover:text-[#22d3ee] transition-colors">
                                    {update.title}
                                </h3>
                                <p className="text-sm text-muted-foreground mb-4">
                                    {update.description}
                                </p>
                                <div className="text-xs text-muted-foreground">
                                    {update.date}
                                </div>
                            </div>
                        );
                    })}
                </div>
            </div>

            {/* Info Cards */}
            <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                <InfoCard
                    icon={Zap}
                    title="LLVM Powered"
                    description="Quanta compiles to native machine code via the LLVM backend, delivering C-level performance without the complexity of manual memory management."
                />
                <InfoCard
                    icon={Shield}
                    title="Memory Safe"
                    description="Dual string system with auto-free tracking for heap strings and safe stack-allocated buffers. No garbage collector pauses, no memory leaks."
                />
                <InfoCard
                    icon={Cpu}
                    title="Embedded Ready"
                    description="Use string[N] static buffers and stack-only loops to write deterministic, heap-free code for microcontrollers and IoT devices."
                />
                <InfoCard
                    icon={Globe}
                    title="Simple Syntax"
                    description="Minimal reserved keywords. Write x = 100 for quick scripts or int8 x = 100 for embedded precision. One unified loop construct replaces while, for, and do-while."
                />
            </div>

            {/* Getting Started */}
            <div className="bg-card border border-border rounded-xl p-8">
                <h2 className="text-2xl font-semibold mb-4">Ready to get started?</h2>
                <p className="text-muted-foreground mb-6">
                    Download and install Quanta, then follow our quickstart guide to run your first <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">.qnt</code> program in minutes.
                </p>
                <div className="flex flex-col sm:flex-row gap-4">
                    <a
                        href="/quickstart"
                        className="inline-flex items-center justify-center px-6 py-3 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all hover:scale-105"
                    >
                        Quickstart Guide →
                    </a>
                    <a
                        href="/syntax"
                        className="inline-flex items-center justify-center px-6 py-3 rounded-lg border border-border text-foreground font-medium hover:bg-card transition-all"
                    >
                        Core Syntax
                    </a>
                </div>
            </div>
        </div>
    );
}

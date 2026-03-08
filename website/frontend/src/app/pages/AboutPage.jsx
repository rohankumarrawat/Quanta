import { Badge } from "../components/Badge";
import { Sparkles, Target, Gauge, Boxes, Github, Linkedin, Users } from "lucide-react";

export function AboutPage() {
    return (
        <div className="space-y-12">
            {/* Header */}
            <div className="space-y-4">
                <div className="flex items-center gap-3 text-muted-foreground text-sm">
                    <span>Documentation</span>
                    <span>/</span>
                    <span>About</span>
                </div>
                <h1 className="text-4xl md:text-5xl font-bold">About Quanta</h1>
                <p className="text-lg md:text-xl text-muted-foreground leading-relaxed max-w-3xl">
                    Learn about the philosophy, design principles, and vision behind Quanta —
                    the programming language built to be beautiful, predictable, and memory-safe out of the box.
                </p>
            </div>

            {/* Purpose Section */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#22d3ee]/20 to-[#a855f7]/20 flex items-center justify-center">
                        <Target className="w-4 h-4 text-[#22d3ee]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Purpose</h2>
                </div>

                <div className="prose prose-invert max-w-none space-y-4">
                    <p className="text-muted-foreground leading-relaxed">
                        Historically, developers have been forced to choose between two extremes. High-level languages
                        like Python are easy to read and manage memory automatically — but they're too heavy and slow
                        for small microcontrollers. Low-level languages like C or Rust are fast and hardware-precise,
                        but they force you to manually manage every byte of memory, leading to complex code and hard-to-track bugs.
                    </p>
                    <p className="text-muted-foreground leading-relaxed">
                        <strong className="text-foreground">Quanta bridges this gap.</strong> It was designed from the ground up to let you
                        write clean, readable code that compiles to highly efficient native instructions via the
                        LLVM backend — running perfectly on both desktop computers and tiny embedded devices.
                        You can write <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">x = 100</code> and
                        let Quanta figure it out, or write <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">int8 x = 100</code> when
                        every single byte matters.
                    </p>
                    <p className="text-muted-foreground leading-relaxed">
                        Quanta was created by <strong className="text-foreground">Rohan Kumar Rawat</strong>, driven by a vision to build
                        developer tools that are predictable, fast, and memory-safe out of the box — without
                        sacrificing the ease of writing.
                    </p>
                </div>
            </div>

            {/* Benefits Section */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#a855f7]/20 to-[#22d3ee]/20 flex items-center justify-center">
                        <Sparkles className="w-4 h-4 text-[#a855f7]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Key Benefits</h2>
                </div>

                <div className="grid gap-6">
                    <div className="bg-card border border-border rounded-xl p-6">
                        <h3 className="font-semibold mb-2 flex items-center gap-2">
                            <Badge variant="primary">Dual String System</Badge>
                        </h3>
                        <p className="text-muted-foreground leading-relaxed">
                            Quanta's unique memory system gives you two string types in one language.
                            Dynamic <code className="px-1 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">string</code> for
                            heap-allocated flexible text in applications, and static{" "}
                            <code className="px-1 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">string[N]</code> for
                            stack-allocated, overflow-safe buffers on embedded devices. No garbage collector pauses.
                            No memory leaks. The auto-free mechanism cleans up heap operations automatically.
                        </p>
                    </div>

                    <div className="bg-card border border-border rounded-xl p-6">
                        <h3 className="font-semibold mb-2 flex items-center gap-2">
                            <Badge variant="primary">Intelligent Type Inference</Badge>
                        </h3>
                        <p className="text-muted-foreground leading-relaxed">
                            Quanta's compiler seamlessly figures out data types when you just want code to run.
                            Write <code className="px-1 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">score = 100</code> and
                            Quanta assigns the safest type. When precision matters, you take full control with
                            explicit types like <code className="px-1 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">int8</code>,{" "}
                            <code className="px-1 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">int32</code>, or{" "}
                            <code className="px-1 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">float</code>.
                        </p>
                    </div>

                    <div className="bg-card border border-border rounded-xl p-6">
                        <h3 className="font-semibold mb-2 flex items-center gap-2">
                            <Badge variant="primary">LLVM Native Performance</Badge>
                        </h3>
                        <p className="text-muted-foreground leading-relaxed">
                            Quanta compiles directly to native machine code using the LLVM backend. This delivers
                            C-level execution speed while keeping a clean, high-level syntax. No interpreter overhead,
                            no virtual machine — just fast, efficient binaries.
                        </p>
                    </div>
                </div>
            </div>

            {/* Performance Section */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#22d3ee]/20 to-[#a855f7]/20 flex items-center justify-center">
                        <Gauge className="w-4 h-4 text-[#22d3ee]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Design Principles</h2>
                </div>

                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                    {[
                        { title: "Few Keywords", desc: "A minimal reserved keyword set keeps Quanta easy to learn and read." },
                        { title: "Unified Loop", desc: "One loop construct replaces while, for, and do-while — eliminating syntactic confusion." },
                        { title: "No Switch Cases", desc: "The compiler automatically converts elif chains into optimized switch logic behind the scenes." },
                        { title: "Safe Truncation", desc: "Assigning a long string to a string[N] buffer safely truncates rather than overrunning memory." },
                    ].map((item) => (
                        <div key={item.title} className="flex gap-3 items-start">
                            <div className="w-1.5 h-1.5 rounded-full bg-[#22d3ee] mt-2 flex-shrink-0" />
                            <div>
                                <p className="font-medium">{item.title}</p>
                                <p className="text-sm text-muted-foreground">{item.desc}</p>
                            </div>
                        </div>
                    ))}
                </div>
            </div>

            {/* Use Cases Section */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#a855f7]/20 to-[#22d3ee]/20 flex items-center justify-center">
                        <Boxes className="w-4 h-4 text-[#a855f7]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Use Cases</h2>
                </div>

                <div className="space-y-4">
                    {[
                        { title: "Desktop Application Scripting", gradient: "from-[#22d3ee]/5 to-[#a855f7]/5", desc: "Build fast command-line tools, automation scripts, and utility applications with clean, readable Quanta syntax and zero GC overhead." },
                        { title: "Embedded Systems & IoT", gradient: "from-[#a855f7]/5 to-[#22d3ee]/5", desc: "Use string[N] static buffers and stack-only loop indexing to write deterministic, heap-free firmware for microcontrollers and IoT devices." },
                        { title: "Systems Programming", gradient: "from-[#22d3ee]/5 to-[#a855f7]/5", desc: "Leverage explicit int8, int32, int64 types and LLVM-compiled native binaries to write performant systems-level code without C's complexity." },
                        { title: "Education & Learning", gradient: "from-[#a855f7]/5 to-[#22d3ee]/5", desc: "Quanta's simple syntax, unified loop, and automatic type inference make it an excellent first language for developers learning systems-level concepts." },
                    ].map((uc) => (
                        <div key={uc.title} className={`bg-gradient-to-r ${uc.gradient} border border-border rounded-lg p-5`}>
                            <h4 className="font-semibold mb-2">{uc.title}</h4>
                            <p className="text-sm text-muted-foreground">{uc.desc}</p>
                        </div>
                    ))}
                </div>
            </div>

            {/* Contributors Section */}
            <div className="space-y-6">
                <div className="flex items-center gap-3">
                    <div className="w-8 h-8 rounded-lg bg-gradient-to-br from-[#22d3ee]/20 to-[#a855f7]/20 flex items-center justify-center">
                        <Users className="w-4 h-4 text-[#22d3ee]" />
                    </div>
                    <h2 className="text-2xl font-semibold">Contributors</h2>
                </div>

                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
                    {[
                        {
                            name: "Harsh Gupta",
                            github: "https://github.com/RealHarshGupta",
                            linkedin: "https://www.linkedin.com/in/harsh-gupta-22a24a286"
                        },
                        {
                            name: "Naman Gupta",
                            github: "https://github.com/Naman-27072004",
                            linkedin: "https://www.linkedin.com/in/namangupta27072004/"
                        },
                        {
                            name: "Shivam Gupta",
                            github: "https://www.github.com/shivamgupta1112",
                            linkedin: "https://linkedin.com/in/shivamgupta11122004/"
                        },
                        {
                            name: "Jiya Jaisingh",
                            github: "https://github.com/jiyajaisingh31",
                            linkedin: "https://www.linkedin.com/in/jiya-jaisingh3110"
                        },
                        {
                            name: "Rupanshi Varshney",
                            github: "https://github.com/Rupsvarshney",
                            linkedin: "https://www.linkedin.com/in/rupanshi-varshney-7630a6270"
                        },
                        {
                            name: "Hardik Dhawan",
                            github: "https://github.com/HardikDhawan9311",
                            linkedin: "https://www.linkedin.com/in/hardikk-dhawann/"
                        },
                        {
                            name: "Siddharth Gupta",
                            github: "https://github.com/siddharthgupta23",
                            linkedin: "https://www.linkedin.com/in/siddharth-gupta-05a65225b"
                        }
                    ].map((person) => (
                        <div key={person.name} className="bg-card border border-border rounded-xl p-6 flex flex-col gap-4 hover:border-[#22d3ee]/50 transition-colors">
                            <h3 className="font-semibold text-lg">{person.name}</h3>
                            <div className="flex items-center gap-3">
                                <a href={person.github} target="_blank" rel="noopener noreferrer" className="text-muted-foreground hover:text-foreground transition-colors" title="GitHub Profile">
                                    <Github className="w-5 h-5" />
                                </a>
                                <a href={person.linkedin} target="_blank" rel="noopener noreferrer" className="text-muted-foreground hover:text-[#0a66c2] transition-colors" title="LinkedIn Profile">
                                    <Linkedin className="w-5 h-5" />
                                </a>
                            </div>
                        </div>
                    ))}
                </div>
            </div>

            {/* Footer CTA */}
            <div className="bg-gradient-to-r from-[#22d3ee]/10 to-[#a855f7]/10 border border-[#22d3ee]/20 rounded-xl p-8 text-center">
                <h3 className="text-2xl font-semibold mb-3">Start Building with Quanta</h3>
                <p className="text-muted-foreground mb-6 max-w-2xl mx-auto">
                    Quanta is open source and actively developed. Download the installer and start writing
                    your first <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">.qnt</code> program today.
                </p>
                <div className="flex flex-col sm:flex-row gap-4 justify-center">
                    <a
                        href="/quickstart"
                        className="inline-flex items-center justify-center px-6 py-3 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all"
                    >
                        Get Started
                    </a>
                    <a
                        href="https://github.com/rohankumarrawat/quanta"
                        target="_blank"
                        rel="noopener noreferrer"
                        className="inline-flex items-center justify-center px-6 py-3 rounded-lg border border-border text-foreground font-medium hover:bg-card transition-all"
                    >
                        View on GitHub
                    </a>
                </div>
            </div>
        </div>
    );
}

import { ModuleCard } from "../components/ModuleCard";
import { CodeBlock } from "../components/CodeBlock";
import { Variable, Repeat, GitBranch, Type } from "lucide-react";

export function CoreSyntaxPage() {
    return (
        <div className="space-y-12">
            {/* Header */}
            <div className="space-y-4">
                <div className="flex items-center gap-3 text-muted-foreground text-sm">
                    <span>Documentation</span>
                    <span>/</span>
                    <span>Core Syntax</span>
                </div>
                <h1 className="text-5xl font-bold">Core Syntax</h1>
                <p className="text-xl text-muted-foreground leading-relaxed max-w-3xl">
                    Explore the fundamental building blocks of Quanta. From variables and types to
                    loops, strings, and functions — everything you need to start building with Quanta.
                </p>
            </div>

            {/* Core Topics */}
            <div className="space-y-6">
                <h2 className="text-2xl font-semibold">Core Topics</h2>
                <p className="text-muted-foreground">
                    Quanta was designed with a minimal keyword set and maximum clarity. Here are the four pillars of the language.
                </p>

                <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                    <ModuleCard
                        icon={Variable}
                        title="Variables & Types"
                        description="Declare variables implicitly (x = 100) or explicitly (int8 x = 100). Quanta infers types automatically, or you take control with int, int8, int32, float, bool, string and more."
                    />
                    <ModuleCard
                        icon={GitBranch}
                        title="Control Flow"
                        description="Readable if, elif, else branching. No switch statements needed — the Quanta compiler automatically optimizes elif chains into fast switch logic behind the scenes."
                    />
                    <ModuleCard
                        icon={Repeat}
                        title="while & for Loops"
                        description="Use while (condition) { } for general iteration and for (i = 0; i < n; i++) { } for counted loops. Also supports loop i in string { } for embedded-friendly string character iteration."
                    />
                    <ModuleCard
                        icon={Type}
                        title="Dual String System"
                        description="Dynamic heap strings (string) for flexible apps, and static stack buffers (string[N]) for embedded devices. Both work with the full standard library of string functions."
                    />
                </div>
            </div>

            {/* Variables Example */}
            <div className="space-y-6">
                <h2 className="text-2xl font-semibold">Variables & Type Inference</h2>
                <p className="text-muted-foreground">
                    Quanta automatically infers types when you write quick scripts. For embedded or precision-critical code, use explicit types to control exact memory usage.
                </p>

                <CodeBlock
                    code={`@ Implicit — Quanta infers the type
x = 100
pi = 3.14159
msg = "Hello Quanta"
flag = true

@ Explicit — you control the memory
int a = 10
int8 tiny = 100         @ 8-bit integer (saves memory on microchips)
int32 medium = 50000    @ 32-bit integer
float temp = 36.6

@ Introspection
print(type(x))          @ Outputs: int
print(bytesize(x))      @ Outputs: 8`}
                    language="quanta"
                />
            </div>

            {/* Control Flow Example */}
            <div className="space-y-6">
                <h2 className="text-2xl font-semibold">Control Flow</h2>
                <p className="text-muted-foreground">
                    Use <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">if</code>,{" "}
                    <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">elif</code>, and{" "}
                    <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">else</code> for branching.
                    Quanta has no <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">switch</code> statement — the compiler optimizes chains automatically.
                </p>

                <CodeBlock
                    code={`score = 75

if (score > 90) {
    print("Grade A")
} elif (score > 70) {
    print("Grade B")
} elif (score > 50) {
    print("Grade C")
} else {
    print("Fail")
}

@ Outputs: Grade B`}
                    language="quanta"
                />
            </div>

            {/* Loop Example */}
            <div className="space-y-6">
                <h2 className="text-2xl font-semibold">Loops — while & for</h2>
                <p className="text-muted-foreground">
                    Quanta supports both{" "}
                    <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">while</code> and{" "}
                    <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">for</code> loops with clean, readable syntax.
                    Additionally, use the powerful{" "}
                    <code className="px-1.5 py-0.5 rounded bg-muted text-[#22d3ee] font-mono text-sm">loop i in string</code> construct for string iteration.
                </p>

                <CodeBlock
                    code={`@ while loop — runs while condition is true
count = 0
while (count < 5) {
    print("Iteration:", count)
    count++
}

@ for loop — classic counted iteration
for (i = 0; i < 3; i++) {
    print("Step:", i)
}

@ for loop over a range
for (x = 1; x <= 10; x++) {
    print(x)
}

@ String iteration — loop i in string
word = "Quanta"
loop i in word {
    print(word[i])    @ Q u a n t a
}`}
                    language="quanta"
                />
            </div>

            {/* Strings Example */}
            <div className="space-y-6">
                <h2 className="text-2xl font-semibold">Dual String System</h2>
                <p className="text-muted-foreground">
                    Quanta provides two string types to serve both high-level apps and memory-constrained embedded targets.
                </p>

                <CodeBlock
                    code={`@ Dynamic string — heap allocated, flexible
string name = "Quanta"
print(upper(name))         @ "QUANTA"
print(len(name))           @ 6
print(reverse(name))       @ "atnauQ"

@ Static string — stack allocated, perfect for IoT
string[16] device = "Sensor_A1"
string[8]  buf = "SuperLong"  @ Stored as "SuperLo" (safely truncated)

@ String operations — work with both types
greeting = "  hello, world  "
print(strip(greeting))        @ "hello, world"
print(capitalize(greeting))    @ "Hello, world"
print(replace(greeting, "world", "quanta"))  @ "  hello, quanta  "

@ Concatenation
first = "Hello"
second = "Quanta"
combined = first + ", " + second + "!"
print(combined)   @ "Hello, Quanta!"`}
                    language="quanta"
                />
            </div>

            {/* Functions Example */}
            <div className="space-y-6">
                <h2 className="text-2xl font-semibold">Functions</h2>

                <CodeBlock
                    code={`@ Function with explicit return type
int add(int a, int b) {
    return a + b
}

@ Function with default arguments
int getArea(width = 10, height = 20) {
    return width * height
}

@ String function returning dynamic string
string greet(string name) {
    return "Hello, " + name + "!"
}

@ Calling functions
print(add(5, 3))           @ 8
print(getArea())           @ 200
print(getArea(width=5))    @ 100
print(greet("Rohan"))      @ Hello, Rohan!`}
                    language="quanta"
                />
            </div>

            {/* Syntax Features */}
            <div className="bg-card border border-border rounded-xl p-8 space-y-6">
                <h3 className="text-xl font-semibold">Key Syntax Features at a Glance</h3>

                <div className="grid gap-4">
                    {[
                        { title: "@ Comments", desc: "The @ symbol marks a comment. Everything after it on the same line is ignored by the compiler." },
                        { title: "Semicolons Optional", desc: "Quanta doesn't require semicolons at the end of statements, but accepts them if you prefer that style." },
                        { title: "Type Introspection", desc: "Use type() and bytesize() to inspect the inferred type and memory size of any variable at runtime." },
                        { title: "Auto-Free Memory", desc: "Heap strings created by operations like upper(), strip(), and concatenation are automatically freed before the function returns — no manual free, no GC pauses." },
                    ].map((item) => (
                        <div key={item.title} className="flex gap-4">
                            <div className="w-2 h-2 rounded-full bg-gradient-to-r from-[#22d3ee] to-[#a855f7] mt-2 flex-shrink-0" />
                            <div>
                                <h4 className="font-semibold mb-1">{item.title}</h4>
                                <p className="text-sm text-muted-foreground">{item.desc}</p>
                            </div>
                        </div>
                    ))}
                </div>
            </div>
        </div>
    );
}

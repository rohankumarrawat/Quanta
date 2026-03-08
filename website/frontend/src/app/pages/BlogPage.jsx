import { useState, useEffect } from "react";
import { useNavigate } from "react-router";
import { ArrowRight, Calendar, User } from "lucide-react";
import { getBlogPosts } from "../../api/blog";

const FALLBACK_POSTS = [
    { _id: "1", title: "Introducing Quanta: A Memory-Safe Language for Embedded and Desktop", preview: "We're excited to introduce Quanta — a modern programming language built on LLVM that bridges the gap between high-level scripting and low-level systems programming. Learn what makes it unique.", author: "Rohan Kumar Rawat", date: "March 1, 2026", readTime: "5 min read" },
    { _id: "2", title: "The Dual String System: How Quanta Handles Memory Safely", preview: "Deep dive into Quanta's unique memory model. Dynamic heap strings with auto-free tracking vs. static string[N] stack buffers — and when to use each in your projects.", author: "Rohan Kumar Rawat", date: "February 24, 2026", readTime: "8 min read" },
    { _id: "3", title: "Why Quanta Has No While, For, or Switch Statements", preview: "Quanta's unified loop construct and intelligent elif-to-switch optimization mean you never have to choose between iteration patterns. Here's the design philosophy behind it.", author: "Rohan Kumar Rawat", date: "February 18, 2026", readTime: "6 min read" },
    { _id: "4", title: "Writing Embedded-Safe Quanta Code for Microcontrollers", preview: "A practical guide to writing deterministic, heap-free Quanta programs for IoT and embedded devices using string[N] buffers, stack-only loops, and explicit integer types.", author: "Rohan Kumar Rawat", date: "February 10, 2026", readTime: "10 min read" },
    { _id: "5", title: "Under the Hood: How Quanta Uses LLVM for Native Compilation", preview: "Quanta compiles to native machine code via the LLVM backend. In this post, we explore the compilation pipeline, from your .qnt source file to a fast native binary.", author: "Rohan Kumar Rawat", date: "February 3, 2026", readTime: "7 min read" },
    { _id: "6", title: "String Functions in Quanta: A Complete Reference", preview: "Quanta ships with a rich string standard library. Learn every built-in string function — from upper(), strip(), and replace() to find(), count(), startswith() and slicing with negative indices.", author: "Rohan Kumar Rawat", date: "January 27, 2026", readTime: "9 min read" },
];

export function BlogPage() {
    const navigate = useNavigate();
    const [blogPosts, setBlogPosts] = useState(FALLBACK_POSTS);
    const [isLoading, setIsLoading] = useState(true);

    useEffect(() => {
        getBlogPosts()
            .then((data) => { if (data && data.length > 0) setBlogPosts(data); })
            .catch(() => { })
            .finally(() => setIsLoading(false));
    }, []);

    return (
        <div className="space-y-12">
            {/* Header */}
            <div className="space-y-4">
                <h1 className="text-3xl md:text-4xl font-bold">Quanta Blog</h1>
                <p className="text-base md:text-lg text-muted-foreground max-w-2xl">
                    Latest updates, tutorials, and insights from the Quanta team and community.
                </p>
            </div>

            {/* Blog Grid */}
            {isLoading ? (
                <div className="text-center text-muted-foreground py-12">Loading posts...</div>
            ) : (
                <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                    {blogPosts.map((post) => (
                        <article
                            key={post._id || post.title}
                            className="group bg-card border border-border rounded-xl p-6 hover:border-[#22d3ee]/30 transition-all cursor-pointer"
                        >
                            <h2 className="text-xl font-semibold mb-3 group-hover:text-[#22d3ee] transition-colors">
                                {post.title}
                            </h2>

                            <p className="text-muted-foreground mb-4 line-clamp-3">
                                {post.preview}
                            </p>

                            <div className="flex flex-wrap items-center gap-x-4 gap-y-2 text-sm text-muted-foreground mb-4">
                                <div className="flex items-center gap-1.5">
                                    <User className="w-4 h-4" />
                                    <span>{post.author}</span>
                                </div>
                                <span className="hidden sm:inline">•</span>
                                <div className="flex items-center gap-1.5">
                                    <Calendar className="w-4 h-4" />
                                    <span>{post.date}</span>
                                </div>
                                <span className="hidden sm:inline">•</span>
                                <span>{post.readTime}</span>
                            </div>

                            <button
                                onClick={(e) => {
                                    e.stopPropagation();
                                    navigate(`/blog/${post._id}`);
                                }}
                                className="flex items-center gap-2 text-[#22d3ee] text-sm font-medium group-hover:gap-3 transition-all"
                            >
                                Read More
                                <ArrowRight className="w-4 h-4" />
                            </button>
                        </article>
                    ))}
                </div>
            )}
        </div>
    );
}

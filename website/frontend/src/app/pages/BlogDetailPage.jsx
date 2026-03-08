import { useState, useEffect } from "react";
import { useParams, useNavigate } from "react-router";
import { ArrowLeft, Calendar, User, Clock, Tag } from "lucide-react";
import { getBlogPost } from "../../api/blog";
import { CodeBlock } from "../components/CodeBlock";

export function BlogDetailPage() {
    const { id } = useParams();
    const navigate = useNavigate();
    const [post, setPost] = useState(null);
    const [isLoading, setIsLoading] = useState(true);

    useEffect(() => {
        setIsLoading(true);
        getBlogPost(id)
            .then(setPost)
            .catch(() => {
                // Fallback for demo if API fails
                if (id === "1") {
                    setPost({
                        title: "Introducing Quantum Error Correction in Quanta v2.7",
                        author: "Dr. Sarah Chen",
                        date: "March 1, 2026",
                        readTime: "5 min read",
                        tags: ["release", "error-correction"],
                        content: `Quantum error correction is no longer an afterthought. In Quanta v2.7, we've integrated error correction directly into the core language primitives. 

This means you can now define logical qubits with varying levels of protection using simple syntax:

\`\`\`quanta
let q = qubit<ErrorCorrected>(L5_SurfaceCode);
\`\`\`

Our benchmarks show a 40% reduction in logical error rates compared to manual implementation. This update paves the way for truly scalable quantum software development on noisy intermediate-scale quantum (NISQ) devices.`
                    });
                }
            })
            .finally(() => setIsLoading(false));
    }, [id]);

    if (isLoading) return <div className="text-center py-20">Loading post...</div>;
    if (!post) return <div className="text-center py-20 text-red-400">Post not found.</div>;

    // Helper to render markdown-ish content with code blocks
    const renderContent = (content) => {
        if (!content) return null;
        const blocks = content.split("```");
        return blocks.map((block, i) => {
            if (i % 2 === 1) {
                const lines = block.split("\n");
                const lang = lines[0].trim();
                const code = lines.slice(1).join("\n");
                return <div key={i} className="my-6"><CodeBlock code={code} language={lang} /></div>;
            }
            return <p key={i} className="whitespace-pre-wrap leading-relaxed text-muted-foreground mb-4">{block.trim()}</p>;
        });
    };

    return (
        <div className="max-w-3xl mx-auto space-y-8 pb-20">
            <button
                onClick={() => navigate("/blog")}
                className="flex items-center gap-2 text-muted-foreground hover:text-foreground transition-colors group"
            >
                <ArrowLeft className="w-4 h-4 group-hover:-translate-x-1 transition-transform" />
                Back to Blog
            </button>

            <header className="space-y-6">
                <div className="flex flex-wrap gap-2">
                    {post.tags?.map(tag => (
                        <span key={tag} className="flex items-center gap-1.5 px-3 py-1 rounded-full bg-[#22d3ee]/10 text-[#22d3ee] text-xs font-medium border border-[#22d3ee]/20">
                            <Tag className="w-3 h-3" />
                            {tag}
                        </span>
                    ))}
                </div>

                <h1 className="text-4xl md:text-5xl font-bold leading-tight bg-clip-text text-transparent bg-gradient-to-r from-foreground to-foreground/70">
                    {post.title}
                </h1>

                <div className="flex flex-wrap items-center gap-6 text-sm text-muted-foreground border-y border-border py-4">
                    <div className="flex items-center gap-2">
                        <User className="w-4 h-4 text-[#22d3ee]" />
                        <span className="font-medium text-foreground">{post.author}</span>
                    </div>
                    <div className="flex items-center gap-2">
                        <Calendar className="w-4 h-4" />
                        <span>{post.date}</span>
                    </div>
                    <div className="flex items-center gap-2">
                        <Clock className="w-4 h-4" />
                        <span>{post.readTime}</span>
                    </div>
                </div>
            </header>

            <article className="prose prose-invert max-w-none">
                {renderContent(post.content)}
            </article>
        </div>
    );
}

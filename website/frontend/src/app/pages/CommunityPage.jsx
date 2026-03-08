import { useState, useEffect } from "react";
import { Search, Plus, TrendingUp, Award, BookOpen } from "lucide-react";
import { QuestionCard } from "../components/QuestionCard";
import { getCommunityQuestions, createQuestion, voteQuestion } from "../../api/community";
import { useAuth } from "../../context/AuthContext";

const FALLBACK_QUESTIONS = [
    { _id: "1", votes: 42, answers: [], views: 1203, title: "How does the Dual String System work in Quanta?", preview: "I'm confused about when to use string vs string[N]. When should I prefer the static buffer approach for my embedded project?", content: "I'm confused about when to use string vs string[N]. When should I prefer the static buffer approach for my embedded project?", tags: ["strings", "embedded", "memory"], username: "embedded_dev", timePosted: "2 hours ago", isAnswered: true },
    { _id: "2", votes: 38, answers: [], views: 892, title: "How do I iterate over a string character by character in Quanta?", preview: "I see there's a 'loop i in s' syntax but the docs are sparse. Can someone show a complete example with indexing?", content: "I see there's a 'loop i in s' syntax but the docs are sparse. Can someone show a complete example with indexing?", tags: ["loop", "strings", "syntax"], username: "alice_dev", timePosted: "5 hours ago", isAnswered: true },
    { _id: "3", votes: 27, answers: [], views: 456, title: "What's the difference between int, int8, and int32 in Quanta?", preview: "When should I use each integer type? Is there a peformance or memory reason for picking int8 on a microcontroller?", content: "When should I use each integer type?", tags: ["types", "embedded", "memory"], username: "qnt_explorer", timePosted: "1 day ago", isAnswered: false },
    { _id: "4", votes: 51, answers: [], views: 2145, title: "Does Quanta have garbage collection?", preview: "I see that heap strings are 'auto-freed'. Is this garbage collection? How does it differ from Rust or Python's approach?", content: "I see that heap strings are auto-freed. Is this garbage collection?", tags: ["memory", "auto-free", "getting-started"], username: "dev_curious", timePosted: "2 days ago", isAnswered: true },
    { _id: "5", votes: 19, answers: [], views: 678, title: "How to use default arguments in Quanta functions?", preview: "The docs mention default arguments with getArea(width=10, height=20). Can I mix positional and keyword arguments in a call?", content: "The docs mention default arguments. Can I mix positional and keyword arguments?", tags: ["functions", "syntax", "parameters"], username: "type_safe_dev", timePosted: "3 days ago", isAnswered: true },
    { _id: "6", votes: 33, answers: [], views: 1567, title: "Best practices for writing embedded-safe Quanta code?", preview: "I'm targeting a microcontroller with 64KB RAM. What patterns should I follow to avoid any heap allocations in Quanta?", content: "Targeting a micro with 64KB RAM. How do I write heap-free Quanta code?", tags: ["embedded", "memory", "best-practices"], username: "iot_builder", timePosted: "4 days ago", isAnswered: true },
];

const TRENDING_TAGS = [
    { name: "embedded", count: 187 },
    { name: "strings", count: 156 },
    { name: "loop", count: 134 },
    { name: "memory", count: 98 },
    { name: "types", count: 87 },
    { name: "functions", count: 76 },
];

const TOP_CONTRIBUTORS = [
    { name: "rohan_rawat", answers: 156, reputation: 4521 },
    { name: "embedded_queen", answers: 134, reputation: 3892 },
    { name: "llvm_wizard", answers: 98, reputation: 2765 },
];

export function CommunityPage() {
    const { user } = useAuth();
    const [activeFilter, setActiveFilter] = useState("all");
    const [sortBy, setSortBy] = useState("newest");
    const [searchQuery, setSearchQuery] = useState("");
    const [questions, setQuestions] = useState(FALLBACK_QUESTIONS);
    const [isLoading, setIsLoading] = useState(true);
    const [showAskForm, setShowAskForm] = useState(false);
    const [newQuestion, setNewQuestion] = useState({ title: "", preview: "", tags: "" });

    const filters = [
        { id: "all", label: "All Questions" },
        { id: "unanswered", label: "Unanswered" },
        { id: "popular", label: "Popular" },
        { id: "recent", label: "Recent" },
    ];

    useEffect(() => {
        setIsLoading(true);
        getCommunityQuestions({ filter: activeFilter, sort: sortBy })
            .then((data) => { if (data && data.length > 0) setQuestions(data); })
            .catch(() => { })
            .finally(() => setIsLoading(false));
    }, [activeFilter, sortBy]);

    const filteredQuestions = questions.filter((q) => {
        if (!searchQuery) return true;
        const q2 = searchQuery.toLowerCase();
        return (
            q.title.toLowerCase().includes(q2) ||
            q.preview.toLowerCase().includes(q2) ||
            q.tags.some((t) => t.toLowerCase().includes(q2))
        );
    });

    const handleAskQuestion = async (e) => {
        e.preventDefault();
        if (!user) { alert("Please login to ask a question!"); return; }
        try {
            const created = await createQuestion({
                title: newQuestion.title,
                preview: newQuestion.preview,
                content: newQuestion.preview, // Use preview for content if not separated
                tags: newQuestion.tags.split(",").map((t) => t.trim()).filter(Boolean),
            });
            setQuestions([created, ...questions]);
        } catch {
            const fake = {
                _id: Date.now().toString(),
                ...newQuestion,
                tags: newQuestion.tags.split(",").map((t) => t.trim()).filter(Boolean),
                votes: 0, answers: 0, views: 0,
                username: user.username,
                timePosted: "just now",
                isAnswered: false,
            };
            setQuestions([fake, ...questions]);
        }
        setNewQuestion({ title: "", preview: "", tags: "" });
        setShowAskForm(false);
    };

    const handleVote = async (id) => {
        try {
            await voteQuestion(id);
        } catch { }
        setQuestions((prev) =>
            prev.map((q) => q._id === id ? { ...q, votes: q.votes + 1 } : q)
        );
    };

    return (
        <div className="flex flex-col lg:flex-row gap-8">
            {/* Main Content */}
            <div className="flex-1 w-full min-w-0 space-y-8">
                {/* Header */}
                <div className="space-y-4">
                    <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4">
                        <div>
                            <h1 className="text-3xl md:text-4xl font-bold mb-2">Community Discussions</h1>
                            <p className="text-muted-foreground">Find answers, share knowledge, build with Quanta.</p>
                        </div>
                        <button
                            onClick={() => user ? setShowAskForm(!showAskForm) : alert("Please login to ask a question!")}
                            className="flex items-center justify-center gap-2 px-5 py-2.5 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all hover:scale-105 whitespace-nowrap"
                        >
                            <Plus className="w-4 h-4" />
                            Ask Question
                        </button>
                    </div>
                </div>

                {/* Ask Question Form */}
                {showAskForm && (
                    <form onSubmit={handleAskQuestion} className="bg-card border border-[#22d3ee]/30 rounded-xl p-6 space-y-4">
                        <h3 className="font-semibold text-lg">Ask a Question</h3>
                        <input
                            type="text"
                            placeholder="Question title..."
                            value={newQuestion.title}
                            onChange={(e) => setNewQuestion({ ...newQuestion, title: e.target.value })}
                            required
                            className="w-full px-4 py-2.5 bg-background border border-border rounded-lg text-foreground placeholder:text-muted-foreground focus:outline-none focus:border-[#22d3ee]/50"
                        />
                        <textarea
                            placeholder="Describe your question..."
                            value={newQuestion.preview}
                            onChange={(e) => setNewQuestion({ ...newQuestion, preview: e.target.value })}
                            required
                            rows={3}
                            className="w-full px-4 py-2.5 bg-background border border-border rounded-lg text-foreground placeholder:text-muted-foreground focus:outline-none focus:border-[#22d3ee]/50 resize-none"
                        />
                        <input
                            type="text"
                            placeholder="Tags (comma separated, e.g. quantum-computing, optimization)"
                            value={newQuestion.tags}
                            onChange={(e) => setNewQuestion({ ...newQuestion, tags: e.target.value })}
                            className="w-full px-4 py-2.5 bg-background border border-border rounded-lg text-foreground placeholder:text-muted-foreground focus:outline-none focus:border-[#22d3ee]/50"
                        />
                        <div className="flex gap-3">
                            <button type="submit" className="px-5 py-2.5 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium hover:shadow-lg transition-all">
                                Post Question
                            </button>
                            <button type="button" onClick={() => setShowAskForm(false)} className="px-5 py-2.5 rounded-lg border border-border text-muted-foreground hover:text-foreground transition-colors">
                                Cancel
                            </button>
                        </div>
                    </form>
                )}

                {/* Search Bar */}
                <div className="relative">
                    <Search className="absolute left-4 top-1/2 -translate-y-1/2 w-5 h-5 text-muted-foreground" />
                    <input
                        type="text"
                        placeholder="Search questions..."
                        value={searchQuery}
                        onChange={(e) => setSearchQuery(e.target.value)}
                        className="w-full pl-12 pr-4 py-3 bg-card border border-border rounded-xl text-foreground placeholder:text-muted-foreground focus:outline-none focus:border-[#22d3ee]/50 transition-colors"
                    />
                </div>

                {/* Filters and Sort */}
                <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4">
                    <div className="flex flex-wrap gap-2 w-full sm:w-auto">
                        {filters.map((filter) => (
                            <button
                                key={filter.id}
                                onClick={() => setActiveFilter(filter.id)}
                                className={`px-4 py-2 rounded-lg text-sm font-medium transition-all ${activeFilter === filter.id
                                    ? "bg-gradient-to-r from-[#22d3ee]/20 to-[#a855f7]/20 text-[#22d3ee] border border-[#22d3ee]/30"
                                    : "text-muted-foreground hover:text-foreground hover:bg-card border border-transparent"
                                    }`}
                            >
                                {filter.label}
                            </button>
                        ))}
                    </div>

                    <select
                        value={sortBy}
                        onChange={(e) => setSortBy(e.target.value)}
                        className="px-4 py-2 w-full sm:w-auto bg-card border border-border rounded-lg text-sm text-foreground focus:outline-none focus:border-[#22d3ee]/50 transition-colors cursor-pointer"
                    >
                        <option value="newest">Newest</option>
                        <option value="votes">Most Votes</option>
                        <option value="activity">Recent Activity</option>
                    </select>
                </div>

                {/* Question List */}
                <div className="space-y-4">
                    {isLoading ? (
                        <div className="text-center text-muted-foreground py-12">Loading questions...</div>
                    ) : filteredQuestions.length === 0 ? (
                        <div className="text-center text-muted-foreground py-12">No questions found.</div>
                    ) : (
                        filteredQuestions.map((question) => (
                            <QuestionCard key={question._id || question.title} {...question} />
                        ))
                    )}
                </div>
            </div>

            {/* Right Sidebar */}
            <aside className="w-full lg:w-[280px] flex-shrink-0 space-y-6">
                {/* Trending Tags */}
                <div className="bg-card border border-border rounded-xl p-6">
                    <div className="flex items-center gap-2 mb-4">
                        <TrendingUp className="w-4 h-4 text-[#22d3ee]" />
                        <h3 className="font-semibold">Trending Tags</h3>
                    </div>
                    <div className="space-y-2">
                        {TRENDING_TAGS.map((tag) => (
                            <button
                                key={tag.name}
                                onClick={() => setSearchQuery(tag.name)}
                                className="flex items-center justify-between w-full px-3 py-2 rounded-lg hover:bg-muted/50 transition-colors text-left group"
                            >
                                <span className="text-sm text-muted-foreground group-hover:text-foreground">
                                    {tag.name}
                                </span>
                                <span className="text-xs text-muted-foreground">{tag.count}</span>
                            </button>
                        ))}
                    </div>
                </div>

                {/* Top Contributors */}
                <div className="bg-card border border-border rounded-xl p-6">
                    <div className="flex items-center gap-2 mb-4">
                        <Award className="w-4 h-4 text-[#a855f7]" />
                        <h3 className="font-semibold">Top Contributors</h3>
                    </div>
                    <div className="space-y-3">
                        {TOP_CONTRIBUTORS.map((contributor, index) => (
                            <div key={contributor.name} className="flex items-center gap-3">
                                <div className="w-8 h-8 rounded-full bg-gradient-to-br from-[#22d3ee]/20 to-[#a855f7]/20 flex items-center justify-center text-xs font-semibold text-[#22d3ee]">
                                    #{index + 1}
                                </div>
                                <div className="flex-1 min-w-0">
                                    <p className="text-sm font-medium text-foreground truncate">{contributor.name}</p>
                                    <p className="text-xs text-muted-foreground">
                                        {contributor.answers} answers • {contributor.reputation} rep
                                    </p>
                                </div>
                            </div>
                        ))}
                    </div>
                </div>

                {/* Community Guidelines */}
                <div className="bg-gradient-to-br from-[#22d3ee]/10 to-[#a855f7]/10 border border-[#22d3ee]/20 rounded-xl p-6">
                    <div className="flex items-center gap-2 mb-3">
                        <BookOpen className="w-4 h-4 text-[#22d3ee]" />
                        <h3 className="font-semibold">Community Guidelines</h3>
                    </div>
                    <ul className="space-y-2 text-sm text-muted-foreground">
                        {["Be respectful and inclusive", "Search before asking", "Provide minimal examples", "Accept helpful answers"].map((g) => (
                            <li key={g} className="flex gap-2">
                                <span className="text-[#22d3ee]">•</span>
                                <span>{g}</span>
                            </li>
                        ))}
                    </ul>
                </div>
            </aside>
        </div>
    );
}

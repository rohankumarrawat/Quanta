import { useState, useEffect } from "react";
import { useParams, useNavigate } from "react-router";
import { ArrowLeft, MessageSquare, ArrowBigUp, Eye, User, Clock, Send } from "lucide-react";
import { getQuestion, voteQuestion, addAnswer } from "../../api/community";
import { useAuth } from "../../context/AuthContext";

export function QuestionDetailPage() {
    const { id } = useParams();
    const navigate = useNavigate();
    const { user } = useAuth();
    const [question, setQuestion] = useState(null);
    const [isLoading, setIsLoading] = useState(true);
    const [newAnswer, setNewAnswer] = useState("");
    const [isSubmitting, setIsSubmitting] = useState(false);

    useEffect(() => {
        setIsLoading(true);
        getQuestion(id)
            .then(setQuestion)
            .catch(() => {
                // Fallback for demo
            })
            .finally(() => setIsLoading(false));
    }, [id]);

    const handleVote = async () => {
        try {
            const updated = await voteQuestion(id);
            setQuestion(updated);
        } catch {
            setQuestion(prev => ({ ...prev, votes: prev.votes + 1 }));
        }
    };

    const handleSubmitAnswer = async (e) => {
        e.preventDefault();
        if (!newAnswer.trim()) return;
        setIsSubmitting(true);
        try {
            const updated = await addAnswer(id, newAnswer);
            setQuestion(updated);
            setNewAnswer("");
        } catch (err) {
            console.error("Failed to post answer:", err);
            // Local fallback for UI demo
            const localAnswer = { content: newAnswer, username: user.username, createdAt: new Date().toISOString() };
            setQuestion(prev => ({
                ...prev,
                answers: [...(prev.answers || []), localAnswer],
                isAnswered: true
            }));
            setNewAnswer("");
        } finally {
            setIsSubmitting(false);
        }
    };

    if (isLoading) return <div className="text-center py-20">Loading conversation...</div>;
    if (!question) return <div className="text-center py-20 text-red-400">Question not found.</div>;

    return (
        <div className="max-w-4xl mx-auto space-y-8 pb-20">
            <button
                onClick={() => navigate("/community")}
                className="flex items-center gap-2 text-muted-foreground hover:text-foreground transition-colors group"
            >
                <ArrowLeft className="w-4 h-4 group-hover:-translate-x-1 transition-transform" />
                Back to Discussions
            </button>

            <div className="flex gap-6">
                {/* Vote Side Pane */}
                <div className="flex flex-col items-center gap-2">
                    <button
                        onClick={handleVote}
                        className="p-2 rounded-lg hover:bg-[#22d3ee]/10 text-muted-foreground hover:text-[#22d3ee] transition-colors"
                    >
                        <ArrowBigUp className="w-8 h-8" />
                    </button>
                    <span className="text-xl font-bold">{question.votes}</span>
                </div>

                {/* Main Content */}
                <div className="flex-1 space-y-6">
                    <div className="space-y-4">
                        <div className="flex flex-wrap gap-2">
                            {question.tags?.map(tag => (
                                <span key={tag} className="px-2 py-0.5 rounded-md bg-muted text-xs text-muted-foreground">
                                    {tag}
                                </span>
                            ))}
                        </div>
                        <h1 className="text-3xl font-bold leading-tight">{question.title}</h1>
                        <div className="flex items-center gap-4 text-sm text-muted-foreground border-b border-border pb-6">
                            <div className="flex items-center gap-2">
                                <Clock className="w-4 h-4" />
                                <span>Asked {question.timePosted}</span>
                            </div>
                            <div className="flex items-center gap-2">
                                <Eye className="w-4 h-4" />
                                <span>{question.views || 0} views</span>
                            </div>
                        </div>
                    </div>

                    <div className="text-lg leading-relaxed whitespace-pre-wrap">
                        {question.content || question.preview}
                    </div>

                    <div className="flex justify-end pt-4">
                        <div className="bg-[#22d3ee]/5 border border-[#22d3ee]/10 rounded-lg p-3 flex items-center gap-3">
                            <div className="w-8 h-8 rounded-full bg-[#22d3ee]/20 flex items-center justify-center text-[#22d3ee] font-bold">
                                {question.username?.[0]?.toUpperCase()}
                            </div>
                            <div>
                                <p className="text-xs text-muted-foreground">Asked by</p>
                                <p className="text-sm font-medium">{question.username}</p>
                            </div>
                        </div>
                    </div>

                    <div className="pt-12 space-y-8">
                        <div className="flex items-center justify-between">
                            <h2 className="text-2xl font-bold flex items-center gap-2">
                                <MessageSquare className="w-6 h-6 text-[#22d3ee]" />
                                {question.answers?.length || 0} Answers
                            </h2>
                        </div>

                        <div className="space-y-8">
                            {question.answers?.map((answer, i) => (
                                <div key={i} className="bg-card border border-border rounded-xl p-6 space-y-4 relative overflow-hidden group">
                                    <div className="absolute left-0 top-0 w-1 h-full bg-gradient-to-b from-[#22d3ee] to-[#a855f7] opacity-50" />
                                    <p className="leading-relaxed whitespace-pre-wrap">{answer.content}</p>
                                    <div className="flex justify-end border-t border-border pt-4">
                                        <div className="flex items-center gap-3">
                                            <div className="text-right">
                                                <p className="text-xs text-muted-foreground">Answered by</p>
                                                <p className="text-sm font-medium">{answer.username}</p>
                                            </div>
                                            <div className="w-8 h-8 rounded-full bg-[#a855f7]/20 flex items-center justify-center text-[#a855f7] font-bold">
                                                {answer.username?.[0]?.toUpperCase()}
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            ))}
                        </div>

                        {/* Answer Form */}
                        {user ? (
                            <form onSubmit={handleSubmitAnswer} className="space-y-4 pt-12">
                                <h3 className="text-xl font-bold">Your Answer</h3>
                                <textarea
                                    value={newAnswer}
                                    onChange={(e) => setNewAnswer(e.target.value)}
                                    placeholder="Write your answer here..."
                                    required
                                    rows={6}
                                    className="w-full px-4 py-3 bg-card border border-border rounded-xl focus:outline-none focus:border-[#22d3ee]/50 resize-none"
                                />
                                <button
                                    type="submit"
                                    disabled={isSubmitting}
                                    className="flex items-center gap-2 px-6 py-3 rounded-xl bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-bold hover:shadow-lg transition-all disabled:opacity-50"
                                >
                                    <Send className="w-4 h-4" />
                                    {isSubmitting ? "Posting..." : "Post Your Answer"}
                                </button>
                            </form>
                        ) : (
                            <div className="bg-muted/30 border border-dashed border-border rounded-xl p-8 text-center mt-12">
                                <p className="text-muted-foreground mb-4">You must be logged in to answer questions.</p>
                            </div>
                        )}
                    </div>
                </div>
            </div>
        </div>
    );
}

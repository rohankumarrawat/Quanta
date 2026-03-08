import { useState, useEffect } from "react";
import { useAuth } from "../../../context/AuthContext";
import axios from "axios";
import { CheckCircle2, XCircle, RefreshCw, Clock, Tag, User } from "lucide-react";

const API = "http://localhost:3001";

export function AdminQuestions() {
    const { token } = useAuth();
    const headers = { Authorization: `Bearer ${token}` };
    const [questions, setQuestions] = useState([]);
    const [loading, setLoading] = useState(true);
    const [tab, setTab] = useState("pending");
    const [toast, setToast] = useState("");

    const showToast = (msg) => { setToast(msg); setTimeout(() => setToast(""), 3000); };

    const load = () => {
        setLoading(true);
        axios.get(`${API}/api/admin/questions`, { headers })
            .then(r => setQuestions(r.data))
            .catch(() => { })
            .finally(() => setLoading(false));
    };

    useEffect(load, []);

    const handleApprove = async (id) => {
        await axios.post(`${API}/api/admin/questions/${id}/approve`, {}, { headers });
        showToast("Question approved and published!");
        load();
    };

    const handleReject = async (id) => {
        await axios.post(`${API}/api/admin/questions/${id}/reject`, {}, { headers });
        showToast("Question rejected.");
        load();
    };

    const filtered = questions.filter(q => q.status === tab);

    const tabCounts = {
        pending: questions.filter(q => q.status === "pending").length,
        approved: questions.filter(q => q.status === "approved").length,
        rejected: questions.filter(q => q.status === "rejected").length,
    };

    return (
        <div className="space-y-6">
            {toast && (
                <div className="fixed top-6 right-6 z-50 bg-[#22d3ee] text-background px-5 py-3 rounded-xl font-medium shadow-lg">
                    {toast}
                </div>
            )}

            <div className="flex items-center justify-between">
                <div>
                    <h1 className="text-3xl font-bold">Question Moderation</h1>
                    <p className="text-muted-foreground text-sm mt-1">Review and approve user-submitted questions</p>
                </div>
                <button onClick={load} className="flex items-center gap-2 px-4 py-2 rounded-lg border border-border hover:bg-muted text-sm transition-all">
                    <RefreshCw className="w-3.5 h-3.5" /> Refresh
                </button>
            </div>

            {/* Tabs */}
            <div className="flex gap-2">
                {[
                    { key: "pending", label: "Pending", color: "text-[#f59e0b]", bg: "bg-[#f59e0b]/10 border-[#f59e0b]/30" },
                    { key: "approved", label: "Approved", color: "text-green-400", bg: "bg-green-500/10 border-green-500/30" },
                    { key: "rejected", label: "Rejected", color: "text-red-400", bg: "bg-red-500/10 border-red-500/30" },
                ].map(t => (
                    <button
                        key={t.key}
                        onClick={() => setTab(t.key)}
                        className={`flex items-center gap-2 px-4 py-2 rounded-lg text-sm font-medium border transition-all ${tab === t.key ? `${t.bg} ${t.color}` : "border-transparent text-muted-foreground hover:border-border hover:text-foreground"}`}
                    >
                        {t.label}
                        <span className={`px-1.5 py-0.5 rounded-full text-xs ${tab === t.key ? "" : "bg-muted"}`}>
                            {tabCounts[t.key]}
                        </span>
                    </button>
                ))}
            </div>

            {/* Question Cards */}
            {loading ? (
                <div className="space-y-3">
                    {[...Array(3)].map((_, i) => <div key={i} className="h-32 rounded-xl bg-card border border-border animate-pulse" />)}
                </div>
            ) : filtered.length === 0 ? (
                <div className="py-20 text-center">
                    {tab === "pending" ? (
                        <>
                            <CheckCircle2 className="w-16 h-16 text-green-400/30 mx-auto mb-4" />
                            <p className="text-muted-foreground">No pending questions — all caught up!</p>
                        </>
                    ) : (
                        <p className="text-muted-foreground">No {tab} questions</p>
                    )}
                </div>
            ) : (
                <div className="space-y-4">
                    {filtered.map((q) => (
                        <div key={q._id} className={`bg-card border rounded-xl p-5 ${tab === "pending" ? "border-[#f59e0b]/20" : tab === "approved" ? "border-green-500/20" : "border-red-500/20"}`}>
                            <div className="flex items-start justify-between gap-4">
                                <div className="flex-1 min-w-0">
                                    <div className="flex items-center gap-2 mb-1.5">
                                        {tab === "pending" && <Clock className="w-3.5 h-3.5 text-[#f59e0b] flex-shrink-0" />}
                                        {tab === "approved" && <CheckCircle2 className="w-3.5 h-3.5 text-green-400 flex-shrink-0" />}
                                        {tab === "rejected" && <XCircle className="w-3.5 h-3.5 text-red-400 flex-shrink-0" />}
                                        <h3 className="font-semibold text-sm line-clamp-2">{q.title}</h3>
                                    </div>
                                    <p className="text-sm text-muted-foreground mb-3 line-clamp-2">{q.preview || q.content}</p>
                                    <div className="flex flex-wrap items-center gap-3 text-xs text-muted-foreground">
                                        <span className="flex items-center gap-1">
                                            <User className="w-3 h-3" /> {q.username}
                                        </span>
                                        <span className="flex items-center gap-1">
                                            <Clock className="w-3 h-3" /> {q.timePosted}
                                        </span>
                                        {(q.tags || []).map(tag => (
                                            <span key={tag} className="flex items-center gap-1 px-2 py-0.5 rounded-full bg-[#22d3ee]/10 text-[#22d3ee]">
                                                <Tag className="w-2.5 h-2.5" /> {tag}
                                            </span>
                                        ))}
                                    </div>
                                </div>

                                {tab === "pending" && (
                                    <div className="flex flex-col gap-2 flex-shrink-0">
                                        <button
                                            onClick={() => handleApprove(q._id)}
                                            className="flex items-center gap-1.5 px-4 py-2 rounded-lg bg-green-500/15 text-green-400 hover:bg-green-500/25 text-xs font-medium transition-all"
                                        >
                                            <CheckCircle2 className="w-3.5 h-3.5" /> Approve
                                        </button>
                                        <button
                                            onClick={() => handleReject(q._id)}
                                            className="flex items-center gap-1.5 px-4 py-2 rounded-lg bg-red-500/15 text-red-400 hover:bg-red-500/25 text-xs font-medium transition-all"
                                        >
                                            <XCircle className="w-3.5 h-3.5" /> Reject
                                        </button>
                                    </div>
                                )}
                            </div>
                        </div>
                    ))}
                </div>
            )}
        </div>
    );
}

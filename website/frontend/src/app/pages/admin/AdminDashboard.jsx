import { useState, useEffect } from "react";
import { useAuth } from "../../../context/AuthContext";
import axios from "axios";
import { Users, MessageSquare, FileText, UserX, Clock } from "lucide-react";

const API = "http://localhost:3001";

export function AdminDashboard() {
    const { token } = useAuth();
    const [stats, setStats] = useState(null);
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        const headers = { Authorization: `Bearer ${token}` };
        axios.get(`${API}/api/admin/stats`, { headers })
            .then(r => setStats(r.data))
            .catch(() => setStats({ totalUsers: 0, pendingQuestions: 0, totalQuestions: 0, totalPosts: 0, blockedUsers: 0 }))
            .finally(() => setLoading(false));
    }, [token]);

    const cards = [
        { label: "Total Users", value: stats?.totalUsers ?? "—", icon: Users, color: "from-[#22d3ee]/20 to-[#22d3ee]/5", iconColor: "text-[#22d3ee]" },
        { label: "Pending Questions", value: stats?.pendingQuestions ?? "—", icon: Clock, color: "from-[#f59e0b]/20 to-[#f59e0b]/5", iconColor: "text-[#f59e0b]" },
        { label: "Total Questions", value: stats?.totalQuestions ?? "—", icon: MessageSquare, color: "from-[#a855f7]/20 to-[#a855f7]/5", iconColor: "text-[#a855f7]" },
        { label: "Blog Posts", value: stats?.totalPosts ?? "—", icon: FileText, color: "from-[#22d3ee]/20 to-[#22d3ee]/5", iconColor: "text-[#22d3ee]" },
        { label: "Blocked Users", value: stats?.blockedUsers ?? "—", icon: UserX, color: "from-red-500/20 to-red-500/5", iconColor: "text-red-400" },
    ];

    return (
        <div className="space-y-8">
            <div>
                <h1 className="text-3xl font-bold mb-1">Dashboard</h1>
                <p className="text-muted-foreground">Welcome to the Quanta Admin Portal</p>
            </div>

            {loading ? (
                <div className="grid grid-cols-1 md:grid-cols-3 xl:grid-cols-5 gap-4">
                    {[...Array(5)].map((_, i) => (
                        <div key={i} className="h-28 rounded-xl bg-card border border-border animate-pulse" />
                    ))}
                </div>
            ) : (
                <div className="grid grid-cols-1 md:grid-cols-3 xl:grid-cols-5 gap-4">
                    {cards.map((card) => {
                        const Icon = card.icon;
                        return (
                            <div key={card.label} className={`bg-gradient-to-br ${card.color} border border-border rounded-xl p-5`}>
                                <div className="flex items-start justify-between mb-3">
                                    <Icon className={`w-5 h-5 ${card.iconColor}`} />
                                </div>
                                <p className="text-3xl font-bold mb-1">{card.value}</p>
                                <p className="text-xs text-muted-foreground">{card.label}</p>
                            </div>
                        );
                    })}
                </div>
            )}

            {/* Quick Actions */}
            <div className="bg-card border border-border rounded-xl p-6">
                <h2 className="text-lg font-semibold mb-4">Quick Actions</h2>
                <div className="grid grid-cols-1 md:grid-cols-2 gap-3">
                    {[
                        { label: "Review Pending Questions", desc: "Approve or reject user-submitted questions", path: "/admin/questions", color: "text-[#f59e0b]" },
                        { label: "Manage Users", desc: "View, block, or delete user accounts", path: "/admin/users", color: "text-[#22d3ee]" },
                        { label: "Write a Blog Post", desc: "Create and publish new content", path: "/admin/blog", color: "text-[#a855f7]" },
                        { label: "Send Notification", desc: "Send a message to all users or a specific user", path: "/admin/notifications", color: "text-green-400" },
                    ].map((item) => (
                        <a
                            key={item.path}
                            href={item.path}
                            className="flex items-center justify-between p-4 rounded-lg border border-border hover:border-[#22d3ee]/30 hover:bg-muted/30 transition-all group"
                        >
                            <div>
                                <p className={`font-medium text-sm ${item.color}`}>{item.label}</p>
                                <p className="text-xs text-muted-foreground mt-0.5">{item.desc}</p>
                            </div>
                            <span className="text-muted-foreground group-hover:text-foreground transition-colors">→</span>
                        </a>
                    ))}
                </div>
            </div>
        </div>
    );
}

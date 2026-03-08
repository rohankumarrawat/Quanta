import { useState, useEffect } from "react";
import { useAuth } from "../../../context/AuthContext";
import axios from "axios";
import { Bell, Send, Users, User } from "lucide-react";

const API = "http://localhost:3001";

export function AdminNotifications() {
    const { token } = useAuth();
    const headers = { Authorization: `Bearer ${token}` };
    const [mode, setMode] = useState("all"); // 'all' | 'specific'
    const [message, setMessage] = useState("");
    const [userId, setUserId] = useState("");
    const [users, setUsers] = useState([]);
    const [sending, setSending] = useState(false);
    const [toast, setToast] = useState("");
    const [history, setHistory] = useState([]);

    useEffect(() => {
        axios.get(`${API}/api/admin/users`, { headers }).then(r => setUsers(r.data)).catch(() => { });
    }, []);

    const showToast = (msg) => { setToast(msg); setTimeout(() => setToast(""), 3000); };

    const handleSend = async () => {
        if (!message.trim()) return;
        if (mode === "specific" && !userId) return;
        setSending(true);
        try {
            const payload = { message, ...(mode === "specific" ? { userId } : {}) };
            const r = await axios.post(`${API}/api/admin/notify`, payload, { headers });
            showToast(r.data.message);
            setHistory(h => [{ message, mode, userId, target: mode === "all" ? "All Users" : users.find(u => (u.id || u._id) === userId)?.username, time: new Date().toLocaleTimeString(), count: r.data.count }, ...h.slice(0, 9)]);
            setMessage("");
        } catch {
            showToast("Failed to send notification.");
        } finally {
            setSending(false);
        }
    };

    return (
        <div className="space-y-6">
            {toast && (
                <div className="fixed top-6 right-6 z-50 bg-[#22d3ee] text-background px-5 py-3 rounded-xl font-medium shadow-lg">
                    {toast}
                </div>
            )}

            <div>
                <h1 className="text-3xl font-bold">Notifications</h1>
                <p className="text-muted-foreground text-sm mt-1">Send messages to users</p>
            </div>

            <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
                {/* Send Panel */}
                <div className="bg-card border border-border rounded-xl p-6 space-y-5">
                    <h2 className="font-semibold flex items-center gap-2">
                        <Bell className="w-4 h-4 text-[#22d3ee]" />
                        Send Notification
                    </h2>

                    {/* Target Mode */}
                    <div className="flex gap-3">
                        <button
                            onClick={() => setMode("all")}
                            className={`flex-1 flex items-center justify-center gap-2 py-2.5 rounded-lg text-sm font-medium border transition-all ${mode === "all" ? "border-[#22d3ee]/30 bg-[#22d3ee]/10 text-[#22d3ee]" : "border-border text-muted-foreground hover:border-[#22d3ee]/20 hover:text-foreground"}`}
                        >
                            <Users className="w-4 h-4" /> All Users
                        </button>
                        <button
                            onClick={() => setMode("specific")}
                            className={`flex-1 flex items-center justify-center gap-2 py-2.5 rounded-lg text-sm font-medium border transition-all ${mode === "specific" ? "border-[#a855f7]/30 bg-[#a855f7]/10 text-[#a855f7]" : "border-border text-muted-foreground hover:border-[#a855f7]/20 hover:text-foreground"}`}
                        >
                            <User className="w-4 h-4" /> Specific User
                        </button>
                    </div>

                    {/* User Select */}
                    {mode === "specific" && (
                        <div className="space-y-1.5">
                            <label className="text-xs font-medium text-muted-foreground">Select User</label>
                            <select
                                value={userId}
                                onChange={e => setUserId(e.target.value)}
                                className="w-full px-4 py-2.5 rounded-lg bg-background border border-border text-sm focus:outline-none focus:border-[#a855f7]/50 transition-colors"
                            >
                                <option value="">Choose a user...</option>
                                {users.map(u => (
                                    <option key={u.id || u._id} value={u.id || u._id}>
                                        {u.username} ({u.email})
                                    </option>
                                ))}
                            </select>
                        </div>
                    )}

                    {/* Message */}
                    <div className="space-y-1.5">
                        <label className="text-xs font-medium text-muted-foreground">Message</label>
                        <textarea
                            value={message}
                            onChange={e => setMessage(e.target.value)}
                            placeholder={mode === "all" ? "Broadcast to all users..." : "Message for this user..."}
                            rows={5}
                            className="w-full px-4 py-3 rounded-lg bg-background border border-border text-sm resize-none focus:outline-none focus:border-[#22d3ee]/50 transition-colors"
                        />
                        <p className="text-xs text-muted-foreground text-right">{message.length} chars</p>
                    </div>

                    <button
                        onClick={handleSend}
                        disabled={sending || !message.trim() || (mode === "specific" && !userId)}
                        className="w-full flex items-center justify-center gap-2 py-3 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium text-sm disabled:opacity-50 hover:shadow-lg hover:shadow-[#22d3ee]/20 transition-all"
                    >
                        <Send className="w-4 h-4" />
                        {sending ? "Sending..." : mode === "all" ? "Broadcast to All Users" : "Send to User"}
                    </button>
                </div>

                {/* History Panel */}
                <div className="bg-card border border-border rounded-xl p-6 space-y-4">
                    <h2 className="font-semibold flex items-center gap-2">
                        <Bell className="w-4 h-4 text-[#a855f7]" />
                        Recent Notifications
                    </h2>

                    {history.length === 0 ? (
                        <div className="py-12 text-center text-muted-foreground text-sm">
                            <Bell className="w-10 h-10 mx-auto opacity-20 mb-3" />
                            No notifications sent yet this session.
                        </div>
                    ) : (
                        <div className="space-y-3 max-h-80 overflow-y-auto">
                            {history.map((h, i) => (
                                <div key={i} className="flex gap-3 p-3 rounded-lg bg-muted/30 border border-border">
                                    <div className={`w-2 h-2 rounded-full mt-1.5 flex-shrink-0 ${h.mode === "all" ? "bg-[#22d3ee]" : "bg-[#a855f7]"}`} />
                                    <div className="min-w-0 flex-1">
                                        <p className="text-xs font-medium mb-0.5">
                                            {h.mode === "all" ? `All Users (${h.count})` : `@${h.target}`}
                                        </p>
                                        <p className="text-sm text-muted-foreground line-clamp-2">{h.message}</p>
                                        <p className="text-xs text-muted-foreground mt-1">{h.time}</p>
                                    </div>
                                </div>
                            ))}
                        </div>
                    )}
                </div>
            </div>
        </div>
    );
}

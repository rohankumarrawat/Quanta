import { useState, useEffect } from "react";
import { useAuth } from "../../../context/AuthContext";
import axios from "axios";
import { Shield, ShieldOff, Trash2, Bell, Search, RefreshCw } from "lucide-react";

const API = "http://localhost:3001";

export function AdminUsers() {
    const { token } = useAuth();
    const headers = { Authorization: `Bearer ${token}` };
    const [users, setUsers] = useState([]);
    const [loading, setLoading] = useState(true);
    const [search, setSearch] = useState("");
    const [notifyUser, setNotifyUser] = useState(null);
    const [notifyMsg, setNotifyMsg] = useState("");
    const [sending, setSending] = useState(false);
    const [toast, setToast] = useState("");

    const load = () => {
        setLoading(true);
        axios.get(`${API}/api/admin/users`, { headers })
            .then(r => setUsers(r.data))
            .catch(() => { })
            .finally(() => setLoading(false));
    };

    useEffect(load, []);

    const showToast = (msg) => { setToast(msg); setTimeout(() => setToast(""), 3000); };

    const handleBlock = async (id) => {
        await axios.post(`${API}/api/admin/users/${id}/block`, {}, { headers });
        showToast("User status updated");
        load();
    };

    const handleDelete = async (id) => {
        if (!confirm("Delete this user? This cannot be undone.")) return;
        await axios.delete(`${API}/api/admin/users/${id}`, { headers });
        showToast("User deleted");
        load();
    };

    const handleNotify = async () => {
        if (!notifyMsg.trim()) return;
        setSending(true);
        await axios.post(`${API}/api/admin/notify`, { userId: notifyUser.id || notifyUser._id, message: notifyMsg }, { headers });
        setSending(false);
        setNotifyUser(null);
        setNotifyMsg("");
        showToast("Notification sent!");
    };

    const filtered = users.filter(u =>
        u.username?.toLowerCase().includes(search.toLowerCase()) ||
        u.email?.toLowerCase().includes(search.toLowerCase())
    );

    return (
        <div className="space-y-6">
            {toast && (
                <div className="fixed top-6 right-6 z-50 bg-[#22d3ee] text-background px-5 py-3 rounded-xl font-medium shadow-lg animate-bounce">
                    {toast}
                </div>
            )}

            <div className="flex items-center justify-between">
                <div>
                    <h1 className="text-3xl font-bold">Users</h1>
                    <p className="text-muted-foreground text-sm mt-1">{users.length} total users</p>
                </div>
                <button onClick={load} className="flex items-center gap-2 px-4 py-2 rounded-lg border border-border hover:bg-muted text-sm transition-all">
                    <RefreshCw className="w-3.5 h-3.5" /> Refresh
                </button>
            </div>

            {/* Search */}
            <div className="relative">
                <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-muted-foreground" />
                <input
                    value={search}
                    onChange={e => setSearch(e.target.value)}
                    placeholder="Search by username or email..."
                    className="w-full pl-10 pr-4 py-2.5 rounded-lg bg-card border border-border text-sm focus:outline-none focus:border-[#22d3ee]/50 transition-colors"
                />
            </div>

            {/* Table */}
            <div className="bg-card border border-border rounded-xl overflow-hidden">
                <table className="w-full text-sm">
                    <thead className="border-b border-border bg-muted/30">
                        <tr>
                            <th className="text-left px-5 py-3 text-muted-foreground font-medium">User</th>
                            <th className="text-left px-5 py-3 text-muted-foreground font-medium">Email</th>
                            <th className="text-left px-5 py-3 text-muted-foreground font-medium">Role</th>
                            <th className="text-left px-5 py-3 text-muted-foreground font-medium">Status</th>
                            <th className="text-right px-5 py-3 text-muted-foreground font-medium">Actions</th>
                        </tr>
                    </thead>
                    <tbody className="divide-y divide-border">
                        {loading ? (
                            [...Array(4)].map((_, i) => (
                                <tr key={i}>
                                    {[...Array(5)].map((_, j) => (
                                        <td key={j} className="px-5 py-4">
                                            <div className="h-4 bg-muted rounded animate-pulse w-24" />
                                        </td>
                                    ))}
                                </tr>
                            ))
                        ) : filtered.length === 0 ? (
                            <tr>
                                <td colSpan={5} className="px-5 py-10 text-center text-muted-foreground">No users found</td>
                            </tr>
                        ) : filtered.map((u) => (
                            <tr key={u.id || u._id} className="hover:bg-muted/20 transition-colors">
                                <td className="px-5 py-3">
                                    <div className="flex items-center gap-2.5">
                                        <div className="w-8 h-8 rounded-full bg-gradient-to-br from-[#22d3ee] to-[#a855f7] flex items-center justify-center text-background text-xs font-bold">
                                            {u.username?.[0]?.toUpperCase()}
                                        </div>
                                        <span className="font-medium">{u.username}</span>
                                    </div>
                                </td>
                                <td className="px-5 py-3 text-muted-foreground">{u.email}</td>
                                <td className="px-5 py-3">
                                    <span className={`px-2 py-0.5 rounded-full text-xs font-medium ${u.role === "admin" ? "bg-[#a855f7]/15 text-[#a855f7]" : "bg-[#22d3ee]/10 text-[#22d3ee]"}`}>
                                        {u.role || "user"}
                                    </span>
                                </td>
                                <td className="px-5 py-3">
                                    <span className={`px-2 py-0.5 rounded-full text-xs font-medium ${u.isBlocked ? "bg-red-500/15 text-red-400" : "bg-green-500/15 text-green-400"}`}>
                                        {u.isBlocked ? "Blocked" : "Active"}
                                    </span>
                                </td>
                                <td className="px-5 py-3">
                                    <div className="flex items-center gap-2 justify-end">
                                        <button
                                            onClick={() => { setNotifyUser(u); setNotifyMsg(""); }}
                                            className="p-1.5 rounded-lg text-muted-foreground hover:text-[#22d3ee] hover:bg-[#22d3ee]/10 transition-all"
                                            title="Send notification"
                                        >
                                            <Bell className="w-4 h-4" />
                                        </button>
                                        <button
                                            onClick={() => handleBlock(u.id || u._id)}
                                            className={`p-1.5 rounded-lg transition-all ${u.isBlocked ? "text-green-400 hover:bg-green-400/10" : "text-orange-400 hover:bg-orange-400/10"}`}
                                            title={u.isBlocked ? "Unblock" : "Block"}
                                        >
                                            {u.isBlocked ? <Shield className="w-4 h-4" /> : <ShieldOff className="w-4 h-4" />}
                                        </button>
                                        {u.role !== "admin" && (
                                            <button
                                                onClick={() => handleDelete(u.id || u._id)}
                                                className="p-1.5 rounded-lg text-red-400 hover:bg-red-400/10 transition-all"
                                                title="Delete user"
                                            >
                                                <Trash2 className="w-4 h-4" />
                                            </button>
                                        )}
                                    </div>
                                </td>
                            </tr>
                        ))}
                    </tbody>
                </table>
            </div>

            {/* Send Notification Modal */}
            {notifyUser && (
                <div className="fixed inset-0 z-50 bg-black/60 flex items-center justify-center p-4">
                    <div className="bg-card border border-border rounded-2xl p-6 w-full max-w-md space-y-4">
                        <h3 className="text-lg font-semibold">Send Notification</h3>
                        <p className="text-sm text-muted-foreground">
                            To: <span className="text-foreground font-medium">@{notifyUser.username}</span>
                        </p>
                        <textarea
                            value={notifyMsg}
                            onChange={e => setNotifyMsg(e.target.value)}
                            placeholder="Type your notification message..."
                            rows={4}
                            className="w-full px-4 py-3 rounded-lg bg-background border border-border text-sm resize-none focus:outline-none focus:border-[#22d3ee]/50"
                        />
                        <div className="flex gap-3">
                            <button
                                onClick={handleNotify}
                                disabled={sending || !notifyMsg.trim()}
                                className="flex-1 py-2.5 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium text-sm disabled:opacity-50"
                            >
                                {sending ? "Sending..." : "Send Notification"}
                            </button>
                            <button onClick={() => setNotifyUser(null)} className="px-5 py-2.5 rounded-lg border border-border text-sm hover:bg-muted transition-colors">
                                Cancel
                            </button>
                        </div>
                    </div>
                </div>
            )}
        </div>
    );
}

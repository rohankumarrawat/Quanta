import { useState, useEffect } from "react";
import { useAuth } from "../../../context/AuthContext";
import axios from "axios";
import { Plus, Edit3, Trash2, X, Save, RefreshCw } from "lucide-react";

const API = "http://localhost:3001";

export function AdminBlog() {
    const { token } = useAuth();
    const headers = { Authorization: `Bearer ${token}` };
    const [posts, setPosts] = useState([]);
    const [loading, setLoading] = useState(true);
    const [editing, setEditing] = useState(null); // null = list, 'new' = new, {post} = edit
    const [form, setForm] = useState({ title: "", preview: "", content: "", author: "", readTime: "5 min read", tags: "" });
    const [saving, setSaving] = useState(false);
    const [toast, setToast] = useState("");

    const showToast = (msg) => { setToast(msg); setTimeout(() => setToast(""), 3000); };

    const load = () => {
        setLoading(true);
        axios.get(`${API}/api/admin/blog`, { headers })
            .then(r => setPosts(r.data))
            .catch(() => { })
            .finally(() => setLoading(false));
    };

    useEffect(load, []);

    const openNew = () => {
        setForm({ title: "", preview: "", content: "", author: "", readTime: "5 min read", tags: "" });
        setEditing("new");
    };

    const openEdit = (post) => {
        setForm({ title: post.title || "", preview: post.preview || "", content: post.content || "", author: post.author || "", readTime: post.readTime || "5 min read", tags: (post.tags || []).join(", ") });
        setEditing(post);
    };

    const handleSave = async () => {
        if (!form.title || !form.preview) return;
        setSaving(true);
        const payload = { ...form, tags: form.tags ? form.tags.split(",").map(t => t.trim()).filter(Boolean) : [] };
        try {
            if (editing === "new") {
                await axios.post(`${API}/api/admin/blog`, payload, { headers });
                showToast("Blog post created!");
            } else {
                await axios.put(`${API}/api/admin/blog/${editing._id}`, payload, { headers });
                showToast("Blog post updated!");
            }
            setEditing(null);
            load();
        } catch {
            showToast("Error saving post.");
        } finally {
            setSaving(false);
        }
    };

    const handleDelete = async (id) => {
        if (!confirm("Delete this blog post?")) return;
        await axios.delete(`${API}/api/admin/blog/${id}`, { headers });
        showToast("Post deleted.");
        load();
    };

    if (editing !== null) {
        return (
            <div className="space-y-6">
                {toast && <div className="fixed top-6 right-6 z-50 bg-[#22d3ee] text-background px-5 py-3 rounded-xl font-medium shadow-lg">{toast}</div>}
                <div className="flex items-center justify-between">
                    <h1 className="text-2xl font-bold">{editing === "new" ? "New Blog Post" : "Edit Blog Post"}</h1>
                    <button onClick={() => setEditing(null)} className="flex items-center gap-2 text-sm text-muted-foreground hover:text-foreground transition-colors">
                        <X className="w-4 h-4" /> Cancel
                    </button>
                </div>
                <div className="bg-card border border-border rounded-xl p-6 space-y-5">
                    <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                        <div className="space-y-1.5">
                            <label className="text-xs font-medium text-muted-foreground">Title *</label>
                            <input value={form.title} onChange={e => setForm(f => ({ ...f, title: e.target.value }))} placeholder="Post title..." className="w-full px-4 py-2.5 rounded-lg bg-background border border-border text-sm focus:outline-none focus:border-[#22d3ee]/50" />
                        </div>
                        <div className="space-y-1.5">
                            <label className="text-xs font-medium text-muted-foreground">Author</label>
                            <input value={form.author} onChange={e => setForm(f => ({ ...f, author: e.target.value }))} placeholder="Author name..." className="w-full px-4 py-2.5 rounded-lg bg-background border border-border text-sm focus:outline-none focus:border-[#22d3ee]/50" />
                        </div>
                    </div>
                    <div className="space-y-1.5">
                        <label className="text-xs font-medium text-muted-foreground">Preview / Excerpt *</label>
                        <textarea value={form.preview} onChange={e => setForm(f => ({ ...f, preview: e.target.value }))} placeholder="Short description shown in the blog list..." rows={3} className="w-full px-4 py-2.5 rounded-lg bg-background border border-border text-sm resize-none focus:outline-none focus:border-[#22d3ee]/50" />
                    </div>
                    <div className="space-y-1.5">
                        <label className="text-xs font-medium text-muted-foreground">Content</label>
                        <textarea value={form.content} onChange={e => setForm(f => ({ ...f, content: e.target.value }))} placeholder="Full blog post content..." rows={10} className="w-full px-4 py-2.5 rounded-lg bg-background border border-border text-sm font-mono resize-none focus:outline-none focus:border-[#22d3ee]/50" />
                    </div>
                    <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                        <div className="space-y-1.5">
                            <label className="text-xs font-medium text-muted-foreground">Read Time</label>
                            <input value={form.readTime} onChange={e => setForm(f => ({ ...f, readTime: e.target.value }))} placeholder="5 min read" className="w-full px-4 py-2.5 rounded-lg bg-background border border-border text-sm focus:outline-none focus:border-[#22d3ee]/50" />
                        </div>
                        <div className="space-y-1.5">
                            <label className="text-xs font-medium text-muted-foreground">Tags (comma-separated)</label>
                            <input value={form.tags} onChange={e => setForm(f => ({ ...f, tags: e.target.value }))} placeholder="release, tutorial, guide" className="w-full px-4 py-2.5 rounded-lg bg-background border border-border text-sm focus:outline-none focus:border-[#22d3ee]/50" />
                        </div>
                    </div>
                    <div className="flex justify-end gap-3 pt-2">
                        <button onClick={() => setEditing(null)} className="px-5 py-2.5 rounded-lg border border-border text-sm hover:bg-muted transition-colors">Cancel</button>
                        <button onClick={handleSave} disabled={saving || !form.title || !form.preview} className="flex items-center gap-2 px-6 py-2.5 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium text-sm disabled:opacity-50 hover:shadow-lg hover:shadow-[#22d3ee]/20 transition-all">
                            <Save className="w-4 h-4" /> {saving ? "Saving..." : "Save Post"}
                        </button>
                    </div>
                </div>
            </div>
        );
    }

    return (
        <div className="space-y-6">
            {toast && <div className="fixed top-6 right-6 z-50 bg-[#22d3ee] text-background px-5 py-3 rounded-xl font-medium shadow-lg">{toast}</div>}
            <div className="flex items-center justify-between">
                <div>
                    <h1 className="text-3xl font-bold">Blog Posts</h1>
                    <p className="text-muted-foreground text-sm mt-1">{posts.length} posts</p>
                </div>
                <div className="flex gap-2">
                    <button onClick={load} className="flex items-center gap-2 px-4 py-2 rounded-lg border border-border hover:bg-muted text-sm transition-all"><RefreshCw className="w-3.5 h-3.5" /> Refresh</button>
                    <button onClick={openNew} className="flex items-center gap-2 px-4 py-2 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background text-sm font-medium hover:shadow-lg hover:shadow-[#22d3ee]/20 transition-all">
                        <Plus className="w-4 h-4" /> New Post
                    </button>
                </div>
            </div>

            {loading ? (
                <div className="space-y-3">{[...Array(4)].map((_, i) => <div key={i} className="h-24 rounded-xl bg-card border border-border animate-pulse" />)}</div>
            ) : posts.length === 0 ? (
                <div className="py-20 text-center">
                    <p className="text-muted-foreground mb-4">No blog posts yet.</p>
                    <button onClick={openNew} className="px-5 py-2.5 rounded-lg bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background text-sm font-medium">Create First Post</button>
                </div>
            ) : (
                <div className="space-y-3">
                    {posts.map((post) => (
                        <div key={post._id} className="bg-card border border-border rounded-xl p-5 flex items-start justify-between gap-4 hover:border-[#22d3ee]/20 transition-all">
                            <div className="flex-1 min-w-0">
                                <h3 className="font-semibold mb-1 truncate">{post.title}</h3>
                                <p className="text-sm text-muted-foreground line-clamp-1 mb-2">{post.preview}</p>
                                <div className="flex items-center gap-3 text-xs text-muted-foreground">
                                    <span>By {post.author}</span>
                                    <span>·</span>
                                    <span>{post.date}</span>
                                    <span>·</span>
                                    <span>{post.readTime}</span>
                                </div>
                            </div>
                            <div className="flex items-center gap-2 flex-shrink-0">
                                <button onClick={() => openEdit(post)} className="p-2 rounded-lg text-muted-foreground hover:text-[#22d3ee] hover:bg-[#22d3ee]/10 transition-all">
                                    <Edit3 className="w-4 h-4" />
                                </button>
                                <button onClick={() => handleDelete(post._id)} className="p-2 rounded-lg text-muted-foreground hover:text-red-400 hover:bg-red-400/10 transition-all">
                                    <Trash2 className="w-4 h-4" />
                                </button>
                            </div>
                        </div>
                    ))}
                </div>
            )}
        </div>
    );
}

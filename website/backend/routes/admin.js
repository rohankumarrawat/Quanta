const express = require("express");
const router = express.Router();
const adminAuth = require("../middleware/adminAuth");
const { usingInMemory } = require("../config/db");

// We'll import models conditionally
let User, Question, BlogPost;
try {
    User = require("../models/User");
    Question = require("../models/Question");
    BlogPost = require("../models/BlogPost");
} catch (e) { }

// In-memory admin data store (shared with other routes via global)
// The actual memUsers, memQuestions, memPosts arrays are in their respective route files
// For admin operations, we use the same in-memory references via global store

// ── Stats ────────────────────────────────────────────────────────────────────
// GET /api/admin/stats
router.get("/stats", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            return res.json({
                totalUsers: (g.users || []).length,
                pendingQuestions: (g.questions || []).filter(q => q.status === "pending").length,
                totalQuestions: (g.questions || []).length,
                totalPosts: (g.posts || []).length,
                blockedUsers: (g.users || []).filter(u => u.isBlocked).length,
            });
        }
        const [totalUsers, pendingQuestions, totalQuestions, totalPosts, blockedUsers] = await Promise.all([
            User.countDocuments(),
            Question.countDocuments({ status: "pending" }),
            Question.countDocuments(),
            BlogPost.countDocuments(),
            User.countDocuments({ isBlocked: true }),
        ]);
        res.json({ totalUsers, pendingQuestions, totalQuestions, totalPosts, blockedUsers });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// ── Users ────────────────────────────────────────────────────────────────────
// GET /api/admin/users
router.get("/users", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const users = (g.users || []).map(u => ({
                id: u.id || u._id,
                username: u.username,
                email: u.email,
                role: u.role || "user",
                isBlocked: u.isBlocked || false,
                createdAt: u.createdAt || new Date().toISOString(),
                notifications: u.notifications || [],
            }));
            return res.json(users);
        }
        const users = await User.find({}, "-password").sort({ createdAt: -1 });
        res.json(users);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/admin/users/:id/block
router.post("/users/:id/block", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const user = (g.users || []).find(u => (u.id || u._id) === req.params.id);
            if (!user) return res.status(404).json({ message: "User not found" });
            user.isBlocked = !user.isBlocked;
            return res.json({ id: user.id || user._id, username: user.username, isBlocked: user.isBlocked });
        }
        const user = await User.findById(req.params.id);
        if (!user) return res.status(404).json({ message: "User not found" });
        user.isBlocked = !user.isBlocked;
        await user.save();
        res.json({ id: user._id, username: user.username, isBlocked: user.isBlocked });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// DELETE /api/admin/users/:id
router.delete("/users/:id", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const idx = (g.users || []).findIndex(u => (u.id || u._id) === req.params.id);
            if (idx === -1) return res.status(404).json({ message: "User not found" });
            g.users.splice(idx, 1);
            return res.json({ message: "User deleted" });
        }
        await User.findByIdAndDelete(req.params.id);
        res.json({ message: "User deleted" });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// ── Notifications ─────────────────────────────────────────────────────────────
// POST /api/admin/notify  body: { userId?: string, message: string }
router.post("/notify", adminAuth, async (req, res) => {
    try {
        const { userId, message } = req.body;
        if (!message) return res.status(400).json({ message: "Message is required" });

        const notification = { message, read: false, createdAt: new Date().toISOString() };

        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            if (userId) {
                const user = (g.users || []).find(u => (u.id || u._id) === userId);
                if (!user) return res.status(404).json({ message: "User not found" });
                if (!user.notifications) user.notifications = [];
                user.notifications.unshift(notification);
                return res.json({ message: "Notification sent", count: 1 });
            }
            // broadcast to all
            (g.users || []).forEach(u => {
                if (!u.notifications) u.notifications = [];
                u.notifications.unshift({ ...notification });
            });
            return res.json({ message: "Notification sent to all users", count: (g.users || []).length });
        }

        if (userId) {
            await User.findByIdAndUpdate(userId, { $push: { notifications: { $each: [notification], $position: 0 } } });
            return res.json({ message: "Notification sent", count: 1 });
        }
        const result = await User.updateMany({}, { $push: { notifications: { $each: [notification], $position: 0 } } });
        res.json({ message: "Notification sent to all users", count: result.modifiedCount });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// ── Questions Moderation ──────────────────────────────────────────────────────
// GET /api/admin/questions/pending
router.get("/questions/pending", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const pending = (g.questions || []).filter(q => q.status === "pending");
            return res.json(pending);
        }
        const questions = await Question.find({ status: "pending" }).sort({ createdAt: -1 });
        res.json(questions);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// GET /api/admin/questions  (all questions for admin view)
router.get("/questions", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            return res.json(g.questions || []);
        }
        const questions = await Question.find().sort({ createdAt: -1 });
        res.json(questions);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/admin/questions/:id/approve
router.post("/questions/:id/approve", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const q = (g.questions || []).find(q => q._id === req.params.id);
            if (!q) return res.status(404).json({ message: "Question not found" });
            q.status = "approved";
            return res.json(q);
        }
        const question = await Question.findByIdAndUpdate(req.params.id, { status: "approved" }, { new: true });
        if (!question) return res.status(404).json({ message: "Question not found" });
        res.json(question);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/admin/questions/:id/reject
router.post("/questions/:id/reject", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const q = (g.questions || []).find(q => q._id === req.params.id);
            if (!q) return res.status(404).json({ message: "Question not found" });
            q.status = "rejected";
            return res.json(q);
        }
        const question = await Question.findByIdAndUpdate(req.params.id, { status: "rejected" }, { new: true });
        if (!question) return res.status(404).json({ message: "Question not found" });
        res.json(question);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// ── Blog (admin) ──────────────────────────────────────────────────────────────
// GET /api/admin/blog
router.get("/blog", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            return res.json(g.posts || []);
        }
        const posts = await BlogPost.find().sort({ createdAt: -1 });
        res.json(posts);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/admin/blog
router.post("/blog", adminAuth, async (req, res) => {
    try {
        const { title, preview, content, author, readTime, tags } = req.body;
        if (!title || !preview) return res.status(400).json({ message: "Title and preview are required" });

        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            if (!g.posts) g.posts = [];
            const post = {
                _id: Date.now().toString(),
                title, preview, content: content || "",
                author: author || req.user.username,
                date: new Date().toLocaleDateString("en-US", { year: "numeric", month: "long", day: "numeric" }),
                readTime: readTime || "5 min read",
                tags: tags || [],
            };
            g.posts.unshift(post);
            return res.status(201).json(post);
        }
        const post = await BlogPost.create({ title, preview, content, author: author || req.user.username, readTime, tags });
        res.status(201).json(post);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// PUT /api/admin/blog/:id
router.put("/blog/:id", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const idx = (g.posts || []).findIndex(p => p._id === req.params.id);
            if (idx === -1) return res.status(404).json({ message: "Post not found" });
            g.posts[idx] = { ...g.posts[idx], ...req.body };
            return res.json(g.posts[idx]);
        }
        const post = await BlogPost.findByIdAndUpdate(req.params.id, req.body, { new: true });
        if (!post) return res.status(404).json({ message: "Post not found" });
        res.json(post);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// DELETE /api/admin/blog/:id
router.delete("/blog/:id", adminAuth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const g = global.__quantaStore || {};
            const idx = (g.posts || []).findIndex(p => p._id === req.params.id);
            if (idx === -1) return res.status(404).json({ message: "Post not found" });
            g.posts.splice(idx, 1);
            return res.json({ message: "Post deleted" });
        }
        await BlogPost.findByIdAndDelete(req.params.id);
        res.json({ message: "Post deleted" });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

module.exports = router;

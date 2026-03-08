const express = require("express");
const BlogPost = require("../models/BlogPost");
const auth = require("../middleware/auth");
const { usingInMemory } = require("../config/db");

const router = express.Router();

// Use global store so admin routes share the same data
if (!global.__quantaStore) global.__quantaStore = { users: [], questions: [], posts: [] };

// Seed default blog posts
if (global.__quantaStore.posts.length === 0) {
    global.__quantaStore.posts = [
        {
            _id: "1",
            title: "Introducing Quanta v1.0 — A Language Built for the Real World",
            preview: "After months of development, Quanta v1.0 is here. Here's everything new: the unified loop construct, dual string system, embedded-ready types, and tight LLVM backend integration.",
            content: `Quanta v1.0 is the first stable release of the Quanta programming language — a modern, memory-safe, general-purpose language designed to bridge the gap between high-level scripting and low-level systems programming.

Key highlights of v1.0:
- Unified loop construct (replaces while, for, and do-while)
- Dual String System: dynamic heap strings and static stack buffers
- Explicit integer types: int8, int32 for embedded precision
- LLVM backend for native performance
- Auto-free memory management for heap strings

Download it today from the website and run your first .qnt file!`,
            author: "Rohan Kumar Rawat",
            date: "March 7, 2026",
            readTime: "4 min read",
            tags: ["release", "v1.0"]
        },
        {
            _id: "2",
            title: "Mastering the Dual String System in Quanta",
            preview: "Quanta's dual string system is one of its most powerful features. Learn when to use dynamic heap strings vs. static stack buffers, and how both integrate with the full string standard library.",
            content: `Quanta gives you two kinds of strings: dynamic strings (string) and static buffers (string[N]).

Dynamic strings are heap-allocated and auto-freed — perfect for desktop scripting where string lengths are unpredictable.

Static stack buffers (string[16], string[64], etc.) are perfect for microcontrollers and embedded devices where you need predictable memory at compile time. If you write more than N characters, Quanta safely truncates — no buffer overflow.

Both work with the full standard library: upper(), lower(), strip(), replace(), reverse(), find(), and more.`,
            author: "Rohan Kumar Rawat",
            date: "February 28, 2026",
            readTime: "6 min read",
            tags: ["tutorial", "strings", "embedded"]
        },
        {
            _id: "3",
            title: "Building Embedded Systems with Quanta",
            preview: "Quanta targets both desktop and microcontroller environments. Here's how to use int8, string[N], loop, and zero-overhead design to write firmware in Quanta.",
            content: `Quanta was designed from day one with embedded systems in mind.

Use int8 and int16 to match the word size of your target MCU. Use string[N] to allocate string buffers on the stack with zero heap usage. The unified loop construct generates tight, predictable machine code.

The LLVM backend ensures your Quanta code compiles to the same efficient output as equivalent C code — without the manual memory management.`,
            author: "Rohan Kumar Rawat",
            date: "February 20, 2026",
            readTime: "8 min read",
            tags: ["embedded", "iot", "tutorial"]
        },
    ];
}

// GET /api/blog
router.get("/", async (_req, res) => {
    try {
        if (usingInMemory()) return res.json(global.__quantaStore.posts);
        const posts = await BlogPost.find().sort({ createdAt: -1 });
        res.json(posts);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// GET /api/blog/:id
router.get("/:id", async (req, res) => {
    try {
        if (usingInMemory()) {
            const post = global.__quantaStore.posts.find((p) => p._id === req.params.id);
            if (!post) return res.status(404).json({ message: "Post not found" });
            return res.json(post);
        }
        const post = await BlogPost.findById(req.params.id);
        if (!post) return res.status(404).json({ message: "Post not found" });
        res.json(post);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/blog/publish (from Quanta Studio Editor)
router.post("/publish", auth, async (req, res) => {
    try {
        const { title, preview, content, readTime, tags } = req.body;
        if (!title || !content) return res.status(400).json({ message: "Title and content are required" });

        const author = req.user.username;
        const date = new Date().toLocaleDateString("en-US", { year: "numeric", month: "long", day: "numeric" });

        // Ensure tags is an array
        let parsedTags = tags;
        if (typeof tags === 'string') {
            parsedTags = tags.split(",").map(t => t.trim()).filter(Boolean);
        } else if (!Array.isArray(tags)) {
            parsedTags = ["ai-generated", "quanta"];
        }

        if (usingInMemory()) {
            const post = {
                _id: Date.now().toString(),
                title,
                preview: preview || content.substring(0, 150) + "...",
                content,
                author,
                date,
                readTime: readTime || "3 min read",
                tags: parsedTags
            };
            global.__quantaStore.posts.unshift(post);
            return res.status(201).json(post);
        }

        const post = await BlogPost.create({
            title,
            preview: preview || content.substring(0, 150) + "...",
            content,
            author,
            date,
            readTime: readTime || "3 min read",
            tags: parsedTags
        });
        res.status(201).json(post);
    } catch (err) {
        console.error("Error publishing blog from editor:", err);
        res.status(500).json({ message: err.message });
    }
});

// POST /api/blog (auth required)
router.post("/", auth, async (req, res) => {
    try {
        const { title, preview, content, author, date, readTime, tags } = req.body;
        if (!title || !preview) return res.status(400).json({ message: "Title and preview are required" });

        if (usingInMemory()) {
            const post = { _id: Date.now().toString(), title, preview, content: content || "", author: author || req.user.username, date: date || new Date().toLocaleDateString("en-US", { year: "numeric", month: "long", day: "numeric" }), readTime: readTime || "5 min read", tags: tags || [] };
            global.__quantaStore.posts.unshift(post);
            return res.status(201).json(post);
        }

        const post = await BlogPost.create({ title, preview, content, author: author || req.user.username, date: date || new Date().toLocaleDateString("en-US", { year: "numeric", month: "long", day: "numeric" }), readTime, tags });
        res.status(201).json(post);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// PUT /api/blog/:id (auth required)
router.put("/:id", auth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const idx = global.__quantaStore.posts.findIndex((p) => p._id === req.params.id);
            if (idx === -1) return res.status(404).json({ message: "Post not found" });
            global.__quantaStore.posts[idx] = { ...global.__quantaStore.posts[idx], ...req.body };
            return res.json(global.__quantaStore.posts[idx]);
        }
        const post = await BlogPost.findByIdAndUpdate(req.params.id, req.body, { new: true });
        if (!post) return res.status(404).json({ message: "Post not found" });
        res.json(post);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// DELETE /api/blog/:id (auth required)
router.delete("/:id", auth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const idx = global.__quantaStore.posts.findIndex((p) => p._id === req.params.id);
            if (idx === -1) return res.status(404).json({ message: "Post not found" });
            global.__quantaStore.posts.splice(idx, 1);
            return res.json({ message: "Post deleted" });
        }
        const post = await BlogPost.findByIdAndDelete(req.params.id);
        if (!post) return res.status(404).json({ message: "Post not found" });
        res.json({ message: "Post deleted" });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

module.exports = router;

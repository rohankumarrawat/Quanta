const express = require("express");
const Question = require("../models/Question");
const auth = require("../middleware/auth");
const { usingInMemory } = require("../config/db");

const router = express.Router();

// Bootstrap in-memory store
if (!global.__quantaStore) global.__quantaStore = { users: [], questions: [], posts: [] };

// Seed default questions (approved) for demo
if (global.__quantaStore.questions.length === 0) {
    global.__quantaStore.questions = [
        { _id: "1", votes: 42, answers: [{ content: "Use string[N] for embedded — it stays on the stack and is truncation-safe. Use the dynamic string type for desktop where length is variable.", username: "embedded_queen", createdAt: "2026-03-07T10:00:00Z" }], views: 1203, title: "How does the Dual String System work in Quanta?", preview: "I'm confused about when to use string vs string[N]. When should I prefer the static buffer approach for my embedded project?", content: "I'm confused about when to use string vs string[N]. When should I prefer the static buffer approach for my embedded project?", tags: ["strings", "embedded", "memory"], username: "embedded_dev", timePosted: "2 hours ago", isAnswered: true, status: "approved" },
        { _id: "2", votes: 38, answers: [], views: 892, title: "How do I iterate over a string character by character in Quanta?", preview: "I see there's a 'loop i in s' syntax but the docs are sparse. Can someone show a complete example with indexing?", content: "I see there's a 'loop i in s' syntax but the docs are sparse. Can someone show a complete example with indexing?", tags: ["loop", "strings", "syntax"], username: "alice_dev", timePosted: "5 hours ago", isAnswered: false, status: "approved" },
        { _id: "3", votes: 27, answers: [], views: 456, title: "What's the difference between int, int8, and int32 in Quanta?", preview: "When should I use each integer type? Is there a performance or memory reason for picking int8 on a microcontroller?", content: "When should I use each integer type?", tags: ["types", "embedded", "memory"], username: "qnt_explorer", timePosted: "1 day ago", isAnswered: false, status: "approved" },
    ];
}

// GET /api/community  — only show approved questions to public
router.get("/", async (req, res) => {
    try {
        const { filter = "all", sort = "newest" } = req.query;

        if (usingInMemory()) {
            let result = global.__quantaStore.questions.filter(q => q.status === "approved");
            if (filter === "unanswered") result = result.filter((q) => !q.isAnswered);
            if (filter === "popular") result = result.sort((a, b) => b.votes - a.votes);
            if (sort === "votes") result = result.sort((a, b) => b.votes - a.votes);
            return res.json(result);
        }

        let query = Question.find({ status: "approved" });
        if (filter === "unanswered") query = query.where("isAnswered").equals(false);
        if (sort === "votes") query = query.sort({ votes: -1 });
        else if (sort === "activity") query = query.sort({ updatedAt: -1 });
        else query = query.sort({ createdAt: -1 });

        const questions = await query.exec();
        res.json(questions);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// GET /api/community/:id
router.get("/:id", async (req, res) => {
    try {
        if (usingInMemory()) {
            const q = global.__quantaStore.questions.find((q) => q._id === req.params.id);
            if (!q) return res.status(404).json({ message: "Question not found" });
            return res.json(q);
        }
        const question = await Question.findById(req.params.id);
        if (!question) return res.status(404).json({ message: "Question not found" });
        res.json(question);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/community (auth required) — new questions default to "pending"
router.post("/", auth, async (req, res) => {
    try {
        const { title, preview, tags, content } = req.body;
        if (!title || !preview) return res.status(400).json({ message: "Title and preview are required" });

        // Check if user is blocked
        const g = global.__quantaStore;
        const user = (g.users || []).find(u => (u.id || u._id?.toString()) === req.user.id);
        if (user && user.isBlocked) {
            return res.status(403).json({ message: "Your account is blocked. You cannot post." });
        }

        if (usingInMemory()) {
            const q = { _id: Date.now().toString(), title, preview, content: content || preview, tags: tags || [], username: req.user.username, votes: 0, answers: [], views: 0, isAnswered: false, timePosted: "just now", status: "pending" };
            global.__quantaStore.questions.unshift(q);
            return res.status(201).json({ ...q, message: "Your question has been submitted and is pending admin approval." });
        }

        const question = await Question.create({ title, preview, content: content || preview, tags: tags || [], username: req.user.username, status: "pending" });
        res.status(201).json({ ...question.toObject(), message: "Your question has been submitted and is pending admin approval." });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/community/:id/vote
router.post("/:id/vote", async (req, res) => {
    try {
        if (usingInMemory()) {
            const q = global.__quantaStore.questions.find((q) => q._id === req.params.id);
            if (!q) return res.status(404).json({ message: "Question not found" });
            q.votes += 1;
            return res.json(q);
        }
        const question = await Question.findByIdAndUpdate(
            req.params.id,
            { $inc: { votes: 1 } },
            { new: true }
        );
        if (!question) return res.status(404).json({ message: "Question not found" });
        res.json(question);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/community/:id/answer (auth required)
router.post("/:id/answer", auth, async (req, res) => {
    try {
        const { answer } = req.body;
        if (!answer) return res.status(400).json({ message: "Answer content is required" });

        if (usingInMemory()) {
            const q = global.__quantaStore.questions.find((q) => q._id === req.params.id);
            if (!q) return res.status(404).json({ message: "Question not found" });
            q.answers.push({ content: answer, username: req.user.username, createdAt: new Date() });
            q.isAnswered = true;
            return res.json(q);
        }

        const question = await Question.findByIdAndUpdate(
            req.params.id,
            { $push: { answers: { content: answer, username: req.user.username } }, isAnswered: true },
            { new: true }
        );
        if (!question) return res.status(404).json({ message: "Question not found" });
        res.json(question);
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

module.exports = router;

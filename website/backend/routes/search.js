const express = require("express");
const BlogPost = require("../models/BlogPost");
const Question = require("../models/Question");
const { usingInMemory } = require("../config/db");

const router = express.Router();

// Inline fallback data for in-memory search (avoiding circular dependency)
const FALLBACK_POSTS = [
    { _id: "1", title: "Introducing Quantum Error Correction in Quanta v2.7", preview: "Built-in quantum error correction capabilities.", tags: ["release", "error-correction"] },
    { _id: "2", title: "Building Your First Quantum Algorithm with Quanta", preview: "A comprehensive guide to creating quantum algorithms.", tags: ["tutorial", "algorithms"] },
    { _id: "3", title: "Performance Benchmarks: Quanta vs Traditional Frameworks", preview: "Performance comparisons between Quanta and other quantum programming frameworks.", tags: ["performance", "benchmarks"] },
    { _id: "4", title: "Understanding Quantum Entanglement with Practical Examples", preview: "Quantum entanglement through hands-on Quanta code examples.", tags: ["quantum", "entanglement"] },
    { _id: "5", title: "Quanta's Type System: Safety Meets Quantum Computing", preview: "How the type system prevents quantum programming errors at compile time.", tags: ["type-system", "safety"] },
    { _id: "6", title: "From Classical to Quantum: A Developer's Journey", preview: "Transitioning from classical programming to quantum computing with Quanta.", tags: ["story", "getting-started"] },
];
const FALLBACK_QUESTIONS = [
    { _id: "1", title: "How to implement quantum error correction in Quanta?", preview: "I need more control over the correction strategy...", tags: ["quantum-computing", "error-correction", "memory-core"] },
    { _id: "2", title: "Best practices for optimizing quantum circuit depth?", preview: "Circuit depth is too large for NISQ devices.", tags: ["optimization", "circuit-design", "performance"] },
    { _id: "3", title: "Entanglement API - creating multi-qubit Bell states", preview: "I need to create 4-qubit GHZ states.", tags: ["entanglement", "bell-states", "api"] },
    { _id: "4", title: "Quanta vs Qiskit - when to use which?", preview: "What are the main advantages of switching to Quanta?", tags: ["comparison", "qiskit", "getting-started"] },
    { _id: "5", title: "Type inference not working for quantum state variables", preview: "The compiler is throwing type errors.", tags: ["type-system", "compiler", "bug"] },
    { _id: "6", title: "Performance benchmarks: Quanta simulator vs real quantum hardware", preview: "Performance comparisons between the simulator and quantum processors.", tags: ["performance", "benchmarks", "hardware"] },
];

// GET /api/search?q=<query>
router.get("/", async (req, res) => {
    try {
        const { q = "" } = req.query;
        const query = q.toLowerCase().trim();
        if (!query) return res.json({ posts: [], questions: [] });

        const matchItem = (item) =>
            item.title.toLowerCase().includes(query) ||
            item.preview.toLowerCase().includes(query) ||
            (item.tags || []).some((t) => t.toLowerCase().includes(query));

        if (usingInMemory()) {
            return res.json({
                posts: FALLBACK_POSTS.filter(matchItem),
                questions: FALLBACK_QUESTIONS.filter(matchItem),
            });
        }

        const [posts, questions] = await Promise.all([
            BlogPost.find({
                $or: [
                    { title: { $regex: query, $options: "i" } },
                    { preview: { $regex: query, $options: "i" } },
                    { tags: { $elemMatch: { $regex: query, $options: "i" } } },
                ],
            }).limit(5),
            Question.find({
                $or: [
                    { title: { $regex: query, $options: "i" } },
                    { preview: { $regex: query, $options: "i" } },
                    { tags: { $elemMatch: { $regex: query, $options: "i" } } },
                ],
            }).limit(5),
        ]);

        res.json({ posts, questions });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

module.exports = router;

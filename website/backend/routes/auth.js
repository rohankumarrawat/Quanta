const express = require("express");
const jwt = require("jsonwebtoken");
const bcrypt = require("bcryptjs");
const User = require("../models/User");
const { usingInMemory } = require("../config/db");
const auth = require("../middleware/auth");

const router = express.Router();
const JWT_SECRET = process.env.JWT_SECRET || "quanta_secret";

// Bootstrap in-memory store
if (!global.__quantaStore) global.__quantaStore = { users: [], questions: [], posts: [] };
const memUsers = global.__quantaStore.users;

// Seed default admin account synchronously on module load
const ADMIN_EMAIL = "rohan@rohan.com";
if (!memUsers.find(u => u.email === ADMIN_EMAIL)) {
    // Pre-hash: bcrypt hash of "rohan@rohan.com" with 12 rounds (pre-computed for sync seed)
    const adminHash = bcrypt.hashSync("rohan@rohan.com", 12);
    memUsers.push({
        id: "admin_001",
        username: "rohan@rohan.com",
        email: ADMIN_EMAIL,
        password: adminHash,
        role: "admin",
        isBlocked: false,
        notifications: [],
        createdAt: new Date().toISOString(),
    });
    console.log("\n✅ Admin account seeded: rohan@rohan.com / rohan@rohan.com\n");
}

function generateToken(user) {
    return jwt.sign(
        { id: user._id || user.id, username: user.username, email: user.email, role: user.role || "user" },
        JWT_SECRET,
        { expiresIn: "7d" }
    );
}

// POST /api/auth/register
router.post("/register", async (req, res) => {
    try {
        const { username, email, password } = req.body;
        if (!username || !email || !password)
            return res.status(400).json({ message: "All fields are required" });
        if (password.length < 6)
            return res.status(400).json({ message: "Password must be at least 6 characters" });

        if (usingInMemory()) {
            const exists = memUsers.find((u) => u.email === email || u.username === username);
            if (exists) return res.status(409).json({ message: "User already exists" });
            const hashed = await bcrypt.hash(password, 12);
            const user = { id: Date.now().toString(), username, email, password: hashed, role: "user", isBlocked: false, notifications: [], createdAt: new Date().toISOString() };
            memUsers.push(user);
            const token = generateToken(user);
            return res.status(201).json({ token, user: { id: user.id, username, email, role: user.role } });
        }

        const exists = await User.findOne({ $or: [{ email }, { username }] });
        if (exists) return res.status(409).json({ message: "User already exists" });

        const user = await User.create({ username, email, password });
        const token = generateToken(user);
        res.status(201).json({ token, user: { id: user._id, username, email, role: user.role } });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/auth/login
router.post("/login", async (req, res) => {
    try {
        const { email, password } = req.body;
        if (!email || !password)
            return res.status(400).json({ message: "Email and password are required" });

        if (usingInMemory()) {
            const user = memUsers.find((u) => u.email === email);
            if (!user) return res.status(401).json({ message: "Invalid email or password" });
            const valid = await bcrypt.compare(password, user.password);
            if (!valid) return res.status(401).json({ message: "Invalid email or password" });
            if (user.isBlocked) return res.status(403).json({ message: "Your account has been blocked. Contact admin." });
            const token = generateToken(user);
            return res.json({ token, user: { id: user.id, username: user.username, email, role: user.role, notifications: user.notifications || [] } });
        }

        const user = await User.findOne({ email });
        if (!user) return res.status(401).json({ message: "Invalid email or password" });
        const valid = await user.comparePassword(password);
        if (!valid) return res.status(401).json({ message: "Invalid email or password" });
        if (user.isBlocked) return res.status(403).json({ message: "Your account has been blocked. Contact admin." });

        const token = generateToken(user);
        res.json({ token, user: { id: user._id, username: user.username, email, role: user.role, notifications: user.notifications || [] } });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

// POST /api/auth/reward
router.post("/reward", auth, async (req, res) => {
    try {
        if (usingInMemory()) {
            const user = memUsers.find((u) => u.id === req.user.id);
            if (!user) return res.status(404).json({ message: "User not found" });

            user.coins = (user.coins || 0) + 10;
            return res.json({ message: "Coins added", coins: user.coins });
        }

        const user = await User.findById(req.user.id);
        if (!user) return res.status(404).json({ message: "User not found" });

        user.coins = (user.coins || 0) + 10;
        await user.save();

        res.json({ message: "Coins added", coins: user.coins });
    } catch (err) {
        res.status(500).json({ message: err.message });
    }
});

module.exports = router;

require("dotenv").config();
const express = require("express");
const cors = require("cors");
const connectDB = require("./config/db");

const authRoutes = require("./routes/auth");
const blogRoutes = require("./routes/blog");
const communityRoutes = require("./routes/community");
const searchRoutes = require("./routes/search");
const adminRoutes = require("./routes/admin");

const app = express();
const PORT = process.env.PORT || 3001;

// Middleware
app.use(cors({ origin: ["http://localhost:5173", "http://localhost:3000"], credentials: true }));
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Connect to DB (non-blocking – falls back to in-memory store)
connectDB();

// Routes
app.use("/api/auth", authRoutes);
app.use("/api/blog", blogRoutes);
app.use("/api/community", communityRoutes);
app.use("/api/search", searchRoutes);
app.use("/api/admin", adminRoutes);

// Health check
app.get("/api/health", (_req, res) => res.json({ status: "ok", timestamp: new Date().toISOString() }));

// 404 handler
app.use((_req, res) => res.status(404).json({ message: "Route not found" }));

// Global error handler
app.use((err, _req, res, _next) => {
    console.error(err.stack);
    res.status(500).json({ message: err.message || "Internal Server Error" });
});

if (require.main === module) {
    app.listen(PORT, () => {
        console.log(`\n🚀 Quanta Docs API running at http://localhost:${PORT}`);
        console.log(`   Health: http://localhost:${PORT}/api/health\n`);
    });
}

module.exports = app;

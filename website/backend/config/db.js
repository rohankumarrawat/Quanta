const mongoose = require("mongoose");

// In-memory fallback store (used when MongoDB is unavailable)
let _usingInMemory = false;

async function connectDB() {
    const uri = process.env.MONGODB_URI || "mongodb://localhost:27017/quanta_docs";
    try {
        await mongoose.connect(uri, { serverSelectionTimeoutMS: 3000 });
        console.log("✅ MongoDB connected:", uri);
    } catch (err) {
        _usingInMemory = true;
        console.warn("⚠️  MongoDB not available. Using in-memory store.");
        console.warn("   Install MongoDB or set MONGODB_URI to persist data.");
    }
}

module.exports = connectDB;
module.exports.usingInMemory = () => _usingInMemory;

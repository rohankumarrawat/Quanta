const mongoose = require("mongoose");

const blogPostSchema = new mongoose.Schema(
    {
        title: { type: String, required: true },
        preview: { type: String, required: true },
        content: { type: String, default: "" },
        author: { type: String, required: true },
        date: { type: String, required: true },
        readTime: { type: String, default: "5 min read" },
        tags: [{ type: String }],
    },
    { timestamps: true }
);

module.exports = mongoose.model("BlogPost", blogPostSchema);

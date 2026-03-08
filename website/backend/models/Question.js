const mongoose = require("mongoose");

const answerSchema = new mongoose.Schema(
    {
        content: { type: String, required: true },
        username: { type: String, required: true },
    },
    { timestamps: true }
);

const questionSchema = new mongoose.Schema(
    {
        title: { type: String, required: true },
        preview: { type: String, required: true },
        tags: [{ type: String }],
        username: { type: String, required: true },
        votes: { type: Number, default: 0 },
        answers: [answerSchema],
        views: { type: Number, default: 0 },
        isAnswered: { type: Boolean, default: false },
        timePosted: { type: String, default: "just now" },
        content: { type: String },
        status: { type: String, enum: ["pending", "approved", "rejected"], default: "pending" },
    },
    { timestamps: true }
);

module.exports = mongoose.model("Question", questionSchema);

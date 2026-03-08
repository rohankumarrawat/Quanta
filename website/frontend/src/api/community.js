import client from "./client";

export async function getCommunityQuestions(params = {}) {
    const res = await client.get("/community", { params });
    return res.data;
}

export async function createQuestion(question) {
    const res = await client.post("/community", question);
    return res.data;
}

export async function voteQuestion(id) {
    const res = await client.post(`/community/${id}/vote`);
    return res.data;
}

export async function getQuestion(id) {
    const res = await client.get(`/community/${id}`);
    return res.data;
}

export async function addAnswer(id, answer) {
    const res = await client.post(`/community/${id}/answer`, { answer });
    return res.data;
}

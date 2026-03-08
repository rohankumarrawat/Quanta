import client from "./client";

export async function getBlogPosts() {
    const res = await client.get("/blog");
    return res.data;
}

export async function getBlogPost(id) {
    const res = await client.get(`/blog/${id}`);
    return res.data;
}

export async function createBlogPost(post) {
    const res = await client.post("/blog", post);
    return res.data;
}

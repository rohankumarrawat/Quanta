const https = require('https');

const query = `
    query questionData($titleSlug: String!) {
        question(titleSlug: $titleSlug) {
            content
        }
    }
`;

const postData = JSON.stringify({
    query: query,
    variables: { titleSlug: 'validate-binary-search-tree' }
});

const options = {
    hostname: 'leetcode.com',
    port: 443,
    path: '/graphql',
    method: 'POST',
    headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(postData),
        'Referer': 'https://leetcode.com/',
        'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7)'
    }
};

const req = https.request(options, (res) => {
    let data = '';
    res.on('data', (d) => data += d);
    res.on('end', () => {
        try {
            const parsed = JSON.parse(data);
            console.log(parsed.data.question.content.substring(0, 1000));
        } catch (e) {
            console.error(e);
        }
    });
});
req.write(postData);
req.end();

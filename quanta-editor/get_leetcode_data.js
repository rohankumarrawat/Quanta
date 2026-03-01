const { GraphQLClient, gql } = require('graphql-request');

async function main() {
    const endpoint = 'https://leetcode.com/graphql';
    const client = new GraphQLClient(endpoint);

    const query = gql`
        query questionData($titleSlug: String!) {
            question(titleSlug: $titleSlug) {
                exampleTestcases
                content
                metaData
            }
        }
    `;

    const variables = { titleSlug: 'two-sum' };

    try {
        const data = await client.request(query, variables);
        console.log("exampleTestcases:", JSON.stringify(data.question.exampleTestcases));
        console.log("content excerpt (Outputs):");
        const matches = Array.from(data.question.content.matchAll(/Output:<\/strong>\s*(?:<[^>]*>)?([^<\n]+)/gi));
        console.log(matches.map(m => m[1]));
        
        console.log("\nTrying new regex:");
        const newMatches = Array.from(data.question.content.matchAll(/Output(?:<\/strong>)?[:]*\s*(?:<[^>]*>)?([^<\n]+)/gi));
        console.log(newMatches.map(m => m[1]));
        
    } catch (e) {
        console.error(e);
    }
}

main();

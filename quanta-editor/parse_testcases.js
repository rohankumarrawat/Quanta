const fs = require('fs');

function parse(content, expectedOutputsRaw) {
    const rawTests = expectedOutputsRaw.split('\n');
    console.log("Raw inputs:", rawTests);

    // Parse outputs from the HTML content
    // LeetCode HTML: <strong>Output:</strong> 3  (plain text follows strong close tag)
    const outMatches = Array.from(
        content.matchAll(/Output:<\/strong>\s*(?:<[^>]*>)?([^<\n]+)/gi)
    );
    const getExt = (i) => {
        const m = outMatches[i];
        return m ? String(m[1]).trim().replace(/&quot;/g, '"').replace(/&amp;/g, '&') : '?';
    };

    // The issue: LeetCode exampleTestcases are separated by \n, but ONE test case
    // might span MULTIPLE lines if it has multiple inputs!
    // We need to parse the metaData to know how many inputs per test case.
}

const content = `
<p>Given an array of integers <code>nums</code>&nbsp;and an integer <code>target</code>, return <em>indices of the two numbers such that they add up to <code>target</code></em>.</p>

<p>You may assume that each input would have <strong><em>exactly</em> one solution</strong>, and you may not use the <em>same</em> element twice.</p>

<p>You can return the answer in any order.</p>

<p>&nbsp;</p>
<p><strong class="example">Example 1:</strong></p>

<pre>
<strong>Input:</strong> nums = [2,7,11,15], target = 9
<strong>Output:</strong> [0,1]
<strong>Explanation:</strong> Because nums[0] + nums[1] == 9, we return [0, 1].
</pre>

<p><strong class="example">Example 2:</strong></p>

<pre>
<strong>Input:</strong> nums = [3,2,4], target = 6
<strong>Output:</strong> [1,2]
</pre>

<p><strong class="example">Example 3:</strong></p>

<pre>
<strong>Input:</strong> nums = [3,3], target = 6
<strong>Output:</strong> [0,1]
</pre>
`;

parse(content, "[2,7,11,15]\n9\n[3,2,4]\n6\n[3,3]\n6");


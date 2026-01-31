#include "../include/quanta.h"
#include <cctype>


std::vector<Token> tokenize(std::string source) {
    std::vector<Token> tokens;
    int i = 0;

    while (i < source.length()) {
        char c = source[i];
     

        if (isspace(c)) {
            i++;
            continue;
        }

        // --- FIXED STRING LOGIC ---
        if (c == '"') {
            std::string strVal;
            i++; // Skip opening "
            while (i < source.length() && source[i] != '"') {
                strVal += source[i++];
            }
            if (i < source.length()) i++; // Skip closing "
            tokens.push_back({TOK_STRING, strVal});
            continue; // We already incremented i, so continue
        }

        if (isalpha(c)) {
            std::string idStr;
            while (i < source.length() && isalnum(source[i])) {
                idStr += source[i++];
            }
            if (idStr == "print") tokens.push_back({TOK_PRINT, idStr});
            else tokens.push_back({TOK_IDENTIFIER, idStr});
            continue;
        }

        if (isdigit(c)) {
            std::string numStr;
            while (i < source.length() && isdigit(source[i])) {
                numStr += source[i++];
            }
            tokens.push_back({TOK_NUMBER, numStr});
            continue;
        }

        // Symbols (+, -, (, ), etc.)
        tokens.push_back({(int)c, std::string(1, c)});
        i++; // Always move forward
    }

    tokens.push_back({TOK_EOF, ""});
    return tokens;
}
#include "../include/quanta.h"
#include <cctype>
#include <iostream>

#include <cmath>
std::vector<Token> tokenize(std::string source) {
    std::vector<Token> tokens;
    int i = 0;
    int line = 1; // Start counting lines at 1

    while (i < source.length()) {
        char c = source[i];

        // --- 1. MULTI-LINE COMMENTS (''' or """) ---
        if (i + 2 < source.length()) {
            bool tripleSingle = (c == '\'' && source[i+1] == '\'' && source[i+2] == '\'');
            bool tripleDouble = (c == '"' && source[i+1] == '"' && source[i+2] == '"');

            if (tripleSingle || tripleDouble) {
                char quoteType = c; 
                i += 3; // Skip the opening quotes

                // Loop until we find the CLOSING quotes
                while (i + 2 < source.length()) {
                    if (source[i] == quoteType && source[i+1] == quoteType && source[i+2] == quoteType) {
                        i += 3; // Skip the closing quotes
                        break;  
                    }
                    if (source[i] == '\n') line++;
                    i++;
                }
                continue; 
            }
        }

        // --- 2. SINGLE LINE COMMENT (@) ---
        if (c == '@') {
            while (i < source.length() && source[i] != '\n') {
                i++;
            }
            continue;
        }

        // --- 3. WHITESPACE ---
        if (c == '\n') {
            line++;
            i++;
            continue;
        }
        if (isspace(c)) {
            i++;
            continue;
        }
        if (c == '+' && i + 1 < source.length() && source[i + 1] == '+') {
            tokens.push_back({TOK_INC, "++", line});
            i += 2; // Skip both '+'
            continue;
        }

        // 2. Handle --
        if (c == '-' && i + 1 < source.length() && source[i + 1] == '-') {
            tokens.push_back({TOK_DEC, "--", line});
            i += 2; // Skip both '-'
            continue;
        }
        if (c == '.') {
        tokens.push_back({TOK_DOT, ".", line}); // <--- NEW
        i++;
        continue;
    }
    // if (c == '*') {
    //     tokens.push_back({TOK_STAR, "*", line}); // <--- NEW (or use char '*')
    //     i++;
    //     continue;
    // }

        // --- 4. CHAR LITERALS ---
       // --- 4. CHAR LITERALS ---
        if (c == '\'') { 
            i++; // Eat start quote
            
            // 1. SAFETY: Check if file ended abruptly (e.g. just "'")
            if (i >= source.length()) {
                std::cerr << "[Quanta Error] Line " << line << ": Empty char literal at end of file." << std::endl;
                continue; 
            }

            char charVal = source[i++]; // Read the character safely
            
            // 2. CHECK: Is the next char a closing quote?
            if (i < source.length() && source[i] == '\'') {
                i++; // Eat end quote (Normal Case)
            } else {
                // ERROR: We expected "'" but found something else (like 'aa')
                std::cerr << "[Quanta Error] Line " << line << ": Missing closing quote for character literal. "  << std::endl;
                // We do NOT exit. We just don't eat the next char, letting the loop handle it next.
                
            }
         

            tokens.push_back({TOK_CHAR, std::string(1, charVal), line});
            continue;
        }

        
        // --- 5. STRING LITERALS ---
        if (c == '"') {
            std::string strVal;
            i++; 
            while (i < source.length() && source[i] != '"') {
                if (source[i] == '\n') line++;
                strVal += source[i++];
            }
            if (i < source.length()) i++;
            tokens.push_back({TOK_STRING, strVal, line}); 
            continue;
        }

        // --- 6. IDENTIFIERS & KEYWORDS ---
        // UPDATED: Allow starting with Underscore (_) or Letter
        if (isalpha(c) || c == '_') {
            std::string idStr;
            
            // UPDATED: Allow Underscores (_) inside the name
            while (i < source.length() && (isalnum(source[i]) || source[i] == '_')) {
                idStr += source[i++];
            }
            
            int type = TOK_IDENTIFIER; // Default to variable name
            
            // --- CHECK FOR RESERVED WORDS ---
            if (idStr == "print") type = TOK_PRINT;
            else if (idStr == "true") type = TOK_TRUE;
            else if (idStr == "false") type = TOK_FALSE;
            else if (idStr == "bool") type = TOK_BOOL;
            else if (idStr == "char") type = TOK_CHAR;
            else if (idStr == "all") type = TOK_ALL;
            else if (idStr == "string") type = TOK_STRING;
            else if (idStr == "var") type = TOK_VAR;
                   else if (idStr == "if") type = TOK_IF;
            else if (idStr == "elif") type = TOK_ELIF; 
            else if (idStr == "else") type = TOK_ELSE;
            else if (idStr == "loop") type= TOK_LOOP;
            else if (idStr == "return") type = TOK_RETURN;
            else if (idStr == "void") type = TOK_VOID;
            else if (idStr == "import") type = TOK_IMPORT;
            else if (idStr == "len") type = TOK_LEN;
            else if (idStr == "upper") type = TOK_UPPER;
            else if (idStr == "lower") type = TOK_LOWER;
            else if (idStr == "reverse") type = TOK_REVERSE;
            else if (idStr == "isupper") type = TOK_ISUPPER;
            else if (idStr == "islower") type = TOK_ISLOWER;
            else if (idStr == "strip") type = TOK_STRIP;
            else if (idStr == "lstrip") type = TOK_LSTRIP;
            else if (idStr == "rstrip") type = TOK_RSTRIP;
            else if (idStr == "capitalize") type = TOK_CAPITALIZE;
            else if (idStr == "title") type = TOK_TITLE;
            else if (idStr == "isalpha") type = TOK_ISALPHA;
            else if (idStr == "isdigit") type = TOK_ISDIGIT;
            else if (idStr == "isspace") type = TOK_ISSPACE;
            else if (idStr == "isalnum") type = TOK_ISALNUM;
            else if (idStr == "find") type = TOK_FIND;
            else if (idStr == "count") type = TOK_COUNT;
            else if (idStr == "startswith") type = TOK_STARTSWITH;
            else if (idStr == "endswith") type = TOK_ENDSWITH;
            else if (idStr == "replace") type = TOK_REPLACE;
            else if (idStr == "in") type = TOK_IN;
            
            
            // --- DETECT "int" and "intN" ---
            else if (idStr.substr(0, 3) == "int") {
                bool isType = true;
                for (size_t k = 3; k < idStr.length(); k++) {
                    if (!isdigit(idStr[k])) { isType = false; break; }
                }
                if (isType) type = TOK_INT;
            }
            // --- DETECT "float" and "floatN" ---
            else if (idStr.substr(0, 5) == "float") {
                 bool isType = true;
                 for (size_t k = 5; k < idStr.length(); k++) {
                    if (!isdigit(idStr[k])) { isType = false; break; }
                }
                if (isType) type = TOK_FLOAT;
            }

            tokens.push_back({type, idStr, line}); 
            continue;
        }

        // --- 7. NUMBERS ---
        if (isdigit(c)) {
            std::string numStr;
            bool hasDecimal = false;

            while (i < source.length() && (isdigit(source[i]) || source[i] == '.')) {
                if (source[i] == '.') {
                    if (hasDecimal) break;
                    hasDecimal = true;
                }
                numStr += source[i];
                i++;
            }

            // --- ERROR CHECK: Variable starting with digit ---
            // If we finished reading numbers but immediately see a letter or _, it's invalid.
            if (i < source.length() && (isalpha(source[i]) || source[i] == '_')) {
                std::cerr << "\n[Quanta Error] Syntax Error at line " << line << std::endl;
                std::cerr << "  Variable names cannot start with a digit." << std::endl;
                
                // Read the rest of the bad identifier so we don't try to parse it next
                std::string badName = numStr;
                while (i < source.length() && (isalnum(source[i]) || source[i] == '_')) {
                    badName += source[i];
                    i++;
                }
                std::cerr << "  -> Invalid identifier: '" << badName << "'\n" << std::endl;
                
                // Skip generating a token for this error
                continue; 
            }
            
           // --- 3. CHECK LIMITS (Both Int and Float) ---
          // --- 3. CHECK LIMITS (Both Int and Float) ---
            if (hasDecimal) {
                // --- FLOAT 64-BIT CHECK ---
                errno = 0; 
                double val = std::strtod(numStr.c_str(), nullptr);

                if (errno == ERANGE || val == HUGE_VAL || val == -HUGE_VAL) {
                    std::cerr << "\n[Quanta Error] Float Overflow at line " << line << std::endl;
                    std::cerr << "  Value '" << numStr << "' exceeds the 64-bit Float limit (~1.79e+308)." << std::endl;
                    // Do NOT exit. Just skip this token.
                    continue; 
                }
                
                // Valid Float
                tokens.push_back({TOK_FLOAT, numStr, line});
            } 
            else {
                // --- INTEGER 64-BIT CHECK ---
                // Max uint64: 18446744073709551615 (20 digits)
                bool isOverflow = false;

                if (numStr.length() > 20) {
                    isOverflow = true;
                } 
                else if (numStr.length() == 20) {
                    std::string maxLimit = "18446744073709551615";
                    if (numStr > maxLimit) {
                        isOverflow = true;
                    }
                }

                if (isOverflow) {
                    std::cerr << "\n[Quanta Error] Integer Overflow at line " << line << std::endl;
                    std::cerr << "  Value '" << numStr << "' exceeds the 64-bit Integer limit." << std::endl;
                    // Do NOT exit. Just skip this token.
                    continue; 
                }

                // Valid Integer
                tokens.push_back({TOK_NUMBER, numStr, line});
            }
            continue;
      
        }


        // --- 8. SYMBOLS ---
        if (c == '=' && i + 1 < source.length() && source[i+1] == '=') {
            tokens.push_back({TOK_EQ, "==", line});
            i += 2; // Eat both chars
            continue;
        }
        // 1. Not Equal (!=)
        if (c == '!' && i + 1 < source.length() && source[i+1] == '=') {
            tokens.push_back({TOK_NEQ, "!=", line});
            i += 2; 
            continue;
        }

        // 2. Greater Than or Equal (>=)
        if (c == '>' && i + 1 < source.length() && source[i+1] == '=') {
            tokens.push_back({TOK_GEQ, ">=", line});
            i += 2; 
            continue;
        }

        // 3. Less Than or Equal (<=)
        if (c == '<' && i + 1 < source.length() && source[i+1] == '=') {
            tokens.push_back({TOK_LEQ, "<=", line});
            i += 2; 
            continue;
        }
        if (c == '.') {
            tokens.push_back({TOK_DOT, ".", line}); // <--- NEW: Dot Token
            i++;
            continue;
        }


        // Single Character Symbols (including ':' for slice s[a:b])
        if (c == '+' || c == '-' || c == '*' || c == '/' || 
            c == '=' || c == '(' || c == ')' || c == '<' || c == '>' || c == ';' || c == '{' || c == '}'|| c == '%'|| 
        c == ',' || c == '[' || c == ']' || c == ':') {
            
            tokens.push_back({(int)c, std::string(1, c), line});
            i++;
            continue;
        }

        std::cerr << "[Quanta Error] Unknown char '" << c << "' at line " << line << std::endl;
        exit(1);
    }

    tokens.push_back({TOK_EOF, "", line});
    return tokens;
}
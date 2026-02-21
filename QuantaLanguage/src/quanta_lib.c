#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* quanta_upper(const char* str) {
    int len = strlen(str);
    char* res = (char*)malloc(len + 1);
    for(int i = 0; i < len; i++) res[i] = toupper((unsigned char)str[i]);
    res[len] = '\0';
    return res;
}

char* quanta_lower(const char* str) {
    int len = strlen(str);
    char* res = (char*)malloc(len + 1);
    for(int i = 0; i < len; i++) res[i] = tolower((unsigned char)str[i]);
    res[len] = '\0';
    return res;
}

char* quanta_reverse(const char* str) {
    int len = strlen(str);
    char* res = (char*)malloc(len + 1);
    for(int i = 0; i < len; i++) res[i] = str[len - 1 - i];
    res[len] = '\0';
    return res;
}

int quanta_isupper(const char* str) {
    for(int i = 0; str[i] != '\0'; i++) {
        if (isalpha((unsigned char)str[i]) && !isupper((unsigned char)str[i])) return 0;
    }
    return 1;
}

int quanta_islower(const char* str) {
    for(int i = 0; str[i] != '\0'; i++) {
        if (isalpha((unsigned char)str[i]) && !islower((unsigned char)str[i])) return 0;
    }
    return 1;
}

/* --- Strip / trim --- */
static int isspace_c(char c) { return (unsigned char)c <= 127 && isspace((unsigned char)c); }

char* quanta_strip(const char* str) {
    if (!str || !*str) { char* e = (char*)malloc(1); e[0] = '\0'; return e; }
    const char* start = str;
    while (*start && isspace_c(*start)) start++;
    const char* end = start + strlen(start);
    while (end > start && isspace_c(*(end - 1))) end--;
    size_t n = (size_t)(end - start);
    char* res = (char*)malloc(n + 1);
    memcpy(res, start, n);
    res[n] = '\0';
    return res;
}

char* quanta_lstrip(const char* str) {
    if (!str || !*str) { char* e = (char*)malloc(1); e[0] = '\0'; return e; }
    while (*str && isspace_c(*str)) str++;
    size_t n = strlen(str);
    char* res = (char*)malloc(n + 1);
    memcpy(res, str, n + 1);
    return res;
}

char* quanta_rstrip(const char* str) {
    if (!str || !*str) { char* e = (char*)malloc(1); e[0] = '\0'; return e; }
    size_t len = strlen(str);
    const char* end = str + len;
    while (end > str && isspace_c(*(end - 1))) end--;
    size_t n = (size_t)(end - str);
    char* res = (char*)malloc(n + 1);
    memcpy(res, str, n);
    res[n] = '\0';
    return res;
}

char* quanta_capitalize(const char* str) {
    int len = strlen(str);
    char* res = (char*)malloc(len + 1);
    if (len > 0) res[0] = toupper((unsigned char)str[0]);
    for (int i = 1; i < len; i++) res[i] = tolower((unsigned char)str[i]);
    res[len] = '\0';
    return res;
}

char* quanta_title(const char* str) {
    int len = strlen(str);
    char* res = (char*)malloc(len + 1);
    int cap = 1;
    for (int i = 0; i < len; i++) {
        if (isspace_c(str[i])) { cap = 1; res[i] = str[i]; }
        else if (cap) { res[i] = toupper((unsigned char)str[i]); cap = 0; }
        else res[i] = tolower((unsigned char)str[i]);
    }
    res[len] = '\0';
    return res;
}

int quanta_isalpha(const char* str) {
    if (!str || !*str) return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isalpha((unsigned char)str[i])) return 0;
    return 1;
}

int quanta_isdigit(const char* str) {
    if (!str || !*str) return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isdigit((unsigned char)str[i])) return 0;
    return 1;
}

int quanta_isspace(const char* str) {
    if (!str || !*str) return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isspace_c(str[i])) return 0;
    return 1;
}

int quanta_isalnum(const char* str) {
    if (!str || !*str) return 0;
    for (int i = 0; str[i] != '\0'; i++)
        if (!isalnum((unsigned char)str[i])) return 0;
    return 1;
}

/* --- Two-arg: find, count, startswith, endswith --- */
int quanta_find(const char* str, const char* sub) {
    if (!str || !sub || !*sub) return -1;
    char* p = strstr(str, sub);
    return p ? (int)(p - str) : -1;
}

int quanta_count(const char* str, const char* sub) {
    if (!str || !sub || !*sub) return 0;
    int n = 0;
    size_t sublen = strlen(sub);
    const char* p = str;
    while ((p = strstr(p, sub)) != NULL) { n++; p += sublen; }
    return n;
}

int quanta_startswith(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    size_t n = strlen(prefix);
    if (strlen(str) < n) return 0;
    return memcmp(str, prefix, n) == 0 ? 1 : 0;
}

int quanta_endswith(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;
    size_t slen = strlen(str), suflen = strlen(suffix);
    if (slen < suflen) return 0;
    return memcmp(str + slen - suflen, suffix, suflen) == 0 ? 1 : 0;
}

/* --- Three-arg: replace(s, old, new) --- */
char* quanta_replace(const char* str, const char* old, const char* newstr) {
    if (!str) { char* e = (char*)malloc(1); e[0] = '\0'; return e; }
    if (!old || !*old) { char* e = (char*)malloc(strlen(str) + 1); strcpy(e, str); return e; }
    if (!newstr) newstr = "";
    size_t slen = strlen(str), oldlen = strlen(old), newlen = strlen(newstr);
    size_t cap = slen + 1;
    char* res = (char*)malloc(cap);
    size_t out = 0;
    const char* p = str;
    for (;;) {
        char* q = strstr(p, old);
        if (!q) {
            size_t rest = strlen(p);
            if (out + rest + 1 > cap) {
                cap = out + rest + 1;
                res = (char*)realloc(res, cap);
            }
            memcpy(res + out, p, rest + 1);
            out += rest;
            break;
        }
        size_t chunk = (size_t)(q - p);
        if (out + chunk + newlen + 1 > cap) {
            cap = out + chunk + newlen + 1;
            res = (char*)realloc(res, cap);
        }
        memcpy(res + out, p, chunk);
        out += chunk;
        memcpy(res + out, newstr, newlen + 1);
        out += newlen;
        p = q + oldlen;
    }
    return res;
}

/* s[start:end:step] - 0-based, end exclusive. Negative = from end. step < 0 = reverse. */
char* quanta_slice(const char* s, int start, int end, int step) {
    if (!s) { char* e = (char*)malloc(1); e[0] = '\0'; return e; }
    int len = (int)strlen(s);
    if (step == 0) step = 1;
    if (step > 0) {
        /* Forward: normalize negative indices (-1 = last) */
        if (start < 0) start = len + start;
        if (end < 0) end = len + end;
        if (start < 0) start = 0;
        if (start > len) start = len;
        if (end < 0) end = 0;
        if (end > len) end = len;
        if (start >= end) { char* e = (char*)malloc(1); e[0] = '\0'; return e; }
        int n = 0;
        for (int i = start; i < end; i += step) n++;
        char* res = (char*)malloc((size_t)n + 1);
        int j = 0;
        for (int i = start; i < end; i += step) res[j++] = s[i];
        res[j] = '\0';
        return res;
    }
    /* Negative step: reverse. end=-1 means "before 0" (include index 0). */
    if (start < 0) start = len + start;
    if (start < 0) start = 0;
    if (start > len) start = len;
    if (end < 0 && end != -1) end = len + end;
    if (end > len) end = len;
    if (start <= end) { char* e = (char*)malloc(1); e[0] = '\0'; return e; }
    int n = 0;
    for (int i = start; i > end; i += step) n++;
    char* res = (char*)malloc((size_t)n + 1);
    int j = 0;
    for (int i = start; i > end; i += step) res[j++] = s[i];
    res[j] = '\0';
    return res;
}
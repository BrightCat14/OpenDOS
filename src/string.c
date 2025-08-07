#include "string.h"
#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = dest;
    const char* s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    char* d = dest;
    const char* s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

void* memset(void* s, int c, size_t n) {
    if (!s) {
        return NULL;
    }
    
    uint8_t* p = (uint8_t*)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    
    return s;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

void* memchr(const void* s, int c, size_t n) {
    const unsigned char* p = s;
    while (n--) {
        if (*p == (unsigned char)c) return (void*)p;
        p++;
    }
    return NULL;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (n-- && (*d++ = *src++));
    while (n--) *d++ = '\0';
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) d++; 
    while ((*d++ = *src++));
    return dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* d = dest;
    while (*d) d++;
    while (n-- && (*d++ = *src++)); 
    *d = '\0'; 
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return n == (size_t)-1 ? 0 : *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strchr(const char* s, int c) {
    while (*s && *s != (char)c) s++;
    return (*s == (char)c) ? (char*)s : NULL;
}

char* strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (char*)last;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char* h = haystack, *n = needle;
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

static char* strtok_save = NULL;

char* strtok(char* str, const char* delim) {
    if (!str && !(str = strtok_save)) return NULL;
    str += strspn(str, delim); 
    if (!*str) {
        strtok_save = NULL;
        return NULL;
    }
    char* end = str + strcspn(str, delim);
    if (*end) {
        *end = '\0';
        strtok_save = end + 1;
    } else {
        strtok_save = NULL;
    }
    return str;
}

size_t strspn(const char* s, const char* accept) {
    size_t len = 0;
    while (*s && strchr(accept, *s++)) len++;
    return len;
}

size_t strcspn(const char* s, const char* reject) {
    size_t len = 0;
    while (*s && !strchr(reject, *s++)) len++;
    return len;
}

int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        unsigned char c1 = (unsigned char) tolower((unsigned char)*s1);
        unsigned char c2 = (unsigned char) tolower((unsigned char)*s2);

        if (c1 != c2) {
            return c1 - c2;
        }
        s1++;
        s2++;
    }
    return (unsigned char) tolower((unsigned char)*s1) - (unsigned char) tolower((unsigned char)*s2);
}

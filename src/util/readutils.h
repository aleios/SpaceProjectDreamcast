#pragma once
#include <kos.h>

#ifdef CD_BUILD
#define DC_FS_TYPE "cd"
#else
#define DC_FS_TYPE "pc"
#endif

static inline bool path_build(char out[static 1], size_t out_size, const char* fs, const char* root, const char* key, const char* ext) {
    int n = snprintf(out, out_size, "/%s/%s/%s.%s", fs, root, key, ext);
    return (n >= 0) && ((size_t)n < out_size);
}

static inline bool path_build_rd(char out[static 1], size_t out_size, const char* root, const char* key, const char* ext) {
    return path_build(out, out_size, "rd", root, key, ext);
}

static inline bool path_build_cd(char out[static 1], size_t out_size, const char* root, const char* key, const char* ext) {
    return path_build(out, out_size, DC_FS_TYPE, root, key, ext);
}

inline static bool str_valid(const char* str) {
    return strcmp(str, "") != 0;
}

inline static bool readutil_readstr(file_t file, char out[static 1], size_t out_size) {
    uint16_t str_len;
    ssize_t read_bytes = fs_read(file, &str_len, sizeof(str_len));

    if(read_bytes <= -1 || read_bytes != sizeof(str_len)) {
        return false;
    }

    if(str_len == 0) {
        out[0] = '\0';
        return true;
    }

    if(str_len > out_size-1) {
        return false;
    }

    read_bytes = fs_read(file, out, str_len);
    
    if(read_bytes != str_len) {
        return false;
    }

    out[str_len] = '\0';
    return true;
}

// inline static char* readutil_readstr(file_t file) {

//     uint16_t str_len;
//     ssize_t read_bytes = fs_read(file, &str_len, sizeof(str_len));

//     if(read_bytes <= -1 || read_bytes != sizeof(str_len)) {
//         return nullptr;
//     }

//     if(str_len == 0) {
//         return nullptr;
//     }

//     char* buffer = malloc((str_len * sizeof(char)) + 1);

//     if(!buffer) {
//         return nullptr;
//     }

//     read_bytes = fs_read(file, buffer, str_len);

//     if(read_bytes <= -1 || read_bytes < str_len) {
//         free(buffer);
//         return nullptr;
//     }

//     buffer[str_len] = '\0';
//     return buffer;
// }
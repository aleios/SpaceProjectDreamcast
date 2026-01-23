#include "texture.h"
#include <pvrtex/file_dctex.h>
#include "../util/readutils.h"

bool texture_init(texture_t* tex, const char* key) {

    if(!tex) {
        return false;
    }

    char path[256] = { 0 };
    path_build_rd(path, sizeof(path), "sprites", key, "dt");

    file_t tex_file = fs_open(path, O_RDONLY);

    if(!tex_file) {
        return false;
    }

    int fileLen = 0;
    fs_seek(tex_file, 0, SEEK_END);
    fileLen = fs_tell(tex_file);
    fs_seek(tex_file, 0, SEEK_SET);

    if(fileLen <= 0) {
        fs_close(tex_file);
        return false;
    }

    fDtHeader header;
    fs_read(tex_file, &header, sizeof(fDtHeader));

    if(!fDtValidateHeader(&header)) {
        fs_close(tex_file);
        return false;
    }

    tex->width = fDtGetWidth(&header);
    tex->height = fDtGetHeight(&header);
    tex->format = header.pvr_type & 0xFFC00000;

    size_t size_bytes = fDtGetTextureSize(&header);

    // TODO: Sanity check size_bytes.
    uint8_t* tex_data = malloc(sizeof(uint8_t) * size_bytes);
    ssize_t bytes_read = fs_read(tex_file, tex_data, sizeof(uint8_t) * size_bytes);

    if(bytes_read <= 0) {
        free(tex_data);
        fs_close(tex_file);
        return false;
    }

    tex->data = pvr_mem_malloc(size_bytes);
    pvr_txr_load(tex_data, tex->data, size_bytes);

    free(tex_data);
    fs_close(tex_file);

    return true;
}

void texture_destroy(texture_t* tex) {
    pvr_mem_free(tex->data);
    tex->data = nullptr;
    tex->width = 0;
    tex->height = 0;
}
#include <string.h>
#include <strings.h>
#include "mime.h"

typedef struct {
    const char *ext;
    const char *mime_type;
} mime_map;

static const mime_map mime_types[] = {
    { ".html", "text/html; charset=UTF-8" },
    { ".css",  "text/css; charset=UTF-8" },
    { ".js",   "application/javascript; charset=UTF-8" },
    { ".png",  "image/png" },
    { ".jpg",  "image/jpeg" }
};

const char* mime(const char *filepath) {
    const char *ext = strrchr(filepath, '.');
    if (!ext){
                return "application/octet-stream";}

    size_t num_mappings = sizeof(mime_types) / sizeof(mime_types[0]);
    for (size_t i = 0; i < num_mappings; i++) {
        if (strcasecmp(ext, mime_types[i].ext) == 0) {
            return mime_types[i].mime_type;
        }
    }
    return "text/plain; charset=UTF-8";
}

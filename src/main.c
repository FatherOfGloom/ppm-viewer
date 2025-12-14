#include <stdio.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <io.h>

#define panic(...) do { fprintf(stderr, __VA_ARGS__); exit(0); } while (0)
#define expect(ptr, ...) __extension__({void* _ptr = (ptr); if (!_ptr) { panic(__VA_ARGS__); } _ptr;})
#define nonnull(ptr) __extension__ ({ void* _ptr = (ptr); if (!_ptr) { panic("f:%s l:%d ERR: %s\n", __FILE__, __LINE__, "unwrap on a null value."); } _ptr; })
#define todo() panic("panic at f:'%s' l:%d todo!", __FILE__, __LINE__)

typedef struct Slice {
    void* ptr;
    size_t len;
} Slice;

Slice slice_cstr_copy(char* cstr, size_t len) {
    return (Slice){.ptr = cstr, .len = len};
}

#define file_exists(file_path) (access(file_path, F_OK) == 0)

typedef FILE file_t;
typedef int errno_t;

typedef struct File {
    file_t* handle;
} File;

errno_t file_size(File* file, size_t* size) {
    long saved = ftell(file->handle);
    if (saved < 0) return errno;
    if (fseek(file->handle, 0, SEEK_END) < 0) return errno;
    long result = ftell(file->handle);
    if (result < 0) return errno;
    if (fseek(file->handle, saved, SEEK_SET) < 0) return errno;
    *size = (size_t)result;
    return 0;
}

File file_open(const char* __restrict__ file_name, const char* __restrict__ mode) {
    return (File){.handle = nonnull(fopen(file_name, mode))};
}

errno_t file_close(File* file) {
    if (file->handle == NULL) return 0;
    errno_t result = fclose(file->handle);
    file->handle = NULL;
    return result;
}

errno_t file_read_exact(File* file, Slice* buffer) {
    assert(buffer->ptr);
    assert(buffer->len);

    fread(buffer->ptr, buffer->len, 1, file->handle);

    if (ferror(file->handle)) return errno;

    return 0;
}

typedef struct PpmParseResult {
    Slice buffer;
    int w;
    int h;
    int ok;
} PpmParseResult;

typedef struct PpmReader {
    size_t cursor;
    Slice* buffer;
} PpmReader;

typedef enum {
    PPM_TOKEN_KIND_NONE = 0,
    PPM_TOKEN_KIND_HEADER_LINE,
    PPM_TOKEN_KIND_COMMENT,
} PpmTokenKind;

typedef struct PpmToken {
    Slice slice;
    PpmTokenKind kind;
} PpmToken;

PpmToken ppm_reader_next(PpmReader* r) {
    PpmToken token = {0};
    char c = *((char*)r->buffer->ptr + r->cursor);
    token.slice.ptr = (char*)r->buffer->ptr + r->cursor;

    if (r->cursor >= r->buffer->len) return token;

    if (c == '#') {
        token.kind = PPM_TOKEN_KIND_COMMENT;
    } else {
        token.kind = PPM_TOKEN_KIND_HEADER_LINE;
    }

    while (r->cursor < r->buffer->len && c != '\n') {
        r->cursor += 1;
        c = *((char*)r->buffer->ptr + r->cursor);
    }

    if (r->cursor < r->buffer->len) {
        r->cursor += 1;
    }

    token.slice.len = (char*)r->buffer->ptr + r->cursor - (char*)token.slice.ptr; 
    return token;
}

PpmParseResult parse_ppm(Slice* file_path) {
    PpmParseResult result = {0};
    Slice buffer = {0};
    File f = file_open(file_path->ptr, "rb");
    {
        size_t size;
        assert(!file_size(&f, &size));
        if (size == 0) {
            goto defer;  
        }

        buffer.len = size;
        buffer.ptr = expect(malloc(size), "No ram, bro.");
        assert(!file_read_exact(&f, &buffer));
    }

    // Parse header
    int img_w, img_h = 0;
    PpmToken token = {0};
    PpmReader ppm_reader = {.buffer = &buffer, .cursor = 0};
    int line = 0;

    do {
        token = ppm_reader_next(&ppm_reader);
        switch (token.kind) {
            case PPM_TOKEN_KIND_COMMENT:
                continue;
            case PPM_TOKEN_KIND_HEADER_LINE: {
                Slice text = token.slice;
                // TODO check for other PPM specifications
                if (line == 1) {
                    (void)sscanf((char*)text.ptr, "%d %d", &img_w, &img_h);
                }

                line += 1;
                break;
            }
            default:
                break;
        }
    } while (token.kind != PPM_TOKEN_KIND_NONE && line < 3);

    if (img_w > 0 && img_h > 0) {
        result.buffer.ptr = (char*)ppm_reader.buffer->ptr + ppm_reader.cursor;
        result.buffer.len = img_w * img_h * 3;
        result.w = img_w;
        result.h = img_h;
        result.ok = 1;
    }

defer:
    file_close(&f);
    return result;
}

const char* usage = "Usage: ppmviewer [file-path]\n";

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 700

// TODO: redraw while resize
// TODO: display filename on title bar
// TODO: render custom titlebar
int main(int argc, char* argv[]) {
    if (argc != 2) {
        panic(usage);
    }

    Slice file_path = slice_cstr_copy(argv[1], strlen(argv[1]));

    if (!file_exists(file_path.ptr)) {
        panic("ppmviewer: unable to find file at path '%s'", (char*)file_path.ptr);
    }

    assert(!SDL_Init(SDL_INIT_VIDEO));
    
    SDL_Window* window = expect(SDL_CreateWindow(
        "ppmviewer", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        SDL_WINDOW_RESIZABLE
    ), "Unable to create sdl window: %s", SDL_GetError());

    SDL_Renderer* renderer = expect(
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED), 
        "Unable to create sdl renderer: %s", 
        SDL_GetError()
    );

    PpmParseResult parse_result = parse_ppm(&file_path);
    SDL_Texture* texture = NULL;
    SDL_Rect img_rect = {0};
    SDL_Rect target_rect = {0};
    target_rect.w = WINDOW_WIDTH; 
    target_rect.h = WINDOW_HEIGHT;

    if (parse_result.ok) {
        SDL_Surface* surface =
            expect(SDL_CreateRGBSurfaceFrom(
                parse_result.buffer.ptr, 
                parse_result.w,
                parse_result.h, 
                24, 
                parse_result.w * 3,
                0x0000FF, 
                0x00FF00, 
                0xFF0000, 
                0
        ), "Unable to create SDL surface from buffer: %s", SDL_GetError());

        texture = nonnull(SDL_CreateTextureFromSurface(renderer, surface));

        img_rect.w = surface->w;
        img_rect.h = surface->h;
    }
    
    int quit = 0;
    int need_redraw = 1;

    while (!quit) {
        SDL_Event e = {0};
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    quit = 1;
                    break;
                }
                case SDL_WINDOWEVENT: {
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                        target_rect.w = e.window.data1;
                        target_rect.h = e.window.data2;
                        need_redraw = 1;
                    }
                    break;
                }
            }
        }

        // TODO: redraw on resize event
        if (need_redraw) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            if (texture) (void)SDL_RenderCopy(renderer, texture, &img_rect, &target_rect);
            SDL_RenderPresent(renderer);
        }

        need_redraw = 0;
        SDL_Delay(1);
    }

    SDL_Quit();

    return 0;
}
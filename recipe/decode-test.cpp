#include <cstdio>
#include <libde265/de265.h>

int main() {
    unsigned char input[65536];
    FILE *file = std::fopen("black.hevc", "rb");
    if (!file) return 1;
    size_t length = std::fread(input, 1, sizeof(input), file);
    std::fclose(file);
    de265_decoder_context *ctx = de265_new_decoder();
    if (!ctx) return 2;
    if (de265_push_data(ctx, input, static_cast<int>(length), 0, NULL) != DE265_OK
        || de265_flush_data(ctx) != DE265_OK) return 3;
    int more = 1, pictures = 0;
    for (int step = 0; more && step < 10000; ++step) {
        de265_error error = de265_decode(ctx, &more);
        if (error != DE265_OK && error != DE265_ERROR_IMAGE_BUFFER_FULL) return 4;
        const de265_image *image;
        while ((image = de265_peek_next_picture(ctx)) != NULL) {
            for (int channel = 0; channel < 3; ++channel) {
                int size = channel == 0 ? 64 : 32;
                int expected = channel == 0 ? 16 : 128;
                if (de265_get_image_width(image, channel) != size ||
                    de265_get_image_height(image, channel) != size) return 5;
                int stride = 0;
                const unsigned char *plane = de265_get_image_plane(image, channel, &stride);
                if (!plane) return 6;
                for (int y = 0; y < size; ++y)
                    for (int x = 0; x < size; ++x)
                        if (plane[y*stride+x] != expected) return 7;
            }
            ++pictures;
            de265_release_next_picture(ctx);
        }
    }
    de265_free_decoder(ctx);
    if (more || pictures != 1) return 8;
    std::puts("Native HEVC decode passed: one 64x64 frame, all YUV pixels exact");
    return 0;
}

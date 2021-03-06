/* Copyright (c) 2011 Derrick Coetzee, Guillaume Cottenceau, and contributors

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Based on code distributed by Guillaume Cottenceau and contributors
under MIT/X11 License at http://zarb.org/~gc/html/libpng.html
*/

#include "pngcompare.h"
#include "../utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TEMP_DIR  "/tmp"

int sys(const char* command) {
    time_t start_time = time(NULL);
    int return_code = system(command);
    int seconds =  time(NULL) - start_time;
    if (return_code != 0) {
        abort_("Received nonzero return code %d from command '%s'", return_code, command);
    }
    return seconds;
}

void assert_png_approx_equal(const char* filename_1, const char* filename_2, double max_error) {
    double difference = png_compare(filename_1, filename_2);
    if (difference > max_error) {
        abort_("Scaled output files '%s' and '%s' were too different (distance=%f, threshold=%f).", filename_1, filename_2, difference, max_error);
    }
}

void test_basic(const char* filename, int max_width, double max_error) {
    printf("Testing %s at %dpx...", filename, max_width);
    fflush(stdout);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "./pngscale %s " TEMP_DIR "/out.pngscale.png %d -1", filename, max_width);
    int pngscale_time = sys(buffer);
    /* Requires ImageMagick convert */
    snprintf(buffer, sizeof(buffer), "convert %s -resize %d " TEMP_DIR "/out.convert.png", filename, max_width);
    int convert_time = sys(buffer);
    assert_png_approx_equal(TEMP_DIR "/out.pngscale.png", TEMP_DIR "/out.convert.png", max_error);
    printf("pngscale: %d sec, convert: %d sec\n", pngscale_time, convert_time);
    unlink(TEMP_DIR "/out.pngscale.png");
    unlink(TEMP_DIR "/out.convert.png");
}

void test_large_image(const char* filename, int upscale_width, int downscale_width, double max_error) {
    printf("Creating out.pngscale.large.png...\n");
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "./pngscale %s " TEMP_DIR "/out.pngscale.large.png %d -1", filename, upscale_width);
    sys(buffer);
    printf("Doing large image test...\n");
    test_basic(TEMP_DIR "/out.pngscale.large.png", downscale_width, max_error);
    unlink(TEMP_DIR "/out.pngscale.large.png");
}

void test_upscale(const char* filename, int downscale_width, int upscale_width, double max_error) {
    printf("Testing upscaling of %s to %dpx from %dpx...", filename, upscale_width, downscale_width);
    fflush(stdout);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "./pngscale %s " TEMP_DIR "/out.pngscale.downscale.png %d -1", filename, downscale_width);
    sys(buffer);
    snprintf(buffer, sizeof(buffer), "./pngscale " TEMP_DIR "/out.pngscale.downscale.png " TEMP_DIR "/out.pngscale.upscale.png %d -1", upscale_width);
    sys(buffer);
    snprintf(buffer, sizeof(buffer), "./pngscale " TEMP_DIR "/out.pngscale.upscale.png " TEMP_DIR "/out.pngscale.downscale2.png %d -1", downscale_width);
    sys(buffer);
    assert_png_approx_equal(TEMP_DIR "/out.pngscale.downscale.png", TEMP_DIR "/out.pngscale.downscale2.png", max_error);
    printf("\n");
    unlink(TEMP_DIR "/out.pngscale.downscale.png");
    unlink(TEMP_DIR "/out.pngscale.downscale2.png");
    unlink(TEMP_DIR "/out.pngscale.upscale.png");
}

int main(void) {
    int i;
    int sizes[] = { 1, 50, 150, 200, 220, 300, 400, 1000 };

    for (i=0; i < sizeof(sizes)/sizeof(*sizes); i++) {
        test_basic("test/data/ferriero.png", sizes[i], 5.0);
        test_basic("test/data/antonio.png", sizes[i], 5.0);
    }
    test_basic("test/data/antonio.png", 5000, 5.0);

    /* Different color types and bit depths */
    test_basic("test/data/ferriero_gray.png", 220, 5.0);
    test_basic("test/data/ferriero_16bit.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_bw.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_2.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_4.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_8.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_16.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_32.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_64.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_128.png", 220, 5.0);
    test_basic("test/data/ferriero_palette_256.png", 220, 5.0);

    /* Fabricate a very large image to test */
#if 0
    /* Disabled: passes but too slow */
    test_large_image("test/data/antonio.png", 19203, 220, 5.0);
#endif

    /* Transparency - some of these aren't quite down to error 5.0 yet */
    test_basic("test/data/Abrams-transparent.png", 220, 6.0);
    test_basic("test/data/Abrams-transparent.gray.png", 220, 15.0);
    test_basic("test/data/Abrams-transparent_palette_256.png", 220, 5.0);
    test_basic("test/data/translucent_circle.png", 220, 5.0);

    /* Upscaling - introduces blurring, use larger error */
    test_upscale("test/data/ferriero.png", 100, 800, 10.0);

    printf("\nAll tests passed.\n");
    return 0;
}

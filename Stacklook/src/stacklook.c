/*
* This file is for integration with KernelShark.
* Inner logic is in file 'Stacklook.cpp'
*/

#include <stdc-predef.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"

// Plugin header
#include "stacklook.h"

// Just familiarizing myself, taken from the talk video

#define FONT_SIZE 30
const char* font_file = "/usr/share/fonts/truetype/FreeSans.ttf";
struct ksplot_font font;

/*Static hello world*/
static struct ksplot_font* get_font() {
    if (!ksplot_font_is_loaded(&font)) {
        ksplot_init_font(&font, FONT_SIZE, font_file);
    }

    return &font;
}

/** Callback **/
static void hello_world(struct kshark_cpp_argv* argv_c, int sd, int val, int draw_action) {
    struct ksplot_color text_col = {
        (uint8_t)128,
        (uint8_t)23,
        (uint8_t)168,
    };
    ksplot_print_text(get_font(), &text_col, 300, 100, "Hello, World!");
}

/** Load the plugin **/
int KSHARK_PLOT_PLUGIN_INITIALIZER(struct kshark_data_stream* stream) {
    kshark_register_draw_handler(stream, hello_world);
    return 1;
}

/** Unload the plugin **/
int KSHARK_PLOT_PLUGIN_DEINITIALIZER(struct kshark_data_stream* stream) {
    kshark_unregister_draw_handler(stream, hello_world);
    return 1;
}
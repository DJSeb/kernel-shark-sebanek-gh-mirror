#include <string>

#include "stacklook.hpp"
#include "StacklookDetailedStack.cpp"

// Just familiarizing myself, taken from the talk video

constexpr float FONT_SIZE = 30.0f;
const char* font_file = "/usr/share/fonts/truetype/FreeSans.ttf";
ksplot_font font;

/*Static hello world*/
static ksplot_font* get_font() {
    if (!ksplot_font_is_loaded(&font)) {
        ksplot_init_font(&font, FONT_SIZE, font_file);
    }

    return &font;
}

static void hello_world(kshark_cpp_argv* argv_c, int sd, int val, int draw_action) {
    ksplot_color text_col;
    text_col.red = 128; text_col.green = 23; text_col.blue = 168;
    ksplot_print_text(get_font(), &text_col, 300, 100, "Hello, World!");
}

int KSHARK_PLOT_PLUGIN_INITIALIZER(struct kshark_data_stream* stream) {
    kshark_register_draw_handler(stream, hello_world);
    return 1;
}
int KSHARK_PLOT_PLUGIN_DEINITIALIZER(struct kshark_data_stream* stream) {
    kshark_unregister_draw_handler(stream, hello_world);
    return 1;
}
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

/**
 * Don't mind me stealing text from public footage on the talk for future work :)
 * For me: Do look at the footage again to check the loot.
*/
// /** COPYRIGHT & so on... */
// // C
// #include <stdio.h>
// // KernelShark examples
// #include "priority_plot.h"
// 
// struct ksplot_font font;
// char *font_file = NULL;
// int ss_event_id;
// 
// struct ksplot_font *get_font_ptr() {
//  if (!ksplot_font_is_loaded(&font))
//      ksplot_init_font(&font, FONT_SIZE, font_file);
//  return &font;
// }
// /** Plugin data instance. */
// static struct kshark_data_container *plugin_data[KS_MAX_NUM_STREAMS] = (NULL);
// /** Get the per Data stream plugin data. */
// struct kshark_data_container *get_plugin_data(int sd)
// {
//  return plugin_data[sd];
// }
// 
// static void plugin_process(struct kshark_data_stream *stream,
// {
//  int64_t next_prio;
//  void *rec, struct kshark_entry *entry)
//  if (kshark_read_record_field(stream, rec, "next_prio", &next_prio) == 0)
//        kshark_data_container_append(plugin_data[stream->stream_id], entry, next_prio);
// }
// int KSHARK_PLOT_PLUGIN_INITIALIZER (struct kshark_data_stream *stream)
// {
// ss_event_id=kshark_find_event_id(stream, "sched/sched_switch");
// if (!font_file)
// font_file = ksplot_find_font_file("FreeSans", "FreeSans");
// if (!font_file)
// return 0;
// plugin_data[stream->stream_id] = kshark_init_data_container();
// kshark_register_event_handler(stream, ss_event_id, plugin_process); kshark_register_draw_handler(stream, draw_priority);
// return 1;
// }
// /** Unload this plugin. */
// int KSHARK PLOT_PLUGIN_DEINITIALIZER (struct kshark_data_stream *stream)
// kshark_unregister_event_handler(stream, ss_event_id, plugin_process); kshark_unregister_draw_handler (stream, draw_priority);
// kshark_free_data_container (plugin_data[stream->stream_id]); plugin_data[stream->stream_id] = NULL;
// return 1;
// }
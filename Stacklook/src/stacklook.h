#ifndef _KS_PLUGIN_STACKLOOK_H
#define _KS_PLUGIN_STACKLOOK_H

#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FONT_SIZE 30

struct plugin_stacklook_ctx {
    /** Data of stack-related events. **/
    struct kshark_data_container* stacks_data;
};

KS_DECLARE_PLUGIN_CONTEXT_METHODS(struct plugin_stacklook_ctx)


#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _KS_PLUGIN_STACKLOOK_H
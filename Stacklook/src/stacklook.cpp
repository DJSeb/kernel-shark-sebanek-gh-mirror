#include "stacklook.hpp"
#include "StacklookDetailedStack.cpp"

int KSHARK_PLOT_PLUGIN_INITIALIZER(struct kshark_data_stream* stream) {
    
    
    return 1;
}
int KSHARK_PLOT_PLUGIN_DEINITIALIZER(struct kshark_data_stream* stream) {
    return 1;
}
// DO NOT SHIP WITH THIS!!!

#include <string>
#include <iostream>
 
static uint32_t log_counter = 0;

void log(const std::string& to_log, bool with_newline = true) {
    std::cout << "  [LOG #" << log_counter << "] " << to_log;
    if (with_newline)
        std::cout << std::endl;
    ++log_counter;
}

template<typename OtherInfoT>
void log(const std::string& to_log, OtherInfoT other, bool with_newline = true) {
    std::cout << "  [LOG #" << log_counter << "] " << to_log;
    std::cout << other;
    if (with_newline)
        std::cout << std::endl;
    ++log_counter;
}

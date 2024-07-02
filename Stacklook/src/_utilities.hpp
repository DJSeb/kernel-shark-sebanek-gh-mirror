// DO NOT SHIP WITH THIS!!!

#include <string>
#include <iostream>

void log(const std::string& to_log, bool with_newline = true) {
    std::cout << "  [LOG] " << to_log;
    if (with_newline)
        std::cout << std::endl;
}

template<typename OtherInfoT>
void log(const std::string& to_log, OtherInfoT other, bool with_newline = true) {
    std::cout << "  [LOG] " << to_log;
    std::cout << other;
    if (with_newline)
        std::cout << std::endl;
}

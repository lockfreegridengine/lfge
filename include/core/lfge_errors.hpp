#include <string>

enum class lfge_errors : uint8_t {
    service_not_found = 1,
};

std::string to_string(lfge_errors x);

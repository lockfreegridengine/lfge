#include "core/lfge_errors.hpp"

std::string to_string(lfge_errors x)
{
    switch (x) {
        case lfge_errors::service_not_found:
        return "service_not_found";
        default:
        return "-unknown-error-";
    }
}
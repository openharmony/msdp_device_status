#ifndef PARSE_MSDP_INT_H
#define PARSE_MSDP_INT_H

#include <cctype>
#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
inline bool ParseMsdpInt32(const std::string &text, int32_t &out)
{
    if (text.empty()) {
        return false;
    }
    const char *first = text.data();
    const char *last = first + text.size();
    while (first < last && std::isspace(static_cast<unsigned char>(*first))) {
        ++first;
    }
    while (last > first && std::isspace(static_cast<unsigned char>(last[-1]))) {
        --last;
    }
    if (first == last) {
        return false;
    }
    auto [ptr, ec] = std::from_chars(first, last, out);
    return ec == std::errc{} && ptr == last;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif

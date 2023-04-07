
#ifndef HALO2_LOGGER_HPP
#define HALO2_LOGGER_HPP

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

namespace halo2 {
    namespace base {
        using Logger = std::shared_ptr<spdlog::logger>;

        /**
        * Provide logger object
        * @param tag - tagging name for identifying logger
        * @return logger object
        */
        Logger createLogger(const std::string &tag);
    }  // namespace base
}  // namespace halo2

#endif  // HALO2_LOGGER_HPP

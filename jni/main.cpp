#include "zygisk.hpp"
#include "module.hpp"
#include "logging.hpp"

// Create and register the module
static fpbypass::FingerprintBypassModule module;

ZYGISK_EXPORT int zygisk_module_entry(void* handle, void* args) {
    zygisk::registerModule(&module);
    return zygisk::entry(handle, args);
}

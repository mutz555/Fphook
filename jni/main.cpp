#include "zygisk.hpp"
#include "module.hpp"
#include "Logger.hpp"

// Create and register the module
static fpbypass::FingerprintBypassModule module;

ZYGISK_EXPORT int zygisk_module_entry(void* handle, void* args) {
    zygisk::registerModule(&module);
    return zygisk::entry(handle, args);
}

#include "loader.h"
#ifndef _WIN32
#include <dlfcn.h>
#endif

namespace Backend {
    LibHandle RyLoader::open(const std::string& path) {
#ifdef _WIN32
        return LoadLibraryA(path.c_str());
#else
        return dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
    }

    void* RyLoader::getSymbol(LibHandle handle, const std::string& symbol) {
#ifdef _WIN32
        return (void*)GetProcAddress(handle, symbol.c_str());
#else
        return dlsym(handle, symbol.c_str());
#endif
    }

    std::string RyLoader::getError() {
#ifdef _WIN32
        return "Windows Library Error (Code " + std::to_string(GetLastError()) + ")";
#else
        const char* err = dlerror();
        return err ? err : "Unknown error";
#endif
    }
}
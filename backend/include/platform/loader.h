#pragma once
#include <string>

#ifdef _WIN32
#include <windows.h>
typedef HMODULE LibHandle;
#else
typedef void* LibHandle;
#endif

namespace Backend {
    class RyLoader {
    public:
        static LibHandle open(const std::string& path);
        static void* getSymbol(LibHandle handle, const std::string& symbol);
        static std::string getError();
    };
}
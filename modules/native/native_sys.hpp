#include <iostream>
#include "colors.h"
#include "common.h"

namespace RyRuntime {
	inline RyValue ry_exit(int argCount, RyValue *args) {
		int exitCode = args->asNumber();
		std::cout << RyColor::BOLD << RyColor::YELLOW << "[Ry] Exited Successfully with exit code: " << exitCode
							<< RyColor::RESET << std::endl;
		exit(0);
	}


	// Native 'clock()' - Useful for benchmarking Ry
	inline RyValue ry_clock(int argCount, RyValue *args) { return RyValue((double) clock() / CLOCKS_PER_SEC); }

	// Native 'clear()' - Useful for clearing output
	inline RyValue ry_clear(int argCount, RyValue *args) {
#ifdef _WIN32
		// Windows specific clear
		std::system("cls");
#else
		// Linux/macOS standard clear
		std::system("clear");
#endif
		return nullptr;
	};
} // namespace Frontend

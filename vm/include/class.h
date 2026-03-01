#include <memory>
#include "unordered_map"
#include "vm.h"

namespace Frontend {
	struct ClassCompiler {
		std::shared_ptr<ClassCompiler> enclosing;
		bool hasSuperclass = false;
	};

	class RyClass {
	public:
		std::string name;
		std::shared_ptr<RyClass> superclass = nullptr;
		std::unordered_map<std::string, std::shared_ptr<RyRuntime::RyClosure>> methods;
		RyClass(std::string n) : name(n) {}
	};

	class RyInstance {
	public:
		std::shared_ptr<RyClass> klass;
		std::unordered_map<std::string, RyValue> fields;
		RyInstance(std::shared_ptr<RyClass> k) : klass(k) {}
	};

	class RyBoundMethod {
	public:
		RyValue receiver;
		std::shared_ptr<RyRuntime::RyClosure> method;
		RyBoundMethod(RyValue r, std::shared_ptr<RyRuntime::RyClosure> m) : receiver(r), method(m) {}
	};
} // namespace Frontend

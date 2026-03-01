#ifndef RY_LIB_DK_H
#define RY_LIB_DK_H

typedef enum { RY_NIL, RY_BOOL, RY_NUMBER, RY_STRING, RY_OBJECT } RyType;

struct RyValue {
	RyType type;
	union {
		double number;
		bool boolean;
		void *ptr;
	} data;
};

typedef RyValue (*RyNativeFn)(int argCount, RyValue *args);
typedef void (*RyRegisterFn)(const char *name, RyNativeFn fn, int arity, void *target);

#endif

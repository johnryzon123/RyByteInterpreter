#ifndef ry_chunk_h
#define ry_chunk_h

#include "common.h"

namespace RyRuntime {

	// The Opcodes: The instructions for Ry
	enum OpCode {
		// Literals
		OP_CONSTANT,
		OP_NULL,
		OP_TRUE,
		OP_FALSE,
		OP_POP,

		// Variables & Scopes
		OP_DEFINE_GLOBAL,
		OP_GET_GLOBAL,
		OP_SET_GLOBAL,
		OP_GET_LOCAL,
		OP_SET_LOCAL,
		OP_GET_PROPERTY,
		OP_SET_PROPERTY,

		// Math
		OP_ADD,
		OP_SUBTRACT,
		OP_MULTIPLY,
		OP_DIVIDE,
		OP_MODULO,
		OP_NEGATE,
		OP_GROUPING,
		OP_CLOSE_GROUPING,
		OP_BUILD_RANGE_LIST,
		OP_BUILD_LIST,



		// Comparison
		OP_EQUAL,
		OP_GREATER,
		OP_LESS,
		OP_NOT,

		// Control Flow
		OP_JUMP, // Forwards (if/else)
		OP_JUMP_IF_FALSE,
		OP_LOOP, // Backwards (while/for/until)
		OP_FOR_EACH_NEXT,

		// Ry Specifics
		OP_CALL,
		OP_CLASS,
		OP_INHERIT,
		OP_PANIC,
		OP_RETURN,
		OP_FUNCTION,
		OP_ATTEMPT,
		OP_END_ATTEMPT,
	};

	// The Chunk: A sequence of bytecode
	struct Chunk {
		std::vector<uint8_t> code; // The Instructions
		std::vector<RyValue> constants; // For numbers/strings

		// For error reporting
		std::vector<int> lines;
		std::vector<int> columns; 

		void write(uint8_t byte, int line, int column) {
			code.push_back(byte);
			lines.push_back(line);
			columns.push_back(column);
		}

		// Returns the index of the constant in the pool
		int addConstant(RyValue value) {
			constants.push_back(value);
			return constants.size() - 1;
		}
	};
} // namespace RyRuntime

#endif

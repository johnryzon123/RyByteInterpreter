#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "chunk.h"
#include "colors.h"
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "tools.h"
#include "vm.h"
#include "func.h"


using namespace RyRuntime;

void interpret(VM &vm, const std::string &source) {
	// Lexing
	Backend::Lexer lexer(source);
	std::vector<Backend::Token> tokens = lexer.scanTokens();

	// Setup Aliases & Parsing
	std::set<std::string> aliases; // Temporary set for the parser
	Backend::Parser parser(tokens, aliases, source);

	std::vector<std::shared_ptr<Backend::Stmt>> statements = parser.parse();

	if (RyTools::hadError)
		return;

	//  Compiling
	Compiler compiler;
	Chunk chunk;
	if (!compiler.compile(statements, &chunk)) {
		return;
	}

	auto function = std::make_shared<Frontend::RyFunction>(std::move(chunk), "<main>", 0);

	// Running
	vm.interpret(function);
}

void runREPL(VM &vm) {
	std::string line;
	std::string buffer;
	int indentLevel = 0;
	std::cout << RyColor::BOLD << "Ry (Ry's for You) REPL - Bytecode Edition\n" << RyColor::RESET;

	while (true) {
		if (buffer.empty()) {
			std::cout << RyColor::BLUE << "ry> " << RyColor::RESET;
		} else {
			std::cout << std::string(indentLevel * 4, '.') << " ";
		}

		if (!std::getline(std::cin, line))
			break;

		// Reset logic
		if (line == "exit")
			break;
		if (line == "clear") {
			system("clear");
			buffer.clear();
			indentLevel = 0;
			continue;
		}

		int change = RyTools::countIndentation(line);
		indentLevel += change;
		buffer += line + "\n";

		if (indentLevel <= 0 && !buffer.empty() && buffer != "\n") {
			interpret(vm, buffer);
			buffer.clear();
			indentLevel = 0;
		}
	}
}

int main(int argc, char *argv[]) {
	VM vm;

	if (argc >= 2) {
		std::string command = argv[1];

		if (command == "run" && argc == 3) {
			std::ifstream inputFile(argv[2]);
			if (!inputFile.is_open()) {
				std::cerr << "Could not open file: " << argv[2] << "\n";
				return 1;
			}
			std::string src((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
			interpret(vm, src);
		} else if (command == "-v" || command == "--version") {
			std::cout << "Ry version 0.2.0 (Bytecode RVM)\n";
		} else {
		}
	} else {
		runREPL(vm);
	}

	return 0;
}

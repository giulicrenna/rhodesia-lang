/**
 * @file main.cpp
 * @brief Rhodesia language interpreter entry point
 * @author Giuliano Crenna
 * 
 * Supports:
 * - REPL mode (interactive)
 * - File execution mode
 * - Code passed via command line
 */

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Evaluator.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <filesystem>

using namespace Rhodesia;

/**
 * @brief Print usage information
 */
void printUsage(const char* program) {
    std::cout << "Rhodesia Language Interpreter v0.1.0\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << program << "              Start REPL (interactive mode)\n";
    std::cout << "  " << program << " <file.rho>   Execute a Rhodesia source file\n";
    std::cout << "  " << program << " -e \"code\"    Execute code from command line\n";
    std::cout << "  " << program << " -h           Show this help\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program << " examples/regression.rho\n";
    std::cout << "  " << program << " -e \"println(2 + 2)\"\n";
}

/**
 * @brief Read entire file content
 */
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * @brief Execute Rhodesia source code
 */
int execute(const std::string& source, Evaluator& evaluator, bool showResult = false) {
    try {
        // Lexical analysis
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        // Parsing
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        // Evaluation
        RhoValue result = evaluator.evaluate(*program);
        
        if (showResult && !std::holds_alternative<int64_t>(result)) {
            std::cout << valueToString(result) << std::endl;
        }
        
        return 0;
    }
    catch (const RhoError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Error: Memory allocation failed. The operation required more memory than available." << std::endl;
        std::cerr << "Suggestion: Try using smaller data sizes or check system memory usage." << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Internal error: " << e.what() << std::endl;
        return 2;
    }
}

/**
 * @brief Run interactive REPL
 */
int runRepl() {
    std::cout << "Rhodesia REPL v0.1.0 (type 'exit' or Ctrl+D to quit)\n";
    std::cout << "Use 'help' for available commands.\n\n";

    Evaluator evaluator;
    // Set base directory to current working directory for REPL
    evaluator.moduleLoader().setBaseDirectory(std::filesystem::current_path().string());

    std::string line;
    std::string multiline;
    int braceCount = 0;
    
    while (true) {
        // Prompt
        if (braceCount > 0) {
            std::cout << "... ";
        } else {
            std::cout << ">>> ";
        }
        
        if (!std::getline(std::cin, line)) {
            std::cout << "\nGoodbye!\n";
            break;
        }
        
        // Handle special commands
        if (braceCount == 0) {
            if (line == "exit" || line == "quit") {
                std::cout << "Goodbye!\n";
                break;
            }
            if (line == "help") {
                std::cout << "Commands:\n";
                std::cout << "  exit, quit  - Exit REPL\n";
                std::cout << "  help        - Show this help\n";
                std::cout << "  vars        - List all variables\n";
                std::cout << "\nEnter Rhodesia code to execute.\n";
                std::cout << "Multi-line input: open brace { continues to next line.\n\n";
                continue;
            }
            if (line == "vars") {
                auto names = evaluator.symbols().getAllVisibleNames();
                if (names.empty()) {
                    std::cout << "(no variables defined)\n";
                } else {
                    for (const auto& name : names) {
                        const auto* sym = evaluator.symbols().lookupSymbol(name);
                        if (sym) {
                            std::cout << typeToString(sym->type) << ": " << name << " = "
                                     << valueToString(sym->value) << "\n";
                        }
                    }
                }
                continue;
            }
        }
        
        // Count braces for multi-line input
        for (char c : line) {
            if (c == '{') braceCount++;
            else if (c == '}') braceCount--;
        }
        
        multiline += line + "\n";
        
        // Execute when balanced
        if (braceCount <= 0) {
            braceCount = 0;
            
            if (!multiline.empty() && multiline != "\n") {
                try {
                    Lexer lexer(multiline);
                    auto tokens = lexer.tokenize();
                    
                    Parser parser(std::move(tokens));
                    auto program = parser.parse();
                    
                    RhoValue result = evaluator.evaluate(*program);
                    
                    // Print result if it's not void/zero
                    RhoType type = getValueType(result);
                    if (type != RhoType::Int || std::get<int64_t>(result) != 0) {
                        std::cout << "= " << valueToString(result) << "\n";
                    }
                }
                catch (const RhoError& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                catch (const std::bad_alloc& e) {
                    std::cerr << "Error: Memory allocation failed. The operation required more memory than available." << std::endl;
                    std::cerr << "Suggestion: Try using smaller data sizes or check system memory usage." << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "Internal error: " << e.what() << "\n";
                }
            }
            
            multiline.clear();
        }
    }
    
    return 0;
}

/**
 * @brief Main entry point
 */
int main(int argc, char* argv[]) {
    if (argc == 1) {
        // REPL mode
        return runRepl();
    }
    
    if (argc >= 2) {
        std::string arg1 = argv[1];
        
        // Help flag
        if (arg1 == "-h" || arg1 == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        
        // Execute code from command line
        if (arg1 == "-e" && argc >= 3) {
            Evaluator evaluator;
            return execute(argv[2], evaluator, true);
        }
        
        // Execute file
        try {
            std::string source = readFile(arg1);
            Evaluator evaluator;

            // Set module loader base directory to the directory of the executed file
            std::filesystem::path filePath(arg1);
            std::filesystem::path baseDir = filePath.parent_path();
            if (baseDir.empty()) {
                baseDir = std::filesystem::current_path();
            }
            evaluator.moduleLoader().setBaseDirectory(baseDir.string());

            return execute(source, evaluator);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    
    printUsage(argv[0]);
    return 1;
}

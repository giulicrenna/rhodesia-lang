/**
 * @file ModuleLoader.hpp
 * @brief Module loading system for Rhodesia language
 *
 * Handles loading and caching of .rho modules for the include system.
 */

#ifndef RHODESIA_MODULE_LOADER_HPP
#define RHODESIA_MODULE_LOADER_HPP

#include "AST.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Error.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace Rhodesia {

/**
 * @brief Represents a loaded module with its AST
 */
struct LoadedModule {
    std::string path;
    std::unique_ptr<ProgramNode> ast;

    LoadedModule(std::string p, std::unique_ptr<ProgramNode> a)
        : path(std::move(p)), ast(std::move(a)) {}
};

/**
 * @brief Module loader with caching
 */
class ModuleLoader {
public:
    /**
     * @brief Set the base directory for module resolution
     * @param baseDir Directory where the main file is located
     */
    void setBaseDirectory(const std::string& baseDir) {
        baseDirectory_ = std::filesystem::absolute(baseDir);
    }

    /**
     * @brief Get the current base directory
     */
    std::string getBaseDirectory() const {
        return baseDirectory_.string();
    }

    /**
     * @brief Load a module by name
     * @param moduleName Name of the module (without .rho extension)
     * @param location Source location for error reporting
     * @return Pointer to the loaded module's AST
     */
    ProgramNode* loadModule(const std::string& moduleName, const SourceLocation& location) {
        // Check cache first
        auto it = moduleCache_.find(moduleName);
        if (it != moduleCache_.end()) {
            return it->second->ast.get();
        }

        // Resolve module path
        std::string modulePath = resolveModulePath(moduleName);

        // Load file contents
        std::string source = readFile(modulePath, location);

        // Lex and parse
        Lexer lexer(source);
        std::vector<Token> tokens;
        try {
            tokens = lexer.tokenize();
        } catch (const LexerError& e) {
            throw RuntimeError("Error in module '" + moduleName + "': " + e.what(), location);
        }

        Parser parser(std::move(tokens));
        std::unique_ptr<ProgramNode> ast;
        try {
            ast = parser.parse();
        } catch (const ParseError& e) {
            throw RuntimeError("Parse error in module '" + moduleName + "': " + e.what(), location);
        }

        // Cache the module
        ProgramNode* astPtr = ast.get();
        moduleCache_[moduleName] = std::make_unique<LoadedModule>(modulePath, std::move(ast));

        return astPtr;
    }

    /**
     * @brief Clear the module cache
     */
    void clearCache() {
        moduleCache_.clear();
    }

    /**
     * @brief Check if a module is already loaded
     */
    bool isModuleLoaded(const std::string& moduleName) const {
        return moduleCache_.find(moduleName) != moduleCache_.end();
    }

private:
    std::filesystem::path baseDirectory_;
    std::unordered_map<std::string, std::unique_ptr<LoadedModule>> moduleCache_;

    /**
     * @brief Resolve module name to file path
     * @param moduleName Name of the module
     * @return Full path to the .rho file
     */
    std::string resolveModulePath(const std::string& moduleName) {
        // Try different resolution strategies:

        // 1. Relative to base directory
        std::filesystem::path relativePath = baseDirectory_ / (moduleName + ".rho");
        if (std::filesystem::exists(relativePath)) {
            return relativePath.string();
        }

        // 2. Try as absolute path
        std::filesystem::path absolutePath(moduleName + ".rho");
        if (std::filesystem::exists(absolutePath)) {
            return absolutePath.string();
        }

        // 3. Try with subdirectories (e.g., "lib/module" -> "lib/module.rho")
        std::filesystem::path subdirPath = baseDirectory_ / (moduleName + ".rho");
        if (std::filesystem::exists(subdirPath)) {
            return subdirPath.string();
        }

        // Module not found
        throw RuntimeError("Module '" + moduleName + "' not found. Searched in: " +
                         baseDirectory_.string(), SourceLocation{});
    }

    /**
     * @brief Read file contents
     * @param path Path to the file
     * @param location Source location for error reporting
     * @return File contents as string
     */
    std::string readFile(const std::string& path, const SourceLocation& location) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw RuntimeError("Could not open module file: " + path, location);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

} // namespace Rhodesia

#endif // RHODESIA_MODULE_LOADER_HPP

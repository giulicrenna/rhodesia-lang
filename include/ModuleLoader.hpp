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
#include <algorithm>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
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
    ModuleLoader()
        : baseDirectory_(std::filesystem::current_path()),
          repoLibsInitialized_(false) {
        loadEnvLibraryPath();
        ensureBaseSearchPath();
        ensureRepoLibSearchPath();
        discoverLibraries();
    }

    /**
     * @brief Set the base directory for module resolution
     * @param baseDir Directory where the main file is located
     */
    void setBaseDirectory(const std::string& baseDir) {
        baseDirectory_ = std::filesystem::absolute(baseDir);
        repoLibsInitialized_ = false;
        libraries_.clear();
        ensureBaseSearchPath();
        ensureRepoLibSearchPath();
        discoverLibraries();
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
    std::unordered_map<std::string, std::filesystem::path> libraries_;
    std::vector<std::filesystem::path> searchPaths_;
    bool repoLibsInitialized_;
    bool basePathRegistered_ = false;

    void loadEnvLibraryPath() {
        const char* envPath = std::getenv("RHODESIA_LIB_PATH");
        if (envPath && *envPath) {
            std::filesystem::path path(envPath);
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
                searchPaths_.push_back(path);
            }
        }
    }

    void ensureBaseSearchPath() {
        if (!basePathRegistered_) {
            searchPaths_.insert(searchPaths_.begin(), baseDirectory_);
            basePathRegistered_ = true;
            return;
        }

        if (!searchPaths_.empty()) {
            searchPaths_[0] = baseDirectory_;
        } else {
            searchPaths_.push_back(baseDirectory_);
            basePathRegistered_ = true;
        }
    }

    void ensureRepoLibSearchPath() {
        if (repoLibsInitialized_) {
            return;
        }

        std::filesystem::path current = baseDirectory_;
        while (true) {
            std::filesystem::path candidate = current / "libs";
            if (std::filesystem::exists(candidate) && std::filesystem::is_directory(candidate)) {
                auto it = std::find(searchPaths_.begin(), searchPaths_.end(), candidate);
                if (it == searchPaths_.end()) {
                    searchPaths_.push_back(candidate);
                }
                break;
            }
            if (current == current.root_path()) {
                break;
            }
            current = current.parent_path();
        }

        repoLibsInitialized_ = true;
    }

    /**
     * @brief Walk every libs/ search path and register each immediate subdirectory
     *        as a library namespace (e.g. "math" -> ".../libs/math"). Resolution
     *        of a bare name like `include math` later looks up libraries_ first
     *        and returns `<lib>/index.rho`.
     */
    void discoverLibraries() {
        for (const auto& searchPath : searchPaths_) {
            // Only treat directories named "libs" as library roots.
            if (searchPath.filename() != "libs") continue;
            if (!std::filesystem::exists(searchPath) || !std::filesystem::is_directory(searchPath)) continue;

            std::error_code ec;
            for (auto it = std::filesystem::directory_iterator(searchPath, ec);
                 !ec && it != std::filesystem::directory_iterator();
                 it.increment(ec)) {
                const auto& entry = *it;
                if (!entry.is_directory()) continue;
                const std::string libName = entry.path().filename().string();
                if (libName.empty() || libName.front() == '.') continue;
                libraries_[libName] = entry.path();
            }
        }
    }

    /**
     * @brief Resolve module name to file path
     * @param moduleName Name of the module
     * @return Full path to the .rho file
     */
    std::string resolveModulePath(const std::string& moduleName) {
        ensureRepoLibSearchPath();

        // Library namespace: `include math` -> `libs/<lib>/index.rho`.
        // Only applies when the name is a single segment (no slashes) and
        // the library was discovered under some libs/ root.
        if (moduleName.find('/') == std::string::npos) {
            auto libIt = libraries_.find(moduleName);
            if (libIt != libraries_.end()) {
                std::filesystem::path entry = libIt->second / "index.rho";
                if (std::filesystem::exists(entry)) {
                    return entry.string();
                }
            }
        }

        for (const auto& searchPath : searchPaths_) {
            std::filesystem::path candidate = searchPath / (moduleName + ".rho");
            if (std::filesystem::exists(candidate)) {
                return candidate.string();
            }
        }

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

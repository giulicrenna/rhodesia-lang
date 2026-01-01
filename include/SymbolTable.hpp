/**
 * @file SymbolTable.hpp
 * @brief Symbol table with nested scope support for Rhodesia
 */

#ifndef RHODESIA_SYMBOLTABLE_HPP
#define RHODESIA_SYMBOLTABLE_HPP

#include "RhoValue.hpp"
#include "AST.hpp"
#include "Error.hpp"
#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <memory>

namespace Rhodesia {

/**
 * @brief Symbol entry containing type and value
 */
struct Symbol {
    std::string name;
    RhoType type;
    RhoValue value;
    bool isConst = false;
    SourceLocation declLocation;
    
    Symbol() : type(RhoType::Unknown) {}
    
    Symbol(std::string n, RhoType t, RhoValue v, SourceLocation loc = {})
        : name(std::move(n)), type(t), value(std::move(v)), declLocation(loc) {}
};

/**
 * @brief Function signature for user-defined functions
 */
struct FunctionSignature {
    std::string name;
    std::vector<FunctionParam> params;
    RhoType returnType;
    const FunctionDeclNode* declaration;  // Non-owning pointer to AST node
    
    FunctionSignature() : returnType(RhoType::Void), declaration(nullptr) {}
};

/**
 * @brief Symbol table with nested scope support
 * 
 * Uses a stack of hash maps to manage variable scopes.
 * Each scope level can shadow variables from outer scopes.
 */
class SymbolTable {
public:
    SymbolTable() {
        // Start with global scope
        enterScope();
    }
    
    /**
     * @brief Enter a new scope level
     */
    void enterScope() {
        scopes_.emplace_back();
    }
    
    /**
     * @brief Exit current scope level
     */
    void exitScope() {
        if (scopes_.size() > 1) {
            scopes_.pop_back();
        }
    }
    
    /**
     * @brief Get current scope depth (0 = global)
     */
    size_t depth() const {
        return scopes_.size() - 1;
    }
    
    /**
     * @brief Declare a new variable in current scope
     * @throws RuntimeError if variable already exists in current scope
     */
    void declare(const std::string& name, RhoType type, const RhoValue& value,
                 SourceLocation loc = {}) {
        auto& currentScope = scopes_.back();
        
        if (currentScope.find(name) != currentScope.end()) {
            throw RuntimeError("Variable '" + name + "' already declared in this scope", loc);
        }
        
        currentScope[name] = Symbol(name, type, value, loc);
    }
    
    /**
     * @brief Assign value to existing variable
     * @throws RuntimeError if variable not found
     */
    void assign(const std::string& name, const RhoValue& value, SourceLocation loc = {}) {
        // Search from innermost to outermost scope
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                // Type check
                RhoType newType = getValueType(value);
                if (found->second.type != newType) {
                    throw TypeError::expectedType(
                        "Assignment to '" + name + "'",
                        found->second.type, newType);
                }

                if (found->second.isConst) {
                    throw RuntimeError("Cannot assign to constant '" + name + "'", loc);
                }

                found->second.value = value;
                return;
            }
        }

        throw RuntimeError::undefinedVariable(name, loc);
    }

    /**
     * @brief Fast assignment without type checking (for performance-critical loops)
     * Only use when you're certain the type is correct!
     * @throws RuntimeError if variable not found
     */
    void assignUnchecked(const std::string& name, const RhoValue& value) {
        // Search only in current scope (loop iterator is always in current scope)
        auto& currentScope = scopes_.back();
        auto found = currentScope.find(name);
        if (found != currentScope.end()) {
            found->second.value = value;
            return;
        }

        // Fallback to full search (shouldn't happen in normal loop usage)
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                found->second.value = value;
                return;
            }
        }
    }
    
    /**
     * @brief Look up variable value
     * @throws RuntimeError if not found
     */
    const RhoValue& lookup(const std::string& name, SourceLocation loc = {}) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second.value;
            }
        }
        
        throw RuntimeError::undefinedVariable(name, loc);
    }
    
    /**
     * @brief Look up full symbol info
     */
    const Symbol* lookupSymbol(const std::string& name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return &found->second;
            }
        }
        return nullptr;
    }
    
    /**
     * @brief Check if variable exists
     */
    bool exists(const std::string& name) const {
        return lookupSymbol(name) != nullptr;
    }
    
    /**
     * @brief Check if variable exists in current scope only
     */
    bool existsInCurrentScope(const std::string& name) const {
        return scopes_.back().find(name) != scopes_.back().end();
    }
    
    /**
     * @brief Get type of variable
     */
    std::optional<RhoType> getType(const std::string& name) const {
        const Symbol* sym = lookupSymbol(name);
        if (sym) return sym->type;
        return std::nullopt;
    }
    
    // ========================================================================
    // Function Management
    // ========================================================================
    
    /**
     * @brief Register a user-defined function
     */
    void declareFunction(const std::string& name, const FunctionDeclNode* decl) {
        FunctionSignature sig;
        sig.name = name;
        sig.params = decl->params;
        sig.returnType = decl->returnType;
        sig.declaration = decl;
        functions_[name] = sig;
    }
    
    /**
     * @brief Look up function signature
     */
    const FunctionSignature* lookupFunction(const std::string& name) const {
        auto it = functions_.find(name);
        if (it != functions_.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
    /**
     * @brief Check if function exists
     */
    bool functionExists(const std::string& name) const {
        return functions_.find(name) != functions_.end();
    }
    
    // ========================================================================
    // Debug/Utility
    // ========================================================================
    
    /**
     * @brief Get all variable names in current scope
     */
    std::vector<std::string> getCurrentScopeNames() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : scopes_.back()) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * @brief Get all symbols from all accessible scopes (for closures)
     * Returns all visible variables from innermost to outermost scope
     */
    std::unordered_map<std::string, Symbol> getAllCurrentScopeSymbols() const {
        std::unordered_map<std::string, Symbol> allSymbols;

        // Iterate from outermost to innermost so inner scopes shadow outer ones
        for (const auto& scope : scopes_) {
            for (const auto& [name, symbol] : scope) {
                allSymbols[name] = symbol;
            }
        }

        return allSymbols;
    }

    /**
     * @brief Get all visible variable names
     */
    std::vector<std::string> getAllVisibleNames() const {
        std::vector<std::string> names;
        std::unordered_map<std::string, bool> seen;

        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            for (const auto& [name, _] : *it) {
                if (!seen[name]) {
                    names.push_back(name);
                    seen[name] = true;
                }
            }
        }
        return names;
    }

    /**
     * @brief Get all symbols (variables) from global scope
     * Used for module imports
     */
    std::vector<std::pair<std::string, Symbol>> getAllGlobalSymbols() const {
        std::vector<std::pair<std::string, Symbol>> symbols;
        if (!scopes_.empty()) {
            const auto& globalScope = scopes_[0];  // Global scope is first
            for (const auto& [name, symbol] : globalScope) {
                symbols.emplace_back(name, symbol);
            }
        }
        return symbols;
    }

    /**
     * @brief Get all registered functions
     * Used for module imports
     */
    std::vector<std::pair<std::string, FunctionSignature>> getAllFunctions() const {
        std::vector<std::pair<std::string, FunctionSignature>> funcs;
        for (const auto& [name, signature] : functions_) {
            funcs.emplace_back(name, signature);
        }
        return funcs;
    }
    
    /**
     * @brief Clear all scopes (reset to initial state)
     */
    void clear() {
        scopes_.clear();
        functions_.clear();
        enterScope();
    }

private:
    // Stack of scopes (outermost first)
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    
    // User-defined functions
    std::unordered_map<std::string, FunctionSignature> functions_;
};

/**
 * @brief RAII scope guard for automatic scope management
 */
class ScopeGuard {
public:
    explicit ScopeGuard(SymbolTable& table) : table_(table) {
        table_.enterScope();
    }
    
    ~ScopeGuard() {
        table_.exitScope();
    }
    
    // Non-copyable
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    SymbolTable& table_;
};

} // namespace Rhodesia

#endif // RHODESIA_SYMBOLTABLE_HPP

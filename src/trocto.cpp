// The compile_trocto pipeline: parse, lower, encode.

#include "encoder.hpp"
#include "lexer.hpp"
#include "lower.hpp"
#include "trocto_ast.hpp"

#include <fstream>
#include <sstream>

namespace trocto {

void Diagnostics::error(unsigned line, std::string message) {
    entries_.push_back(Diagnostic{line, std::move(message)});
}

void Diagnostics::clear() { entries_.clear(); }

namespace {

bool read_file(const std::string& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::ostringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

// Resolve import paths relative to the source file's directory.
std::string resolve_import_path(const std::string& source_path,
                                const std::string& import_path) {
    if (import_path.empty()) return import_path;
    // If the import path is absolute, use it as-is.
    if (!import_path.empty() &&
        (import_path[0] == '/' || import_path[0] == '\\' ||
         (import_path.size() > 1 && import_path[1] == ':'))) {
        return import_path;
    }
    // Find the last directory separator in the source path.
    size_t pos = source_path.find_last_of("/\\");
    if (pos == std::string::npos) return import_path;
    return source_path.substr(0, pos + 1) + import_path;
}

}  // namespace

std::optional<CompileResult> compile_trocto(const std::string& source,
                                            const CompileOptions& options,
                                            Diagnostics& diagnostics,
                                            const std::string& source_path) {
    auto contract = parse_contract(source, diagnostics);
    if (!contract) return std::nullopt;

    // Resolve imports: read each imported file, parse it, and link
    // its public function declarations into the importing contract.
    // Imported functions are prefixed with the module name (filename without extension).
    for (ImportDecl& imp : contract->imports) {
        std::string resolved = resolve_import_path(source_path, imp.path);
        std::string imported_source;
        if (!read_file(resolved, imported_source)) {
            diagnostics.error(imp.line,
                              "cannot read import '" + imp.path + "'");
            return std::nullopt;
        }
        // Validate the imported file compiles.
        Diagnostics import_diag;
        auto imported = parse_contract(imported_source, import_diag);
        if (!imported) {
            for (const auto& d : import_diag.entries()) {
                diagnostics.error(imp.line,
                                  "in import '" + imp.path + "': " +
                                      d.message);
            }
            return std::nullopt;
        }
        
        // Extract module name from path (e.g., "utils/math.tc" -> "math")
        size_t last_sep = imp.path.find_last_of("/\\");
        size_t last_dot = imp.path.find_last_of('.');
        if (last_dot == std::string::npos) last_dot = imp.path.size();
        if (last_sep == std::string::npos) last_sep = 0; else last_sep++;
        imp.module_name = imp.path.substr(last_sep, last_dot - last_sep);
        
        // Link public functions from the imported module.
        // Functions are prefixed with module_name:: to allow qualified calls.
        for (const FunctionDecl& fn : imported->functions) {
            if (!fn.public_abi) continue;  // Only link public functions
            
            FunctionDecl linked = fn;
            linked.name = imp.module_name + "::" + fn.name;
            imp.functions.push_back(linked);
            
            // Also add to contract's function list for compilation.
            contract->functions.push_back(linked);
        }
    }

    auto module = lower_contract(*contract, diagnostics);
    if (!module) return std::nullopt;

    CompileResult result;
    if (options.keep_ir) result.regol_text = regol_text(*module);

    auto container = encode_module(*module, diagnostics,
                                   options.skip_validation);
    if (!container) return std::nullopt;
    result.container = std::move(*container);
    return result;
}

std::optional<ModuleIR> lower_source_for_debug(const std::string& source,
                                               Diagnostics& diagnostics) {
    auto contract = parse_contract(source, diagnostics);
    if (!contract) return std::nullopt;
    return lower_contract(*contract, diagnostics);
}

}  // namespace trocto

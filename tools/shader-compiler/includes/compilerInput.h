//
// Created by anton on 11/29/25.
//

#ifndef CYCLONITE_COMPILER_INPUT_H
#define CYCLONITE_COMPILER_INPUT_H

#include "common.h"
#include <string>
#include <unordered_map>

namespace cyclonite::tools {
struct Options
{
    uint64_t noWarningOnUnusedDriverArgs : 1;
    uint64_t allResourcesBound : 1;
    uint64_t autoBindingSpace : 1;
    uint64_t enable16bitTypes : 1;
    uint64_t enableLifetimeMarkers : 1;
    uint64_t exportShadersOnly : 1;

    LinkageType linkage = LinkageType::Undefined;

    Encoding encoding = Encoding::Undefined;

    DiagnosticMessageFormat diagnosticMessageFormat = DiagnosticMessageFormat::Undefined;

    std::wstring entryPointName;

    std::unordered_map<std::wstring, std::wstring> definitions;
};
}

#endif // CYCLONITE_COMPILER_INPUT_H
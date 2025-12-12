//
// Created by anton on 11/29/25.
//

#ifndef CYCLONITE_COMPILER_INPUT_H
#define CYCLONITE_COMPILER_INPUT_H

#include "common.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace cyclonite::tools {
struct Options
{
    uint64_t noWarningOnUnusedDriverArgs : 1; // -Qunused-arguments
    uint64_t allResourcesBound : 1; // -all-resources-bound
    uint64_t autoBindingSpace : 1; // -auto-binding-space
    uint64_t enable16bitTypes : 1; // -enable-16bit-types
    uint64_t enableLifetimeMarkers : 1; // -enable-lifetime-markers
    uint64_t exportShadersOnly : 1; // -export-shaders-only
    uint64_t disableLocTracking : 1; // -fdisable-loc-tracking
    uint64_t legacyMacroExpansion : 1; // -flegacy-macro-expansion
    uint64_t newInliningBehavior : 1; // -fnew-inlining-behavior
    uint64_t forceRootsigVersion : 1; // -force-rootsig-ver
    uint64_t enableStrictMode : 1; // -Ges
    uint64_t forceIEEEStrictness : 1; // -Gis
    uint64_t ignoreLineDirectives : 1; // -ignore-line-directives
    uint64_t addsInstructionNummbersToAssemblerListing : 1; // -Ni
    uint64_t declareGlobalCB : 1; // -decl-global-cb
    uint64_t extractEntryUniforms : 1; // -extract-entry-uniforms
    uint64_t globalExternByDefault : 1; // -global-extern-by-default
    uint64_t keepUserMacro : 1; // -keep-user-macro
    uint64_t removeUnusedFunctions : 1; // -remove-unused-functions
    uint64_t removeUnusedGlobals : 1; // -remove-unused-globals
    uint64_t skipFnBody : 1; // -skip-fn-body
    uint64_t skipStatic : 1; // -skip-static
    uint64_t unchanged : 1; // -unchanged
    uint64_t spvEnableMaximalReconvergence : 1; // -fspv-enable-maximal-reconvergence
    uint64_t spvFlattenResourceArrays : 1; // -fspv-flatten-resource-arrays
    uint64_t spvPreserveBindings : 1; // -fspv-preserve-bindings
    uint64_t spvPreserveInterface : 1; // -fspv-preserve-interface
    uint64_t spvReduceLoadSize : 1; // -fspv-reduce-load-size
    uint64_t spvReflect : 1; // -fspv-reflect

    OptionValue showDiagnostics = OptionValue::Default; // -fno-diagnostics-show-option / fdiagnostics-show-option
    
    OptionValue flowControl = OptionValue::Default; // -Gfa / -Gfp

    LinkageType linkage = LinkageType::Undefined; // -default-linkage

    Encoding encoding = Encoding::Undefined; // -encoding

    DiagnosticMessageFormat diagnosticMessageFormat = DiagnosticMessageFormat::Undefined; // -fdiagnostics-format

    HV hv = HV::Undefined;

    Optimization optimization = Optimization::Disable; // -Od, O0, O1, O2, O3

    SpvDebug spvDebug = SpvDebug::Undefined; // fspv-debug

    std::wstring debugFileName; // -Fd
    std::wstring warninAndErrorsFileName; // -Fe
    std::wstring preprocessedCodeFileName; // -Fi
    std::wstring outputFileName; // -Fo
    std::wstring reflectionFileName; // -Fre
    std::wstring rootsigFileName; // -Frs
    std::wstring shaderHashFileName; // -Fsh

    std::wstring entryPointName; // -E / fspv-entrypoint-name

    std::unordered_map<std::wstring, std::wstring> definitions;
    std::vector<std::wstring> includeDirs;
    std::vector<std::wstring> spvExtensions; // -fspv-extension

    uint32_t spvMaxId = 0x3FFFFF;
};
}

#endif // CYCLONITE_COMPILER_INPUT_H
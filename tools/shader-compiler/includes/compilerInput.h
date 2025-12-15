//
// Created by anton on 11/29/25.
//

#ifndef CYCLONITE_COMPILER_INPUT_H
#define CYCLONITE_COMPILER_INPUT_H

#include "common.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include <limits>

namespace cyclonite::tools {
struct Options
{
    uint64_t noWarningOnUnusedDriverArgs : 1;               // -Qunused-arguments
    uint64_t allResourcesBound : 1;                         // -all-resources-bound
    uint64_t autoBindingSpace : 1;                          // -auto-binding-space
    uint64_t enable16bitTypes : 1;                          // -enable-16bit-types
    uint64_t enableLifetimeMarkers : 1;                     // -enable-lifetime-markers
    uint64_t exportShadersOnly : 1;                         // -export-shaders-only
    uint64_t disableLocTracking : 1;                        // -fdisable-loc-tracking
    uint64_t legacyMacroExpansion : 1;                      // -flegacy-macro-expansion
    uint64_t newInliningBehavior : 1;                       // -fnew-inlining-behavior
    uint64_t forceRootsigVersion : 1;                       // -force-rootsig-ver
    uint64_t backwardCompatibilityMode : 1;                 // -Gec
    uint64_t enableStrictMode : 1;                          // -Ges
    uint64_t forceIEEEStrictness : 1;                       // -Gis
    uint64_t ignoreLineDirectives : 1;                      // -ignore-line-directives
    uint64_t addsInstructionNummbersToAssemblerListing : 1; // -Ni
    uint64_t spvEnableMaximalReconvergence : 1;             // -fspv-enable-maximal-reconvergence
    uint64_t spvFlattenResourceArrays : 1;                  // -fspv-flatten-resource-arrays
    uint64_t spvPreserveBindings : 1;                       // -fspv-preserve-bindings
    uint64_t spvPreserveInterface : 1;                      // -fspv-preserve-interface
    uint64_t spvReduceLoadSize : 1;                         // -fspv-reduce-load-size
    uint64_t spvReflect : 1;                                // -fspv-reflect
    uint64_t spvUseLegacyBufferMatrixOrder : 1;             // -fspv-use-legacy-buffer-matrix-order
    uint64_t spvUseVulkanMemoryModel : 1;                   // -fspv-use-vulkan-memory-model
    uint64_t vkAutoShiftBindings : 1;                       // -fvk-auto-shift-bindings
    uint64_t noWarnings : 1;                                // -no-warnings
    uint64_t packOptimized : 1;                             // -pack-optimized
    uint64_t packPrefixStable : 1;                          // -pack-prefix-stable
    uint64_t resMayAlias : 1;                               // -res-may-alias
    uint64_t disableValidation : 1;                         // -Vd
    uint64_t verify : 1;                                    // -verify
    uint64_t warningsAsErrors : 1;                          // -Wx
    uint64_t disableIncludeProcessingDetails : 1;           // -Vi
    uint64_t enableDebugInformation : 1;                    // -Zi
    uint64_t matrixColumnMajorLayout : 1;                   // -Zpc
    uint64_t matrixRowMajorLayout : 1;                      // -Zpr
    uint64_t shaderHashBasedOnBinary : 1;                   // -Zsb
    uint64_t shaderHashBasedOnSource : 1;                   // -Zss
    uint64_t generateSmallPDB : 1;                          // -Zs
    uint64_t vkInvertY : 1;                                 // -fvk-invert-y
    uint64_t vkSupportNonzeroBaseInstance : 1;              // -fvk-support-nonzero-base-instance
    uint64_t vkSupportNonzeroBaseVertex : 1;                // -fvk-support-nonzero-base-vertex
    uint64_t useDXLayout : 1;                               // -fvk-use-dx-layout
    uint64_t useGLLayout : 1;                               // -fvk-use-gl-layout
    uint64_t vkUseDXPositionW : 1;                          // -fvk-use-dx-position-w
    uint64_t useScalarLayout : 1;                           // -fvk-use-scalar-layout
    uint64_t spirv : 1;                                     // -spirv
    uint64_t metal : 1;                                     // -metal
    uint64_t preprocessToFile : 1;                          // -P
    uint64_t embedDebug : 1;                                // -Qembed_debug
    uint64_t sourceInDebugModule : 1;                       // -Qsource_in_debug_module
    uint64_t stripDebug : 1;                                // -Qstrip_debug
    uint64_t stripRootSignature : 1;                        // -Qstrip_rootsignature
    uint64_t verifyRootSignature : 1;                       // -verifyrootsignature

    TargetPlatform platform;
    TargetGAPI gapi;

    OptionValue finiteMathOnly = OptionValue::Default; // ffinite-math-only / fno-finite-math-only

    OptionValue showDiagnostics = OptionValue::Default; // -fno-diagnostics-show-option / fdiagnostics-show-option

    OptionValue flowControl = OptionValue::Default; // -Gfa / -Gfp

    LinkageType linkage = LinkageType::Undefined; // -default-linkage

    Encoding encoding = Encoding::Undefined; // -encoding

    DiagnosticMessageFormat diagnosticMessageFormat = DiagnosticMessageFormat::Undefined; // -fdiagnostics-format

    HV hv = HV::Undefined; // -HV

    Optimization optimization = Optimization::Disable; // -Od, O0, O1, O2, O3

    SpvDebug spvDebug = SpvDebug::Undefined; // fspv-debug

    SpvTargetEnv targetEnv = SpvTargetEnv::Default; // spv-target-env

    std::string debugFileName;            // -Fd
    std::string warninAndErrorsFileName;  // -Fe
    std::string preprocessedCodeFileName; // -Fi
    std::string outputFileName;           // -Fo
    std::string reflectionFileName;       // -Fre
    std::string rootsigFileName;          // -Frs
    std::string shaderHashFileName;       // -Fsh

    std::wstring entryPointName; // -E / fspv-entrypoint-name

    std::wstring rootSigDefine; //-rootsig-define

    std::unordered_map<std::wstring, std::wstring> definitions; // -D
    std::vector<std::wstring> includeDirs;                      // -I
    std::vector<std::wstring> spvExtensions;                    // -fspv-extension

    uint32_t spvMaxId = 0x3FFFFF; // -fspv-max-id
    uint32_t vkBShift = std::numeric_limits<uint32_t>::max(); // -fvk-b-shift
    uint32_t vkSShift = std::numeric_limits<uint32_t>::max(); // -fvk-s-shift
    uint32_t vkTShift = std::numeric_limits<uint32_t>::max(); // -fvk-t-shift
    uint32_t vkUShift = std::numeric_limits<uint32_t>::max(); // -fvk-u-shift

    std::array<uint32_t, 2> vkBindCounterHeap;  // -fvk-bind-counter-heap
    std::array<uint32_t, 2> vkBindGlobals;      // -fvk-bind-globals
    std::array<uint32_t, 2> vkBindRegister;     // -fvk-bind-register
    std::array<uint32_t, 2> vkBindResourceHeap; // -fvk-bind-resource-heap
    std::array<uint32_t, 2> vkBindSamplerHeap;  // -fvk-bind-sampler-heap

    TargetProfile targetProfile = TargetProfile::ps_6_0; // -T
};
}

#endif // CYCLONITE_COMPILER_INPUT_H
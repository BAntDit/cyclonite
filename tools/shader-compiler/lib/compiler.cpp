//
// Created by anton on 11/29/25.
//

#include "compiler.h"
#include "dxReflection.h"
#include <cassert>
#include <format>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace cyclonite::tools {
namespace {
auto getDxcCodePage(Encoding encoding) -> UINT32
{
    auto result = UINT32{ DXC_CP_ACP };
    switch (encoding) {
        case Encoding::Utf8:
            result = DXC_CP_UTF8;
            break;
        case Encoding::Utf16Win:
            result = DXC_CP_UTF16;
            break;
        case Encoding::Utf32Nix:
            result = DXC_CP_UTF32;
            break;
        case Encoding::Wide:
            result = DXC_CP_WIDE;
            break;
        case Encoding::Undefined:
            [[fallthrough]];
        default:
            result = DXC_CP_UTF8; // default
    }
    return result;
}

auto getDxcStage(TargetProfile profile) -> std::wstring
{
    auto result = std::wstring{ L"" };

    switch (profile) {
        case TargetProfile::vs_6_0:
            result = L"vs_6_0";
            break;
        case TargetProfile::vs_6_1:
            result = L"vs_6_1";
            break;
        case TargetProfile::vs_6_2:
            result = L"vs_6_2";
            break;
        case TargetProfile::vs_6_3:
            result = L"vs_6_3";
            break;
        case TargetProfile::vs_6_4:
            result = L"vs_6_4";
            break;
        case TargetProfile::vs_6_5:
            result = L"vs_6_5";
            break;
        case TargetProfile::vs_6_6:
            result = L"vs_6_6";
            break;
        case TargetProfile::vs_6_7:
            result = L"vs_6_7";
            break;
        case TargetProfile::vs_6_8:
            result = L"vs_6_8";
            break;
        case TargetProfile::vs_6_9:
            result = L"vs_6_9";
            break;
        case TargetProfile::ps_6_0:
            result = L"ps_6_0";
            break;
        case TargetProfile::ps_6_1:
            result = L"ps_6_1";
            break;
        case TargetProfile::ps_6_2:
            result = L"ps_6_2";
            break;
        case TargetProfile::ps_6_3:
            result = L"ps_6_3";
            break;
        case TargetProfile::ps_6_4:
            result = L"ps_6_4";
            break;
        case TargetProfile::ps_6_5:
            result = L"ps_6_5";
            break;
        case TargetProfile::ps_6_6:
            result = L"ps_6_6";
            break;
        case TargetProfile::ps_6_7:
            result = L"ps_6_7";
            break;
        case TargetProfile::ps_6_8:
            result = L"ps_6_8";
            break;
        case TargetProfile::ps_6_9:
            result = L"ps_6_9";
            break;
        case TargetProfile::cs_6_0:
            result = L"cs_6_0";
            break;
        case TargetProfile::cs_6_1:
            result = L"cs_6_1";
            break;
        case TargetProfile::cs_6_2:
            result = L"cs_6_2";
            break;
        case TargetProfile::cs_6_3:
            result = L"cs_6_3";
            break;
        case TargetProfile::cs_6_4:
            result = L"cs_6_4";
            break;
        case TargetProfile::cs_6_5:
            result = L"cs_6_5";
            break;
        case TargetProfile::cs_6_6:
            result = L"cs_6_6";
            break;
        case TargetProfile::cs_6_7:
            result = L"cs_6_7";
            break;
        case TargetProfile::cs_6_8:
            result = L"cs_6_8";
            break;
        case TargetProfile::cs_6_9:
            result = L"cs_6_9";
            break;
        case TargetProfile::ds_6_0:
            result = L"ds_6_0";
            break;
        case TargetProfile::ds_6_1:
            result = L"ds_6_1";
            break;
        case TargetProfile::ds_6_2:
            result = L"ds_6_2";
            break;
        case TargetProfile::ds_6_3:
            result = L"ds_6_3";
            break;
        case TargetProfile::ds_6_4:
            result = L"ds_6_4";
            break;
        case TargetProfile::ds_6_5:
            result = L"ds_6_5";
            break;
        case TargetProfile::ds_6_6:
            result = L"ds_6_6";
            break;
        case TargetProfile::ds_6_7:
            result = L"ds_6_7";
            break;
        case TargetProfile::ds_6_8:
            result = L"ds_6_8";
            break;
        case TargetProfile::ds_6_9:
            result = L"ds_6_9";
            break;
        case TargetProfile::hs_6_0:
            result = L"hs_6_0";
            break;
        case TargetProfile::hs_6_1:
            result = L"hs_6_1";
            break;
        case TargetProfile::hs_6_2:
            result = L"hs_6_2";
            break;
        case TargetProfile::hs_6_3:
            result = L"hs_6_3";
            break;
        case TargetProfile::hs_6_4:
            result = L"hs_6_4";
            break;
        case TargetProfile::hs_6_5:
            result = L"hs_6_5";
            break;
        case TargetProfile::hs_6_6:
            result = L"hs_6_6";
            break;
        case TargetProfile::hs_6_7:
            result = L"hs_6_7";
            break;
        case TargetProfile::hs_6_8:
            result = L"hs_6_8";
            break;
        case TargetProfile::hs_6_9:
            result = L"hs_6_9";
            break;
        case TargetProfile::gs_6_0:
            result = L"gs_6_0";
            break;
        case TargetProfile::gs_6_1:
            result = L"gs_6_1";
            break;
        case TargetProfile::gs_6_2:
            result = L"gs_6_2";
            break;
        case TargetProfile::gs_6_3:
            result = L"gs_6_3";
            break;
        case TargetProfile::gs_6_4:
            result = L"gs_6_4";
            break;
        case TargetProfile::gs_6_5:
            result = L"gs_6_5";
            break;
        case TargetProfile::gs_6_6:
            result = L"gs_6_6";
            break;
        case TargetProfile::gs_6_7:
            result = L"gs_6_7";
            break;
        case TargetProfile::gs_6_8:
            result = L"gs_6_8";
            break;
        case TargetProfile::gs_6_9:
            result = L"gs_6_9";
            break;
        case TargetProfile::lib_6_1:
            result = L"lib_6_1";
            break;
        case TargetProfile::lib_6_2:
            result = L"lib_6_2";
            break;
        case TargetProfile::lib_6_3:
            result = L"lib_6_3";
            break;
        case TargetProfile::lib_6_4:
            result = L"lib_6_4";
            break;
        case TargetProfile::lib_6_5:
            result = L"lib_6_5";
            break;
        case TargetProfile::lib_6_6:
            result = L"lib_6_6";
            break;
        case TargetProfile::lib_6_7:
            result = L"lib_6_7";
            break;
        case TargetProfile::lib_6_8:
            result = L"lib_6_8";
            break;
        case TargetProfile::lib_6_9:
            result = L"lib_6_9";
            break;
        case TargetProfile::as_6_5:
            result = L"as_6_5";
            break;
        case TargetProfile::as_6_6:
            result = L"as_6_6";
            break;
        case TargetProfile::as_6_7:
            result = L"as_6_7";
            break;
        case TargetProfile::as_6_8:
            result = L"as_6_8";
            break;
        case TargetProfile::as_6_9:
            result = L"as_6_9";
            break;
        case TargetProfile::ms_6_5:
            result = L"ms_6_5";
            break;
        case TargetProfile::ms_6_6:
            result = L"ms_6_6";
            break;
        case TargetProfile::ms_6_7:
            result = L"ms_6_7";
            break;
        case TargetProfile::ms_6_8:
            result = L"ms_6_8";
            break;
        case TargetProfile::ms_6_9:
            result = L"ms_6_9";
            break;
        default:
            assert(false);
    }

    return result;
}

auto getDxcOptions(Options const& options) -> std::vector<const wchar_t*>
{
    auto result = std::vector<const wchar_t*>{};

    if (options.allResourcesBound) {
        result.push_back(DXC_ARG_ALL_RESOURCES_BOUND);
    }
    if (options.autoBindingSpace) {
        result.push_back(L"-auto-binding-space");
    }
    if (options.enable16bitTypes) {
        result.push_back(L"-enable-16bit-types");
    }
    if (options.enableLifetimeMarkers) {
        result.push_back(L"-enable-lifetime-markers");
    }
    if (options.exportShadersOnly) {
        result.push_back(L"-export-shaders-only");
    }
    if (options.disableLocTracking) {
        result.push_back(L"-fdisable-loc-tracking");
    }
    if (options.legacyMacroExpansion) {
        result.push_back(L"-flegacy-macro-expansion");
    }
    if (options.newInliningBehavior) {
        result.push_back(L"-fnew-inlining-behavior");
    }
    if (options.backwardCompatibilityMode) {
        result.push_back(DXC_ARG_ENABLE_BACKWARDS_COMPATIBILITY);
    }
    if (options.enableStrictMode) {
        result.push_back(DXC_ARG_ENABLE_STRICTNESS);
    }
    if (options.forceIEEEStrictness) {
        result.push_back(DXC_ARG_IEEE_STRICTNESS);
    }
    if (options.forceRootsigVersion) {
        result.push_back(L"-force-rootsig-ver");
    }
    if (options.ignoreLineDirectives) {
        result.push_back(L"-ignore-line-directives");
    }
    if (options.addsInstructionNummbersToAssemblerListing) {
        result.push_back(L"-Ni");
    }
    if (options.spvEnableMaximalReconvergence) {
        result.push_back(L"-fspv-enable-maximal-reconvergence");
    }
    if (options.spvFlattenResourceArrays) {
        result.push_back(L"-fspv-flatten-resource-arrays");
    }
    if (options.spvPreserveBindings) {
        result.push_back(L"-fspv-preserve-bindings");
    }
    if (options.spvPreserveInterface) {
        result.push_back(L"-fspv-preserve-interface");
    }
    if (options.spvReduceLoadSize) {
        result.push_back(L"-fspv-reduce-load-size");
    }
    if (options.spvReflect) {
        result.push_back(L"-fspv-reflect");
    }
    if (options.spvUseLegacyBufferMatrixOrder) {
        result.push_back(L"-fspv-use-legacy-buffer-matrix-order");
    }
    if (options.spvUseVulkanMemoryModel) {
        result.push_back(L"-fspv-use-vulkan-memory-model");
    }
    if (options.vkAutoShiftBindings) {
        result.push_back(L"-fvk-auto-shift-bindings");
    }
    if (options.noWarnings) {
        result.push_back(L"-no-warnings");
    }
    if (options.packOptimized) {
        result.push_back(L"-pack-optimized");
    }
    if (options.packPrefixStable) {
        result.push_back(L"-pack-prefix-stable");
    }
    if (options.resMayAlias) {
        result.push_back(DXC_ARG_RESOURCES_MAY_ALIAS);
    }
    if (options.disableValidation) {
        result.push_back(DXC_ARG_SKIP_VALIDATION);
    }
    if (options.verify) {
        result.push_back(L"-verify");
    }
    if (options.warningsAsErrors) {
        result.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
    }
    if (options.disableIncludeProcessingDetails) {
        result.push_back(L"-Vi");
    }
    if (options.vkInvertY) {
        result.push_back(L"-fvk-invert-y");
    }
    if (options.vkSupportNonzeroBaseInstance) {
        result.push_back(L"-fvk-support-nonzero-base-instance");
    }
    if (options.vkSupportNonzeroBaseVertex) {
        result.push_back(L"-fvk-support-nonzero-base-vertex");
    }

    if (options.enableDebugInformation) {
        result.push_back(DXC_ARG_DEBUG);
    } else if (options.generateSmallPDB) {
        result.push_back(L"-Zs");
    }

    if (options.shaderHashBasedOnBinary) {
        result.push_back(DXC_ARG_DEBUG_NAME_FOR_BINARY);
    } else if (options.shaderHashBasedOnSource) {
        result.push_back(DXC_ARG_DEBUG_NAME_FOR_SOURCE);
    }

    if (options.matrixLayout == MatrixLayout::ColumnMajor) {
        result.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR);
    } else if (options.matrixLayout == MatrixLayout::RowMajor) {
        result.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);
    }

    if (options.linkage != LinkageType::Undefined) {
        if (options.linkage == LinkageType::External) {
            result.push_back(L"-default-linkage=external");
        } else if (options.linkage == LinkageType::Internal) {
            result.push_back(L"-default-linkage=internal");
        }
    }

    if (options.hv != HV::Undefined) {
        result.push_back(L"-HV");
        switch (options.hv) {
            case HV::_2016:
                result.push_back(L"2016");
                break;
            case HV::_2017:
                result.push_back(L"2017");
                break;
            case HV::_2018:
                result.push_back(L"2018");
                break;
            case HV::_2021:
                result.push_back(L"2021");
                break;
            default:
                assert(false);
        }
    }

    if (options.useDXLayout) {
        result.push_back(L"-fvk-use-dx-layout");
    } else if (options.useGLLayout) {
        result.push_back(L"-fvk-use-gl-layout");
    } else if (options.useScalarLayout) {
        result.push_back(L"-fvk-use-scalar-layout");
    }

    if (options.spirv) {
        result.push_back(L"-spirv");
    } else if (options.metal) {
        result.push_back(L"-metal");
    }

    if (options.embedDebug) {
        result.push_back(L"-Qembed_debug");
    }
    if (options.sourceInDebugModule) {
        result.push_back(L"-Qsource_in_debug_module");
    }
    if (options.stripDebug) {
        result.push_back(L"-Qstrip_debug");
    }
    if (options.stripRootSignature) {
        result.push_back(L"-Qstrip_rootsignature");
    }
    if (options.verifyRootSignature) {
        result.push_back(L"-verifyrootsignature");
    }

    if (options.finiteMathOnly != OptionValue::Default) {
        if (options.finiteMathOnly == OptionValue::Enable) {
            result.push_back(L"-ffinite-math-only");
        } else if (options.finiteMathOnly == OptionValue::Disable) {
            result.push_back(L"-fno-finite-math-only");
        }
    }

    if (options.showDiagnostics == OptionValue::Enable) {
        result.push_back(L"-fdiagnostics-show-option");
    } else if (options.showDiagnostics == OptionValue::Disable) {
        result.push_back(L"-fno-diagnostics-show-option");
    }

    if (options.flowControl == OptionValue::Enable) {
        result.push_back(DXC_ARG_PREFER_FLOW_CONTROL);
    } else if (options.flowControl == OptionValue::Disable) {
        result.push_back(DXC_ARG_AVOID_FLOW_CONTROL);
    }

    switch (options.optimization) {
        case Optimization::Disable:
            result.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
            break;
        case Optimization::Level0:
            result.push_back(DXC_ARG_OPTIMIZATION_LEVEL0);
            break;
        case Optimization::Level1:
            result.push_back(DXC_ARG_OPTIMIZATION_LEVEL1);
            break;
        case Optimization::Level2:
            result.push_back(DXC_ARG_OPTIMIZATION_LEVEL2);
            break;
        case Optimization::Level3:
            result.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
            break;
        default:
            assert(false);
    }

    if (options.spvDebug != SpvDebug::Undefined) {
        switch (options.spvDebug) {
            case SpvDebug::Line:
                result.push_back(L"-fspv-debug=line");
                break;
            case SpvDebug::Source:
                result.push_back(L"-fspv-debug=source");
                break;
            case SpvDebug::File:
                result.push_back(L"-fspv-debug=file");
                break;
            case SpvDebug::VulkanWithSource:
                result.push_back(L"-fspv-debug=vulkan-with-source");
                break;
            default:
                assert(false);
        }
    }

    if (options.targetEnv != SpvTargetEnv::Default) {
        switch (options.targetEnv) {
            case SpvTargetEnv::Vulkan_1_0:
                result.push_back(L"-fspv-target-env=vulkan1.0");
                break;
            case SpvTargetEnv::Vulkan_1_1:
                result.push_back(L"-fspv-target-env=vulkan1.1");
                break;
            case SpvTargetEnv::Vulkan_1_2:
                result.push_back(L"-fspv-target-env=vulkan1.2");
                break;
            case SpvTargetEnv::Vulkan_1_3:
                result.push_back(L"-fspv-target-env=vulkan1.3");
                break;
            case SpvTargetEnv::Vulkan_1_1_Spirv_1_4:
                result.push_back(L"-fspv-target-env=vulkan1.1spirv1.4");
                break;
            case SpvTargetEnv::Universal_1_5:
                result.push_back(L"-fspv-target-env=universal1.5");
                break;
            default:
                assert(false);
        }
    }

    if (!options.rootSigDefine.empty()) {
        result.push_back(L"-rootsig-define");
        result.push_back(options.rootSigDefine.data());
    }

    for (auto const& idir : options.includeDirs) {
        result.push_back(L"-I");
        result.push_back(idir.data());
    }

    for (auto const& ext : options.spvExtensions) {
        result.push_back(L"-fspv-extension");
        result.push_back(ext.data());
    }

    if (options.spvMaxId != 0x3FFFFF) {
        result.push_back(L"-fspv-max-id");
        result.push_back(options.spvMaxIdStr.data());
    }

    if (options.vkBShift != std::numeric_limits<uint32_t>::max()) {
        result.push_back(L"-fvk-b-shift");
        result.push_back(options.vkBShiftStr.data());
    }
    if (options.vkSShift != std::numeric_limits<uint32_t>::max()) {
        result.push_back(L"-fvk-s-shift");
        result.push_back(options.vkSShiftStr.data());
    }
    if (options.vkTShift != std::numeric_limits<uint32_t>::max()) {
        result.push_back(L"-fvk-t-shift");
        result.push_back(options.vkTShiftStr.data());
    }
    if (options.vkUShift != std::numeric_limits<uint32_t>::max()) {
        result.push_back(L"-fvk-u-shift");
        result.push_back(options.vkUShiftStr.data());
    }

    if (!options.vkBindCounterHeapStr.empty()) {
        result.push_back(options.vkBindCounterHeapStr.data());
    }

    if (!options.vkBindGlobalsStr.empty()) {
        result.push_back(options.vkBindGlobalsStr.data());
    }

    if (!options.vkBindRegisterStr.empty()) {
        result.push_back(options.vkBindRegisterStr.data());
    }

    if (!options.vkBindResourceHeapStr.empty()) {
        result.push_back(options.vkBindResourceHeapStr.data());
    }

    return result;
}

auto getDxcDefines(Options const& options) -> std::vector<DxcDefine>
{
    auto result = std::vector<DxcDefine>();

    for (auto const& [define, value] : options.definitions) {
        auto dxcDef = DxcDefine{};
        dxcDef.Name = define.data();
        dxcDef.Value = value.data();

        result.push_back(dxcDef);
    }

    return result;
}
}

Compiler::Compiler()
  : dxcLibrary_{ nullptr }
  , dxcUtils_{ nullptr }
  , dxcCompiler_{ nullptr }
{
    if (auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxcLibrary_)); FAILED(hr)) {
        throw std::runtime_error("could not create DxcLibrary instance");
    }

    if (auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_)); FAILED(hr)) {
        throw std::runtime_error("could not create DxcUtils instance");
    }

    if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_)); FAILED(hr)) {
        throw std::runtime_error("could not create DxcCompiler3 instance");
    }
}

Compiler::Compiler(Compiler&& compiler) noexcept
  : dxcLibrary_{ compiler.dxcLibrary_ }
  , dxcUtils_{ compiler.dxcUtils_ }
  , dxcCompiler_{ compiler.dxcCompiler_ }
{
    compiler.dxcLibrary_ = nullptr;
    compiler.dxcUtils_ = nullptr;
    compiler.dxcCompiler_ = nullptr;
}

Compiler::~Compiler()
{
    if (dxcCompiler_ != nullptr) {
        dxcCompiler_->Release();
        dxcCompiler_ = nullptr;
    }

    if (dxcUtils_ != nullptr) {
        dxcUtils_->Release();
        dxcUtils_ = nullptr;
    }

    if (dxcLibrary_ != nullptr) {
        dxcLibrary_->Release();
        dxcLibrary_ = nullptr;
    }
}

auto Compiler::operator=(Compiler&& rhs) noexcept -> Compiler&
{
    dxcLibrary_ = std::exchange(rhs.dxcLibrary_, nullptr);
    dxcUtils_ = std::exchange(rhs.dxcUtils_, nullptr);
    dxcCompiler_ = std::exchange(rhs.dxcCompiler_, nullptr);
    return *this;
}

void Compiler::compile(std::wstring_view source, Options const& options, IDxcResult*& compileResult)
{
    auto* sourceBlob = std::add_pointer_t<IDxcBlobEncoding>{ nullptr };
    auto codePage = getDxcCodePage(options.encoding);

    if (auto result = dxcUtils_->LoadFile(source.data(), &codePage, &sourceBlob); !SUCCEEDED(result)) {
        throw std::runtime_error("could not load source file");
    }

    auto sourceBuffer = DxcBuffer{};
    sourceBuffer.Encoding = codePage;
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();

    auto dxcStage = getDxcStage(options.targetProfile);
    auto dxcOptions = getDxcOptions(options);
    auto dxcDefines = getDxcDefines(options);

    auto* dxcInputArguments = std::add_pointer_t<IDxcCompilerArgs>{ nullptr };
    if (auto result = dxcUtils_->BuildArguments(source.data(),
                                                options.entryPointName.data(),
                                                dxcStage.data(),
                                                dxcOptions.data(),
                                                static_cast<UINT32>(dxcOptions.size()),
                                                dxcDefines.data(),
                                                static_cast<UINT32>(dxcDefines.size()),
                                                &dxcInputArguments);
        !SUCCEEDED(result)) {
        throw std::runtime_error("could not build arguments");
    }

    auto* includeHandler = std::add_pointer_t<IDxcIncludeHandler>{ nullptr };
    dxcUtils_->CreateDefaultIncludeHandler(&includeHandler);

    if (auto result = dxcCompiler_->Compile(&sourceBuffer,
                                            dxcInputArguments->GetArguments(),
                                            dxcInputArguments->GetCount(),
                                            includeHandler,
                                            IID_PPV_ARGS(&compileResult));
        !SUCCEEDED(result)) {
        throw std::runtime_error("could not compile source");
    }

    if (compileResult->HasOutput(DXC_OUT_ERRORS)) {
        auto errors = std::add_pointer_t<IDxcBlobUtf8>{ nullptr };
        if (auto result = compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
            !SUCCEEDED(result)) {
            throw std::runtime_error("could not extract error block");
        } else if (errors && errors->GetStringLength() > 0) {
            const char* err = errors->GetStringPointer();
            std::cout << "error on attempt to compile shader: " << err << std::endl;
        }
    }
}

void Compiler::collectReflection(std::wstring_view source, Options const& options)
{
    auto* dxcCompileResult = std::add_pointer_t<IDxcResult>{ nullptr };
    compile(source, options, dxcCompileResult);

    if (dxcCompileResult == nullptr || dxcCompileResult->HasOutput(DXC_OUT_REFLECTION)) {
        throw std::runtime_error("could not extract reflection");
    }

    auto* dxcOutReflection = std::add_pointer_t<IDxcBlob>{ nullptr };
    if (auto result = dxcCompileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxcOutReflection), nullptr);
        !SUCCEEDED(result)) {
        throw std::runtime_error("could not extract reflection");
    }

    auto dxcReflectionBuffer = DxcBuffer{};
    dxcReflectionBuffer.Ptr = dxcOutReflection->GetBufferPointer();
    dxcReflectionBuffer.Size = dxcOutReflection->GetBufferSize();
    dxcReflectionBuffer.Encoding = DXC_CP_ACP;

    auto* dxcShaderReflection = std::add_pointer_t<ID3D12ShaderReflection>{ nullptr };
    if (auto result = dxcUtils_->CreateReflection(&dxcReflectionBuffer, IID_PPV_ARGS(&dxcShaderReflection));
        !SUCCEEDED(result)) {
        throw std::runtime_error("could not create reflection object");
    }

    auto dxReflection = DxReflection{};
    dxReflection.getShaderDesc(dxcShaderReflection);
}
}
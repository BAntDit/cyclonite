//
// Created by anton on 11/29/25.
//

#include "compiler.h"
#if !defined(_WIN32) // _WIN32 / _WIN64 at once
#include <dxc/WinAdapter.h>
#endif
#include <stdexcept>
#include <utility>

namespace cyclonite::tools {
namespace {
auto getDxcCodePage(Encoding encoding)-> UINT32 {
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

auto getDxcStage(TargetProfile profile) -> std::wstring {
    auto result = std::wstring{L""};

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

auto getDxcOptions(Options const& options) -> std::vector<const wchar_t*> {
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

    if (options.linkage != LinkageType::Undefined) {
        if (options.linkage == LinkageType::External) {
            result.push_back(L"-default-linkage=external");
        } else if (options.linkage == LinkageType::Internal) {
            result.push_back(L"-default-linkage=internal");
        }
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

void Compiler::compile(std::wstring_view source, Options const& options)
{
    auto* compileResult = std::add_pointer_t<IDxcResult>{ nullptr };
    auto* sourceBlob = std::add_pointer_t<IDxcBlobEncoding>{ nullptr };
    auto codePage = getDxcCodePage(options.encoding);

    if (auto result = dxcUtils_->LoadFile(source.data(), &codePage, &sourceBlob); !SUCCEEDED(result)) {
        throw std::runtime_error("could not load source file");
    }

    auto sourceBuffer = DxcBuffer{};
    sourceBuffer.Encoding = codePage;
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();

    auto* dxcInputArguments = std::add_pointer_t<IDxcCompilerArgs>{nullptr};

}
}
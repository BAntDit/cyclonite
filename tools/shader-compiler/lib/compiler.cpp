//
// Created by anton on 11/29/25.
//

#include "compiler.h"
#include <dxc/WinAdapter.h>
#include <stdexcept>

namespace cyclonite::tools
{
Compiler::Compiler() :
    dxcLibrary_{ nullptr }
    , dxcUtils_{ nullptr }
    , dxcCompiler_{ nullptr }
{
    if (auto hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxcLibrary_)); FAILED(hr)) {
        throw std::runtime_error("could not create DxcLibrary instance");
    }

    if (auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
       FAILED(hr)) {
        throw std::runtime_error("could not create DxcUtils instance");
    }

    if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
        FAILED(hr))
    {
        throw std::runtime_error("could not create DxcCompiler3 instance");
    }
}

Compiler::Compiler(Compiler&& compiler) noexcept:
    dxcLibrary_{ compiler.dxcLibrary_ }
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
}
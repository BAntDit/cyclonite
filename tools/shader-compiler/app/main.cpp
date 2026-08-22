
#include "compiler.h"
#include "compilerOutput.h"
#include "serialization.h"
#include "binaryStreamWriter.h"
#include "shaderModuleBinary.h"
#include <boost/program_options.hpp>
#include <filesystem>
#ifdef uuid
#undef uuid
#endif
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <codecvt>
#include <format>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "dxReflection.h"

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    auto desc = po::options_description{ "shader-compiler options", 160 };
    desc.add_options()                                                           // options:
      ("help", "Produce help message")                                           // --help
      ("version", "Display compiler version information")                        // --version
      ("source", po::value<std::string>(), "Source file to compile")             // --source
      ("target-platform", po::value<std::string>(), "Specifies target platform") // --target-platform
      ("target-gapi", po::value<std::string>(), "Specifies target GAPI")         // --target-gapi
      ("Qunused-arguments", "Don’t emit warning for unused driver arguments")    // --Qunused-arguments
      ("all-resources-bound", "Enables aggressive flattening")                   //
      ("auto-binding-space", "Set auto binding space - enables auto resource binding in libraries") //
      ("default-linkage",
       po::value<std::string>(),
       "Set default linkage for non-shader functions when compiling or linking to "
       "a library target (internal, external)") //
      ("D",
       po::value<std::vector<std::string>>()->multitoken(),
       "Define macros") // --D <name1[=value1]> [name2[=value2]] ...
      ("enable-16bit-types",
       "Enable 16bit types and disable min precision types. Available in HLSL "
       "2018 and shader model 6.2")                                        //
      ("enable-lifetime-markers", "Enable generation of lifetime markers") //
      ("encoding",
       po::value<std::string>(),
       "Set default encoding for source inputs and text outputs (utf8	utf16(win) utf32(*nix) wide) default=utf8") //
      ("export-shaders-only", "Only export shaders when compiling a library.")                                      //
      ("E", po::value<std::string>(), "Entry point name")                                                           //
      ("fdiagnostics-format",
       po::value<std::string>(),
       "Select diagnostic message format. Supported values: clang, msvc, mdvc-fallback, vi") //
      ("fdiagnostics-show-option", "Print option name with mappable diagnostics")            //
      ("fdisable-loc-tracking",
       "Disable source location tracking in IR. This will break diagnostic "
       "generation for late validation. (Ignored if /Zi is passed)") //
      ("Fd",
       po::value<std::string>(),
       "Write debug information to the given file, or automatically named file in directory") //
      ("Fe", po::value<std::string>(), "Output warnings and errors to the given file")        //
      ("Fi", po::value<std::string>(), "Set preprocess output file name (with /P)")           //
      ("flegacy-macro-expansion",
       "Expand the operands before performing token-pasting operation (fxc behavior)") //
      ("fnew-inlining-behavior",
       "Experimental option to use heuristics-driven late inlining "
       "and disable alwaysinline annotation for library shaders")                             //
      ("fno-diagnostics-show-option", "Do not print option name with mappable diagnostics")   //
      ("force-rootsig-ver", "force root signature version (rootsig_1_1 if omitted)")          //
      ("Fo", po::value<std::string>(), "Output file")                                         //
      ("Fre", po::value<std::string>(), "Output reflection to the given file")                //
      ("Frs", po::value<std::string>(), "Output root signature to the given file")            //
      ("Fsh", po::value<std::string>(), "Output shader hash to the given file")               //
      ("Gec", "Enable backward compatibility mode")                                           //
      ("Ges", "Enable strict mode")                                                           //
      ("Gfa", "Avoid flow control constructs")                                                //
      ("Gfp", "Prefer flow control constructs")                                               //
      ("Gis", "Force IEEE strictness")                                                        //
      ("HV", po::value<uint16_t>(), "HLSL version (2016, 2017, 2018, 2021). Default is 2021") //
      ("ignore-line-directives", "Ignore line directives")                                    //
      ("I", po::value<std::string>(), "Add directory to include search path")                 //
      ("Ni", "Output instruction numbers in assembly listings")                               //
      ("no-warnings", "Suppress warnings")                                                    //
      ("Od", "Disable optimizations")                                                         //
      ("pack-optimized",
       "Optimize signature packing assuming identical signature provided for each connecting stage") //
      ("pack-prefix-stable",
       "(default) Pack signatures preserving prefix-stable property - appended "
       "elements will not disturb placement of prior elements")                          //
      ("res-may-alias", "Assume that UAVs/SRVs may alias")                               //
      ("rootsig-define", po::value<std::string>(), "Read root signature from a #define") //
      ("T",
       po::value<std::string>(),
       "Set target profile. profile: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, "
       "ps_6_5, ps_6_6, ps_6_7, ps_6_8, ps_6_9, vs_6_0, vs_6_1, vs_6_2, vs_6_3, "
       "vs_6_4, vs_6_5, vs_6_6, vs_6_7, vs_6_8, vs_6_9, gs_6_0, gs_6_1, gs_6_2, "
       "gs_6_3, gs_6_4, gs_6_5, gs_6_6, gs_6_7, gs_6_8, gs_6_9, hs_6_0, hs_6_1, "
       "hs_6_2, hs_6_3, hs_6_4, hs_6_5, hs_6_6, hs_6_7, hs_6_8, hs_6_9, ds_6_0, "
       "ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, ds_6_6, ds_6_7, ds_6_8, ds_6_9, "
       "cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4, cs_6_5, cs_6_6, cs_6_7, cs_6_8, "
       "cs_6_9, lib_6_1, lib_6_2, lib_6_3, lib_6_4, lib_6_5, lib_6_6, lib_6_7, "
       "lib_6_8, lib_6_9, ms_6_5, ms_6_6, ms_6_7, ms_6_8, ms_6_9, as_6_5, as_6_6, "
       "as_6_7, as_6_8, as_6_9")                                           //
      ("Vd", "Disable validation")                                         //
      ("verify", "Verify diagnostic output using comment directives")      //
      ("Vi", "Display details about the include process.")                 //
      ("Wx", "Treat warnings as errors")                                   //
      ("Zi", "Enable debug information. Cannot be used together with -Zs") //
      ("Zpc", "Pack matrices in column-major order")                       //
      ("Zpr", "Pack matrices in row-major order")                          //
      ("Zsb", "Compute Shader Hash considering only output binary")        //
      ("Zss", "Compute Shader Hash considering source information")        //
      ("Zs",
       "Generate small PDB with just sources and compile options. Cannot be used together with -Zi") //
      ("ffinite-math-only",
       "Allow optimizations for floating-point arithmetic that assume that "
       "arguments and results are not NaNs or +-Infs.") //
      ("fno-finite-math-only",
       "Disallow optimizations for floating-point arithmetic that assume that "
       "arguments and results are not NaNs or +-Infs.") //
      ("O0", "Optimization Level 0")                    //
      ("O1", "Optimization Level 1")                    //
      ("O2", "Optimization Level 2")                    //
      ("O3", "Optimization Level 3")                    //
      ("decl-global-cb",
       "Collect all global constants outside cbuffer declarations into cbuffer "
       "GlobalCB")                                                                           //
      ("extract-entry-uniforms", "Move uniform parameters from entry point to global scope") //
      ("global-extern-by-default", "Set extern on non-static globals")                       //
      ("keep-user-macro", "Write out user defines after rewritten HLSL")                     //
      ("remove-unused-functions", "Remove unused functions and types")                       //
      ("remove-unused-globals", "Remove unused static globals and functions")                //
      ("skip-fn-body", "Translate function definitions to declarations")                     //
      ("skip-static", "Remove static functions and globals when used with -skip-fn-body")    //
      ("unchanged", "Rewrite HLSL, without changes.")                                        //
      ("fspv-debug",
       po::value<std::string>(),
       "Specify whitelist of debug info category (file -> source -> line, tool, "
       "vulkan-with-source)") //
      ("fspv-enable-maximal-reconvergence",
       "Enables the MaximallyReconvergesKHR execution mode for this module.") //
      ("fspv-entrypoint-name",
       po::value<std::string>(),
       "Specify the SPIR-V entry point name. Defaults to the HLSL entry point name.") //
      ("fspv-extension",
       po::value<std::vector<std::string>>()->multitoken(),
       "Specify SPIR-V extension permitted to use.") //
      ("fspv-flatten-resource-arrays",
       "Flatten arrays of resources so each array element takes one binding number.") //
      ("fspv-max-id",
       po::value<uint32_t>()->default_value(0x3FFFFF),
       "Set the maximum value for an id in the SPIR-V binary. Default is "
       "0x3FFFFF, which is the largest value all drivers must support.") //
      ("fspv-preserve-bindings",
       "Preserves all bindings declared within the module, even when those bindings are unused") //
      ("fspv-preserve-interface",
       "Preserves all interface variables in the entry point, even when those variables are unused") //
      ("fspv-reduce-load-size",
       "Replaces loads of composite objects to reduce memory pressure for the loads")                      //
      ("fspv-reflect", "Emit additional SPIR-V instructions to aid reflection")                            //
      ("fspv-target-env", po::value<std::string>(), "Specify the target environment: vulkan1.0 (default)") //
      ("fspv-use-legacy-buffer-matrix-order",
       "Assume the legacy matrix order (row major) when accessing raw buffers (e.g., ByteAdddressBuffer)") //
      ("fspv-use-vulkan-memory-model",
       "Generates SPIR-V modules that use the Vulkan memory model instead of GLSL450.")                      //
      ("fvk-auto-shift-bindings", "Apply fvk-*-shift to resources without an explicit register assignment.") //
      ("fvk-b-shift", po::value<uint32_t>(), "Specify Vulkan binding number shift for b-type register")      //
      ("fvk-bind-counter-heap",
       po::value<std::vector<uint32_t>>()->multitoken(),
       "Specify Vulkan binding number and set number for the counter heap.") //
      ("fvk-bind-globals",
       po::value<std::vector<uint32_t>>()->multitoken(),
       "Specify Vulkan binding number and set number for the globals cbuffer") //
      ("fvk-bind-register",
       po::value<std::vector<uint32_t>>()->multitoken(),
       "Specify Vulkan descriptor set and binding for a specific register") //
      ("fvk-bind-resource-heap",
       po::value<std::vector<uint32_t>>()->multitoken(),
       "Specify Vulkan binding number and set number for the resource heap.") //
      ("fvk-bind-sampler-heap",
       po::value<std::vector<uint32_t>>()->multitoken(),
       "Specify Vulkan binding number and set number for the sampler heap.") //
      ("fvk-invert-y",
       "Negate SV_Position.y before writing to stage output in VS/DS/GS/MS/Lib to "
       "accommodate Vulkan’s coordinate system.")                                                       //
      ("fvk-s-shift", po::value<uint32_t>(), "Specify Vulkan binding number shift for s-type register") //
      ("fvk-support-nonzero-base-instance",
       "Follow Vulkan spec to use gl_BaseInstance as the first vertex instance, "
       "which makes SV_InstanceID = gl_InstanceIndex - gl_BaseInstance (without "
       "this option, SV_InstanceID = gl_InstanceIndex)") //
      ("fvk-support-nonzero-base-vertex",
       "Follow Vulkan spec to use gl_BaseVertex as the first vertex, which makes "
       "SV_VertexID = gl_VertexIndex - gl_BaseVertex (without this option, "
       "SV_VertexID = gl_VertexIndex)")                                                                 //
      ("fvk-t-shift", po::value<uint32_t>(), "Specify Vulkan binding number shift for t-type register") //
      ("fvk-u-shift", po::value<uint32_t>(), "Specify Vulkan binding number shift for u-type register") //
      ("fvk-use-dx-layout", "Use DirectX memory layout for Vulkan resources")                           //
      ("fvk-use-dx-position-w",
       "Reciprocate SV_Position.w after reading from stage input in PS to "
       "accommodate the difference between Vulkan and DirectX")                                   //
      ("fvk-use-gl-layout", "Use strict OpenGL std140/std430 memory layout for Vulkan resources") //
      ("fvk-use-scalar-layout", "Use scalar memory layout for Vulkan resources")                  //
      ("metal", "Generate Metal code")                                                            //
      ("spirv", "Generate SPIR-V code")                                                           //
      ("P", "Preprocess to file")                                                                 //
      ("Qembed_debug", "Embed PDB in shader container (must be used with /Zi)")                   //
      ("Qsource_in_debug_module", "Embed source code in PDB")                                     //
      ("Qstrip_debug",
       "Strip debug information from 4_0+ shader bytecode (must be used with /Fo )")                      //
      ("Qstrip_rootsignature", "Strip root signature data from shader bytecode (must be used with /Fo )") //
      ("setrootsignature", "Attach root signature to shader bytecode")                                    //
      ("verifyrootsignature", "Verify shader bytecode with root signature");

    auto vm = po::variables_map{};
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    auto options = cyclonite::tools::Options{};

    if (vm.contains("target-platform")) {
        auto s = vm["target-platform"].as<std::string>();
        if (s == "nix") {
            options.platform = cyclonite::tools::TargetPlatform::Nix;
        } else if (s == "windows") {
            options.platform = cyclonite::tools::TargetPlatform::Windows;
        } else if (s == "all") {
            options.platform = cyclonite::tools::TargetPlatform::All;
        } else {
            throw std::invalid_argument("Unknown platform: " + s);
        }
    }

    if (vm.contains("target-gapi")) {
        auto s = vm["target-gapi"].as<std::string>();
        if (s == "vulkan") {
            options.gapi = cyclonite::tools::TargetGAPI::Vulkan;
        } else if (s == "d3d12") {
            options.gapi = cyclonite::tools::TargetGAPI::D3D12;
        } else if (s == "all") {
            options.gapi = cyclonite::tools::TargetGAPI::All;
        } else {
            throw std::invalid_argument("Unknown gapi: " + s);
        }
    }

    options.noWarningOnUnusedDriverArgs = vm.contains("Qunused-arguments");
    options.allResourcesBound = vm.contains("all-resources-bound");
    options.autoBindingSpace = vm.contains("auto-binding-space");

    if (vm.contains("default-linkage")) {
        auto s = vm["default-linkage"].as<std::string_view>();
        if (s == "internal") {
            options.linkage = cyclonite::tools::LinkageType::Internal;
        } else if (s == "external") {
            options.linkage = cyclonite::tools::LinkageType::External;
        } else {
            throw std::invalid_argument("Unknown linkage type");
        }
    }

    if (vm.contains("encoding")) {
        auto s = vm["encoding"].as<std::string>();
        if (s == "utf-8") {
            options.encoding = cyclonite::tools::Encoding::Utf8;
        } else if (s == "utf32(*nix)") {
            options.encoding = cyclonite::tools::Encoding::Utf32Nix;
        } else if (s == "utf16(win)") {
            options.encoding = cyclonite::tools::Encoding::Utf16Win;
        } else if (s == "wide") {
            options.encoding = cyclonite::tools::Encoding::Wide;
        } else {
            throw std::invalid_argument("Unknown encoding type");
        }
    }

    if (vm.contains("D")) {
        auto macros = vm["D"].as<std::vector<std::string>>();
        for (auto& m : macros) {
            auto r = m | std::views::split('=') | std::views::transform([](auto&& subs) -> std::string_view {
                         return std::string_view(subs.begin(), subs.end());
                     });

            auto p = std::pair<std::wstring, std::wstring>{};
            auto conv = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};

            for (auto i = size_t{ 0 }; i < 2; i++) {
                auto it = std::next(std::ranges::begin(r), i);
                if (it != std::ranges::end(r)) {
                    auto sv = *it;
                    if (i == 0) {
                        p.first = conv.from_bytes(sv.data(), sv.data() + sv.size());
                    } else {
                        p.second = conv.from_bytes(sv.data(), sv.data() + sv.size());
                    }
                }
            }

            options.definitions.insert(p);
        }
    } // macro definitions

    options.enable16bitTypes = vm.contains("enable-16bit-types");
    options.enableLifetimeMarkers = vm.contains("enable-lifetime-markers");
    options.exportShadersOnly = vm.contains("export-shaders-only");

    auto entryPointName = std::string();
    if (vm.contains("fspv-entrypoint-name")) {
        auto s = vm["fspv-entrypoint-name"].as<std::string>();
        auto conv = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
        entryPointName = s;
        options.entryPointName = conv.from_bytes(s.data(), s.data() + s.size());
    } else if (vm.contains("E")) {
        auto s = vm["E"].as<std::string>();
        auto conv = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
        entryPointName = s;
        options.entryPointName = conv.from_bytes(s.data(), s.data() + s.size());
    }

    if (vm.contains("fdiagnostics-format")) {
        auto s = vm["fdiagnostics-format"].as<std::string>();
        if (s == "clang") {
            options.diagnosticMessageFormat = cyclonite::tools::DiagnosticMessageFormat::Clang;
        } else if (s == "msvc") {
            options.diagnosticMessageFormat = cyclonite::tools::DiagnosticMessageFormat::Msvc;
        } else if (s == "mdvc-fallback") {
            options.diagnosticMessageFormat = cyclonite::tools::DiagnosticMessageFormat::MdvcFallBack;
        } else if (s == "vi") {
            options.diagnosticMessageFormat = cyclonite::tools::DiagnosticMessageFormat::Vi;
        } else {
            throw std::invalid_argument("Unknown diagnostic format type");
        }
    }

    if (vm.contains("fdiagnostics-show-option") && !vm.contains("fno-diagnostics-show-option")) {
        options.showDiagnostics = cyclonite::tools::OptionValue::Enable;
    } else if (vm.contains("fno-diagnostics-show-option") && !vm.contains("fdiagnostics-show-option")) {
        options.showDiagnostics = cyclonite::tools::OptionValue::Disable;
    }

    options.disableLocTracking = vm.contains("fdisable-loc-tracking");
    options.legacyMacroExpansion = vm.contains("flegacy-macro-expansion");
    options.newInliningBehavior = vm.contains("fnew-inlining-behavior");
    options.forceRootsigVersion = vm.contains("force-rootsig-ver");

    if (vm.contains("Fd")) {
        options.debugFileName = vm["Fd"].as<std::string>();
    }

    if (vm.contains("Fe")) {
        options.warninAndErrorsFileName = vm["Fe"].as<std::string>();
    }

    if (vm.contains("Fi")) {
        options.preprocessedCodeFileName = vm["Fi"].as<std::string>();
    }

    if (vm.contains("Fo")) {
        options.outputFileName = vm["Fo"].as<std::string>();
    }

    auto reflectionFileName = std::string{};
    if (vm.contains("Fre")) {
        reflectionFileName = vm["Fre"].as<std::string>();
    }

    if (vm.contains("Frs")) {
        options.rootsigFileName = vm["Frs"].as<std::string>();
    }

    if (vm.contains("Fsh")) {
        options.shaderHashFileName = vm["Fsh"].as<std::string>();
    }

    options.backwardCompatibilityMode = vm.contains("Gec");
    options.enableStrictMode = vm.contains("Ges");
    options.forceIEEEStrictness = vm.contains("Gis");
    options.ignoreLineDirectives = vm.contains("ignore-line-directives");

    if (vm.contains("HV")) {
        auto v = vm["HV"].as<uint16_t>();
        switch (v) {
            case 2016:
                options.hv = cyclonite::tools::HV::_2016;
                break;
            case 2017:
                options.hv = cyclonite::tools::HV::_2017;
                break;
            case 2018:
                options.hv = cyclonite::tools::HV::_2018;
                break;
            case 2021:
                options.hv = cyclonite::tools::HV::_2021;
                break;
            default:
                throw std::invalid_argument("Unknown hlsl version");
        }
    }

    if (vm.contains("I")) {
        auto incldirs = vm["I"].as<std::vector<std::string>>();
        for (auto& i : incldirs) {
            auto conv = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
            options.includeDirs.emplace_back(conv.from_bytes(i.data(), i.data() + i.size()));
        }
    }

    options.addsInstructionNummbersToAssemblerListing = vm.contains("Ni");
    options.noWarnings = vm.contains("no-warnings");

    if (vm.contains("Od")) {
        options.optimization = cyclonite::tools::Optimization::Disable;
    } else if (vm.contains("O0")) {
        options.optimization = cyclonite::tools::Optimization::Level0;
    } else if (vm.contains("O1")) {
        options.optimization = cyclonite::tools::Optimization::Level1;
    } else if (vm.contains("O2")) {
        options.optimization = cyclonite::tools::Optimization::Level2;
    } else if (vm.contains("O3")) {
        options.optimization = cyclonite::tools::Optimization::Level3;
    }

    options.packOptimized = vm.contains("pack-optimized");
    options.packPrefixStable = vm.contains("pack-prefix-stable");
    options.resMayAlias = vm.contains("res-may-alias");

    if (vm.contains("rootsig-define")) {
        auto conv = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
        options.rootSigDefine = conv.from_bytes(vm["rootsig-define"].as<std::string>());
    }

    if (vm.contains("T")) {
        auto profile = vm["T"].as<std::string>();
        if (profile == "ps_6_0") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_0;
        } else if (profile == "ps_6_1") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_1;
        } else if (profile == "ps_6_2") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_2;
        } else if (profile == "ps_6_3") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_3;
        } else if (profile == "ps_6_4") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_4;
        } else if (profile == "ps_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_5;
        } else if (profile == "ps_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_6;
        } else if (profile == "ps_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_7;
        } else if (profile == "ps_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_8;
        } else if (profile == "ps_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::ps_6_9;
        } else if (profile == "vs_6_0") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_0;
        } else if (profile == "vs_6_1") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_1;
        } else if (profile == "vs_6_2") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_2;
        } else if (profile == "vs_6_3") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_3;
        } else if (profile == "vs_6_4") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_4;
        } else if (profile == "vs_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_5;
        } else if (profile == "vs_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_6;
        } else if (profile == "vs_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_7;
        } else if (profile == "vs_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_8;
        } else if (profile == "vs_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::vs_6_9;
        } else if (profile == "gs_6_0") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_0;
        } else if (profile == "gs_6_1") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_1;
        } else if (profile == "gs_6_2") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_2;
        } else if (profile == "gs_6_3") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_3;
        } else if (profile == "gs_6_4") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_4;
        } else if (profile == "gs_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_5;
        } else if (profile == "gs_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_6;
        } else if (profile == "gs_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_7;
        } else if (profile == "gs_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_8;
        } else if (profile == "gs_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::gs_6_9;
        } else if (profile == "hs_6_0") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_0;
        } else if (profile == "hs_6_1") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_1;
        } else if (profile == "hs_6_2") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_2;
        } else if (profile == "hs_6_3") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_3;
        } else if (profile == "hs_6_4") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_4;
        } else if (profile == "hs_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_5;
        } else if (profile == "hs_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_6;
        } else if (profile == "hs_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_7;
        } else if (profile == "hs_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_8;
        } else if (profile == "hs_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::hs_6_9;
        } else if (profile == "ds_6_0") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_0;
        } else if (profile == "ds_6_1") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_1;
        } else if (profile == "ds_6_2") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_2;
        } else if (profile == "ds_6_3") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_3;
        } else if (profile == "ds_6_4") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_4;
        } else if (profile == "ds_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_5;
        } else if (profile == "ds_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_6;
        } else if (profile == "ds_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_7;
        } else if (profile == "ds_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_8;
        } else if (profile == "ds_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::ds_6_9;
        } else if (profile == "cs_6_0") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_0;
        } else if (profile == "cs_6_1") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_1;
        } else if (profile == "cs_6_2") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_2;
        } else if (profile == "cs_6_3") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_3;
        } else if (profile == "cs_6_4") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_4;
        } else if (profile == "cs_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_5;
        } else if (profile == "cs_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_6;
        } else if (profile == "cs_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_7;
        } else if (profile == "cs_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_8;
        } else if (profile == "cs_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::cs_6_9;
        } else if (profile == "lib_6_1") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_1;
        } else if (profile == "lib_6_2") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_2;
        } else if (profile == "lib_6_3") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_3;
        } else if (profile == "lib_6_4") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_4;
        } else if (profile == "lib_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_5;
        } else if (profile == "lib_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_6;
        } else if (profile == "lib_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_7;
        } else if (profile == "lib_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_8;
        } else if (profile == "lib_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::lib_6_9;
        } else if (profile == "ms_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::ms_6_5;
        } else if (profile == "ms_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::ms_6_6;
        } else if (profile == "ms_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::ms_6_7;
        } else if (profile == "ms_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::ms_6_8;
        } else if (profile == "ms_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::ms_6_9;
        } else if (profile == "as_6_5") {
            options.targetProfile = cyclonite::tools::TargetProfile::as_6_5;
        } else if (profile == "as_6_6") {
            options.targetProfile = cyclonite::tools::TargetProfile::as_6_6;
        } else if (profile == "as_6_7") {
            options.targetProfile = cyclonite::tools::TargetProfile::as_6_7;
        } else if (profile == "as_6_8") {
            options.targetProfile = cyclonite::tools::TargetProfile::as_6_8;
        } else if (profile == "as_6_9") {
            options.targetProfile = cyclonite::tools::TargetProfile::as_6_9;
        }
    }

    options.disableValidation = vm.contains("Vd");
    options.verify = vm.contains("verify");
    options.disableIncludeProcessingDetails = vm.contains("Vi");
    options.warningsAsErrors = vm.contains("Wx");
    options.enableDebugInformation = vm.contains("Zi");

    if (vm.contains("Zpc") && !vm.contains("Zpr")) {
        options.matrixLayout = cyclonite::tools::MatrixLayout::ColumnMajor;
    } else if (!vm.contains("Zpc") && vm.contains("Zpr")) {
        options.matrixLayout = cyclonite::tools::MatrixLayout::RowMajor;
    }

    options.shaderHashBasedOnBinary = vm.contains("Zsb");
    options.shaderHashBasedOnSource = vm.contains("Zss");
    options.generateSmallPDB = vm.contains("Zs");

    if (vm.contains("ffinite-math-only") && !vm.contains("fno-finite-math-only")) {
        options.finiteMathOnly = cyclonite::tools::OptionValue::Enable;
    } else if (!vm.contains("ffinite-math-only") && vm.contains("fno-finite-math-only")) {
        options.finiteMathOnly = cyclonite::tools::OptionValue::Disable;
    }

    if (vm.contains("fspv-debug")) {
        auto s = vm["fspv-debug"].as<std::string>();
        if (s == "line") {
            options.spvDebug = cyclonite::tools::SpvDebug::Line;
        } else if (s == "source") {
            options.spvDebug = cyclonite::tools::SpvDebug::Source;
        } else if (s == "file") {
            options.spvDebug = cyclonite::tools::SpvDebug::File;
        } else if (s == "vulkan-with-source") {
            options.spvDebug = cyclonite::tools::SpvDebug::VulkanWithSource;
        } else {
            throw std::invalid_argument("Unknown spv-debug option value");
        }
    }

    options.spvEnableMaximalReconvergence = vm.contains("fspv-enable-maximal-reconvergence");

    if (vm.contains("fspv-extension")) {
        auto extensions = vm["fspv-extension"].as<std::vector<std::string>>();
        auto conv = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};

        for (auto& ext : extensions) {
            options.spvExtensions.emplace_back(conv.from_bytes(ext.data(), ext.data() + ext.size()));
        }
    }

    options.spvFlattenResourceArrays = vm.contains("fspv-flatten-resource-arrays");
    options.spvPreserveBindings = vm.contains("fspv-preserve-bindings");
    options.spvPreserveInterface = vm.contains("fspv-preserve-interface");
    options.spvReduceLoadSize = vm.contains("fspv-reduce-load-size");
    options.spvReflect = vm.contains("fspv-reflect");
    options.spvMaxId = vm["fspv-max-id"].as<uint32_t>();
    options.spvMaxIdStr = std::to_wstring(options.spvMaxId);

    if (vm.contains("fspv-target-env")) {
        auto s = vm["fspv-target-env"].as<std::string>();
        if (s == "vulkan1.0") {
            options.targetEnv = cyclonite::tools::SpvTargetEnv::Vulkan_1_0;
        } else if (s == "vulkan1.1") {
            options.targetEnv = cyclonite::tools::SpvTargetEnv::Vulkan_1_1;
        } else if (s == "vulkan1.2") {
            options.targetEnv = cyclonite::tools::SpvTargetEnv::Vulkan_1_2;
        } else if (s == "vulkan1.3") {
            options.targetEnv = cyclonite::tools::SpvTargetEnv::Vulkan_1_3;
        } else if (s == "vulkan1.1_spv1.4") {
            options.targetEnv = cyclonite::tools::SpvTargetEnv::Vulkan_1_1_Spirv_1_4;
        } else if (s == "universal1.5") {
            options.targetEnv = cyclonite::tools::SpvTargetEnv::Universal_1_5;
        } else {
            throw std::invalid_argument("Unknown spv-target-env option value");
        }
    }

    options.spvUseLegacyBufferMatrixOrder = vm.contains("fspv-use-legacy-buffer-matrix-order");
    options.spvUseVulkanMemoryModel = vm.contains("fspv-use-vulkan-memory-model");
    options.vkAutoShiftBindings = vm.contains("fvk-auto-shift-bindings");

    if (vm.contains("fvk-b-shift")) {
        options.vkBShift = vm["fvk-b-shift"].as<uint32_t>();
    }
    if (vm.contains("fvk-s-shift")) {
        options.vkSShift = vm["fvk-s-shift"].as<uint32_t>();
    }
    if (vm.contains("fvk-t-shift")) {
        options.vkTShift = vm["fvk-t-shift"].as<uint32_t>();
    }
    if (vm.contains("fvk-u-shift")) {
        options.vkUShift = vm["fvk-u-shift"].as<uint32_t>();
    }

    if (vm.contains("fvk-bind-counter-heap")) {
        auto v = vm["fvk-bind-counter-heap"].as<std::vector<uint32_t>>();
        for (auto i = size_t{ 0 }; i < v.size() && i < options.vkBindCounterHeap.size(); i++) {
            options.vkBindCounterHeap[i] = v[i];
        }
        options.vkBindCounterHeapStr =
          std::format(L"-fvk-bind-counter-heap {0} {1}", options.vkBindCounterHeap[0], options.vkBindCounterHeap[1]);
    }

    if (vm.contains("fvk-bind-globals")) {
        auto v = vm["fvk-bind-globals"].as<std::vector<uint32_t>>();
        for (auto i = size_t{ 0 }; i < v.size() && i < options.vkBindGlobals.size(); i++) {
            options.vkBindGlobals[i] = v[i];
        }
        options.vkBindGlobalsStr =
          std::format(L"-fvk-bind-globals {0} {1}", options.vkBindGlobals[0], options.vkBindGlobals[1]);
    }

    if (vm.contains("fvk-bind-register")) {
        auto v = vm["fvk-bind-register"].as<std::vector<uint32_t>>();
        for (auto i = size_t{ 0 }; i < v.size() && i < options.vkBindRegister.size(); i++) {
            options.vkBindRegister[i] = v[i];
        }
        options.vkBindRegisterStr =
          std::format(L"-fvk-bind-register {0} {1}", options.vkBindRegister[0], options.vkBindRegister[1]);
    }

    if (vm.contains("fvk-bind-resource-heap")) {
        auto v = vm["fvk-bind-resource-heap"].as<std::vector<uint32_t>>();
        for (auto i = size_t{ 0 }; i < v.size() && i < options.vkBindResourceHeap.size(); i++) {
            options.vkBindResourceHeap[i] = v[i];
        }
        options.vkBindResourceHeapStr =
          std::format(L"-fvk-bind-resource-heap {0} {1}", options.vkBindResourceHeap[0], options.vkBindResourceHeap[1]);
    }

    if (vm.contains("fvk-bind-sampler-heap")) {
        auto v = vm["fvk-bind-sampler-heap"].as<std::vector<uint32_t>>();
        for (auto i = size_t{ 0 }; i < v.size() && i < options.vkBindSamplerHeap.size(); i++) {
            options.vkBindSamplerHeap[i] = v[i];
        }
        options.vkBindSamplerHeapStr =
          std::format(L"-fvk-bind-sampler-heap {0} {1}", options.vkBindSamplerHeap[0], options.vkBindSamplerHeap[1]);
    }

    auto source = std::wstring{};
    if (vm.contains("source")) {
        auto s = vm["source"].as<std::string>();
        auto conv = std::wstring_convert<std::codecvt_utf8<wchar_t>>{};
        source = conv.from_bytes(s.data(), s.data() + s.size());
    }

    if (source.empty()) {
        throw std::runtime_error("source file is not defined");
    }

    options.spirv = vm.contains("spirv");
    options.metal = vm.contains("metal");

    auto compilerOutput = cyclonite::tools::CompilerOutput{};
    auto compiler = cyclonite::tools::Compiler{};

    auto compileToSpv = static_cast<bool>(options.spirv);
    auto compileToMetal = static_cast<bool>(options.metal);
    options.spirv = false;
    options.metal = false;

    compiler.collectReflection(source, options, compilerOutput.reflectionData());
    if (!reflectionFileName.empty()) {
        throw std::runtime_error("not implemented");
    }

    options.spirv = compileToSpv;
    options.metal = compileToMetal;

    if (options.spirv) {
        compiler.compileToSpirv(source, options, compilerOutput.spirvModule());
    }

    auto path = std::filesystem::path(options.outputFileName);
    auto shaderModuleBinary = cyclonite::shared::ShaderModuleBinary{};
    shaderModuleBinary.infoBlock.entryPoint = entryPointName;
    shaderModuleBinary.infoBlock.targetProfile = static_cast<uint32_t>(metrix::value_cast(options.targetProfile));
    shaderModuleBinary.infoBlock.name = path.filename().string();

    auto gen = boost::uuids::random_generator_mt19937{};
    auto uuid = boost::uuids::uuid{ gen() };
    shaderModuleBinary.infoBlock.uuid = boost::uuids::to_string(uuid);

    auto shaderModuleBlockCount = uint32_t{ 3 }; // info block + spir-v + reflection
    shaderModuleBinary.blockHeaders.reserve(shaderModuleBlockCount);

    auto headerSerializer =  cyclonite::shared::Serializer{
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBlockHeader::baseOffset>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBlockHeader::blockOffset>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBlockHeader::size>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBlockHeader::id>()
    };

    auto emptyHeader = cyclonite::shared::ShaderModuleBlockHeader{}; // to define size (all headers has the same size)
    auto baseOffset = headerSerializer.computeSize(emptyHeader) * shaderModuleBlockCount
        + sizeof(shaderModuleBlockCount) + sizeof(cyclonite::shared::SHADER_MODULE_MAGIC_NUMBER);

    auto blockOffset = uint64_t{ 0 };

    // info block:
    auto infoBlockSerializer = cyclonite::shared::Serializer{
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderInfoBlock::entryPoint>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderInfoBlock::targetProfile>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderInfoBlock::name>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderInfoBlock::uuid>()
    };

    auto& infoBlockHeader = shaderModuleBinary.blockHeaders.emplace_back();
    infoBlockHeader.id = cyclonite::shared::SHADER_MODULE_INFO_BLOCK;
    infoBlockHeader.baseOffset = baseOffset;
    infoBlockHeader.blockOffset = blockOffset;
    infoBlockHeader.size = infoBlockSerializer.computeSize(shaderModuleBinary.infoBlock);
    blockOffset += infoBlockHeader.size;

    // spir-v blocK:
    auto& spirvBlockHeader = shaderModuleBinary.blockHeaders.emplace_back();
    spirvBlockHeader.id = cyclonite::shared::SHADER_MODULE_SPIRV_BLOCK;
    spirvBlockHeader.baseOffset = baseOffset;
    spirvBlockHeader.blockOffset = blockOffset;
    spirvBlockHeader.size = sizeof(uint32_t) + compilerOutput.spirvModule().size() * sizeof(uint32_t);
    baseOffset += spirvBlockHeader.size;

    // reflection block:
    auto reflectionSerializer = cyclonite::shared::Serializer{
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderReflectionData::version>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderReflectionData::generatorName>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderReflectionData::boundResources,
                                           &cyclonite::shared::BoundResource::getResourceData>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderReflectionData::constantBuffers,
                                           &cyclonite::shared::ConstantBufferReflection::getBufferData>()
    };

    auto& reflectionBlockHeader = shaderModuleBinary.blockHeaders.emplace_back();
    reflectionBlockHeader.id = cyclonite::shared::SHADER_MODULE_REFLECTION_BLOCK;
    reflectionBlockHeader.baseOffset = baseOffset;
    reflectionBlockHeader.blockOffset = blockOffset;
    reflectionBlockHeader.size = reflectionSerializer.computeSize(compilerOutput.reflectionData());

    shaderModuleBinary.spirvCode = compilerOutput.spirvModule();
    shaderModuleBinary.reflectionData = compilerOutput.reflectionData();

    auto serializer = cyclonite::shared::Serializer{
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::getMagicNumber>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::blockHeaders,
                                           &cyclonite::shared::ShaderModuleBlockHeader::getBlockHeaderData>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::infoBlock,
                                           &cyclonite::shared::ShaderInfoBlock::entryPoint>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::infoBlock,
                                           &cyclonite::shared::ShaderInfoBlock::targetProfile>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::infoBlock,
                                           &cyclonite::shared::ShaderInfoBlock::name>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::infoBlock,
                                           &cyclonite::shared::ShaderInfoBlock::uuid>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::spirvCode>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::reflectionData,
                                           &cyclonite::shared::ShaderReflectionData::version>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::reflectionData,
                                           &cyclonite::shared::ShaderReflectionData::generatorName>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::reflectionData,
                                           &cyclonite::shared::ShaderReflectionData::boundResources,
                                           &cyclonite::shared::BoundResource::getResourceData>(),
        cyclonite::shared::makeAccessChain<&cyclonite::shared::ShaderModuleBinary::reflectionData,
                                           &cyclonite::shared::ShaderReflectionData::constantBuffers,
                                           &cyclonite::shared::ConstantBufferReflection::getBufferData>()
    };

    auto binaryWriter = cyclonite::shared::BinaryStreamWriter{ path, cyclonite::shared::Endian::Little };
    serializer.serialize(shaderModuleBinary, binaryWriter);

    return 0;
}

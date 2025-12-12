
#include <boost/program_options.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    auto desc = po::options_description{ "shader-compiler options" };
    desc.add_options()("help", "Produce help message")("version", "Display compiler version information")(
      "Qunused-arguments", "Don’t emit warning for unused driver arguments")("all-resources-bound",
                                                                             "Enables aggressive flattening")(
      "auto-binding-space", "Set auto binding space - enables auto resource binding in libraries")(
      "default-linkage",
      "Set default linkage for non-shader functions when compiling or linking to "
      "a library target (internal, external)")("D", "Define macro")(
      "enable-16bit-types",
      "Enable 16bit types and disable min precision types. Available in HLSL "
      "2018 and shader model 6.2")("enable-lifetime-markers", "Enable generation of lifetime markers")(
      "encoding",
      "Set default encoding for source inputs and text outputs "
      "(utf8	utf16(win)	utf32(*nix)	wide) default=utf8")(
      "export-shaders-only", "Only export shaders when compiling a library.")("E", "Entry point name")(
      "fdiagnostics-format",
      "Select diagnostic message format. Supported values: clang, msvc, "
      "mdvc-fallback, vi")("fdiagnostics-show-option", "Print option name with mappable diagnostics")(
      "fdisable-loc-tracking",
      "Disable source location tracking in IR. This will break diagnostic "
      "generation for late validation. (Ignored if /Zi is passed)")(
      "Fd",
      "Write debug information to the given file, or automatically named file in "
      "directory")("Fe", "Output warnings and errors to the given file")(
      "Fh", "Output header file containing object code")("Fi", "Set preprocess output file name (with /P)")(
      "flegacy-macro-expansion",
      "Expand the operands before performing token-pasting operation (fxc "
      "behavior)")("fnew-inlining-behavior",
                   "Experimental option to use heuristics-driven late inlining "
                   "and disable alwaysinline annotation for library shaders")(
      "fno-diagnostics-show-option", "Do not print option name with mappable diagnostics")(
      "force-rootsig-ver", "force root signature version (rootsig_1_1 if omitted)")("Fo", "Output file")(
      "Fre", "Output reflection to the given file")("Frs", "Output root signature to the given file")(
      "Fsh", "Output shader hash to the given file")("Gec", "Enable backward compatibility mode")(
      "Ges", "Enable strict mode")("Gfa", "Avoid flow control constructs")("Gfp", "Prefer flow control constructs")(
      "Gis", "Force IEEE strictness")("HV", "HLSL version (2016, 2017, 2018, 2021). Default is 2021")(
      "ignore-line-directives", "Ignore line directives")("I", "Add directory to include search path")(
      "Ni", "Output instruction numbers in assembly listings")("no-warnings",
                                                               "Suppress warnings")("Od", "Disable optimizations")(
      "pack-optimized",
      "Optimize signature packing assuming identical signature provided for each "
      "connecting stage")("pack-prefix-stable",
                          "(default) Pack signatures preserving prefix-stable property - appended "
                          "elements will not disturb placement of prior elements")(
      "res-may-alias", "Assume that UAVs/SRVs may alias")("rootsig-define", "Read root signature from a #define")(
      "T",
      "Set target profile. profile: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, "
      "ps_6_5, ps_6_6, ps_6_7, ps_6_8, ps_6_9, vs_6_0, vs_6_1, vs_6_2, vs_6_3, "
      "vs_6_4, vs_6_5, vs_6_6, vs_6_7, vs_6_8, vs_6_9, gs_6_0, gs_6_1, gs_6_2, "
      "gs_6_3, gs_6_4, gs_6_5, gs_6_6, gs_6_7, gs_6_8, gs_6_9, hs_6_0, hs_6_1, "
      "hs_6_2, hs_6_3, hs_6_4, hs_6_5, hs_6_6, hs_6_7, hs_6_8, hs_6_9, ds_6_0, "
      "ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, ds_6_6, ds_6_7, ds_6_8, ds_6_9, "
      "cs_6_0, cs_6_1, cs_6_2, cs_6_3, cs_6_4, cs_6_5, cs_6_6, cs_6_7, cs_6_8, "
      "cs_6_9, lib_6_1, lib_6_2, lib_6_3, lib_6_4, lib_6_5, lib_6_6, lib_6_7, "
      "lib_6_8, lib_6_9, ms_6_5, ms_6_6, ms_6_7, ms_6_8, ms_6_9, as_6_5, as_6_6, "
      "as_6_7, as_6_8, as_6_9")("Vd", "Disable validation")("verify",
                                                            "Verify diagnostic output using comment directives")(
      "Vi", "Display details about the include process.")("Wx", "Treat warnings as errors")(
      "Zi", "Enable debug information. Cannot be used together with -Zs")("Zpc", "Pack matrices in column-major order")(
      "Zpr", "Pack matrices in row-major order")("Zsb", "Compute Shader Hash considering only output binary")(
      "Zss", "Compute Shader Hash considering source information")(
      "Zs",
      "Generate small PDB with just sources and compile options. Cannot be used "
      "together with -Zi")("ffinite-math-only",
                           "Allow optimizations for floating-point arithmetic that assume that "
                           "arguments and results are not NaNs or +-Infs.")(
      "fno-finite-math-only",
      "Disallow optimizations for floating-point arithmetic that assume that "
      "arguments and results are not NaNs or +-Infs.")("O0", "Optimization Level 0")("O1", "Optimization Level 1")(
      "O2", "Optimization Level 2")("O3", "Optimization Level 3")(
      "decl-global-cb",
      "Collect all global constants outside cbuffer declarations into cbuffer "
      "GlobalCB")("extract-entry-uniforms", "Move uniform parameters from entry point to global scope")(
      "global-extern-by-default", "Set extern on non-static globals")("keep-user-macro",
                                                                      "Write out user defines after rewritten HLSL")(
      "line-directive", "Add line directive")("remove-unused-functions", "Remove unused functions and types")(
      "remove-unused-globals",
      "Remove unused static globals and functions")("skip-fn-body", "Translate function definitions to declarations")(
      "skip-static", "Remove static functions and globals when used with -skip-fn-body")(
      "unchanged", "Rewrite HLSL, without changes.")(
      "fspv-debug",
      "Specify whitelist of debug info category (file -> source -> line, tool, "
      "vulkan-with-source)")("fspv-enable-maximal-reconvergence",
                             "Enables the MaximallyReconvergesKHR execution mode for this module.")(
      "fspv-entrypoint-name",
      "Specify the SPIR-V entry point name. Defaults to the HLSL entry point "
      "name.")("fspv-extension", "Specify SPIR-V extension permitted to use.")(
      "fspv-flatten-resource-arrays",
      "Flatten arrays of resources so each array element takes one binding "
      "number.")("fspv-flatten-resource-arrays",
                 "Flatten arrays of resources so each array element takes one "
                 "binding number.")("fspv-max-id",
                                    "Set the maximum value for an id in the SPIR-V binary. Default is "
                                    "0x3FFFFF, which is the largest value all drivers must support.")(
      "fspv-preserve-bindings",
      "Preserves all bindings declared within the module, even when those "
      "bindings are unused")("fspv-preserve-interface",
                             "Preserves all interface variables in the entry "
                             "point, even when those variables are unused")(
      "fspv-reduce-load-size",
      "Replaces loads of composite objects to reduce memory pressure for the "
      "loads")("fspv-reflect", "Emit additional SPIR-V instructions to aid reflection")(
      "fspv-target-env", "Specify the target environment: vulkan1.0 (default)")(
      "fspv-use-legacy-buffer-matrix-order",
      "Assume the legacy matrix order (row major) when accessing raw buffers "
      "(e.g., ByteAdddressBuffer)")("fspv-use-vulkan-memory-model",
                                    "Generates SPIR-V modules that use the "
                                    "Vulkan memory model instead of GLSL450.")(
      "fvk-auto-shift-bindings", "Apply fvk-*-shift to resources without an explicit register assignment.")(
      "fvk-b-shift", "Specify Vulkan binding number shift for b-type register")(
      "fvk-bind-counter-heap", "Specify Vulkan binding number and set number for the counter heap.")(
      "fvk-bind-globals", "Specify Vulkan binding number and set number for the globals cbuffer")(
      "fvk-bind-register", "Specify Vulkan descriptor set and binding for a specific register")(
      "fvk-bind-resource-heap", "Specify Vulkan binding number and set number for the resource heap.")(
      "fvk-bind-sampler-heap", "Specify Vulkan binding number and set number for the sampler heap.")(
      "fvk-invert-y",
      "Negate SV_Position.y before writing to stage output in VS/DS/GS/MS/Lib to "
      "accommodate Vulkan’s coordinate system.")("fvk-s-shift",
                                                 "Specify Vulkan binding number shift for s-type register")(
      "fvk-support-nonzero-base-instance",
      "Follow Vulkan spec to use gl_BaseInstance as the first vertex instance, "
      "which makes SV_InstanceID = gl_InstanceIndex - gl_BaseInstance (without "
      "this option, SV_InstanceID = gl_InstanceIndex)")(
      "fvk-support-nonzero-base-vertex",
      "Follow Vulkan spec to use gl_BaseVertex as the first vertex, which makes "
      "SV_VertexID = gl_VertexIndex - gl_BaseVertex (without this option, "
      "SV_VertexID = gl_VertexIndex)")("fvk-t-shift", "Specify Vulkan binding number shift for t-type register")(
      "fvk-u-shift", "Specify Vulkan binding number shift for u-type register")(
      "fvk-use-dx-layout", "Use DirectX memory layout for Vulkan resources")(
      "fvk-use-dx-position-w",
      "Reciprocate SV_Position.w after reading from stage input in PS to "
      "accommodate the difference between Vulkan and DirectX")(
      "fvk-use-gl-layout", "Use strict OpenGL std140/std430 memory layout for Vulkan resources")(
      "fvk-use-scalar-layout", "Use scalar memory layout for Vulkan resources")("metal", "Generate Metal code")(
      "Oconfig",
      "Specify a comma-separated list of SPIRV-Tools passes to customize "
      "optimization configuration (see http://khr.io/hlsl2spirv#optimization)")("spirv", "Generate SPIR-V code")(
      "P", "Preprocess to file")("Qembed_debug", "Embed PDB in shader container (must be used with /Zi)")(
      "Qsource_in_debug_module", "Embed source code in PDB")("Qstrip_debug",
                                                             "Strip debug information from 4_0+ shader "
                                                             "bytecode (must be used with /Fo )")(
      "Qstrip_rootsignature", "Strip root signature data from shader bytecode (must be used with /Fo )")(
      "setrootsignature", "Attach root signature to shader bytecode")(
      "verifyrootsignature", "Verify shader bytecode with root signature")("Wno",
                                                                           "Enable/Disable the specified warning");

    auto vm = po::variables_map{};
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help")) {
        std::cout << desc << "\n";
        return 0;
    }
    // TODO:: save cmd into options struct

    return 0;
}

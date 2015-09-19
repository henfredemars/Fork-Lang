//===- llvm-build generated file --------------------------------*- C++ -*-===//
//
// Component Library Depenedency Table
//
// Automatically generated file, do not edit!
//
//===----------------------------------------------------------------------===//

struct AvailableComponent {
  /// The name of the component.
  const char *Name;

  /// The name of the library for this component (or NULL).
  const char *Library;

  /// Whether the component is installed.
  bool IsInstalled;

  /// The list of libraries required when linking this component.
  const char *RequiredLibraries[16];
} AvailableComponents[53] = {
  { "all", 0, 1, { "interpreter", "lineeditor", "debuginfodwarf", "engine", "passes", "nativecodegen", "orcjit", "tablegen", "gtest_main", "debuginfopdb", "libdriver", "mirparser", "all-targets", "irreader", "lto" } },
  { "all-targets", 0, 1, { "x86", "x86" } },
  { "analysis", "libLLVMAnalysis.a", 1, { "core", "support" } },
  { "asmparser", "libLLVMAsmParser.a", 1, { "core", "support" } },
  { "asmprinter", "libLLVMAsmPrinter.a", 1, { "analysis", "codegen", "core", "mc", "mcparser", "support", "target", "transformutils" } },
  { "bitreader", "libLLVMBitReader.a", 1, { "core", "support" } },
  { "bitwriter", "libLLVMBitWriter.a", 1, { "core", "support" } },
  { "codegen", "libLLVMCodeGen.a", 1, { "analysis", "bitreader", "bitwriter", "core", "instrumentation", "mc", "scalaropts", "support", "target", "transformutils" } },
  { "core", "libLLVMCore.a", 1, { "support" } },
  { "debuginfodwarf", "libLLVMDebugInfoDWARF.a", 1, { "object", "support" } },
  { "debuginfopdb", "libLLVMDebugInfoPDB.a", 1, { "object", "support" } },
  { "engine", 0, 1, { "mcjit", "native" } },
  { "executionengine", "libLLVMExecutionEngine.a", 1, { "core", "mc", "object", "runtimedyld", "support", "target" } },
  { "gtest", "libgtest.a", 0, { "support" } },
  { "gtest_main", "libgtest_main.a", 0, { "gtest" } },
  { "instcombine", "libLLVMInstCombine.a", 1, { "analysis", "core", "support", "transformutils" } },
  { "instrumentation", "libLLVMInstrumentation.a", 1, { "analysis", "core", "mc", "support", "transformutils" } },
  { "interpreter", "libLLVMInterpreter.a", 1, { "codegen", "core", "executionengine", "support" } },
  { "ipo", "libLLVMipo.a", 1, { "analysis", "core", "instcombine", "profiledata", "scalaropts", "support", "transformutils", "vectorize" } },
  { "irreader", "libLLVMIRReader.a", 1, { "asmparser", "bitreader", "core", "support" } },
  { "libdriver", "libLLVMLibDriver.a", 1, { "object", "option", "support" } },
  { "lineeditor", "libLLVMLineEditor.a", 1, { "support" } },
  { "linker", "libLLVMLinker.a", 1, { "core", "support", "transformutils" } },
  { "lto", "libLLVMLTO.a", 1, { "analysis", "bitreader", "bitwriter", "codegen", "core", "ipo", "instcombine", "linker", "mc", "objcarcopts", "object", "scalaropts", "support", "target" } },
  { "mc", "libLLVMMC.a", 1, { "support" } },
  { "mcdisassembler", "libLLVMMCDisassembler.a", 1, { "mc", "support" } },
  { "mcjit", "libLLVMMCJIT.a", 1, { "core", "executionengine", "object", "runtimedyld", "support", "target" } },
  { "mcparser", "libLLVMMCParser.a", 1, { "mc", "support" } },
  { "mirparser", "libLLVMMIRParser.a", 1, { "asmparser", "codegen", "core", "mc", "support", "target" } },
  { "native", 0, 1, { "x86" } },
  { "nativecodegen", 0, 1, { "x86codegen" } },
  { "objcarcopts", "libLLVMObjCARCOpts.a", 1, { "analysis", "core", "support", "transformutils" } },
  { "object", "libLLVMObject.a", 1, { "bitreader", "core", "mc", "mcparser", "support" } },
  { "option", "libLLVMOption.a", 1, { "support" } },
  { "orcjit", "libLLVMOrcJIT.a", 1, { "core", "executionengine", "object", "runtimedyld", "support", "transformutils" } },
  { "passes", "libLLVMPasses.a", 1, { "analysis", "core", "ipo", "instcombine", "scalaropts", "support", "transformutils", "vectorize" } },
  { "profiledata", "libLLVMProfileData.a", 1, { "core", "support", "object" } },
  { "runtimedyld", "libLLVMRuntimeDyld.a", 1, { "mc", "object", "support" } },
  { "scalaropts", "libLLVMScalarOpts.a", 1, { "analysis", "core", "instcombine", "support", "transformutils" } },
  { "selectiondag", "libLLVMSelectionDAG.a", 1, { "analysis", "codegen", "core", "mc", "support", "target", "transformutils" } },
  { "support", "libLLVMSupport.a", 1, {  } },
  { "tablegen", "libLLVMTableGen.a", 1, { "support" } },
  { "target", "libLLVMTarget.a", 1, { "analysis", "core", "mc", "support" } },
  { "transformutils", "libLLVMTransformUtils.a", 1, { "analysis", "core", "support" } },
  { "vectorize", "libLLVMVectorize.a", 1, { "analysis", "core", "support", "transformutils" } },
  { "x86", 0, 1, { "x86utils", "x86asmprinter", "x86info", "x86desc", "x86codegen", "x86asmparser", "x86disassembler" } },
  { "x86asmparser", "libLLVMX86AsmParser.a", 1, { "mc", "mcparser", "support", "x86codegen", "x86desc", "x86info" } },
  { "x86asmprinter", "libLLVMX86AsmPrinter.a", 1, { "mc", "support", "x86utils" } },
  { "x86codegen", "libLLVMX86CodeGen.a", 1, { "analysis", "asmprinter", "codegen", "core", "mc", "selectiondag", "support", "target", "x86asmprinter", "x86desc", "x86info", "x86utils" } },
  { "x86desc", "libLLVMX86Desc.a", 1, { "mc", "mcdisassembler", "object", "support", "x86asmprinter", "x86info" } },
  { "x86disassembler", "libLLVMX86Disassembler.a", 1, { "mcdisassembler", "support", "x86info" } },
  { "x86info", "libLLVMX86Info.a", 1, { "support" } },
  { "x86utils", "libLLVMX86Utils.a", 1, { "core", "support" } },
};

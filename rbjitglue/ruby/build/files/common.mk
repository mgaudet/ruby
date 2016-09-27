###################################################################################
# (c) Copyright IBM Corp. 2000, 2016
#
#  This program and the accompanying materials are made available
#  under the terms of the Eclipse Public License v1.0 and
#  Apache License v2.0 which accompanies this distribution.
#
#      The Eclipse Public License is available at
#      http://www.eclipse.org/legal/epl-v10.html
#
#      The Apache License v2.0 is available at
#      http://www.opensource.org/licenses/apache2.0.php
#
# Contributors:
#    Multiple authors (IBM Corp.) - initial implementation and documentation
##################################################################################


#
# This is the giant list of files that make up Ruby
#
# Note - Give the name of the SOURCE FILE, not the object!
#
# Also, all paths are relative to JIT_SRCBASE
#
# Please note - This is for static source files only.
#       Source files that are generated by a tool will need to be treated
#       on a case-by-case basis, although that isn't too hard either
#
JIT_PRODUCT_SOURCE_FILES+=\
    $(JIT_OMR_DIRTY_DIR)/runtime/Alignment.cpp \
    $(JIT_OMR_DIRTY_DIR)/runtime/CodeCacheTypes.cpp \
    $(JIT_OMR_DIRTY_DIR)/runtime/OMRCodeCache.cpp \
    $(JIT_OMR_DIRTY_DIR)/runtime/OMRCodeCacheManager.cpp \
    $(JIT_OMR_DIRTY_DIR)/runtime/OMRCodeCacheMemorySegment.cpp \
    $(JIT_OMR_DIRTY_DIR)/runtime/OMRCodeCacheConfig.cpp \
    $(JIT_PRODUCT_DIR)/env/RubyFE.cpp \
    $(JIT_PRODUCT_DIR)/env/RubyMethod.cpp \
    $(JIT_PRODUCT_DIR)/il/RubyNode.cpp \
    $(JIT_PRODUCT_DIR)/ilgen/RubyIlGeneratorMethodDetails.cpp \
    $(JIT_PRODUCT_DIR)/ilgen/RubyByteCodeIterator.cpp \
    $(JIT_PRODUCT_DIR)/ilgen/RubyIlGenerator.cpp \
    $(JIT_PRODUCT_DIR)/infra/RubyMonitor.cpp \
    $(JIT_PRODUCT_DIR)/runtime/RubyCodeCacheManager.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/FEBase.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/Globals.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRCompilerEnv.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/PersistentAllocator.cpp \
    $(JIT_OMR_DIRTY_DIR)/control/CompileMethod.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRIO.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRKnownObjectTable.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/JitConfig.cpp \
    $(JIT_PRODUCT_DIR)/control/RubyJit.cpp \
    $(JIT_OMR_DIRTY_DIR)/control/CompilationController.cpp \
    $(JIT_OMR_DIRTY_DIR)/runtime/Runtime.cpp \
    $(JIT_OMR_DIRTY_DIR)/runtime/Trampoline.cpp \
    $(JIT_PRODUCT_DIR)/compile/RubySymbolReferenceTable.cpp \
    $(JIT_PRODUCT_DIR)/compile/RubyCompilation.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRAutomaticSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRLabelSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRMethodSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRParameterSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRRegisterMappedSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRResolvedMethodSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRStaticSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/TRPersistentMemory.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/TRMemory.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/VerboseLog.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/Aliases.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/NodePool.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/NodeUtils.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/symbol/OMRSymbol.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/OMRBlock.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/OMRDataTypes.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/OMRILOps.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/OMRSymbolReference.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/OMRTreeTop.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/OMRNode.cpp \
    $(JIT_OMR_DIRTY_DIR)/il/OMRIL.cpp \
    $(JIT_OMR_DIRTY_DIR)/ilgen/IlGenRequest.cpp \
    $(JIT_OMR_DIRTY_DIR)/compile/OMRCompilation.cpp \
    $(JIT_OMR_DIRTY_DIR)/compile/TLSCompilationManager.cpp \
    $(JIT_OMR_DIRTY_DIR)/control/OMRRecompilation.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRObjectModel.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRArithEnv.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRClassEnv.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRDebugEnv.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRVMEnv.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRCPU.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/SegmentProvider.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/SystemSegmentProvider.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/Region.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/StackMemoryRegion.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/OMRPersistentInfo.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRCodeGenPhase.cpp \
    $(JIT_OMR_DIRTY_DIR)/compile/OMRSymbolReferenceTable.cpp \
    $(JIT_OMR_DIRTY_DIR)/compile/OMRAliasBuilder.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/Assert.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/BitVector.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/CfgFrequencyCompletion.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/Checklist.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/HashTab.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/IGBase.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/IGNode.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/ILWalk.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/InterferenceGraph.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/OMRMonitorTable.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/Random.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/Timer.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/TreeServices.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/OMRCfg.cpp \
    $(JIT_OMR_DIRTY_DIR)/infra/SimpleRegex.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/CFGChecker.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/CallStack.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/Debug.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/DebugCounter.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/IgnoreLocale.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/LimitFile.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/LogTracer.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/OptionsDebug.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/PPCOpNames.cpp \
    $(JIT_OMR_DIRTY_DIR)/ras/Tree.cpp \
    $(JIT_OMR_DIRTY_DIR)/control/OMROptions.cpp \
    $(JIT_OMR_DIRTY_DIR)/control/OptimizationPlan.cpp \
    $(JIT_OMR_DIRTY_DIR)/compile/OSRData.cpp \
    $(JIT_OMR_DIRTY_DIR)/compile/Method.cpp \
    $(JIT_OMR_DIRTY_DIR)/compile/VirtualGuard.cpp \
    $(JIT_OMR_DIRTY_DIR)/env/ExceptionTable.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRAheadOfTimeCompile.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/Analyser.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/CodeGenPrep.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/CodeGenGC.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/CodeGenRA.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/FrontEnd.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRLinkage.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/LiveRegister.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OutOfLineCodeSection.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRRegisterDependency.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/Relocation.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/ScratchRegisterManager.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/StorageInfo.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRTreeEvaluator.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/PreInstructionSelection.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/NodeEvaluation.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRRegister.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRSnippet.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRSnippetGCMap.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRUnresolvedDataSnippet.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRCodeGenerator.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRMemoryReference.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRMachine.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRRealRegister.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRRegisterPair.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRInstruction.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRGCRegisterMap.cpp \
    $(JIT_OMR_DIRTY_DIR)/codegen/OMRGCStackAtlas.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/GlobalRegisterAllocator.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/LiveVariableInformation.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/Liveness.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/LoopCanonicalizer.cpp \
    $(JIT_PRODUCT_DIR)/optimizer/RubyOptimizationManager.cpp \
    $(JIT_PRODUCT_DIR)/optimizer/Optimizer.cpp \
    $(JIT_PRODUCT_DIR)/optimizer/RubyIlFastpather.cpp \
    $(JIT_PRODUCT_DIR)/optimizer/RubyCallInfo.cpp \
    $(JIT_PRODUCT_DIR)/optimizer/RubyInliner.cpp \
    $(JIT_PRODUCT_DIR)/optimizer/RubyTrivialInliner.cpp \
    $(JIT_PRODUCT_DIR)/optimizer/RubyLowerMacroOps.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/BackwardBitVectorAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/BackwardIntersectionBitVectorAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/BackwardUnionBitVectorAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/BitVectorAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/DataFlowAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/DeadStoreElimination.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/DebuggingCounters.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMRDeadTreesElimination.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/Dominators.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/DominatorVerifier.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/DominatorsChk.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/Inliner.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/PreExistence.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/IntersectionBitVectorAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/IsolatedStoreElimination.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/LoadExtensions.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMRLocalCSE.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMRLocalDeadStoreElimination.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/LocalOpts.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMROptimization.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMROptimizationManager.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMRTransformUtil.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMROptimizer.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OrderBlocks.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OSRDefAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/ReachingBlocks.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/Reachability.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/ReachingDefinitions.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/RegisterAnticipatability.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/RegisterAvailability.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/RegisterCandidate.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/ShrinkWrapping.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMRSimplifierHelpers.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMRSimplifierHandlers.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/OMRSimplifier.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/StructuralAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/Structure.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/UnionBitVectorAnalysis.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/UseDefInfo.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/ValueNumberInfo.cpp \
    $(JIT_OMR_DIRTY_DIR)/optimizer/VirtualGuardHeadMerger.cpp

# Include files that are specific to the host the JIT is running on
include $(JIT_MAKE_DIR)/files/host/$(HOST_ARCH).mk

# Include files that are specific to the target the JIT is generating
# code for
include $(JIT_MAKE_DIR)/files/target/$(TARGET_ARCH).mk

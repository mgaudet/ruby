/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

//On zOS XLC linker can't handle files with same name at link time
//This workaround with pragma is needed. What this does is essentially
//give a different name to the codesection (csect) for this file. So it
//doesn't conflict with another file with same name.
#pragma csect(CODE,"RubyOptimizer#C")
#pragma csect(STATIC,"RubyOptimizer#S")
#pragma csect(TEST,"RubyOptimizer#T")

#include "optimizer/Optimizer.hpp"

#include "optimizer/CFGSimplifier.hpp"
#include "optimizer/CompactLocals.hpp"
#include "optimizer/CopyPropagation.hpp"
#include "optimizer/DeadStoreElimination.hpp"
#include "optimizer/DeadTreesElimination.hpp"
#include "optimizer/ExpressionsSimplification.hpp"
#include "optimizer/GeneralLoopUnroller.hpp"
#include "optimizer/GlobalRegisterAllocator.hpp"
#include "optimizer/LocalCSE.hpp"
#include "optimizer/LocalDeadStoreElimination.hpp"
#include "optimizer/LocalLiveRangeReducer.hpp"
#include "optimizer/LocalOpts.hpp"
#include "optimizer/LocalReordering.hpp"
#include "optimizer/LoopCanonicalizer.hpp"
#include "optimizer/LoopReducer.hpp"
#include "optimizer/LoopReplicator.hpp"
#include "optimizer/LoopVersioner.hpp"
#include "optimizer/OrderBlocks.hpp"
#include "optimizer/PartialRedundancy.hpp"
#include "optimizer/IsolatedStoreElimination.hpp"
#include "optimizer/RegDepCopyRemoval.hpp"
#include "optimizer/Simplifier.hpp"
#include "optimizer/SinkStores.hpp"
#include "optimizer/ShrinkWrapping.hpp"
#include "optimizer/TrivialDeadBlockRemover.hpp"

#include "optimizer/RubyIlFastpather.hpp"
#include "optimizer/RubyLowerMacroOps.hpp"
#include "optimizer/RubyTrivialInliner.hpp"
#include "optimizer/RubyInliner.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"


// **********************************************************
//
// Ruby Strategies
//
// **********************************************************

static const OptimizationStrategy rubyTacticalGlobalRegisterAllocatorOpts[] =
   {
   { OMR::inductionVariableAnalysis,             OMR::IfLoops                      },
   { OMR::loopCanonicalization,                  OMR::IfLoops                      },
   { OMR::liveRangeSplitter,                     OMR::IfLoops                      },
   { OMR::redundantGotoElimination,              OMR::IfNotProfiling               }, // need to be run before global register allocator
   { OMR::treeSimplification,                    OMR::MarkLastRun                  }, // Cleanup the trees after redundantGotoElimination
   { OMR::lowerRubyMacroOps,                     OMR::MustBeDone      }, // Also must be done before GRA, to break asyncheck blocks
   { OMR::tacticalGlobalRegisterAllocator,       OMR::IfEnabled                    },
   { OMR::localCSE                                                                 },
// { isolatedStoreGroup,                         OMR::IfEnabled                    }, // if global register allocator created stores from registers
   { OMR::globalCopyPropagation,                 OMR::IfEnabledAndMoreThanOneBlock }, // if live range splitting created copies
   { OMR::localCSE                                                                 }, // localCSE after post-PRE + post-GRA globalCopyPropagation to clean up whole expression remat (rtc 64659)
   { OMR::globalDeadStoreGroup,                  OMR::IfEnabled                    },
   { OMR::redundantGotoElimination,              OMR::IfEnabled                    }, // if global register allocator created new block
   { OMR::deadTreesElimination                                                     }, // remove dangling GlRegDeps
   { OMR::deadTreesElimination,                  OMR::IfEnabled                    }, // remove dead RegStores produced by previous deadTrees pass
   { OMR::deadTreesElimination,                  OMR::IfEnabled                    }, // remove dead RegStores produced by previous deadTrees pass
   { OMR::endGroup                                                                 }
   };

static const OptimizationStrategy rubyCheapTacticalGlobalRegisterAllocatorOpts[] =
   {
   { OMR::redundantGotoElimination,          OMR::IfNotProfiling  }, // need to be run before global register allocator
   { OMR::lowerRubyMacroOps,                 OMR::MustBeDone      }, // Also must be done before GRA, to break asyncheck blocks
   { OMR::tacticalGlobalRegisterAllocator                         },
   { OMR::endGroup                                                }
   };

static const OptimizationStrategy rubyNoOptStrategyOpts[] =
   {
   { OMR::lowerRubyMacroOps,                         OMR::MustBeDone              },
   { OMR::endOpts                                                            },
   };

static const OptimizationStrategy rubyColdStrategyOpts[] =
   {
   { OMR::rubyIlFastpather                                                   },
   { OMR::basicBlockExtension                                                },
   { OMR::localCSE                                                           },
   { OMR::treeSimplification                                                 },
   { OMR::localCSE                                                           },
   { OMR::localDeadStoreElimination                                          },
   { OMR::globalDeadStoreGroup                                               },
   { OMR::isolatedStoreGroup                                                 },
   { OMR::deadTreesElimination                                               },
   { OMR::cheapTacticalGlobalRegisterAllocatorGroup                          },
   { OMR::lowerRubyMacroOps,                         OMR::MustBeDone              },
   { OMR::endOpts                                                            },
   };


static const OptimizationStrategy rubyWarmStrategyOpts[] =
   {
   { OMR::trivialInlining                                                    },
   { OMR::rubyIlFastpather                                                   },
   { OMR::basicBlockExtension                                                },
   { OMR::localCSE                                                           },
   { OMR::treeSimplification                                                 },
   { OMR::localCSE                                                           },
   { OMR::localDeadStoreElimination                                          },
   { OMR::globalDeadStoreGroup                                               },
   { OMR::isolatedStoreGroup                                                 },
   { OMR::deadTreesElimination                                               },
   { OMR::cheapTacticalGlobalRegisterAllocatorGroup                          },
   { OMR::lowerRubyMacroOps,                         OMR::MustBeDone              },
   { OMR::endOpts                                                            },
   };

static const OptimizationStrategy rubyHotStrategyOpts[] = // Mark's new JB Warm. 
   {
   { OMR::trivialInlining                                                          },
   { OMR::rubyIlFastpather                                                         },
   { OMR::treeSimplification                                                       }, // lots to simplify
   { OMR::localCSE                                                                 }, // common as much as possible
   { OMR::treeSimplification                                                       }, // simplify again
   { OMR::localCSE                                                                 }, // and common
   { OMR::basicBlockOrdering,                                                      }, // straighten goto's
   { OMR::globalCopyPropagation,                                                   },
   { OMR::globalDeadStoreElimination,                                              },
   { OMR::deadTreesElimination,                                                    },
   { OMR::treeSimplification,                                                      },
   { OMR::deadTreesElimination,                                                    },
   { OMR::lastLoopVersionerGroup,                    OMR::IfLoops                  },
   { OMR::globalDeadStoreElimination,                OMR::IfEnabledAndLoops        },
   { OMR::deadTreesElimination                                                     },
   { OMR::treeSimplification                                                       },
   { OMR::blockSplitter                                                            },
   { OMR::treeSimplification                                                       },

   { OMR::globalValuePropagation,                    OMR::IfMoreThanOneBlock       },
   { OMR::localValuePropagation,                     OMR::IfOneBlock               },
   { OMR::localCSE                                                                 },
   { OMR::treeSimplification,                                                      },
   { OMR::trivialDeadTreeRemoval,                    OMR::IfEnabled                },

   { OMR::inductionVariableAnalysis,                 OMR::IfLoops                  },
   { OMR::generalLoopUnroller,                       OMR::IfLoops                  },
   { OMR::basicBlockExtension,                       OMR::MarkLastRun              }, // extend blocks; move trees around if reqd
   { OMR::treeSimplification                                                       }, // revisit; not really required ?
   { OMR::localCSE                                                                 },
   { OMR::treeSimplification,                        OMR::MarkLastRun              },
   { OMR::trivialDeadTreeRemoval,                    OMR::IfEnabled                }, // final cleanup before opcode expansion
   { OMR::cheapTacticalGlobalRegisterAllocatorGroup, OMR::IfEnabled                },
   { OMR::globalDeadStoreGroup,                                                    },
   { OMR::rematerialization                                                        },
   { OMR::deadTreesElimination,                      OMR::IfEnabled                }, // remove dead anchors created by check/store removal
   { OMR::deadTreesElimination,                      OMR::IfEnabled                }, // remove dead RegStores produced by previous deadTrees pass
   { OMR::regDepCopyRemoval                                                        },
   { OMR::lowerRubyMacroOps,                         OMR::MustBeDone              },

#if 0
   // omrWarm strategy!
   { OMR::basicBlockExtension                  },
   { OMR::localCSE                             },
   { OMR::treeSimplification                   },
   { OMR::localCSE                             },
   { OMR::localDeadStoreElimination            },
   { OMR::globalDeadStoreGroup                 },
#endif

   { OMR::endOpts                                                                  },
   };


const OptimizationStrategy *rubyCompilationStrategies[] =
   {
   rubyNoOptStrategyOpts,// only must-be-done opts
   rubyColdStrategyOpts, // <<  specialized
   rubyWarmStrategyOpts, // <<  specialized
   rubyHotStrategyOpts, // <<  specialized
   };


const OptimizationStrategy *
Ruby::Optimizer::optimizationStrategy(TR::Compilation *c)
   {
   TR_Hotness strategy = c->getMethodHotness();
   TR_ASSERT(strategy <= lastRubyStrategy, "Invalid optimization strategy");

   // Downgrade strategy rather than crashing in prod.
   if (strategy > lastRubyStrategy)
      strategy = lastRubyStrategy;

   return rubyCompilationStrategies[strategy];
   }


Ruby::Optimizer::Optimizer(TR::Compilation *comp, TR::ResolvedMethodSymbol *methodSymbol, bool isIlGen,
      const OptimizationStrategy *strategy, uint16_t VNType)
   : OMR::Optimizer(comp, methodSymbol, isIlGen, strategy, VNType)
   {

   _opts[OMR::inductionVariableAnalysis] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_InductionVariableAnalysis::create, OMR::inductionVariableAnalysis, "O^O INDUCTION VARIABLE ANALYSIS: ");
   _opts[OMR::partialRedundancyElimination] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_PartialRedundancy::create, OMR::partialRedundancyElimination, "O^O PARTIAL REDUNDANCY ELIMINATION: ");
   _opts[OMR::isolatedStoreElimination] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_IsolatedStoreElimination::create, OMR::isolatedStoreElimination, "O^O ISOLATED STORE ELIMINATION: ");
   _opts[OMR::tacticalGlobalRegisterAllocator] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_GlobalRegisterAllocator::create, OMR::tacticalGlobalRegisterAllocator, "O^O GLOBAL REGISTER ASSIGNER: ");
   _opts[OMR::loopInversion] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_LoopInverter::create, OMR::loopInversion, "O^O LOOP INVERTER: ");
   _opts[OMR::loopSpecializer] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_LoopSpecializer::create, OMR::loopSpecializer, "O^O LOOP SPECIALIZER: ");
   _opts[OMR::trivialStoreSinking] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_TrivialSinkStores::create, OMR::trivialStoreSinking, "O^O TRIVIAL SINK STORES: ");
   _opts[OMR::generalStoreSinking] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_GeneralSinkStores::create, OMR::generalStoreSinking, "O^O GENERAL SINK STORES: ");
   _opts[OMR::liveRangeSplitter] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_LiveRangeSplitter::create, OMR::liveRangeSplitter, "O^O LIVE RANGE SPLITTER: ");
   _opts[OMR::redundantInductionVarElimination] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_RedundantInductionVarElimination::create, OMR::redundantInductionVarElimination, "O^O REDUNDANT INDUCTION VAR ELIMINATION: ");
   _opts[OMR::trivialDeadBlockRemover] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_TrivialDeadBlockRemover::create, OMR::trivialDeadBlockRemover, "O^O TRIVIAL DEAD BLOCK REMOVAL ");
   _opts[OMR::regDepCopyRemoval] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR::RegDepCopyRemoval::create, OMR::regDepCopyRemoval, "O^O REGISTER DEPENDENCY COPY REMOVAL: ");
   _opts[OMR::arrayPrivatizationGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::arrayPrivatizationGroup, "", arrayPrivatizationOpts);
   _opts[OMR::reorderArrayExprGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::reorderArrayExprGroup, "", reorderArrayIndexOpts);
   _opts[OMR::partialRedundancyEliminationGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::partialRedundancyEliminationGroup, "", partialRedundancyEliminationOpts);
   _opts[OMR::isolatedStoreGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::isolatedStoreGroup, "", isolatedStoreOpts);
   _opts[OMR::cheapTacticalGlobalRegisterAllocatorGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::cheapTacticalGlobalRegisterAllocatorGroup, "", rubyCheapTacticalGlobalRegisterAllocatorOpts);
   _opts[OMR::tacticalGlobalRegisterAllocatorGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::tacticalGlobalRegisterAllocatorGroup, "", rubyTacticalGlobalRegisterAllocatorOpts);
   _opts[OMR::finalGlobalGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::finalGlobalGroup, "", finalGlobalOpts);
   _opts[OMR::loopVersionerGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::loopVersionerGroup, "", loopVersionerOpts);
   _opts[OMR::lastLoopVersionerGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::lastLoopVersionerGroup, "", lastLoopVersionerOpts);
   _opts[OMR::blockManipulationGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::blockManipulationGroup, "", blockManipulationOpts);
   _opts[OMR::eachLocalAnalysisPassGroup] =
      new (comp->allocator()) TR::OptimizationManager(self(), NULL, OMR::eachLocalAnalysisPassGroup, "", eachLocalAnalysisPassOpts);
   _opts[OMR::isolatedStoreElimination] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_IsolatedStoreElimination::create, OMR::isolatedStoreElimination, "O^O ISOLATED STORE ELIMINATION: ");
   _opts[OMR::redundantGotoElimination] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_EliminateRedundantGotos::create, OMR::redundantGotoElimination, "O^O GOTO ELIMINATION: ");
   _opts[OMR::tacticalGlobalRegisterAllocator] =
      new (comp->allocator()) TR::OptimizationManager(self(), TR_GlobalRegisterAllocator::create, OMR::tacticalGlobalRegisterAllocator, "O^O GLOBAL REGISTER ASSIGNER: ");

   _opts[OMR::rubyIlFastpather] =
      new (comp->allocator()) TR::OptimizationManager(self(), Ruby::IlFastpather::create, OMR::rubyIlFastpather, "O^O RUBY IL FASTPATHER");
   _opts[OMR::lowerRubyMacroOps] =
      new (comp->allocator()) TR::OptimizationManager(self(), Ruby::LowerMacroOps::create, OMR::lowerRubyMacroOps, "O^O LOWER RUBY MACRO OPS");
   _opts[OMR::trivialInlining] =
      new (comp->allocator()) TR::OptimizationManager(self(), Ruby::TrivialInliner::create, OMR::trivialInlining, "O^O RUBY TRIVIAL INLINER: ");


   // turn requested on for optimizations/groups
   _opts[OMR::lowerRubyMacroOps]->setRequested();

   self()->setRequestOptimization(OMR::tacticalGlobalRegisterAllocatorGroup, true);
   self()->setRequestOptimization(OMR::tacticalGlobalRegisterAllocator, true);
   }

inline
TR::Optimizer *Ruby::Optimizer::self()
   {
   return (static_cast<TR::Optimizer *>(this));
   }

OMR_InlinerUtil *Ruby::Optimizer::getInlinerUtil()
   {
   return new (comp()->allocator()) Ruby::InlinerUtil(comp());
   }

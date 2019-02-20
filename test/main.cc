#include <iostream>

#include "islutils/access_patterns.h"
#include "islutils/builders.h"
#include "islutils/ctx.h"
#include "islutils/locus.h"
#include "islutils/matchers.h"
#include "islutils/pet_wrapper.h"


using util::ScopedCtx;
using namespace matchers;

namespace blasMatchers {

bool findGemmTree (isl::schedule_node, isl::schedule_node *);
bool findBatchedGemmTree (isl::schedule_node, isl::schedule_node *);
bool findGemmAccess (isl::ctx, isl::union_map, isl::union_map);
bool findAndReplaceGemm (isl::ctx, Scop);
bool findBatchedGemm(isl::ctx, Scop);


} //namespace blasMatchers


int main(int argc, char **argv) {
    // 1. Parse program and generate Pet and schedule tree
    // 2. Pattern match (find patterns, meaning that I should have implemented a collection of patterns)
    // 3. Generate CUDA code with BLAS calls (cuBLAS I guess) (find a way to hack AST to embed cuBLAS calls)
    // Laisser peut-oetre la partie generation de code pour la semaine prochaine. Cette semaine on se concentre
    // sur trouver des patterns.

    auto inputFile = argv[1];
    auto ctx = pet::allocCtx();
    auto petScop = pet::Scop::parseFile(ctx, inputFile);
    auto scop = petScop.getScop();

    //auto gemm = blasMatchers::findAndReplaceGemm(ctx, scop);


    auto bgemm = blasMatchers::findBatchedGemm(ctx, scop);
    std::cout << inputFile << std::endl;
    // Create a collection of matchers per type of kernel.
    // i.e. a matcher for gemm, tgemm, batchgemm, axpy, whatver. matrix-vector product, dot product, axpy.
    // on peut probablement s'inspirer du matcher de gemm déja existant.
    // Creer une classe de telle sorte que on ait un accès du style:
    // -- project::matchers::gemm


    return 0;
}





namespace blasMatchers {


bool findGemmTree(isl::schedule_node root, isl::schedule_node *node) {

    // For the moment reusing what is already implemented in test_transformer.cc
    //isl::schedule_node node;
    auto matcher = band(
        [&node] (isl::schedule_node n) {
            if (isl_schedule_node_band_n_member(n.get()) < 3) {
                return false;
            }
            else {
                node = &n;
                return true;
            }
        },
        leaf()
    );

    auto test = ScheduleNodeMatcher::isMatching(matcher, root.child(0));

    return test;
}

bool findBatchedGemmTree(isl::schedule_node root, isl::schedule_node *node) {

    //isl::schedule_node node;
    auto matcher = band(
        [&node] (isl::schedule_node n) {
            if (isl_schedule_node_band_n_member(n.get()) < 4) {
                return false;
            }
            else {
                node = &n;
                return true;
            }
        },
        leaf()
    );

    auto test = ScheduleNodeMatcher::isMatching(matcher, root.child(0));

    return test;
}

bool findGemmAccess (isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
    
    auto _i = placeholder(ctx);
    auto _j = placeholder(ctx);
    auto _k = placeholder(ctx);
    auto _ii = placeholder(ctx);
    auto _jj = placeholder(ctx);

    auto _A = arrayPlaceholder();
    auto _B = arrayPlaceholder();
    auto _C = arrayPlaceholder();

    auto localReads = allOf(access(_A, _i, _j), access(_B, _i, _k), access(_C, _k, _j));
    auto localWrites = allOf(access(_A, _ii, _jj));

    auto matchReads = match(reads, localReads);
    auto matchWrites = match(writes, localWrites);

    // Not sure of what needs to be tested.
    return ((matchReads.size() == 1u) && (matchWrites.size() == 1u));
// isl::schedule_node replaceWithGemm(isl::schedule_node root, isl::schedule_node node) {
//     if root == node {

//     }
// }
}

// bool find2DTransposedAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
//     auto i = placeholder(ctx);
//     auto j = placeholder(ctx);
//     auto ii = placeholder(ctx);
//     auto jj = placeholder(ctx);

//     auto A = arrayPlaceholder();

//     auto localRead = allOf(access(A, j, i));
//     auto localWrite = allOf(access(A, i, j));

//     auto matchRead = match(reads, localRead);
//     auto matchWrite = match(writes, localWrite);

//     return ((matchRead.size() == 1u) && (matchWrite.size() == 1u));
// }

bool findTransposeGemmAccesses (isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
    auto i = placeholder(ctx);
    auto j = placeholder(ctx);
    auto k = placeholder(ctx);
    auto ii = placeholder(ctx);
    auto jj = placeholder(ctx);

    auto A = arrayPlaceholder();
    auto B = arrayPlaceholder();
    auto C = arrayPlaceholder();

    // Vérifier si ca marche vraiment.
    auto localReads = allOf(access(A, i, j), access(B, i, k), access(C, j, k), access(C, k, j));
    auto localWrites = allOf(access(C, ii, jj), access(A, ii, jj));

    auto matchReads = match(reads, localReads);
    auto matchWrites = match(writes, localWrites);

    // Not sure of what needs to be tested.
    return ((matchReads.size() == 1u) && (matchWrites.size() == 1u));
}


bool findBatchedGemmAccess (isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
    auto i = placeholder(ctx);
    auto j = placeholder(ctx);
    auto k = placeholder(ctx);
    auto b = placeholder(ctx);

    auto ii = placeholder(ctx);
    auto jj = placeholder(ctx);
    auto bb = placeholder(ctx);

    auto A = arrayPlaceholder();
    auto B = arrayPlaceholder();
    auto C = arrayPlaceholder();

    auto localReads = allOf(access(A, b, i, j), access(B, b, i, k), access(C, b, k, j));
    auto localWrites = allOf(access(A, bb, ii, jj));

    auto matchReads = match(reads, localReads);
    auto matchWrites = match(writes, localWrites);

    return ((matchReads.size() == 1u) && (matchWrites.size() == 1u));
}


static inline isl::union_map
filterOutCarriedDependences(isl::union_map dependences,
                            isl::schedule_node node) {
  auto partialSchedule = node.get_prefix_schedule_multi_union_pw_aff();
  return dependences.eq_at(partialSchedule);
}

static bool canMerge(isl::schedule_node parentBand,
                     isl::union_map dependences) {
  // Permutability condition: there are no negative distances along the
  // dimensions that are not carried until now by any of dimensions.
  auto t1 = parentBand.band_get_partial_schedule();
  auto t2 = parentBand.child(0).band_get_partial_schedule();
  auto schedule = isl::union_map::from(t1.flat_range_product(t2));
  auto scheduleSpace = isl::set(schedule.range()).get_space();
  auto positiveOrthant =
      isl::set(isl::basic_set::positive_orthant(scheduleSpace));
  dependences = filterOutCarriedDependences(dependences, parentBand);
  return dependences.apply_domain(schedule)
      .apply_range(schedule)
      .deltas()
      .is_subset(positiveOrthant);
}

static inline isl::schedule_node
rebuild(isl::schedule_node node,
        const builders::ScheduleNodeBuilder &replacement) {
  // this may not be always legal...
  node = node.cut();
  node = replacement.insertAt(node);
  return node;
}

isl::schedule_node
replaceRepeatedly(isl::schedule_node node,
                  const matchers::ScheduleNodeMatcher &pattern,
                  const builders::ScheduleNodeBuilder &replacement) {
  while (matchers::ScheduleNodeMatcher::isMatching(pattern, node)) {
    node = rebuild(node, replacement);
  }
  return node;
}

isl::schedule_node
replaceDFSPreorderRepeatedly(isl::schedule_node node,
                             const matchers::ScheduleNodeMatcher &pattern,
                             const builders::ScheduleNodeBuilder &replacement) {
  node = replaceRepeatedly(node, pattern, replacement);
  for (int i = 0; i < node.n_children(); ++i) {
    node = replaceDFSPreorderRepeatedly(node.child(i), pattern, replacement)
               .parent();
  }
  return node;
}


isl::schedule_node mergeIfTilable(isl::schedule_node node,
                                  isl::union_map dependences) {
  isl::schedule_node parent, child, grandchild;

  auto canMergeCaptureChild = [&child, dependences](isl::schedule_node node) {
    if (canMerge(node.parent(), dependences)) {
      child = node;
      return true;
    }
    return false;
  };

  auto matcher = [&]() {
    using namespace matchers;
    // clang-format off
    return band(parent,
             band(canMergeCaptureChild,
               anyTree(grandchild)));
    // clang-format on
  }();

  // Use lambdas to lazily initialize the builder with the nodes and values yet
  // to be captured by the matcher.
  auto declarativeMerger = builders::ScheduleNodeBuilder();
  {
    using namespace builders;
    auto schedule = [&]() {
      auto descr =
          BandDescriptor(parent.band_get_partial_schedule().flat_range_product(
              child.band_get_partial_schedule()));
      descr.permutable = 1;
      return descr;
    };
    auto st = [&]() { return subtreeBuilder(grandchild); };
    declarativeMerger = band(schedule, subtree(st));
  }

  return replaceDFSPreorderRepeatedly(node, matcher, declarativeMerger);
}

static isl::union_map computeAllDependences(const Scop &scop) {
  // For the simplest possible dependence analysis, get rid of reference tags.
  auto reads = scop.reads.domain_factor_domain();
  auto mayWrites = scop.mayWrites.domain_factor_domain();
  auto mustWrites = scop.mustWrites.domain_factor_domain();

  // False dependences (output and anti).
  // Sinks are writes, sources are reads and writes.
  auto falseDepsFlow = isl::union_access_info(mayWrites.unite(mustWrites))
                           .set_may_source(mayWrites.unite(reads))
                           .set_must_source(mustWrites)
                           .set_schedule(scop.schedule)
                           .compute_flow();

  isl::union_map falseDeps = falseDepsFlow.get_may_dependence();

  // Flow dependences.
  // Sinks are reads and sources are writes.
  auto flowDepsFlow = isl::union_access_info(reads)
                          .set_may_source(mayWrites)
                          .set_must_source(mustWrites)
                          .set_schedule(scop.schedule)
                          .compute_flow();

  isl::union_map flowDeps = flowDepsFlow.get_may_dependence();

  return flowDeps.unite(falseDeps);
}

bool findBatchedGemm( isl::ctx ctx, Scop scop) {
 auto dependences = computeAllDependences(scop);
    scop.schedule = mergeIfTilable(scop.schedule.get_root(), dependences).get_schedule();


    isl::schedule_node root = scop.schedule.get_root();
    isl::schedule_node node;

    bool foundGemm = false;
    std::cout << foundGemm << std::endl;

    root.dump();
    if (findBatchedGemmTree(root, &node) == true) {
        foundGemm = true;
    }


 if (foundGemm == true) {
        std::cout << "It matches" << std::endl;
    }
    else {
        std::cout << "It doesn't match" << std::endl;
    }

    std::cout << foundGemm << std::endl;

    return foundGemm;
}

bool findAndReplaceGemm (isl::ctx ctx, Scop scop) {

    auto dependences = computeAllDependences(scop);
    scop.schedule = mergeIfTilable(scop.schedule.get_root(), dependences).get_schedule();


    isl::schedule_node root = scop.schedule.get_root();
    isl::schedule_node node;

    bool foundGemm = false;
    std::cout << foundGemm << std::endl;

    root.dump();
    if (findGemmTree(root, &node) == true) {
        
        isl::union_map reads = scop.reads;
        isl::union_map writes = scop.mustWrites;

        //foundGemm = true;
         if (findGemmAccess(ctx, reads, writes) == true) {
                 foundGemm = true;
         }

    }
    if (foundGemm == true) {
        std::cout << "It matches" << std::endl;
    }
    else {
        std::cout << "It doesn't match" << std::endl;
    }

    std::cout << foundGemm << std::endl;

    return foundGemm;

    // if (foundGemm == true) {

    // } 
    // else {

    // }

}

} // namespace blasMatchers
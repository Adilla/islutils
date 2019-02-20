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
bool findGemmAccesses (util::ScopedCtx, isl::union_map, isl::union_map);
bool findAndReplaceGemm (Scop);

} //namespace blasMatchers


int main(int argc, char **argv) {
    // 1. Parse program and generate Pet and schedule tree
    // 2. Pattern match (find patterns, meaning that I should have implemented a collection of patterns)
    // 3. Generate CUDA code with BLAS calls (cuBLAS I guess) (find a way to hack AST to embed cuBLAS calls)
    // Laisser peut-oetre la partie generation de code pour la semaine prochaine. Cette semaine on se concentre
    // sur trouver des patterns.

    auto inputFile = "inputs/gemm.c";
    auto ctx = pet::allocCtx();
    auto petScop = pet::Scop::parseFile(ctx, inputFile);
    auto scop = petScop.getScop();

    auto gemm = blasMatchers::findAndReplaceGemm(scop);

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

    return ScheduleNodeMatcher::isMatching(matcher, root.child(0));
}

bool findGemmAccesses (util::ScopedCtx ctx, isl::union_map reads, isl::union_map writes) {
    
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
}

// isl::schedule_node replaceWithGemm(isl::schedule_node root, isl::schedule_node node) {
//     if root == node {

//     }
// }


bool findAndReplaceGemm (Scop scop) {

    isl::schedule_node root = scop.schedule.get_root();
    isl::schedule_node node;

    bool foundGemm = false;
    std::cout << foundGemm << std::endl;

    root.dump();
    if (findGemmTree(root, &node) == true) {
        
        isl::union_map reads = scop.reads;
        isl::union_map writes = scop.mustWrites;

        foundGemm = true;
        // if (findGemmAccesses(ctx, reads, writes) == true) {
        //     foundGemm = true;
        // }

    }
    std::cout << foundGemm << std::endl;
    return foundGemm;

    // if (foundGemm == true) {

    // } 
    // else {

    // }

}

} // namespace blasMatchers
#include <iostream>

#include "blas_matchers.h"

using util::ScopedCtx;
//using namespace matchers;



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


    auto bgemm = blasMatchers::findTransposedGemm(ctx, scop);
    std::cout << inputFile << std::endl;
    // Create a collection of matchers per type of kernel.
    // i.e. a matcher for gemm, tgemm, batchgemm, axpy, whatver. matrix-vector product, dot product, axpy.
    // on peut probablement s'inspirer du matcher de gemm déja existant.
    // Creer une classe de telle sorte que on ait un accès du style:
    // -- project::matchers::gemm


    return 0;
}




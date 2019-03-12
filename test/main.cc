#include <iostream>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <isl/ctx.h>
#include <isl/id.h>
#include <isl/val.h>
#include <isl/set.h>
#include <isl/union_set.h>
#include <isl/union_map.h>
#include <isl/aff.h>
#include <isl/flow.h>
#include <isl/options.h>
#include <isl/schedule.h>
#include <isl/ast.h>
#include <isl/id_to_ast_expr.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <pet.h>

#include "ppcg.h"
#include "ppcg_options.h"
#include "cuda.h"
#include "opencl.h"
#include "cpu.h"

//#include "blas_matchers.h"

//using util::ScopedCtx;
//using namespace matchers;



// int main(int argc, char **argv) {
//     // 1. Parse program and generate Pet and schedule tree
//     // 2. Pattern match (find patterns, meaning that I should have implemented a collection of patterns)
//     // 3. Generate CUDA code with BLAS calls (cuBLAS I guess) (find a way to hack AST to embed cuBLAS calls)
//     // Laisser peut-oetre la partie generation de code pour la semaine prochaine. Cette semaine on se concentre
//     // sur trouver des patterns.

//     auto inputFile = argv[1];
//     auto ctx = pet::allocCtx();
//     auto petScop = pet::Scop::parseFile(ctx, inputFile);
//     auto scop = petScop.getScop();

//     //auto gemm = blasMatchers::findAndReplaceGemm(ctx, scop);

    
//     auto pattern = blasMatchers::findPatterns(ctx, scop);
//     petScop.schedule() = pattern;

//     std::cout << petScop.codegen() << std::endl;;


//     //std::cout << pattern << std::endl;
//     std::cout << inputFile << std::endl;
//     std::cout << petScop.codegen() << std::endl;
//     // Create a collection of matchers per type of kernel.
//     // i.e. a matcher for gemm, tgemm, batchgemm, axpy, whatver. matrix-vector product, dot product, axpy.
//     // on peut probablement s'inspirer du matcher de gemm déja existant.
//     // Creer une classe de telle sorte que on ait un accès du style:
//     // -- project::matchers::gemm


//     return 0;
// }




int main(int argc, char **argv)
{
  int r;
  isl_ctx *ctx;
  struct options *options;

  options = options_new_with_defaults();
  assert(options);

  ctx = isl_ctx_alloc_with_options(&options_args, options);
  ppcg_options_set_target_defaults(options->ppcg);
  isl_options_set_ast_build_detect_min_max(ctx, 1);
  isl_options_set_ast_print_macro_once(ctx, 1);
  isl_options_set_schedule_whole_component(ctx, 0);
  isl_options_set_schedule_maximize_band_depth(ctx, 1);
  isl_options_set_schedule_maximize_coincidence(ctx, 1);
  pet_options_set_encapsulate_dynamic_control(ctx, 1);
  argc = options_parse(options, argc, argv, ISL_ARG_ALL);

  if (check_options(ctx) < 0)
    r = EXIT_FAILURE;
  else if (options->ppcg->target == PPCG_TARGET_CUDA)
    r = generate_cuda(ctx, options->ppcg, options->input);
  else if (options->ppcg->target == PPCG_TARGET_OPENCL)
    r = generate_opencl(ctx, options->ppcg, options->input,
			options->output);
  else
    r = generate_cpu(ctx, options->ppcg, options->input,
		     options->output);

  isl_ctx_free(ctx);

  return r;
}

#ifndef BLAS_ACCESS_MATCHERS_H
#define BLAS_ACCESS_MATCHERS_H

#include "utils.h"
using namespace matchers;

namespace blasAccessMatchers {

bool findGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
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

	return ((matchReads.size() == 1u) && (matchWrites.size() == 1u));
}


bool findBatchedGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto _j = placeholder(ctx);
	auto _k = placeholder(ctx);
	auto _b = placeholder(ctx);
	auto _ii = placeholder(ctx);
	auto _jj = placeholder(ctx);
	auto _bb = placeholder(ctx);
	auto _A = arrayPlaceholder();
	auto _B = arrayPlaceholder();
	auto _C = arrayPlaceholder();
	auto localReads = allOf(access(_A, _b, _i, _j), 
													access(_B, _b, _i, _k), 
													access(_C, _b, _k, _j));
	auto localWrites = allOf(access(_A, _bb, _ii, _jj));
	auto matchReads = match(reads, localReads);
	auto matchWrites = match(writes, localWrites);

	// From what I understand, this test should be enough to ensure that 
	// we indeed are in the presence of a bached gemm-like access function.
	return ((matchReads.size() == 1u) && (matchWrites.size() == 1u));
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

bool findTransposeGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto _j = placeholder(ctx);
	auto _k = placeholder(ctx);
	auto _it = placeholder(ctx);
	auto _jt = placeholder(ctx);
	// auto ii = placeholder(ctx);
	// auto jj = placeholder(ctx);

	auto A = arrayPlaceholder();
	auto B = arrayPlaceholder();
	auto C = arrayPlaceholder();
	auto D = arrayPlaceholder();


	reads.dump();


	// Vérifier si ca marche vraiment.
	auto localReads = allOf(access(A, _i, _j), 
													access(B, _i, _k), 
													access(C, _j, _k), 
													access(B, _j, _i));
	//auto localWrites = allOf(access(A, _i, _j), access(B, _it, _jt));

	auto matchReads = match(reads, localReads);
	

	for (const auto &mr : matchReads) {
		std::cout << mr[_i].payload().inputDimPos_ << std::endl;
		std::cout << "\n" << std::endl;
		std::cout << mr[_i].candidateSpaces().size() << std::endl;
		std::cout << "\n" << std::endl;
		std::cout << "\n" << std::endl;
	}
	
	//auto matchWrites = match(writes, localWrites);

	// Not sure of what needs to be tested.
	std::cout << matchReads.size() << std::endl;

	// int it = matchReads[0][_it].payload().inputDimPos_;
	// int jt = matchReads[0][_jt].payload().inputDimPos_;
	// int i = matchReads[0][_i].payload().inputDimPos_;
	// int j = matchReads[0][_j].payload().inputDimPos_;
	// int k = matchReads[0][_k].payload().inputDimPos_;

	// std::cout << it << std::endl;
	// std::cout << jt << std::endl;
	// std::cout << i << std::endl;
	// std::cout << j << std::endl;
	// std::cout << k << std::endl; 
	
	
	return true;//((matchReads.size() == 1u) && (matchWrites.size() == 1u));
}



} // namespace blasMatchers
#endif
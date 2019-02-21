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

bool findTransposeGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
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

bool findBatchedGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
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


} // namespace blasMatchers
#endif
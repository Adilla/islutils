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
	auto localReads = allOf(access(_i, _j), access(_i, _k), access(_k, _j));
	auto localWrites = allOf(access(_ii, _jj));
	auto matchReads = match(reads, localReads);
	auto matchWrites = match(writes, localWrites);

	if ((matchReads.size() == 1u) && (matchWrites.size() == 1u)) {
		int i = matchReads[0][_i].payload().inputDimPos_;
		int j = matchReads[0][_j].payload().inputDimPos_;
		int k = matchReads[0][_k].payload().inputDimPos_;
		int ii = matchWrites[0][_ii].payload().inputDimPos_;
		int jj = matchWrites[0][_jj].payload().inputDimPos_;
		bool isMatch = ((ii == 0) && (jj == 1) && 
										(ii == i) && (jj == j) && 
										(k == 2)); 
		return isMatch;
	} else { 
		return false; 
	}
}

bool findBatchGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto _j = placeholder(ctx);
	auto _k = placeholder(ctx);
	auto _b = placeholder(ctx);
	auto _ii = placeholder(ctx);
	auto _jj = placeholder(ctx);
	auto _bb = placeholder(ctx);
	auto localReads = allOf(access(_b, _i, _j), 
													access(_b, _i, _k), 
													access(_b, _k, _j));
	auto localWrites = allOf(access(_bb, _ii, _jj));
	auto matchReads = match(reads, localReads);
	auto matchWrites = match(writes, localWrites);


	// The following is mostly to ensure that _b == _bb,
	// _ii == _i and _jj == _i, since I cannot use the 
	// same placeholder for reads and writes.
	if ((matchReads.size() == 1u) && (matchWrites.size() == 1u)) {
		int b = matchReads[0][_b].payload().inputDimPos_;
		int i = matchReads[0][_i].payload().inputDimPos_;
		int j = matchReads[0][_j].payload().inputDimPos_;
		int k = matchReads[0][_k].payload().inputDimPos_;
		int bb = matchWrites[0][_bb].payload().inputDimPos_;
		int ii = matchWrites[0][_ii].payload().inputDimPos_;
		int jj = matchWrites[0][_jj].payload().inputDimPos_;
		bool isMatch = ((bb == 0) && (ii == 1) && (jj == 2) && 
										(bb == b) && (ii == i) && (jj == j) && 
										(k == 3)); 
		return isMatch;
	} else { 
		return false; 
	}
}

bool findTransposeGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto _j = placeholder(ctx);
	auto _k = placeholder(ctx);
	auto _ii = placeholder(ctx);
	auto _jj = placeholder(ctx);
	auto localReads = allOf(access(_i, _j), access(_i, _k), access(_j, _k));
	auto localWrites = allOf(access(_ii, _jj));
	auto matchReads = match(reads, localReads);
	auto matchWrites = match(writes, localWrites);

	if ((matchReads.size() == 1u) && (matchWrites.size() == 1u)) {
		int i = matchReads[0][_i].payload().inputDimPos_;
		int j = matchReads[0][_j].payload().inputDimPos_;
		int k = matchReads[0][_k].payload().inputDimPos_;
		int ii = matchWrites[0][_ii].payload().inputDimPos_;
		int jj = matchWrites[0][_jj].payload().inputDimPos_;
		bool isMatch = ((ii == 0) && (jj == 1) && 
										(ii == i) && (jj == j) && 
										(k == 2)); 
		return isMatch;
	} else { 
		return false; 
	}
}

bool findTransposeBatchGemmAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto _j = placeholder(ctx);
	auto _k = placeholder(ctx);
	auto _b = placeholder(ctx);
	auto _ii = placeholder(ctx);
	auto _jj = placeholder(ctx);
	auto _bb = placeholder(ctx);
	auto localReads = allOf(access(_b, _i, _j), 
													access(_b, _i, _k), 
													access(_b, _j, _k));
	auto localWrites = allOf(access(_bb, _ii, _jj));
	auto matchReads = match(reads, localReads);
	auto matchWrites = match(writes, localWrites);


	// The following is mostly to ensure that _b == _bb,
	// _ii == _i and _jj == _i, since I cannot use the 
	// same placeholder for reads and writes.
	if ((matchReads.size() == 1u) && (matchWrites.size() == 1u)) {
		int b = matchReads[0][_b].payload().inputDimPos_;
		int i = matchReads[0][_i].payload().inputDimPos_;
		int j = matchReads[0][_j].payload().inputDimPos_;
		int k = matchReads[0][_k].payload().inputDimPos_;
		int bb = matchWrites[0][_bb].payload().inputDimPos_;
		int ii = matchWrites[0][_ii].payload().inputDimPos_;
		int jj = matchWrites[0][_jj].payload().inputDimPos_;
		bool isMatch = ((bb == 0) && (ii == 1) && (jj == 2) && 
										(bb == b) && (ii == i) && (jj == j) && 
										(k == 3)); 
		return isMatch;
	} else { 
		return false; 
	}
}

bool findAxpyAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto _ii = placeholder(ctx);

	auto localReads = allOf(access(_i));
	auto localWrites = allOf(access(_ii));

	auto matchReads = match(reads, localReads);
	auto matchWrites = match(writes, localWrites);

	if ((matchReads.size() == 2u) && (matchWrites.size() == 1u)) {
		int i = matchReads[0][_i].payload().inputDimPos_;
		int i1 = matchReads[1][_i].payload().inputDimPos_;
		int ii = matchWrites[0][_ii].payload().inputDimPos_;
		// If I understand well, at this point, we should now that 
		// both i occurences are equals, otherwise there would be no
		// match. So testing with i should be enough.
		bool isMatch = ((ii == 0) && (ii == i)); 
		return isMatch;
	} else {
		return false;
	}
}

bool findDotProductAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto localReads = allOf(access(_i));
	auto matchReads = match(reads, localReads);
	// Since the fact that there is a match means that there are only
	// two patterns that follow this access same specification, then
	// the condition on have 2 reads is enough to ensure that it is 
	// a dotProduct. However, I don't know how to ensure that we are indeed
	// in the presence of a reduction on a scalar variable, so I guess this 
	// test is incomplete
	if ((matchReads.size() == 2u)) {
		return true;
	} else {
		return false;
	}
}
} // namespace blasMatchers
#endif
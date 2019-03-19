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
	if ((matchReads.size() == 2u)) {
    int i = matchReads[0][_i].payload().inputDimPos_;
    int i2 = matchReads[1][_i].payload().inputDimPos_;
    auto localWrite = writes.range().unwrap();
    auto writeSpaceDim = localWrite.dim(isl::dim::out);
    // Another condition is the that the scalar variable
    // should be inductive.
    // I include writes.is_subset(reads) as a 
    // condition to ensure that but the code crashes 
    // when == False.
    bool isMatch = (i == i2) && (writeSpaceDim == 0) && (writes.is_subset(reads));
		return isMatch;
	} else {
		return false;
	}
}


std::map<std::string, std::vector<int>> 
reconstruct (isl::ctx ctx, isl::union_map umap) {
  auto _k = placeholder(ctx);
  std::map<std::string, std::vector<int>> output;
  int counter = 0;
  auto acc = match(umap, allOf(access(dim(counter, _k))));

  while (acc.size() > 0u) {

    for (int i = 0; i < acc.size(); ++i) {
      auto space = acc[i][_k].candidateSpaces();
      auto name = space[0].range().unwrap().get_tuple_name(isl::dim::out);
      output[name].push_back(acc[i][_k].payload().inputDimPos_);
    }
    counter += 1;
    acc = match(umap, allOf(access(dim(counter, _k))));
  }
  return output;
}

bool
hasNoRedundancy(std::vector<int> vec) {
  bool isNotRedundant = true;
  for (int i = 1; i < vec.size(); ++i) {
    if (vec[i] == vec[i-1]) {
      isNotRedundant = false;
      break;
    }
  }
  return isNotRedundant;
}

/* Conditions for contraction: C = A . B
  1. There is a reduction
  2. None of the access functions have redundant iterators
  3. All iterators in A not appearing in C do apepar in B.
  4. The remaining iterators of A and B == those of C. */
bool findContractionAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
  std::vector<int> diff1, diff2;

  auto _reads = reconstruct(ctx, reads);
  auto _writes = reconstruct(ctx, writes);

  auto wname = _writes.begin()->first;
  auto wacc = _writes.begin()->second;
  auto op1 = _reads.begin();
  auto op2 = std::next(op1);

  auto isReduction = (_reads.find(wname) != _reads.end()) && (wacc == _reads[wname]);

  // Clean up
  _reads.erase(wname);

  // Sort arrays before continuing. This is necessary
  // to use the following std:: functions.
  std::sort(wacc.begin(), wacc.end());
  std::sort(op1->second.begin(), op1->second.end());
  std::sort(op2->second.begin(), op2->second.end());

  bool noRedundancy = hasNoRedundancy(wacc);
  for (auto r : _reads) {
    noRedundancy = noRedundancy && hasNoRedundancy(r.second);
  }
 
  std::set_difference(op1->second.begin(), op1->second.end(), 
                      wacc.begin(), wacc.end(), 
                      std::back_inserter(diff1));
  std::set_difference(op2->second.begin(), op2->second.end(), 
                      wacc.begin(), wacc.end(), 
                      std::back_inserter(diff2));

  bool hasContractionAxes = (diff1.size() > 0) && (diff1 == diff2);

  return isReduction && noRedundancy && hasContractionAxes;

}

bool findTransposeAccess(isl::ctx ctx, isl::union_map reads, isl::union_map writes) {
	auto _i = placeholder(ctx);
	auto _j = placeholder(ctx);
	auto _ii = placeholder(ctx);
	auto _jj = placeholder(ctx);
	auto localReads = allOf(access(_i, _j));
	auto localWrites = allOf(access(_ii, _jj));

	auto matchReads = match(reads, localReads);
	auto matchWrites = match(writes, localWrites);

	if ((matchReads.size() == 1u) && (matchWrites.size() == 1u)) {
		// This far, at least we ensure that there is only one read 
		// and one write. Then we need to make sure that the correspond
		// to a transposition
		int i1 = matchReads[0][_i].payload().inputDimPos_;
		int j1 = matchReads[0][_j].payload().inputDimPos_;
		auto i2 = matchWrites[0][_ii].payload().inputDimPos_;
		auto j2 = matchWrites[0][_jj].payload().inputDimPos_;
		bool isMatch = ((i1 == j2) && (i2 == j1));
		return isMatch;
	} else {
		return false;
	}
}
} // namespace blasMatchers
#endif
#ifndef BLAS_TREE_MATCHERS_H
#define BLAS_TREE_MATCHERS_H

#include "utils.h"
using namespace matchers;

namespace blasTreeMatchers {

bool find1DTree(isl::schedule_node root) {
	// Ensure if u need to check if strided or not.
	auto matcher = band(
									leaf()
								);
	return ScheduleNodeMatcher::isMatching(matcher, root);
} 

bool find2DTree(isl::schedule_node root) {
	auto matcher = band(
									band(
										leaf()));									
	return ScheduleNodeMatcher::isMatching(matcher, root);
}

bool findNDPermutableBand(isl::schedule_node root, isl::schedule_node *node, int nbDim) {
	// For the moment reusing what is already implemented in test_transformer.cc
	// This is also valid for transposeGemm
	auto matcher = band(
			[&node, nbDim](isl::schedule_node n) {
				if (isl_schedule_node_band_n_member(n.get()) < nbDim) {
					return false;
				} else {
					node = &n;
					return true;
				}
			}, leaf());
	return ScheduleNodeMatcher::isMatching(matcher, root);
}

bool findGemmTree(isl::schedule_node root, isl::schedule_node *node) {
	return findNDPermutableBand(root, node, 3u);
}

bool findBatchedGemmTree(isl::schedule_node root, isl::schedule_node *node) {
	return findNDPermutableBand(root, node, 4u);
}

bool findTransposeTree(isl::schedule_node root, isl::schedule_node *node) {
	// Must check if this is the proper condition.
	return find2DTree(root);
}

bool findAxpyTree(isl::schedule_node root, isl::schedule_node *node) {
	return find1DTree(root);
}

bool findDotProductTree(isl::schedule_node root, isl::schedule_node *node) {
	return find1DTree(root);
}
} // namespace blasTreeMatchers

#endif
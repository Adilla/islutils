#ifndef BLAS_TREE_MATCHERS_H
#define BLAS_TREE_MATCHERS_H

#include "utils.h"
using namespace matchers;

namespace blasTreeMatchers {

bool findGemmTree(isl::schedule_node root, isl::schedule_node *node) {

	// For the moment reusing what is already implemented in test_transformer.cc
	// This is also valid for transposeGemm
	//isl::schedule_node node;
	auto matcher = band(
			[&node](isl::schedule_node n) {
				if (isl_schedule_node_band_n_member(n.get()) < 3) {
					return false;
				} else {
					node = &n;
					return true;
				}
			}, leaf());
	auto res = ScheduleNodeMatcher::isMatching(matcher, root.child(0));
	return res;
}


bool findBatchedGemmTree(isl::schedule_node root, isl::schedule_node *node) {
	//isl::schedule_node node;
	// This is also valid for transposed Batched 
	auto matcher = band(
			[&node](isl::schedule_node n) {
				if (isl_schedule_node_band_n_member(n.get()) < 4) {
					return false;
				} else {
					node = &n;
					return true;
				}
			}, leaf());
	auto res = ScheduleNodeMatcher::isMatching(matcher, root.child(0));
	return res;
}

bool findTransposeTree(isl::schedule_node root, isl::schedule_node *node) {
	auto matcher = band(
		[&node] (isl::schedule_node n) {
			if (isl_schedule_node_band_n_member(n.get()) < 2) {
				return false;
			} else {
				node = &n;
				return true;
			}
		}, leaf());
		auto res = ScheduleNodeMatcher::isMatching(matcher, root.child(0));
		return res;
}

bool find1DTree(isl::schedule_node root) {
	// Ensure if u need to check if strided or not.
	auto matcher = band(
									leaf()
								);
	return ScheduleNodeMatcher::isMatching(matcher, root.child(0));
} 

bool findAxpyTree(isl::schedule_node root, isl::schedule_node *node) {
	return find1DTree(root);
}

bool findDotProductTree(isl::schedule_node root, isl::schedule_node *node) {
	return find1DTree(root);
}
} // namespace blasTreeMatchers

#endif
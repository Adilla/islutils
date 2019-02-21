#ifndef BLAS_MATCHERS_H
#define BLAS_MATCHERS_H

#include "blas_access_matchers.h"
#include "blas_tree_matchers.h"

using namespace blasAccessMatchers;
using namespace blasTreeMatchers;


namespace blasMatchers {

bool findBatchedGemm(isl::ctx ctx, Scop scop) {
	auto dependences = computeAllDependences(scop);
	scop.schedule = mergeIfTilable(scop.schedule.get_root(), dependences).get_schedule();

	isl::schedule_node root = scop.schedule.get_root();
	isl::schedule_node node;

	bool foundGemm = false;
	// std::cout << foundGemm << std::endl;

	// root.dump();
	// if (findBatchedGemmTree(root, &node) == true) {
	//     foundGemm = true;
	// }

	isl::union_map reads = scop.reads;
	isl::union_map writes = scop.mustWrites;

	reads.dump();
	writes.dump();

	//foundGemm = true;
	if (findTransposeGemmAccess(ctx, reads, writes) == true) {
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

bool findAndReplaceGemm(isl::ctx ctx, Scop scop) {
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
}

} // namespace blasMathers

#endif
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
	if (findBatchGemmAccess(ctx, reads, writes) == true) {
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



bool findTransposedGemm(isl::ctx ctx, Scop scop) {
	isl::union_map reads = scop.reads.curry();
	isl::union_map writes = scop.mustWrites.curry();

	// std::cout <<	reads.get_map_list().size() << std::endl;


	// std::vector<isl::union_map> allreads;
	
	// auto mapList = reads.get_map_list();
	// int max = mapList.size();

	// if (max > 0) {
	// 	auto firstDomain = mapList.get_map(0).domain();

	// 	int i;
	// 	// Iterate over list to identify different domain and 
	// 	// split the list into lists of maps.
	// 	for (i = 0; i < max; ++i) {
	// 		auto thisMap = mapList.get_map(i);
	// 		auto test = reads.get_map_list().get_map(i);
		
	// }


	auto res = findTransposeGemmAccess(ctx, reads, writes);

	if (res == true) {
		std::cout << "found transposed access" << std::endl;
	} else {
		std::cout << "did not find" << std::endl;
	}
	return true;
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



bool findDotProduct(isl::ctx ctx, Scop scop) {
	isl::union_map reads = scop.reads;
	isl::union_map writes = scop.mustWrites;
	bool test = findDotProductAccess(ctx, reads, writes);
}

bool findAxpy(isl::ctx ctx, Scop scop) {
	isl::schedule_node root = scop.schedule.get_root();
	isl::schedule_node node;
	bool test = findAxpyTree(root, &node); 
	if (test == true) {
		std::cout << "matches" << std::endl;
	} else {
		std::cout << "no match" << std::endl;
	}
}

bool findDotProduct(isl::ctx ctx, Scop scop) {
	isl::schedule_node root = scop.schedule.get_root();
	isl::schedule_node node;
	if (findDotProductTree(root, &node) == true) {
		std::cout << "matches" << std::endl;
	}	else {
		std::cout << "no match" << std::endl;
	}
}
} // namespace blasMathers

#endif
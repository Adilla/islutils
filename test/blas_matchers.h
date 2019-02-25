#ifndef BLAS_MATCHERS_H
#define BLAS_MATCHERS_H
#include <map>


#include "blas_access_matchers.h"
#include "blas_tree_matchers.h"

#include <isl/cpp.h>

using namespace blasAccessMatchers;
using namespace blasTreeMatchers;
//using namespace isl;


namespace blasMatchers {


std::map<int, isl::union_map> 
restructureUnionMap(isl::ctx ctx, isl::union_map umap) {
	std::map<int, isl::union_map> rumap;
	// I am not sure if this is a necessary step 
	// but we'll do it this way for now.
	isl::map_list mapList = umap.get_map_list();
	auto map = isl::union_map(ctx, mapList.get_at(0).to_str());

	rumap[0] = map;
	int count = 0;

	for (int i = 1; i < mapList.size(); ++i) {
		auto thisMap = mapList.get_map(i);
		if (rumap[count].domain().is_equal(thisMap.domain())) {
			rumap[count] = rumap[count].add_map(thisMap);
		} else {
			count += 1;
			rumap[count] = isl::union_map(ctx, thisMap.to_str());
		}
	}
	return rumap;
}


int separateScops(isl::ctx ctx, Scop scop) {
	isl::union_map reads = scop.reads.curry();	
	isl::union_map writes = scop.mustWrites.curry();

	//reads.dump();

	auto mreads = restructureUnionMap(ctx, reads);
	auto mwrites = restructureUnionMap(ctx, writes);

	// isl::map_list wList = writes.get_map_list();
	// isl::map_list rList = reads.get_map_list();

	// std::map<int, isl::union_map> sReads, sWrites;

	// auto bRead = isl::union_map(ctx, rList.get_at(0).to_str());
	// auto bWrite = isl::union_map(ctx, wList.get_at(0).to_str());

	// sReads[0] = bRead;
	// sWrites[0] = bWrite;
	// int count = 0;

	// for (int i = 1; i < rList.size(); ++i) {
	// 	auto thisRead = rList.get_map(i);
	// 	if (sReads[count].domain().is_equal(thisRead.domain())) {
	// 		sReads[count] = sReads[count].add_map(thisRead);
 	// 	} else {
	// 		 count += 1;
	// 		 sReads[count] = isl::union_map(ctx, thisRead.to_str());
	// 	}
	// }

	std::cout << "Reads" << std::endl;
	for (auto map : mreads) {
		map.second.dump();
		std::cout << "\n" << std::endl;
 	}


	std::cout << "Writes" << std::endl;
	for (auto map : mwrites) {
		map.second.dump();
		std::cout << "\n" << std::endl;
	}




	// std::cout << rList.size() << std::endl;

	// if (rList.size() < 1) {
	// 	std::cout << "return single map" << std::endl;
	// } else {
	// 	std::cout << "Do the separation" << std::endl;
	// }

	// if (rList.size() < 1) {
	// 	std::cout << "bb" << std::endl;
	// } else {
	// 	std::cout << "cc" << std::endl;
	// }
	return 0;
}

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

// bool findDotProduct(isl::ctx ctx, Scop scop) {
// 	isl::union_map reads = scop.reads;
// 	isl::union_map writes = scop.mustWrites;
// 	bool test = findDotProductAccess(ctx, reads, writes);
// }

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
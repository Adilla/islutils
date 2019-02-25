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

using UmapPair = std::map<int, isl::union_map>;
using Accesses = std::vector<std::pair<isl::union_map, isl::union_map>>;

bool findAndReplaceGemm(isl::ctx, Scop);
std::vector<std::pair<isl::union_map, isl::union_map>> associateRW(UmapPair, UmapPair);

std::map<int, isl::union_map> 
restructureUnionMap(isl::ctx ctx, 
										isl::union_map umap) {	
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



Accesses
restructureScop(isl::ctx ctx,	
								isl::union_map reads, 
								isl::union_map writes) {

	return associateRW(restructureUnionMap(ctx, reads),
										 restructureUnionMap(ctx, writes));

}

// Associate reads and writes from same scop
Accesses
associateRW(UmapPair reads, UmapPair writes) {
	Accesses rw;

	if (reads.size() != writes.size()) {
		std::cout << "error" << std::endl;
	} else {
		for (auto w : writes) {
			for (auto r : reads) {
				if (w.second.domain().is_equal(r.second.domain())) {
					rw.push_back(std::make_pair(w.second, r.second));
				}
			}
		}
	}
	return rw;
}

void findPatterns(isl::ctx ctx, Scop scop) {
	isl::union_map _reads = scop.reads.curry();
	isl::union_map _writes = scop.mustWrites.curry();

	auto accesses = restructureScop(ctx, _reads, _writes);
	// Accesses accesses = restructureScop(ctx, _reads, _writes);

	// auto reads = accesses.first;
	// auto writes = accesses.second;


	// for (auto it = reads.begin(); it != reads.end(); ++it) {
	// 	auto thisRead = it
	// }

	// auto test = findAndReplaceGemm(ctx, scop, accesses);
}


bool findAndReplaceGemm(isl::ctx ctx, Scop scop, Accesses accesses) {
		//scop.dump();
			std::cout << "test" << std::endl;
			scop.schedule.dump();
	// auto dependences = computeAllDependences(scop);
	// scop.schedule = mergeIfTilable(scop.schedule.get_root(), dependences).get_schedule();

	// isl::schedule_node root = scop.schedule.get_root();
	// isl::schedule_node node;

	// bool foundGemm = false;
	// std::cout << foundGemm << std::endl;

	// root.dump();
	// if (findGemmTree(root, &node) == true) {

	// 	isl::union_map reads = scop.reads;
	// 	isl::union_map writes = scop.mustWrites;

	// 	//foundGemm = true;
	// 	if (findGemmAccess(ctx, reads, writes) == true) {
	// 		foundGemm = true;
	// 	}
	// }
	// if (foundGemm == true) {
	// 	std::cout << "It matches" << std::endl;
	// }
	// else {
	// 	std::cout << "It doesn't match" << std::endl;
	// }

	// std::cout << foundGemm << std::endl;

	return true;
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
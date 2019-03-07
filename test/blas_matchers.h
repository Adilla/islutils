#ifndef BLAS_MATCHERS_H
#define BLAS_MATCHERS_H
#include <map>


#include "blas_access_matchers.h"
#include "blas_tree_matchers.h"

#include <isl/cpp.h>

using namespace blasAccessMatchers;
using namespace blasTreeMatchers;
using namespace builders;
//using namespace isl;


namespace blasMatchers {

enum Kernel {
	Gemm,
	Transpose,
	TransposeGemm
};

const int nbKernels = 3;

using UmapPair = std::map<int, isl::union_map>;
using Accesses = std::vector<std::pair<isl::union_map, isl::union_map>>;

bool findAndReplaceGemm(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findAndReplaceTranspose(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findAndReplaceTransposeGemm(isl::ctx, Scop, isl::union_map, isl::union_map);
Accesses associateRW(UmapPair, UmapPair);

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

int 
findPatterns(isl::ctx ctx, 
						 Scop scop) {
	isl::union_map _reads = scop.reads.curry();
	isl::union_map _writes = scop.mustWrites.curry();

	auto accesses = restructureScop(ctx, _reads, _writes);

	// The idea is as follows.
	// We first go through pair of <read, write> (i.e accesses,
	// that represent a single statement) to find if it corresponds
	// to a known BLAS access pattern. If true, then confirm that
	// the schedule tree enclosing the statement does match the
	// hypothetical BLAS. If both conditions are met for a given
	// BLAS pattern, then return the schedule tree node that
	// should be transformed into a runtime call.

	for (auto acc : accesses) {
		auto writes = acc.first;
		auto reads = acc.second;

		for (int i = 0; i < nbKernels; ++i) {
			Kernel k = (Kernel)i;
			switch(k) {
				case Gemm :
					if (findAndReplaceGemm(ctx, scop, reads, writes) == true)
						return Gemm;
					break;
				case Transpose : {
					if (findAndReplaceTranspose(ctx, scop, reads, writes) == true)
						return Transpose;
				}
				break;
				case TransposeGemm : {
					if (findAndReplaceTransposeGemm(ctx, scop, reads, writes) == true)
						return TransposeGemm;
				}
				break;
			}
		}
	}
}


bool
findAndReplaceGemm(isl::ctx ctx,
									 Scop scop,
									 isl::union_map reads,
									 isl::union_map writes) {
	auto isGemm = findGemmAccess(ctx, reads, writes);
	if (isGemm == true) {
		std::cout << "Found Gemm" << std::endl;

		auto dom =reads.domain();
		scop.schedule.dump();
		auto test = scop.schedule.get_domain();
		//	auto test3 = test.get_root();

		dom.dump();
		test.dump();

		if (dom.is_subset(test)) {
			std::cout << "Is subset" << std::endl;
		}

	//test3.dump();
	} else {
		return false;
	}
}

// If this function is called, then &subnode will necessary be 
// updated with a node because we ensured that the domain was 
// indeed a subset of the Scop's domain.
void
searchRootNodeMatchingDomain(isl::schedule_node node, 
														 isl::union_set domain,
														 isl::schedule_node &subnode) {
	if (node.get_domain().is_equal(domain)) {
		subnode = node;		
	}
	else {
		for (int i = 0; i < node.n_children(); ++i) 
			searchRootNodeMatchingDomain(node.get_child(i), domain, subnode);
	}
}


bool
findAndReplaceTranspose(isl::ctx ctx,
									 			Scop scop,
									 			isl::union_map reads,
									 			isl::union_map writes) {
	bool isTranspose = findTransposeAccess(ctx, reads, writes);
	if (isTranspose == true) {
		//std::cout << "Found Transpose" << std::endl;
		// At this point it doesn't matter whether we use the
		// domain of reads or writes, it's the same
		auto accessdom = reads.domain();
		auto scheddom = scop.schedule.get_domain();
	
		if (accessdom.is_subset(scheddom)) {
			isl::schedule_node root = scop.schedule.get_root();
			isl::schedule_node *_node;
			isl::schedule_node subnode;
			searchRootNodeMatchingDomain(root, accessdom, subnode);
			// Update isTranspose, perhaps the result is False, 
			// then the functions shall return false.
			isTranspose = findTransposeTree(subnode, _node); 
		}
	}
	return isTranspose;
}
	


bool 
findAndReplaceTransposeGemm(isl::ctx ctx,
									 					Scop scop,
									 					isl::union_map reads,
									 					isl::union_map writes) {
														
	bool isTransposeGemm = findTransposeGemmAccess(ctx, reads, writes);
	if (isTransposeGemm == true) {
		std::cout << "Found transpose gemm access" << std::endl;
		// At this point it doesn't matter whether we use the
		// domain of reads or writes, it's the same
		auto accessdom = reads.domain();
		auto scheddom = scop.schedule.get_domain();

		if (accessdom.is_subset(scheddom)) {
			isl::schedule_node root = scop.schedule.get_root();
			isl::schedule_node subnode;
			searchRootNodeMatchingDomain(root, accessdom, subnode);
			isl::schedule_node *_node;
			// Of course, the tree should be the same as 
			// standard Gemm.
			auto dependences = computeAllDependences(scop);
			subnode = mergeIfTilable(subnode, dependences);
			isTransposeGemm = findGemmTree(subnode, _node);
		}
	}
	return isTransposeGemm;
}							 
									 



bool 
findBatchedGemm(isl::ctx ctx, 
								Scop scop) {
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
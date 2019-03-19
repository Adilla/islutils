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
	TransposeGemm,
	BatchGemm,
	Axpy,
	DotProduct,
  Contraction
};

const int nbKernels = 7;

using UmapPair = std::map<int, isl::union_map>;
using Accesses = std::vector<std::pair<isl::union_map, isl::union_map>>;

bool findGemm(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findTranspose(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findTransposeGemm(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findBatchGemm(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findAxpy(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findDotProduct(isl::ctx, Scop, isl::union_map, isl::union_map);
bool findContraction(isl::ctx, Scop, isl::union_map, isl::union_map);

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
					//if (findGemm(ctx, scop, reads, writes) == true)
					//	return Gemm;
					break;
				case Transpose : {
					if (findTranspose(ctx, scop, reads, writes) == true)
						return Transpose;
				}
				break;
				case TransposeGemm : {
					if (findTransposeGemm(ctx, scop, reads, writes) == true)
						return TransposeGemm;
				}
				break;
				case BatchGemm : {
					if (findBatchGemm(ctx, scop, reads, writes) == true)
						return BatchGemm;
				}
				break;
				case DotProduct : {
					if (findDotProduct(ctx, scop, reads, writes) == true)
						return DotProduct;
				}
				break;
				case Axpy : {
					if (findAxpy(ctx, scop, reads, writes) == true)
						return Axpy;
				}
        break;
        case Contraction : {
          if (findContraction(ctx, scop, reads, writes) == true)
            return Contraction;
        }
        break;
			}
		}
	}
}


bool
findGemm(isl::ctx ctx,
									 Scop scop,
									 isl::union_map reads,
									 isl::union_map writes) {
	bool isGemm = findGemmAccess(ctx, reads, writes);
	if (isGemm == true) {
		auto accessdom = reads.domain();
		auto scheddom = scop.schedule.get_domain();

		if (accessdom.is_subset(scheddom)) {
			isl::schedule_node root = scop.schedule.get_root();
			isl::schedule_node subnode;
			searchRootNodeMatchingDomain(root, accessdom, subnode);
			isl::schedule_node *_node;
			auto dependences = computeAllDependences(scop);
			subnode = mergeIfTilable(subnode, dependences);
			isGemm = findGemmTree(subnode, _node);
		}
	}
	return isGemm;
}


bool
findTranspose(isl::ctx ctx,
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
findTransposeGemm(isl::ctx ctx,
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
findBatchGemm(isl::ctx ctx, 
								Scop scop,
								isl::union_map reads,
								isl::union_map writes) {
	bool isBatchGemm = findBatchGemmAccess(ctx, reads, writes);
	if (isBatchGemm == true) {
		auto accessdom = reads.domain();
		auto scheddom = scop.schedule.get_domain();

		if (accessdom.is_subset(scheddom)) {
			isl::schedule_node root = scop.schedule.get_root();
			isl::schedule_node subnode;
			searchRootNodeMatchingDomain(root, accessdom, subnode);
			isl::schedule_node *_node;
			auto dependences = computeAllDependences(scop);
			subnode = mergeIfTilable(subnode, dependences);
			isBatchGemm = findGemmTree(subnode, _node);
		}
	}
	return isBatchGemm;
								}


bool 
findAxpy(isl::ctx ctx, 
				 Scop scop,
				 isl::union_map reads,
				 isl::union_map writes) {
	bool isAxpy = findAxpyAccess(ctx, reads, writes);
	if (isAxpy == true) {
		auto accessdom = reads.domain();
		auto scheddom = scop.schedule.get_domain();

		if (accessdom.is_subset(scheddom)) {
			isl::schedule_node root = scop.schedule.get_root();
			isl::schedule_node subnode;
			searchRootNodeMatchingDomain(root, accessdom, subnode);
			isl::schedule_node *_node;
			isAxpy = findAxpyTree(subnode, _node);
		}
	}
	return isAxpy;
}

bool 
findDotProduct(isl::ctx ctx, 
							 Scop scop,
							 isl::union_map reads,
							 isl::union_map writes) {
	bool isDotProduct = findDotProductAccess(ctx, reads, writes);
	if (isDotProduct == true) {
		auto accessdom = reads.domain();
		auto scheddom = scop.schedule.get_domain();

		if (accessdom.is_subset(scheddom)) {
			isl::schedule_node root = scop.schedule.get_root();
			isl::schedule_node subnode;
			searchRootNodeMatchingDomain(root, accessdom, subnode);
			isl::schedule_node *_node;
			isDotProduct = findDotProductTree(subnode, _node);
		}
	}
	return isDotProduct;
}

bool 
findContraction(isl::ctx ctx,
                Scop scop,
                isl::union_map reads,
                isl::union_map writes) {
  bool isContraction = findContractionAccess(ctx, reads, writes);
  return isContraction;
  }
} // namespace blasMathers

#endif
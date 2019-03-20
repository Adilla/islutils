#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <iostream>
#include <stack>
#include <sstream>
#include <isl/cpp.h>
#include <vector>


#include "utils.h"

using namespace std;

namespace conversion {

template < typename T > std::string to_string( const T& n ) {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
}

} // end namespace conversion


void codegen(int);
void printHeader(std::string &);
using util::ScopedCtx;


// class GpuArrayInfo {
//   public:

    

//     //ArrayType type = ArrayType::not_assigned;
// };

typedef struct {
  std::string name;
  unsigned n_index;
  std::string element_type;
  isl::set extent;
  std::vector<isl::map> accesses;
} GpuArrayInfo;


static std::string getAccessName(isl::map m) {
  return m.range().get_tuple_id().get_name();
}

static std::string getAccessName(isl::set s) {
  return s.get_tuple_id().get_name();
}

static unsigned getAccessIndexes(isl::map m) {
  return m.dim(isl::dim::out);
}

static unsigned getAccessIndexes(isl::set s) {
  return s.dim(isl::dim::out);
}

static std::vector<GpuArrayInfo> collectArrayInfo(Scop scop) {

  std::vector<GpuArrayInfo> res;

  isl::union_set arrays;
  isl::union_map accesses;

  std::vector<isl::set> arraysAsSet; 
  std::vector<isl::map> accessesAsMap;

  isl::union_map reads = scop.reads;
  accesses = reads;
  arrays = reads.range();
  isl::union_map writes = scop.mustWrites;
  accesses = accesses.unite(writes);

  arrays = arrays.unite(writes.range()); 

  arrays = arrays.coalesce();
  accesses = accesses.coalesce();
  arrays.foreach_set([&arraysAsSet](isl::set s) { arraysAsSet.push_back(s); });
  accesses.foreach_map([&accessesAsMap](isl::map m) { accessesAsMap.push_back(m); });

   
  for(size_t i = 0; i < arraysAsSet.size(); ++i) {
    GpuArrayInfo ga;
    ga.name = getAccessName(arraysAsSet[i]);
    for(int j = 0; j < scop.n_array; ++j) {
      std::string arrayName = getAccessName(scop.arrays[j].extent);
      if(arrayName.compare(ga.name) == 0) {
        ga.n_index = getAccessIndexes(arraysAsSet[i]);
        ga.element_type = scop.arrays[j].element_type;
        ga.extent = scop.arrays[j].extent;
        //for(size_t u = 0; u < accessesAsMap.size(); ++u) {
        //  if(getAccessName(accessesAsMap[u]).compare(ga.name)) {
        //    ga.accesses.push_back(accessesAsMap[u]);
        //  }
        //}
      } 
    }
    //ga.printArrayInfo();
    res.push_back(ga);
  }

  for(size_t i = 0; i < res.size(); ++i) {
    for(size_t j = 0; j < accessesAsMap.size(); ++j) {
      if(getAccessName(accessesAsMap[j]).compare(res[i].name) == 0) {
        res[i].accesses.push_back(accessesAsMap[j]);
      }
    }
  } 

  return res;
}

static std::string getNumberOfElementArray(GpuArrayInfo g) {
  
  isl::set extent = g.extent;
  std::vector<int> bounds;

  for(size_t i = 0; i < g.n_index; ++i) {

    auto allPoints = 
      isl::map::from_domain_and_range(extent, extent);
    isl::pw_aff min = allPoints.dim_min(i);
    isl::pw_aff max = allPoints.dim_max(i);

    EXPECT_TRUE(min.n_piece() == 1); 
    EXPECT_TRUE(max.n_piece() == 1);

    isl::val min_val;
    isl::val max_val;
 
    min.foreach_piece([&](isl::set s, isl::aff aff) -> void {
      min_val = aff.get_constant_val(); 
    });
    max.foreach_piece([&](isl::set s, isl::aff aff) -> void {
      max_val = aff.get_constant_val();
    });

    max_val = max_val.sub(min_val);
    int bound = atoi(max_val.to_str().c_str());
    bounds.push_back(bound);
  }

  unsigned numberOfElement = 1; 
  for(size_t i = 0; i < bounds.size(); ++i) {
    numberOfElement *= static_cast<unsigned>(bounds[i]);
  }

  return conversion::to_string(numberOfElement);
}

std::string indent () {
  return "\t";
}

std::string newLine(int count) {
  std::string n = "\n";
  for (int i = 0; i < count-1; ++i) {
    n += "\n";
  }
   return n;
}

std::string initHandle() {
  std::string s;
  s = "cublasHandle_t handle;\n";
  s += "cublasCreate(&handle);\n";

  return s;  
}


std::string freeHandle() {
  return "cublasDestroy(handle);\n";
}


std::string printHeader() {
  std::string output;
  output = "#include <stdio.h>\n";
  output += "#include <stdlib.h>\n";
  output += "#include <cuda_runtime.h>\n";
  output += "#include \"cublas_v2.h\"\n";

  return output;
}


void codegen(int pattern) {

  std::string output;

  /* Header */
  output += printHeader();
  output += newLine(4);

  /* Main functions */   
  output += "int main() {\n";
  
  output += indent();
  output += initHandle();

}


#endif 


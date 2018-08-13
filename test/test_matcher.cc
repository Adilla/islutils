#include <islutils/builders.h>
#include <islutils/matchers.h>
#include <islutils/parser.h>
#include <iostream>

#include "gtest/gtest.h"

TEST(TreeMatcher, ReadFromFile) {
  Scop S = Parser("inputs/one-dimensional-init.c").getScop();
  EXPECT_TRUE(!S.schedule.is_null());
}

TEST(TreeMatcher, CompileTest) {
  using namespace matchers;

  // clang-format off
  auto matcher =
      domain(
        context(
          sequence(
            band(
              leaf()),
            band(
              leaf()),
            filter(
              any()))));
  // clang-format on
  auto m2 = sequence(any());
  auto m3 = sequence(filter(any()), filter(any()));
  auto m4 = sequence([](isl::schedule_node n) { return true; }, any());
  auto m5 = sequence([](isl::schedule_node n) { return true; }, filter(leaf()),
                     filter(leaf()));

  auto m6 = sequence(filter(hasNextSibling(filter(any())), any()));
  auto m7 = sequence(filter(
      hasNextSibling(filter(hasPreviousSibling(filter(any())), any())), any()));
  auto m8 = sequence(filter(hasSibling(filter(any())), any()));

  auto m9 = sequence(hasDescendant(band(leaf())), any());
  auto m10 = band(leaf());
  auto m11 = band([](isl::schedule_node n) { return true; }, leaf());

  // access pattern matchers.
  auto m12 = read('X');
  auto m13 = read('X', 'Y');
  auto m14 = write('Z');
  auto m15 = write('S', 'Y');
  auto m16 = readAndWrite('X','Y');
}

static isl::schedule_node makeGemmTree() {
  using namespace builders;
  auto ctx = isl::ctx(isl_ctx_alloc());
  auto iterationDomain = isl::union_set(
      ctx, "{S1[i,j]: 0 <= i,j < 10; S2[i,j,k]: 0 <= i,j,k < 42}");
  auto sched =
      isl::multi_union_pw_aff(ctx, "[{S1[i,j]->[(i)]; S2[i,j]->[(i)]}, "
                                   "{S1[i,j]->[(j)]; S2[i,j]->[(j)]}]");
  auto filterS1 = isl::union_set(ctx, "{S1[i,j]}");
  auto filterS2 = isl::union_set(ctx, "{S2[i,j]}");
  auto innerSched = isl::multi_union_pw_aff(ctx, "[{S2[i,j,k]->[(k)]}]");

  // clang-format off
  auto builder =
    domain(iterationDomain,
      band(sched,
        sequence(
          filter(filterS1),
          filter(filterS2,
            band(innerSched)))));
  // clang-format on

  return builder.build();
}

TEST(TreeMatcher, AnyMatchesLeaf) {
  using namespace matchers;
  // clang-format off
  auto matcher =
    band(
      sequence(
        filter(
          leaf()),
        filter(
          band(any()))));
  // clang-format on

  auto node = makeGemmTree();
  EXPECT_TRUE(ScheduleNodeMatcher::isMatching(matcher, node.child(0)));
}

TEST(TreeMatcher, LeafMatchesLeaf) {
  using namespace matchers;
  // clang-format off
  auto matcher =
    band(
      sequence(
        filter(
          leaf()),
        filter(
          band(
            leaf()))));
  // clang-format on

  auto node = makeGemmTree();
  EXPECT_TRUE(ScheduleNodeMatcher::isMatching(matcher, node.child(0)));
}

TEST(TreeMatcher, finderConstructor) {
  using namespace matchers;
  Scop S = Parser("inputs/matmul.c").getScop();
  EXPECT_TRUE(!S.schedule.is_null());
  //S.dump();
  isl::union_map reads = S.reads;
  //std::cout << reads << std::endl;
  isl::union_map writes = S.mustWrites;
  //std::cout << writes << std::endl;
  auto M1 = read('X', 'Y');
  std::vector<RelationMatcher> matchers;
  matchers.push_back(M1);
  auto f = Finder(reads, writes, matchers);
  EXPECT_TRUE(f.getSizeReadMatchers() == 1);
  auto M2 = read('X','Y');
  auto M3 = read('X','Y');
  matchers.push_back(M2);
  matchers.push_back(M3);
  auto f1 = Finder(reads, writes, matchers);
  EXPECT_TRUE(f1.getSizeReadMatchers() == 3);
  auto M4 = write('X','Y');
  matchers.push_back(M4);
  auto f2 = Finder(reads, writes, matchers);
  EXPECT_TRUE(f2.getSizeReadMatchers() == 3 && f2.getSizeWriteMatchers() == 1);
  auto M5 = readAndWrite('X','Y');
  matchers.push_back(M5);
  auto f3 = Finder(reads, writes, matchers);
  EXPECT_TRUE(f3.getSizeReadAndWriteMatchers() == 1);
}

TEST(TreeMatcher, finderTest) {
  using namespace matchers;
  Scop S = Parser("inputs/matmul.c").getScop();
  EXPECT_TRUE(!S.schedule.is_null());
  isl::union_map reads = S.reads;
  isl::union_map writes = S.mustWrites;
  EXPECT_TRUE(reads.n_map() != 0);
  EXPECT_TRUE(writes.n_map() != 0);
  auto m1 = read('A','B');
  auto m2 = read('A','B');
  //auto m3 = read('B','C');
  auto m4 = write('D','B');
  auto m5 = write('D','B');
  std::vector<RelationMatcher> v;
  v.push_back(m1);
  v.push_back(m2);
  //v.push_back(m3);
  v.push_back(m4);
  v.push_back(m5);
  Finder f = Finder(reads, writes, v);
  f.findAndPrint();
}
/*
static __isl_give isl_schedule_node *
	patternMatching(__isl_take isl_schedule_node *node, void *User) {
  isl::schedule_node target = isl::manage_copy(node);
  std::cout << "TargetNode " << target.to_str() << std::endl;
  return node;
} 

static void walkTree(isl::schedule_node node) {
  node = isl::manage(isl_schedule_node_map_descendant_bottom_up(
    node.release(),patternMatching,NULL));
}

TEST(TreeMatcher, matmulFlow) {
  using namespace matchers;
  Scop S = Parser("inputs/matmul.c").getScop();
  auto root = S.schedule.get_root();
  walkTree(root);
	
}
*/
   
TEST(TreeMatcher, matmul) {

/*
  // inputs/matmul.c
  #pragma scop
  for(int i=0; i<N; ++i)
    for(int j=0; j<N; ++j) {
      C[i][j] = 0;
      for(int k=0; k<N; ++k)
        C[i][j] += B[k][j] * A[i][k];
    }
  #pragma endscop
*/

  //using namespace matchers;
  //using namespace constraints;
  //std::cout << "matmul test" << std::endl;
  //Scop S = Parser("inputs/matmul.c").getScop();
  //auto A = read('X','Y');
  //auto B = read('Y', 'Z');
  //S.dump();
  
  //auto listA = buildMatcherConstraintsReads(A,S.reads);
  //std::cout << "listA" << listA << std::endl;
  //auto listB = buildMatcherConstraintsReads(B,S.reads);
  //std::cout << "listB" << listB << std::endl;
  //auto listRes = compareLists(listA, listB);
  //std::cout << "listRes" << listRes << std::endl;

  //A.setDims(listRes.constraints);
  //B.setDims(listRes.constraints);
  //std::cout << A << std::endl;
  //std::cout << B << std::endl;
  
  //auto resA = A.getAccesses(S.reads);
  //std::cout << resA << std::endl;
  //auto resB = B.getAccesses(S.reads);
  //std::cout << resB << std::endl;
  //EXPECT_TRUE((resA.size() == 1)&&(resB.size()==1));

  //auto C = read('X','Y');
  //auto D = read('X','Z');
  //auto listC = buildMatcherConstraintsReads(C,S.reads);
  //auto listD = buildMatcherConstraintsReads(D,S.reads);
  //auto listRess = compareLists(listC, listD);
  //C.setDims(listRess.constraints);
  //D.setDims(listRess.constraints);
  //auto resC = C.getAccesses(S.reads);
  //auto resD = D.getAccesses(S.reads);
  //std::cout << resC << std::endl;
  //std::cout << resD << std::endl;
  //EXPECT_TRUE((resD.size() == 1)&&(resC.size()==1));
}

TEST(TreeMatcher, transpose) {
  using namespace matchers;
  using namespace constraints;
  auto A = read('X','Y');
  auto B = read('Y','X');
}
   
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#include "islutils/access.h"
#include "islutils/access_patterns.h"
#include "islutils/matchers.h"
#include "islutils/builders.h"
#include "islutils/ctx.h"
#include "islutils/parser.h"

#include "gtest/gtest.h"

using util::ScopedCtx;
using namespace matchers;

// print simple C code pretty printer
static void printAST(__isl_keep isl_ast_node *ast, __isl_keep isl_ctx *ctx) {
  isl_ast_print_options *options =
    isl_ast_print_options_alloc(ctx);
  isl_printer *p = isl_printer_to_str(ctx);
  p = isl_printer_set_output_format(p, ISL_FORMAT_C);
  p = isl_printer_indent(p,1);
  p = isl_ast_node_print(ast, p, options);
  char *ASTStr = isl_printer_get_str(p);
  std::cout << "AST Code Gen" << std::endl;
  std::cout << ASTStr << std::endl;
  free(ASTStr);
  isl_printer_free(p);
}

// walk the AST
static void walkASTTree(__isl_keep isl_ast_node *ast) {
  isl_ast_node_foreach_descendant_top_down(
  ast,
  [](__isl_keep isl_ast_node *node, void *user) -> isl_bool {
    // code 
    return isl_bool_true;
  }, nullptr);
}

// Simple function to generate and print AST.
static void simpleASTGen(Scop scop, isl::schedule_node root) {

  auto CTX = ScopedCtx();

  isl_options_set_ast_build_atomic_upper_bound(CTX, true);
  isl_options_set_ast_build_detect_min_max(CTX, true);
  isl_ast_build *build;
  build = isl_ast_build_from_context(scop.context.release());
  isl::schedule schedule = root.get_schedule();
  auto rootAST =
    isl_ast_build_node_from_schedule(build, schedule.release());
  walkASTTree(rootAST);

  /// print AST.
  printAST(rootAST, CTX);

  isl_ast_build_free(build);

}

static int walkScheduleTree(const ScheduleNodeMatcher &matcher,
                                isl::schedule schedule) {
  using namespace matchers;
  auto root = schedule.get_root();
  if(!root)
    return false;

  struct Payload {
    int counter;
    const ScheduleNodeMatcher* m;
  }payload;

  payload.counter = 0;
  payload.m = &matcher;

  isl_schedule_node_foreach_descendant_top_down(
  root.get(),
  [](__isl_keep isl_schedule_node *nodePtr, void *user) -> isl_bool {
    isl::schedule_node node = isl::manage_copy(nodePtr);
    Payload& payloadPtr = *static_cast<Payload*>(user);
    bool res = false;
    res = matchers::ScheduleNodeMatcher::isMatching(*payloadPtr.m, node);
    if(res) {
      payloadPtr.counter++;
    }
    return isl_bool_true;
  }, &payload);

  return payload.counter;
}

namespace builders{

class TileNode{

  public:

    TileNode() : tileSize(32) {}

    TileNode(int x) : tileSize(x) {}

    virtual isl::schedule_node operator() (isl::schedule_node node) {
      assert(isl_schedule_node_get_type(node.get()) == isl_schedule_node_band &&
      "expect band node");

      isl::ctx ctx = node.get_ctx();
      // markers.
      std::string marker = "tiling - tiles";
      isl::id markerTile =
        isl::id::alloc(ctx, marker, nullptr);
      marker = "tiling - points";
      isl::id markerPoints =
        isl::id::alloc(ctx, marker, nullptr);
      
      node = node.insert_mark(markerTile);
      node = node.child(0);
      node = tileNode(node, tileSize);
      node = node.child(0);
      node = node.insert_mark(markerPoints);
      return node.child(0);
    }

  private:
    int tileSize;
    isl::schedule_node tileNode(isl::schedule_node node, int tileSize);
};

isl::schedule_node TileNode::tileNode(isl::schedule_node node, int tileSize) {
  assert(isl_schedule_node_get_type(node.get()) == isl_schedule_node_band &&
         "expect band node");
  isl::ctx ctx = node.get_ctx();
  isl::space space =
      isl::manage(isl_schedule_node_band_get_space(node.get()));
  isl::multi_val sizes = isl::multi_val::zero(space);

  for(size_t i=0; i<space.dim(isl::dim::set); ++i)
    sizes = sizes.set_val(i, isl::val(ctx,tileSize));

  isl::schedule_node tiledNode =
    isl::manage(isl_schedule_node_band_tile(node.release(), sizes.release()));

  return tiledNode;
}

/*
ScheduleNodeBuilder tileAndUnroll(isl::schedule_node node, int tileSize) {
  std::cout << "tile and unroll\n"; 
  assert(isl_schedule_node_get_type(node.get()) == isl_schedule_node_band &&
         "expect band node");
 
  isl::schedule_node tiledNode = tileNode(node, tileSize);
  isl::ctx ctx = tiledNode.get_ctx();
  isl::union_set astOptions = isl::union_set(ctx, "{unroll[x]}");
  
  //clang-format off
  return
    band(tiledNode.band_get_partial_schedule(),
      band(tiledNode.child(0).band_get_partial_schedule(), astOptions));
  //clang-format on 
}

static isl::schedule_node transform(isl::schedule_node t,
           std::function<ScheduleNodeBuilder(isl::schedule_node,int)>) {
  return t;
}
*/

// check if two band nodes are the same. 
static bool isSameBand(isl::schedule_node node, isl::schedule_node target) {
  isl::union_map nodeSchedule = node.get_prefix_schedule_relation();
  isl::union_map targetSchedule = target.get_prefix_schedule_relation();
  if(nodeSchedule.is_equal(targetSchedule))
    return true;
  else return false;
}

// DFS search.
// TODO: change this to return isl::schedule_node
static std::vector<isl::schedule_node> dfsFirst(isl::schedule_node node, 
					isl::schedule_node target) {

  if(isSameBand(node, target)) {
    return {node};
  }

  for(int i = 0, e = node.n_children(); i < e; ++i) {
    node = node.child(i);
    auto res = dfsFirst(node, target);
    if(!res.empty()) {
      return res;
    }
    node = node.parent();
  }

  return{};
}

template <typename Func>
static isl::schedule_node transform(isl::schedule_node root,
 				    isl::schedule_node target,
                                    Func tile) {

  assert(isl_schedule_node_get_type(target.get()) != isl_schedule_node_domain &&
         "target domain not allowed");

  auto nodeTarget = dfsFirst(root, target);
  assert(nodeTarget.size() == 1 && "expect single match");

  nodeTarget[0] = tile(nodeTarget[0]);
  return nodeTarget[0].root();
}

} // end namespace builders

////////////////////////////////////
/// tests
////////////////////////////////////

TEST(Integration, 1mmF) {
  using namespace matchers;
  using namespace builders;
  
  auto ctx = ScopedCtx();
  auto scop = Parser("inputs/1mmF.c").getScop();
  ASSERT_FALSE(scop.schedule.is_null());

  scop.schedule =
    scop.schedule.intersect_domain(scop.nonKillDomain);
  auto root = scop.schedule.get_root();

  // accesses
  isl::union_map reads = scop.reads.curry();
  isl::union_map writes = scop.mustWrites.curry();

  // code gen not opt.
  simpleASTGen(scop,root);

  std::vector<isl::schedule_node> bandTarget(3);
  std::vector<isl::schedule_node> filterTarget(2);
  std::vector<isl::schedule_node> leafTarget(2);

  auto matcher = [&]() {
    using namespace matchers;
    // clang-format off
    return
      band(bandTarget[0],
        band(bandTarget[1],
          sequence(
            filter(filterTarget[0], leaf(leafTarget[0])),
            filter(filterTarget[1],
              band(bandTarget[2], leaf(leafTarget[1]))))));
    // clang-format on
  }();

  // walk the entire tree, we expect three matches.
  int numberOfMatches =
    walkScheduleTree(matcher, scop.schedule);

  EXPECT_EQ(numberOfMatches, 1);
 
  // check accesses
  auto _1 = placeholder(ctx);
  auto _2 = placeholder(ctx);
  auto _3 = placeholder(ctx); 
 
  auto psRead = allOf(access(dim(0, _1), dim(1, _2)),
                      access(dim(0, _3), dim(1, _2)),
                      access(dim(0, _1), dim(1, _3)));

  auto psWrite = allOf(access(dim(0, _1), dim(1, _2)));

  auto matches = match(reads, psRead);
  EXPECT_EQ(matches.size(), 1);
  matches = match(writes, psWrite);
  EXPECT_EQ(matches.size(), 2);

  // check stride.
  EXPECT_EQ(match(reads, allOf(access(dim(1, stride(ctx,1))))).size(), 1);

  // Ideas for transformation:
  // 1. completely change a give subtree (i.e. matmul)
  // Cons: in the paper we claim to provide a composable set
  // of transformations, but this is not a case.
  // 2. the user can specify for each node the type of transformations
  // he/she wants (i.e. transform(targetNode, tile))
  // Cons: the user may loose track of the transformation applied. 
 
  TileNode applyTiling = TileNode(32); 
  root = transform(root, bandTarget[2], applyTiling);
  //std::cout << root.to_str() << std::endl;
  root = transform(root, bandTarget[1], applyTiling);
  //std::cout << root.to_str() << std::endl;
  root = transform(root, bandTarget[0], applyTiling);
  //std::cout << root.to_str() << std::endl;
  
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

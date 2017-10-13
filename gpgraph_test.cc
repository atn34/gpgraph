#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "gpgraph.h"

TEST(Cycle, NoCycle) {
  GpGraph g(3);
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 2);
  EXPECT_FALSE(g.find_cycle());
}

TEST(Cycle, Cycle1) {
  GpGraph g(3);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(2, 0);
  std::vector<int> cycle;
  ASSERT_TRUE(g.find_cycle(&cycle));
  EXPECT_THAT(cycle, testing::ElementsAre(2, 1, 0));
}

TEST(Cycle, Cycle2) {
  GpGraph g(5);
  g.add_edge(0, 1);
  g.add_edge(0, 4);
  g.add_edge(1, 2);
  g.add_edge(1, 3);
  g.add_edge(1, 4);
  g.add_edge(2, 0);
  std::vector<int> cycle;
  ASSERT_TRUE(g.find_cycle(&cycle));
  EXPECT_THAT(cycle, testing::ElementsAre(2, 1, 0));
}

TEST(TopoSort, NoCycle) {
  GpGraph g(4);
  // Tree edges
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(2, 3);
  // Forward edges
  g.add_edge(0, 2);
  g.add_edge(0, 3);
  g.add_edge(1, 3);

  std::vector<int> sorted;
  ASSERT_TRUE(g.topo_sort(&sorted));
  EXPECT_THAT(sorted, testing::ElementsAre(0, 1, 2, 3));
}

TEST(TopoSort, Cycle) {
  GpGraph g(3);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(2, 0);
  std::vector<int> sorted;
  EXPECT_FALSE(g.topo_sort(&sorted));
}

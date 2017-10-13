#ifndef GPGRAPH_H
#define GPGRAPH_H

#include <algorithm>
#include <vector>

template <int SCRATCH_SIZE_PER_NODE = 8>
class GpGraph {
 public:
  GpGraph(int size) : nodes_(size) {}
  virtual ~GpGraph() {}

  void add_edge(int u, int v) { nodes_[u].neighbors.push_back(v); }

  /**
   * Reports if a cycle exists. If `cycle` != nullptr, push elements along one
   * cycle in reverse order onto `cycle`. If multiple cycles exist, the
   * particular cycle returned is unspecified.
   *
   * See gpgraph_test.cc for examples.
   */
  bool find_cycle() const {
    init_scratch<ScratchFindCycle>();
    bool cycle_exists = false;
    dfs(NoopNode{},
        [&cycle_exists](const Node &, const Node &v_node) {
          if (reinterpret_cast<ScratchFindCycle *>(&v_node.scratch)->color ==
              Color::GREY) {
            cycle_exists = true;
            return false;
          }
          return true;
        },
        NoopNode{});
    return cycle_exists;
  }
  bool find_cycle(std::vector<int> *cycle) const {
    init_scratch<ScratchReportCycle>();
    bool cycle_exists = false;
    dfs(NoopNode{},
        [this, cycle, &cycle_exists](const Node &u_node, const Node &v_node) {
          if (reinterpret_cast<ScratchReportCycle *>(&v_node.scratch)->color ==
              Color::GREY) {
            cycle_exists = true;
            if (cycle != nullptr) {
              int u = NodeIndex(u_node);
              int v = NodeIndex(v_node);
              while (u != v) {
                cycle->push_back(u);
                u = reinterpret_cast<ScratchReportCycle *>(&nodes_[u].scratch)
                        ->parent;
              }
              cycle->push_back(u);
            }
            return false;
          }
          return true;
        },
        NoopNode{});
    return cycle_exists;
  }
  /**
   * Modifies `order` to contain an ordering of elements such that for each edge
   * (u, v), u is before v in order.
   *
   * Returns false if none exists.
   */
  bool topo_sort(std::vector<int> *order) const {
    init_scratch<ScratchFindCycle>();
    order->clear();
    bool cycle_exists = false;
    dfs(NoopNode{},
        [&cycle_exists](const Node &, const Node &v_node) {
          if (reinterpret_cast<ScratchFindCycle *>(&v_node.scratch)->color ==
              Color::GREY) {
            cycle_exists = true;
            return false;
          }
          return true;
        },
        [this, order](const Node &u_node) {
          order->push_back(NodeIndex(u_node));
        });
    std::reverse(order->begin(), order->end());
    return !cycle_exists;
  }

 private:
  enum class Color : char { WHITE, GREY, BLACK };

  struct ScratchFindCycle {
    Color color = Color::WHITE;
  };

  struct ScratchReportCycle {
    Color color = Color::WHITE;
    int parent = -1;
  };

  struct Node {
    std::vector<int> neighbors = {};
    mutable char scratch[SCRATCH_SIZE_PER_NODE];
  };

  template <typename Scratch>
  void init_scratch() const {
    for (auto &node : nodes_) {
      new (&node.scratch) Scratch;
    }
  }

  struct NoopNode {
    bool operator()(const Node &) const { return true; }
  };

  struct NoopEdge {
    bool operator()(const Node &, const Node &) const { return true; }
  };

  template <typename ProcessNodeEarly, typename ProcessEdge,
            typename ProcessNodeLate>
  void dfs(const ProcessNodeEarly &process_node_early,
           const ProcessEdge &process_edge,
           const ProcessNodeLate &process_node_late) const {
    Dfs<const ProcessNodeEarly &, const ProcessEdge &, const ProcessNodeLate &>
        d(process_node_early, process_edge, process_node_late, nodes_);
    d.perform_dfs();
  }

  template <typename ProcessNodeEarly, typename ProcessEdge,
            typename ProcessNodeLate>
  struct Dfs {
    Dfs(ProcessNodeEarly process_node_early, ProcessEdge process_edge,
        ProcessNodeLate process_node_late, const std::vector<Node> &nodes)
        : process_node_early_(process_node_early),
          process_edge_(process_edge),
          process_node_late_(process_node_late),
          nodes_(nodes) {}
    ProcessNodeEarly process_node_early_;
    ProcessEdge process_edge_;
    ProcessNodeLate process_node_late_;
    const std::vector<Node> &nodes_;

    void perform_dfs() {
      for (auto &node : nodes_) {
        if (reinterpret_cast<ScratchReportCycle *>(&node.scratch)->color ==
            Color::WHITE) {
          dfs_helper(&node - &nodes_[0]);
        }
      }
    }

    void dfs_helper(int u) {
      auto &u_node = nodes_[u];
      reinterpret_cast<ScratchReportCycle *>(&u_node.scratch)->color =
          Color::GREY;
      if (!process_node_early_(u_node)) {
        return;
      }
      for (int v : u_node.neighbors) {
        auto &v_node = nodes_[v];
        if (reinterpret_cast<ScratchReportCycle *>(&v_node.scratch)->color ==
            Color::WHITE) {
          reinterpret_cast<ScratchReportCycle *>(&v_node.scratch)->parent = u;
        }
        if (!process_edge_(u_node, v_node)) {
          return;
        }
        if (reinterpret_cast<ScratchReportCycle *>(&v_node.scratch)->color ==
            Color::WHITE) {
          dfs_helper(v);
        }
      }
      reinterpret_cast<ScratchReportCycle *>(&u_node.scratch)->color =
          Color::BLACK;
      process_node_late_(u_node);
    }
  };

  int NodeIndex(const Node &node) const { return &node - &nodes_[0]; }

  std::vector<Node> nodes_;
};

#endif /* ifndef GPGRAPH_H */

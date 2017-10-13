#ifndef GPGRAPH_H
#define GPGRAPH_H

#include <vector>

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
  bool find_cycle(std::vector<int> *cycle = nullptr) {
    bool cycle_exists = false;
    dfs(NoopNode{},
        [this, cycle, &cycle_exists](Node &u_node, Node &v_node) {
          if (v_node.color == Color::GREY) {
            cycle_exists = true;
            if (cycle != nullptr) {
              int u = NodeIndex(u_node);
              int v = NodeIndex(v_node);
              while (u != v) {
                cycle->push_back(u);
                u = nodes_[u].parent;
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

 private:
  enum class Color : char { WHITE, GREY, BLACK };

  struct Node {
    std::vector<int> neighbors = {};
    Color color = Color::WHITE;
    int parent = -1;
  };

  void reset() {
    for (auto &node : nodes_) {
      node.color = Color::WHITE;
    }
  }

  struct NoopNode {
    bool operator()(Node &) const { return true; }
  };

  struct NoopEdge {
    bool operator()(Node &, Node &) const { return true; }
  };

  template <typename ProcessNodeEarly, typename ProcessEdge,
            typename ProcessNodeLate>
  void dfs(const ProcessNodeEarly &process_node_early,
           const ProcessEdge &process_edge,
           const ProcessNodeLate &process_node_late) {
    Dfs<const ProcessNodeEarly &, const ProcessEdge &, const ProcessNodeLate &>
        d(process_node_early, process_edge, process_node_late, nodes_);
    d.perform_dfs();
  }

  template <typename ProcessNodeEarly, typename ProcessEdge,
            typename ProcessNodeLate>
  struct Dfs {
    Dfs(ProcessNodeEarly process_node_early, ProcessEdge process_edge,
        ProcessNodeLate process_node_late, std::vector<Node> &nodes)
        : process_node_early_(process_node_early),
          process_edge_(process_edge),
          process_node_late_(process_node_late),
          nodes_(nodes) {}
    ProcessNodeEarly process_node_early_;
    ProcessEdge process_edge_;
    ProcessNodeLate process_node_late_;
    std::vector<Node> &nodes_;

    /**
     * Precondition: \forall node \in nodes_ :
     *      node.color == WHITE &&
     *      node.parent == -1 &&
     */
    void perform_dfs() {
      for (auto &node : nodes_) {
        if (node.color == Color::WHITE) {
          dfs_helper(&node - &nodes_[0]);
        }
      }
    }

    void dfs_helper(int u) {
      auto &u_node = nodes_[u];
      u_node.color = Color::GREY;
      if (!process_node_early_(u_node)) {
        return;
      }
      for (int v : u_node.neighbors) {
        auto &v_node = nodes_[v];
        if (v_node.color == Color::WHITE) {
          v_node.parent = u;
        }
        if (!process_edge_(u_node, v_node)) {
          return;
        }
        if (v_node.color == Color::WHITE) {
          dfs_helper(v);
        }
      }
      u_node.color = Color::BLACK;
      process_node_late_(u_node);
    }
  };

  int NodeIndex(Node &node) { return &node - &nodes_[0]; }

  std::vector<Node> nodes_;
};

#endif /* ifndef GPGRAPH_H */

#ifndef GPGRAPH_H
#define GPGRAPH_H

#include <vector>

class GpGraph {
 public:
  GpGraph(int size) { nodes_.resize(size); };
  virtual ~GpGraph() {}

  void add_edge(int u, int v) { nodes_[u].neighbors.push_back(v); }

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
    std::vector<int> neighbors;
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
        dfs(process_node_early, process_edge, process_node_late, nodes_);
    dfs.perform_dfs();
  }

  template <typename ProcessNodeEarly, typename ProcessEdge,
            typename ProcessNodeLate>
  struct Dfs {
    Dfs(ProcessNodeEarly process_node_early, ProcessEdge process_edge,
        ProcessNodeLate process_node_late, std::vector<Node> &nodes)
        : process_node_early{process_node_early},
          process_edge{process_edge},
          process_node_late{process_node_late},
          nodes{nodes} {}
    ProcessNodeEarly process_node_early;
    ProcessEdge process_edge;
    ProcessNodeLate process_node_late;
    std::vector<Node> &nodes;

    /**
     * Precondition: \forall node \in nodes :
     *      node.color == WHITE &&
     *      node.parent == -1 &&
     */
    void perform_dfs() {
      for (auto &node : nodes) {
        if (node.color == Color::WHITE) {
          dfs_helper(&node - &nodes[0]);
        }
      }
    }

    void dfs_helper(int u) {
      auto &u_node = nodes[u];
      u_node.color = Color::GREY;
      if (!process_node_early(u_node)) {
        return;
      }
      for (int v : u_node.neighbors) {
        auto &v_node = nodes[v];
        if (v_node.color == Color::WHITE) {
          v_node.parent = u;
        }
        if (!process_edge(u_node, v_node)) {
          return;
        }
        if (v_node.color == Color::WHITE) {
          dfs_helper(v);
        }
      }
      u_node.color = Color::BLACK;
      process_node_late(u_node);
    }
  };

  int NodeIndex(Node &node) { return &node - &nodes_[0]; }

  std::vector<Node> nodes_;
};

#endif /* ifndef GPGRAPH_H */

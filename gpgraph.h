#ifndef GPGRAPH_H
#define GPGRAPH_H

#include <algorithm>
#include <vector>

struct Empty {};

/**
 * Graph library that can use a configurable amount of scratch space per node.
 *
 * That way, if you only use algorithms that require little scratch space you
 * don't have to pay the extra memory.
 */
template <int SCRATCH_SIZE_PER_NODE = 8, typename EdgeData = Empty>
class GpGraph {
 public:
  explicit GpGraph(int size) : nodes_(size) {}
  virtual ~GpGraph() {}

  template <typename... Ts>
  void add_edge(int u, int v, Ts &&... args) {
    nodes_[u].neighbors.emplace_back(v, std::forward<Ts>(args)...);
  }

  /**
   * Reports if a cycle exists. If `cycle` != nullptr, push elements along one
   * cycle in reverse order onto `cycle`. If multiple cycles exist, the
   * particular cycle returned is unspecified.
   *
   * See gpgraph_test.cc for examples.
   */
  bool find_cycle() const {
    using Scratch = ScratchFindCycle;
    static_assert(SCRATCH_SIZE_PER_NODE >= sizeof(Scratch),
                  "Increase SCRATCH_SIZE_PER_NODE");
    init_scratch<Scratch>();
    bool cycle_exists = false;
    dfs<Scratch>(NoopNode{},
                 [this, &cycle_exists](const Node &, const Edge &edge) {
                   const Node &v_node = nodes_[edge.neighbor];
                   if (reinterpret_cast<Scratch *>(&v_node.scratch)->color ==
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
    using Scratch = ScratchReportCycle;
    static_assert(SCRATCH_SIZE_PER_NODE >= sizeof(Scratch),
                  "Increase SCRATCH_SIZE_PER_NODE");
    init_scratch<Scratch>();
    bool cycle_exists = false;
    dfs<Scratch>(
        NoopNode{},
        [this, cycle, &cycle_exists](const Node &u_node, const Edge &edge) {
          const Node &v_node = nodes_[edge.neighbor];
          int u = NodeIndex(u_node);
          int v = NodeIndex(v_node);
          if (reinterpret_cast<Scratch *>(&v_node.scratch)->color ==
              Color::WHITE) {
            reinterpret_cast<Scratch *>(&v_node.scratch)->parent = u;
          }
          if (reinterpret_cast<Scratch *>(&v_node.scratch)->color ==
              Color::GREY) {
            cycle_exists = true;
            if (cycle != nullptr) {
              while (u != v) {
                cycle->push_back(u);
                u = reinterpret_cast<Scratch *>(&nodes_[u].scratch)->parent;
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
    using Scratch = ScratchFindCycle;
    static_assert(SCRATCH_SIZE_PER_NODE >= sizeof(Scratch),
                  "Increase SCRATCH_SIZE_PER_NODE");
    init_scratch<Scratch>();
    order->clear();
    bool cycle_exists = false;
    dfs<Scratch>(NoopNode{},
                 [this, &cycle_exists](const Node &, const Edge &edge) {
                   const Node &v_node = nodes_[edge.neighbor];
                   if (reinterpret_cast<Scratch *>(&v_node.scratch)->color ==
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
    int parent = -1;
    Color color = Color::WHITE;
  };

  // Inherit from EdgeData to enable empty base optimization if EdgeData is an
  // empty class.
  struct Edge : public EdgeData {
    template <typename... Ts>
    Edge(int n, Ts &&... args)
        : EdgeData(std::forward<Ts>(args)...), neighbor{n} {}

    int neighbor;
  };

  struct Node {
    std::vector<Edge> neighbors = {};
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

  template <typename Scratch, typename ProcessNodeEarly, typename ProcessEdge,
            typename ProcessNodeLate>
  void dfs(const ProcessNodeEarly &process_node_early,
           const ProcessEdge &process_edge,
           const ProcessNodeLate &process_node_late) const {
    Dfs<const ProcessNodeEarly &, const ProcessEdge &, const ProcessNodeLate &,
        Scratch>
        d(process_node_early, process_edge, process_node_late, nodes_);
    d.perform_dfs();
  }

  template <typename ProcessNodeEarly, typename ProcessEdge,
            typename ProcessNodeLate, typename Scratch>
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
        if (reinterpret_cast<Scratch *>(&node.scratch)->color == Color::WHITE) {
          dfs_helper(&node - &nodes_[0]);
        }
      }
    }

    void dfs_helper(int u) {
      auto &u_node = nodes_[u];
      reinterpret_cast<Scratch *>(&u_node.scratch)->color = Color::GREY;
      if (!process_node_early_(u_node)) {
        return;
      }
      for (const auto &edge : u_node.neighbors) {
        int v = edge.neighbor;
        auto &v_node = nodes_[v];
        if (!process_edge_(u_node, edge)) {
          return;
        }
        if (reinterpret_cast<Scratch *>(&v_node.scratch)->color ==
            Color::WHITE) {
          dfs_helper(v);
        }
      }
      reinterpret_cast<Scratch *>(&u_node.scratch)->color = Color::BLACK;
      process_node_late_(u_node);
    }
  };

  int NodeIndex(const Node &node) const { return &node - &nodes_[0]; }

  std::vector<Node> nodes_;
};

#endif /* ifndef GPGRAPH_H */

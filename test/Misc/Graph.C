// RUN: %cladclang %s -I%S/../../include -oGraph.out 2>&1
// RUN: ./Graph.out | FileCheck -check-prefix=CHECK-EXEC %s
// CHECK-NOT: {{.*error|warning|note:.*}}

#include "clad/Differentiator/Graph.h"
#include <string>
#include <vector>

// Custom type for representing nodes in the graph.
struct Node {
  std::string name;
  int id;

  Node(std::string name, int id) : name(name), id(id) {}

  bool operator==(const Node& other) const {
      return name == other.name && id == other.id;
  }

  // string operator for printing the node.
  operator std::string() const {
    return name + std::to_string(id);
  }
};

// Specialize std::hash for the Node type.
template<>
struct std::hash<Node> {
  std::size_t operator()(const Node& n) const {
    return std::hash<std::string>()(n.name) ^ std::hash<int>()(n.id);
  }
};

int main () {
  clad::Graph<Node> G;
  for (int i = 0; i < 6; i++) {
    Node n("node", i);
    if (i == 0) {
      G.addNode(n, true/*isSource*/);
    }
    Node m("node", i + 1);
    G.addEdge(n, m);
  }
  std::vector <Node> nodes = G.getNodes();
  std::cout << "Nodes in the graph: " << nodes.size() << "\n";
  // CHECK-EXEC: Nodes in the graph: 7

  // edge from node 0 to node 3 and node 4 to node 0.
  G.addEdge(nodes[0], nodes[3]);
  G.addEdge(nodes[4], nodes[0]);
  std::vector <Node> nodes2 = G.getNodes();
  std::cout << "Nodes in the graph: " << nodes2.size() << "\n";
  // CHECK-EXEC: Nodes in the graph: 7

  // remove node 4
  G.removeNode(nodes[4]);
  G.removeNonReachable(); // removes node 5 and 6
  G.print();
  // CHECK-EXEC: node0: #0 (source)
  // CHECK-EXEC-NEXT: node1: #1
  // CHECK-EXEC-NEXT: node2: #2
  // CHECK-EXEC-NEXT: node3: #3
  // CHECK-EXEC-NEXT: 0 -> 1
  // CHECK-EXEC-NEXT: 0 -> 3
  // CHECK-EXEC-NEXT: 1 -> 2
  // CHECK-EXEC-NEXT: 2 -> 3
}


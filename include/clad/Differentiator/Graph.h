#ifndef CLAD_GRAPH_H
#define CLAD_GRAPH_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace clad{
template<typename T>
class Graph {
private:

  // Storing nodes in the graph. The index of the node in the vector is used as
  // a unique identifier for the node in the adjacency list.
  std::vector<T> nodes;

  // Store the nodes in the graph as an unordered map from the node to a boolean
  // indicating whether the node is still present in the graph, along with an
  // integer indicating the insertion order of the node.
  std::unordered_map<T, std::pair<bool, int>> nodeMap;

  // Store the adjacency list for the graph. The adjacency list is a map from
  // a node to the set of nodes that it has an edge to. We use integers inside
  // the set to avoid copying the nodes.
  std::unordered_map<size_t, std::set<size_t>> adjList;

  // Store the reverse adjacency list for the graph. The reverse adjacency list
  // is a map from a node to the set of nodes that have an edge to it. We use
  // integers inside the set to avoid copying the nodes.
  std::unordered_map<size_t, std::set<size_t>> revAdjList;

  // Set of source nodes in the graph.
  std::set<size_t> sources;

public:
  Graph() = default;

  // Add an edge from src to dest
  void addEdge(const T& src, const T& dest) {
    addNode(src);
    addNode(dest);
    size_t srcId = nodeMap[src].second;
    size_t destId = nodeMap[dest].second;
    adjList[srcId].insert(destId);
    revAdjList[destId].insert(srcId);
  }

  // Add a node to the graph
  void addNode(const T& node, bool isSource = false) {
    if (nodeMap.find(node) == nodeMap.end()) {
      size_t id = nodes.size();
      nodes.push_back(node);
      nodeMap[node] = {true, id};
      adjList[id] = {};
      revAdjList[id] = {};
      if (isSource)
        sources.insert(id);
    } else if (nodeMap[node].first == false) {
      nodeMap[node].first = true;
    }
  }

  // Remove a node from the graph. This will also remove all edges to and from
  // the node.
  void removeNode(const T& node) {
    if (nodeMap.find(node) != nodeMap.end()) {
      size_t id = nodeMap[node].second;
      nodeMap[node].first = false;
      for (size_t destId : adjList[id]) {
        revAdjList[destId].erase(id);
      }
      adjList[id].clear();
      for (size_t srcId : revAdjList[id]) {
        adjList[srcId].erase(id);
      }
      revAdjList[id].clear();
    }
  }

  // Get nodeMap in the graph in the order they were inserted.
  std::vector<T> getNodes() {
    std::vector<T> res;
    // iterate over the nodes vector and add the nodes that are still present in
    // the graph.
    for (const T& node : nodes) {
      if (nodeMap[node].first)
        res.push_back(node);
    }
    return res;
  }

  // Check if two nodes are connected in the graph.
  bool isConnected(const T& src, const T& dest) {
    if (nodeMap.find(src) == nodeMap.end() || nodeMap.find(dest) == nodeMap.end())
      return false;
    size_t srcId = nodeMap[src].second;
    size_t destId = nodeMap[dest].second;
    return adjList[srcId].find(destId) != adjList[srcId].end();
  }

  // Print the graph in a human-readable format.
  void print() {
    // First print the nodes with their insertion order.
    for (const T& node : nodes) {
      std::pair<bool, int> nodeInfo = nodeMap[node];
      if (nodeInfo.first) {
        std::cout << (std::string)node << ": #" << nodeInfo.second;
        if (sources.find(nodeInfo.second) != sources.end())
          std::cout << " (source)";
        std::cout << "\n";
      }
    }
    // Then print the edges.
    for (int i = 0; i < nodes.size(); i++) {
      if (!nodeMap[nodes[i]].first)
        continue;
      for (size_t dest : adjList[i])
        std::cout << i << " -> " << dest << "\n";
    }
  }

  // Remove non reachable nodes from the sources.
  void removeNonReachable() {
    std::unordered_set<size_t> visited;
    std::vector<size_t> stack;
    for (size_t source : sources) {
      stack.push_back(source);
      visited.insert(source);
    }
    while (!stack.empty()) {
      size_t node = stack.back();
      stack.pop_back();
      for (size_t dest : adjList[node]) {
        if (visited.find(dest) == visited.end()) {
          stack.push_back(dest);
          visited.insert(dest);
        }
      }
    }
    for (auto it = nodeMap.begin(); it != nodeMap.end(); ++it) {
      if (it->second.first && visited.find(it->second.second) == visited.end())
        removeNode(it->first);
    }
  }

  // Topological sort of the directed graph. If the graph is not a DAG, the
  // result will be a partial order. Use a recursive dfs heler function to
  // implement the topological sort. If a->b, then a will come before b in the
  // topological sort. In reverseOrder mode, the result will be in reverse
  // topological order, i.e a->b, then b will come before a in the result.
  std::vector<T> topologicalSort(bool reverseOrder = false) {
    std::vector<T> res;
    std::unordered_set<size_t> visited;

    std::function<void(size_t)> dfs = [&](size_t node) -> void {
      visited.insert(node);
      for (size_t dest : adjList[node])
        if (visited.find(dest) == visited.end())
          dfs(dest);
      res.push_back(nodes[node]);
    };
    for (size_t source : sources)
      if (visited.find(source) == visited.end())
        dfs(source);

    if (reverseOrder)
      return res;
    std::reverse(res.begin(), res.end());
    return res;
  }
};
} // end namespace clad

#endif // CLAD_GRAPH_H
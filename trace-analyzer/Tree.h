#ifndef TREE_TREE_H
#define TREE_TREE_H

#include <string>
#include <vector>
#include <memory>

using namespace std;

struct instruction {
  uint64_t sid;
  std::string type;
  uint64_t post;
  std::string val;
  uint64_t line;
};

struct Node {
public:
  std::shared_ptr<Node> parent;
  instruction in;
  vector<std::shared_ptr<Node>> children;
  string s;
  uint64_t id;
};

class Tree {
public:
  Tree(std::shared_ptr<Node> n);
  std::shared_ptr<Node> root;
};

#endif //TREE_TREE_H

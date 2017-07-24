#include "Tree.h"

Tree::Tree(std::shared_ptr<Node> n) {
	root = n;
}

// void Tree::Insert(std::shared_ptr<Node> parent, std::shared_ptr<Node> child) {
// 	parent->children.push_back(child);
// }
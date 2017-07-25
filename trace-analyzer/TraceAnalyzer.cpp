#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <set>

#include "Tree.h"

using namespace std;

uint64_t idCounter = 0;

vector<instruction> parseInstructions() {
	string line;
	std::vector<instruction> instructions;
	// TODO: soft code this.
	ifstream myfile ("trace.txt");
	std::string token;
	std::string delimiter = "|";
	std::string s;

	while (getline (myfile, s )) {
		uint64_t pos = 0;
		int count = 0;
		instruction in;
		while ((pos = s.find(delimiter)) != std::string::npos) {
			token = s.substr(0, pos);
			if (count == 0) {
				in.sid = stoi(token);
			} else if (count == 1) {
				in.type = token;
			} else if (count == 2) {
				in.post = stoi(token);
			} else if (count == 3)  {
				in.line = stoll(token);
			} else if (count == 4) {
				in.val = token;
			}
			count++;
			s.erase(0, pos + delimiter.length());
		}
		instructions.push_back(in);
	}
	return instructions;
}

void updateCDS(vector<std::shared_ptr<Node> > &cds, instruction in, int &index) {
	while (cds.size() != 0 && cds.back()->in.post == in.sid) {
		cds.pop_back();
		index--;
	}

	if (in.type == "MethodCall" || in.type == "Predicate") {
		std::shared_ptr<Node> node(new Node());
		node->in = in;
		
		node->id = idCounter;
		node->s = std::to_string(in.sid);
		idCounter++;
		cds.push_back(node);
	}
}

void constructTree(instruction in, vector<std::shared_ptr<Node> > &cds, Tree &tree, int& index) {
	updateCDS(cds, in, index);
	if ((in.type == "MethodCall" || in.type == "Predicate") && in.val != "") {
		std::shared_ptr<Node> ix(new Node());
		ix->s = in.val;

		ix->id = idCounter;
		idCounter++;

		while (index < (signed)cds.size() - 1) {
			

			if (index == -1) {
				tree.root = cds.at(0);
			} else {
				cds.at(index)->children.push_back(cds.at(index+1));
				cds.at(index+1)->parent = cds.at(index);
			}

			index++;
		}
		cds.at(index)->children.push_back(ix);
	}
}

void transformTree(std::shared_ptr<Node> node) {
	for (int i = 0; i < node->children.size(); i++) {
		if(node->children.at(i)->children.size() == 1) {
			node->children.at(i)->children.at(0)->parent = node;
			node->children.at(i) = node->children.at(i)->children.at(0);
		}
	}
	for (int i = 0; i < node->children.size(); i++) {
		transformTree(node->children.at(i));
	}
}

void generateCSV(std::shared_ptr<Node> node, std::ostream& os) {
	for (int i = 0; i < node->children.size(); i++) {
		if (node->children.at(i)->children.size() != 0) {
			os << node->id << ", " << node->s << ", " << node->in.line << ", "
				 << node->children.at(i)->id << ", 1" << endl;
		} else {
			os << node->id << ", " << node->s << ", " << node->in.line << ", "
				 << node->children.at(i)->id << "[" + node->children.at(i)->s + "]" << ", 1" << endl;
		}
		generateCSV(node->children.at(i), os); 
	}
}

void printNonTerminals(std::shared_ptr<Node> node, std::ostream& os) {
	if (node->children.size() != 0) {
		os << node->s << "\n";
	}
	for (int i=0; i < node->children.size(); i++) {
		printNonTerminals(node->children.at(i), os);
	}
}

void printTerminals(std::shared_ptr<Node> node, std::vector<std::string> &vec) {
	if (node->children.size() == 0) {
		vec.push_back(node->s);
	}
	for (int i=0; i < node->children.size(); i++) {
		printTerminals(node->children.at(i), vec);
	}
}

void printRules(std::shared_ptr<Node> node, std::ostream& os) {
	if (node->children.size() != 0) {
		os << node->s << " -> ";
		for (int i=0; i < node->children.size(); i++) {
			if (i == node->children.size() -1) {
				os << node->children.at(i)->s;
			} else {
				os << node->children.at(i)->s << " + ";
			}
		} 
		os << endl;
	}
	for (int i=0; i < node->children.size(); i++) {
		printRules(node->children.at(i), os);
	}
}

void makeGrammar(std::shared_ptr<Node> root, std::ostream& os) {
	os << "START:\n";
	os << root->s << "\n";
	os << "NONTERMINALS:\n";
	printNonTerminals(root, os);

	vector<std::string> vec;
	os << "TERMINALS:\n";
	printTerminals(root, vec);
	std::set<std::string> st(vec.begin(), vec.end());
	for (auto s : st) {
		os << s << endl;
	}
	os << "RULES\n";
	printRules(root, os);
}

int main() {
	vector<instruction> instructions = parseInstructions();
	vector<std::shared_ptr<Node> > cds;
	int index = -1;
	std::shared_ptr<Node> n;
	Tree tree(n);

	for(auto in : instructions) {
		constructTree(in, cds, tree, index);
	}

	transformTree(tree.root);

	ofstream csvfile;
	ofstream grammar;
	// TODO: remove the hard dependency on these file names.
	csvfile.open("file.csv");
	grammar.open("grammar.txt");
	generateCSV(tree.root, csvfile);

	makeGrammar(tree.root, grammar);

	return 0;
}

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
#include "cxxopts.hpp"
#include "enumerate.hpp"

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
		auto node = std::make_shared<Node>();
		node->in = in;
		
		node->id = idCounter;
		node->s = std::to_string(in.sid);
		idCounter++;
		cds.push_back(node);
	}
}

void constructTree(
	instruction in,
	vector<std::shared_ptr<Node> > &cds,
	Tree &tree,
	int& index
) {
	updateCDS(cds, in, index);

	if ((in.type == "MethodCall" || in.type == "Predicate") && in.val != "") {
		auto ix = std::shared_ptr<Node>();
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
	for (const auto& x : enumerate(node->children)) {
		const auto i = x.index;
		const auto& child = x.item;
		if (child->children.size() == 1) {
			child->children.at(0)->parent = node;
			node->children.at(i) = node->children.at(i)->children.at(0);
		}
	}
	for (auto& child : node->children) {
		transformTree(child);
	}
}

void generateCSV(std::shared_ptr<Node> node, std::ostream& os) {
	for (auto& child : node->children) {
		if (child->children.size() != 0) {
			os << node->id << ", " << node->s << ", " << node->in.line << ", "
				 << child->id << ", 1" << endl;
		} else {
			os << node->id << ", " << node->s << ", " << node->in.line << ", "
				 << child->id << "[" + child->s + "]" << ", 1" << endl;
		}
		generateCSV(child, os); 
	}
}

void printNonTerminals(std::shared_ptr<Node> node, std::ostream& os) {
	if (node->children.size() != 0) {
		os << node->s << "\n";
	}
	for (auto& child : node->children) {
		printNonTerminals(child, os);
	}
}

void printTerminals(std::shared_ptr<Node> node, std::vector<std::string> &vec) {
	if (node->children.size() == 0) {
		vec.push_back(node->s);
	}
	for (auto& child : node->children) {
		printTerminals(child, vec);
	}
}

void printRules(std::shared_ptr<Node> node, std::ostream& os) {
	if (node->children.size() != 0) {
		os << node->s << " -> ";
		for (const auto& x : enumerate(node->children)) {
			const auto i = x.index;
			const auto& child = x.item;

			if (i == node->children.size() - 1) {
				os << child->s;
			} else {
				os << child->s << " + ";
			}
		}
		os << endl;
	}
	for (auto& child : node->children) {
		printRules(child, os);
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

int main(int argc, char* argv[]) {
	try {
		cxxopts::Options options(argv[0], "Parse the trace for generating grammar\n");
		options.add_options()
			("t,trace", "The trace file", cxxopts::value<std::string>())
			("c,csv", "The CSV metadata output", cxxopts::value<std::string>())
			("g,grammar", "The grammar tree output", cxxopts::value<std::string>())
		;

		options.parse(argc, argv);

		if (!options.count("t")) {
			std::cout << "Please supply a trace file" << std::endl << std::endl;
			std::cout << options.help() << std::endl;
			exit(1);
		}

		if (!options.count("g")) {
			std::cout << "Please supply a trace file" << std::endl << std::endl;
			std::cout << options.help() << std::endl;
			exit(1);
		}

		auto instructions = parseInstructions();
		vector<std::shared_ptr<Node> > cds;
		int index = -1;
		std::shared_ptr<Node> n;
		Tree tree(n);

		for(auto in : instructions) {
			constructTree(in, cds, tree, index);
		}

		transformTree(tree.root);

		if (options.count("c")) {
			ofstream csvfile;
			auto& csvFilename = options["c"].as<std::string>();
			csvfile.open(csvFilename);
			generateCSV(tree.root, csvfile);
		}

		ofstream grammar;
		auto& grammarFilename = options["g"].as<std::string>();
		grammar.open(grammarFilename);
		makeGrammar(tree.root, grammar);

	} catch (const cxxopts::OptionException& e) {
		std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
	}

	return 0;
}

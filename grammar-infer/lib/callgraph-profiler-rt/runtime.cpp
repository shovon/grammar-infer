#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <queue>
using namespace std;

extern "C" {

  // This macro allows us to prefix strings so that they are less likely to
  // conflict with existing symbol names in the examined programs.
  // e.g. CGPROF(entry) yields CaLlPrOfIlEr_entry
  #define GIPROF(X) G1Pr0_##X

  FILE* GIPROF(file) = fopen("test.txt" , "r");
  FILE* GIPROF(traceFile) = fopen("ended_trace.txt", "w");

  struct GIPROF(instruction) {
    GIPROF(instruction)() {
      isParameterizedGetChar = false;
    }
    uint64_t sid;
    std::string type;
    uint64_t post;
    std::string val;
    uint64_t line;
    uint64_t lastGetChar;
    bool isParameterizedGetChar;
  };

  std::vector<GIPROF(instruction)> *GIPROF(instructions)
    = new std::vector<GIPROF(instruction)>();
  std::queue<char> *GIPROF(fgetcCharacters)
    = new std::queue<char>();

  void
  GIPROF(fgetcCalled)(int cint) {
    // The function accepts an int because fgetc returns an int. Otherwise, this
    // function whould have accepted a char.

    char c = cint;

    GIPROF(fgetcCharacters)->push(c);
  }

  void
  GIPROF(programEnded)() {
    int count = 0;
    for (auto& inst : *GIPROF(instructions)) {
      count++;
      if (inst.type == "GetChar") {
        inst.val = GIPROF(fgetcCharacters)->front();
        GIPROF(fgetcCharacters)->pop();
        inst.val += "|";
      } else if (inst.isParameterizedGetChar) {
        inst.val = GIPROF(instructions)->at(inst.lastGetChar).val;
      }
      fprintf(
        GIPROF(traceFile),
        "%llu|%s|%llu|%llu|%s\n",
        inst.sid,
        inst.type.c_str(),
        inst.post,
        inst.line,
        inst.val.c_str()
      );
    }
  }

  void
  GIPROF(called)(char* str) {
    std::string s = str; 

    GIPROF(instruction) in;

    uint64_t pos = 0;
    int count = 0;
    std::string token;
    std::string delimiter = "|";

    // TODO: this is where we indicate that we are interested in a particular
    //   character.
    //
    // Look at the SUGGESTs below to understand what needs fixing.

    while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      if (count == 0) {
        in.sid = stoi(token);
      }
      else if (count == 1) {
        in.type = token;
      }
      else if (count == 2) {
        in.post = stoi(token);
        if (in.type == "GetChar") {
          // SUGGEST: since this is already a GetChar, we don't need to indicate
          // anything else.

          // TODO: delete the fgetc.
          // in.val = fgetc(GIPROF(file));
          in.val += "|";
        } else if (in.type == "UngetChar") {
          unsigned index = GIPROF(instructions)->size() - 1;
          while (GIPROF(instructions)->at(index).type != "GetChar") {
            index--;
          }
          char c = GIPROF(instructions)->at(index).val[0];
          ungetc(c, GIPROF(file));
        } else if (in.type == "MethodCall") {
          in.post = in.sid + 1;
        }
      } else if(count == 3) {
        in.line = stoll(token);
      } else if (count == 4 && token != "") {
        if (in.type == "Predicate" || in.type == "MethodCall") {
          // SUGGEST: perhaps store the index of the GetChar instruction in
          //   order to retrieve the char later.
          unsigned index = GIPROF(instructions)->size() - 1;
          //TODO get last getchar related to operand
          while (GIPROF(instructions)->at(index).type != "GetChar") {
            index--;
          }
          in.val = GIPROF(instructions)->at(index).val;
          in.lastGetChar = index;
          in.isParameterizedGetChar = true;
        }
      }

      count++;
      s.erase(0, pos + delimiter.length());
    }
    GIPROF(instructions)->push_back(in);
  }
}

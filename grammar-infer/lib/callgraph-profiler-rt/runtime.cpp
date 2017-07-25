#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

extern "C" {

  // This macro allows us to prefix strings so that they are less likely to
  // conflict with existing symbol names in the examined programs.
  // e.g. CGPROF(entry) yields CaLlPrOfIlEr_entry
  #define GIPROF(X) G1Pr0_##X

  FILE* GIPROF(file) = fopen("test.txt" , "r");
  FILE* GIPROF(myfile) = fopen("trace.txt", "w");

  struct GIPROF(instruction) {
    uint64_t sid;
    std::string type;
    uint64_t post;
    std::string val;
    uint64_t line;
  };

  std::vector<GIPROF(instruction)> GIPROF(instructions);

  void
  GIPROF(called)(char* str) {
    std::string s = str; 

    GIPROF(instruction) in;

    uint64_t pos = 0;
    int count = 0;
    std::string token;
    std::string delimiter = "|";
    
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
          in.val = fgetc(GIPROF(file));
          in.val += "|";
        } else if (in.type == "UngetChar") {
          unsigned index = GIPROF(instructions).size() - 1;
          while (GIPROF(instructions).at(index).type != "GetChar") {
            index--;
          }
          char c = GIPROF(instructions).at(index).val[0];
          ungetc(c, GIPROF(file));
        } else if (in.type == "MethodCall") {
          in.post = in.sid + 1;
        }
      } else if(count == 3) {
        in.line = stoll(token);
      } else if (count == 4 && token != "") {
        if (in.type == "Predicate" || in.type == "MethodCall") {
          unsigned index = GIPROF(instructions).size() - 1;
          //TODO get last getchar related to operand
          while (GIPROF(instructions).at(index).type != "GetChar") {
            index--;
          }
          in.val = GIPROF(instructions).at(index).val;
        }
      }

      count++;
      s.erase(0, pos + delimiter.length());
    }
    GIPROF(instructions).push_back(in);

    fprintf(GIPROF(myfile), "%llu|%s|%llu|%llu|%s\n", in.sid, in.type.c_str(), in.post, in.line, in.val.c_str());
  }
}

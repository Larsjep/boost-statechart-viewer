#include "llvm/Support/CommandLine.h"

using namespace std;

llvm::cl::opt<string> outputFile("o", llvm::cl::desc("Specify output filename"), llvm::cl::value_desc("filename"), llvm::cl::init("Graph")); //option -o is not required. implicit filename is Graph

llvm::cl::list<string> includeFiles("I", llvm::cl::desc("Specify the location of include files"),  llvm::cl::value_desc("Source location"), llvm::cl::OneOrMore); //option -I must be there at least once


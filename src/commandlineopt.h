#include "llvm/Support/CommandLine.h"

using namespace std;

llvm::cl::opt<string> outputFile("o", llvm::cl::Prefix, llvm::cl::desc("Specify output filename"), llvm::cl::value_desc("filename"), llvm::cl::init("graph.dot")); //option -o is not required. implicit filename is Graph

llvm::cl::list<string> includeFiles("I", llvm::cl::Prefix, llvm::cl::desc("Specify the location of included files"),  llvm::cl::value_desc("directory")); //option -I must be there at least once

llvm::cl::opt<string> inputFilename(llvm::cl::Positional, llvm::cl::desc("input file"), llvm::cl::Required); // input file

llvm::cl::list<std::string> LibPaths("L", llvm::cl::Prefix, llvm::cl::desc("Specify a library search path"), llvm::cl::value_desc("directory"));

llvm::cl::list<std::string> Frameworks("framework", llvm::cl::desc("Specify frameworks to link to"), llvm::cl::value_desc("framework"));

llvm::cl::list<std::string> OptWarnings("W", llvm::cl::Prefix, llvm::cl::ValueOptional);

llvm::cl::list<std::string> D_macros("D", llvm::cl::value_desc("macro"), llvm::cl::Prefix, llvm::cl::desc("Predefine the specified macro"));

llvm::cl::list<std::string> U_macros("U", llvm::cl::value_desc("macro"), llvm::cl::Prefix, llvm::cl::desc("Undefine the specified macro"));


llvm::cl::opt<string> other(llvm::cl::Sink, llvm::cl::desc("other arguments")); // other arguments are thrown away


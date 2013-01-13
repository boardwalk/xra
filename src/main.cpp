#include "common.hpp"
#include "typechecker.hpp"
#include "compiler.hpp"
#include "buffered-lexer.hpp"

#include <fstream>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace xra;

int main(int argc, char** argv)
{
  enum Mode { LexMode, ParseMode, AnalyzeMode, CompileMode };
  Mode mode = CompileMode;

  ifstream ifs;
  ofstream ofs;
  string source = "stdin";
  bool bitcode = false;

  // parse options
  int c;
  while((c = getopt(argc, argv, "lpacmo:b")) != -1) {
    switch(c) {
    case 'l':
      mode = LexMode;
      break;
    case 'p':
      mode = ParseMode;
      break;
    case 'a':
      mode = AnalyzeMode;
      break;
    case 'c':
      mode = CompileMode;
      break;
    case 'o':
      ofs.open(optarg);
      if(!ofs) {
        cerr << "could not open output file " << optarg << endl;
        return EXIT_FAILURE;
      }
      break;
    case 'b':
      bitcode = true;
      break;
    }
  }

  if(optind < argc && strcmp(argv[optind], "-") != 0) {
    ifs.open(argv[optind]);
    if(!ifs) {
      cerr << "could not open input file " << argv[optind] << endl;
      return EXIT_FAILURE;
    }
    source = argv[optind];
  }

  istream& inputStream = ifs.is_open() ? ifs : cin;
  ostream& outputStream = ofs.is_open() ? ofs : cout;

  Lexer lexer(inputStream, source);  
  BufferedLexer bufferedLexer(lexer);

  /*
   * Lexing (testing only)
   */
  if(mode == LexMode)
  {
    bool ok = true;
    while(true) {
      outputStream << bufferedLexer() << endl;

      if(bufferedLexer().type == Token::Error)
        ok = false;
      else if(bufferedLexer().type == Token::EndOfFile)
        break;

      bufferedLexer.Consume();
    }

    if(!ok) {
      cerr << "lexing failed" << endl;
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }

  /*
   * Parsing
   */
  ExprPtr expr = ParseTopLevel(bufferedLexer);

  string errors = Error::Get();
  if(!errors.empty()) {
    cerr << errors;
    cerr << "parsing failed" << endl;
    return EXIT_FAILURE;
  }
  if(mode == ParseMode) {
    cout << *expr << endl;
    cerr << "parsing ok" << endl;
    return EXIT_SUCCESS;
  }

  /*
   * Analysis
   */
  Env env;

  TypeChecker checker;
  AddBuiltins(checker.env);
  checker.Visit(expr.get());

  errors = Error::Get();
  if(!errors.empty()) {
    cerr << errors;
    cerr << "analysis failed" << endl;
    return EXIT_FAILURE;
  }
  if(mode == AnalyzeMode) {
    cout << *expr << endl;
    cerr << "analysis ok" << endl;
    return EXIT_SUCCESS;
  }

  /*
   * Compilation
   */
  auto module = make_unique<llvm::Module>(source, llvm::getGlobalContext());

  Compiler compiler(*module);
  compiler.Visit(expr.get());

  auto mainFunc = module->begin();
  mainFunc->setName("main");
  mainFunc->setLinkage(llvm::Function::ExternalLinkage);

  llvm::raw_os_ostream llvmos(outputStream);
  if(bitcode) {
    if(!ofs.is_open()) {
      cerr << "refusing to write bitcode to stdout" << endl;
      return EXIT_FAILURE;
    }
    llvm::WriteBitcodeToFile(module.get(), llvmos);
  }
  else {
    module->print(llvmos, nullptr);
  }

  cerr << "compilation ok" << endl;
  return EXIT_SUCCESS;
}

#include "common.hpp"
#include "buffered-lexer.hpp"
#include "env.hpp"
#include "compiler.hpp"

#include <llvm/Support/raw_os_ostream.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace xra;

namespace xra {
  void AddBuiltins(Env&);
}

int main(int argc, char** argv)
{
  enum Mode { LexMode, ParseMode, AnalyzeMode, CompileMode };
  Mode mode = CompileMode;

  ifstream ifs;
  ofstream ofs;
  string source = "stdin";

  // parse options
  int c;
  while((c = getopt(argc, argv, "lpaco:")) != -1) {
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
  
  if(mode == LexMode)
  {
    while(true) {
      outputStream << bufferedLexer() << endl;

      if(bufferedLexer().type == Token::Error) {
        cerr << "lexing failed" << endl;
        return EXIT_FAILURE;
      }

      if(bufferedLexer().type == Token::EndOfFile)
        break;

      bufferedLexer.Consume();
    }
    return EXIT_SUCCESS;
  }

  ExprPtr expr = Expr::Parse(bufferedLexer);

  string errors = Error::Get();
  if(!errors.empty()) {
    cerr << errors;
    cerr << "parsing failed" << endl;
    return EXIT_FAILURE;
  }
  cerr << "parsing ok" << endl;

  if(mode == ParseMode)
    return EXIT_SUCCESS;
 
  Env env;
  TypeSubst subst;
  AddBuiltins(env);
  expr->Infer(env, subst);

  errors = Error::Get();
  if(!errors.empty()) {
    cerr << errors;
    cerr << "analysis failed" << endl;
    return EXIT_FAILURE;
  }
  cerr << "analysis ok" << endl;

  if(mode == AnalyzeMode)
    return EXIT_SUCCESS;
 
  auto module = make_unique<llvm::Module>(source, llvm::getGlobalContext());
  Compiler(*module).Visit(expr.get());

  llvm::raw_os_ostream llvmos(outputStream);
  module->print(llvmos, nullptr);

  return EXIT_SUCCESS;
}

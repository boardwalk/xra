#include "common.hpp"
#include "buffered-lexer.hpp"
#include "expr.hpp"
#include "env.hpp"
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
  enum Mode { LexMode, ParseMode, AnalyzeMode };
  Mode mode = ParseMode;

  ifstream ifs;
  ofstream ofs;
  string source = "stdin";

  // parse options
  int c;
  while((c = getopt(argc, argv, "lpao:")) != -1) {
    if(c == 'l') {
      mode = LexMode;
    }
    else if(c == 'p') {
      mode = ParseMode;
    }
    else if(c == 'a') {
      mode = AnalyzeMode;
    }
    else if(c == 'o') {
      ofs.open(optarg);
      if(!ofs) {
        cerr << "could not open output file " << optarg << endl;
        return EXIT_FAILURE;
      }
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
  }
  else if(mode == ParseMode || mode == AnalyzeMode)
  {
    ExprPtr expr = Expr::Parse(bufferedLexer);

    string errors = Error::Get();
    if(!errors.empty()) {
      cerr << errors;
      cerr << "parsing failed" << endl;
      return EXIT_FAILURE;
    }
    cerr << "parsing ok" << endl;

    if(mode == AnalyzeMode)
    {
      Env env;
      TypeSubst subst;
      AddBuiltins(env);
      expr->Infer(env, subst);
      cout << env << endl;

      errors = Error::Get();
      if(!errors.empty()) {
        cerr << errors;
        cerr << "analysis failed" << endl;
        return EXIT_FAILURE;
      }
      cerr << "analysis ok" << endl;
    }

    outputStream << *expr << endl;
  }

  return EXIT_SUCCESS;
}

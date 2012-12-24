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
  }

  istream& inputStream = ifs.is_open() ? ifs : cin;
  ostream& outputStream = ofs.is_open() ? ofs : cout;

  Lexer lexer(inputStream);  
  BufferedLexer bufferedLexer(lexer);
  
  if(mode == LexMode)
  {
    while(true) {
      outputStream << bufferedLexer.Get() << endl;

      if(bufferedLexer.Get().type == Token::Error)
        return EXIT_FAILURE;

      if(bufferedLexer.Get().type == Token::EndOfFile)
        break;

      bufferedLexer.Consume();
    }
  }
  else if(mode == ParseMode || mode == AnalyzeMode)
  {
    ExprPtr expr = Expr::Parse(bufferedLexer);

    Env env;
    AddBuiltins(env);

    if(mode == AnalyzeMode) {
      TypeSubst subst;
      expr->Infer(env, subst);
      env.Apply(subst);
    }

    cout << env << endl;

    string errors = expr->GetErrors();
    if(!errors.empty()) {
      cerr << errors << endl;
      return EXIT_FAILURE;
    }

    outputStream << *expr << endl;
  }

  return EXIT_SUCCESS;
}

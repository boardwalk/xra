#include "common.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace xra;

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

  if(mode == LexMode)
  {
    while(true) {
      Token token = lexer.Get();
      outputStream << token << endl;

      if(token.type == Token::Error)
        return EXIT_FAILURE;

      if(token.type == Token::EndOfFile)
        break;
    }
  }
  else if(mode == ParseMode || mode == AnalyzeMode)
  {
    BufferedLexer bufferedLexer(lexer);
    ExprPtr expr = Parse(bufferedLexer);

    if(mode == AnalyzeMode)
      expr->Analyze();

    string errors = expr->GetErrors();
    if(!errors.empty()) {
      cerr << errors << endl;
      return EXIT_FAILURE;
    }

    outputStream << *expr << endl;
  }

  return EXIT_SUCCESS;
}

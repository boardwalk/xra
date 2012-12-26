CXX = g++-4.7

CXXFLAGS = -I/Users/at0m13/homebrew/include \
  -Wall -Werror -pedantic-errors \
  -std=c++11 -g -O0 \
  `llvm-config --cxxflags --ldflags --libs core`

OBJECTS = common.o \
	main.o \
	lexer.o \
	expr-parser.o \
	expr-tostring.o \
	expr-infer.o \
	value-tostring.o \
	type.o \
	type-parser.o \
	type-tostring.o \
	type-apply.o \
	type-unify.o \
	type-getvariables.o \
	builtins.o \
	compile.o

all: xra

clean:
	rm -f *.o *.gch *.d xra

xra: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	dsymutil $@

$(OBJECTS): common.hpp.gch

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.gch: %
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.d: %.cpp
	$(CXX) -MM $(CXXFLAGS) $< > $@
	sed -i '' 's/\($*\)\.o[ :]*/\1.o $@ : /g' $@

-include $(OBJECTS:.o=.d)

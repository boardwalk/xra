CXX = g++-4.7

CXXFLAGS = -I/Users/at0m13/homebrew/include \
  -Wall -Werror -pedantic-errors \
  -std=c++11 -g -O0 \
  `llvm-config --cxxflags`
LDFLAGS = -g -O0 `llvm-config --ldflags`
LIBS = `llvm-config --libs core bitwriter`

OBJECTS = common.o \
	main.o \
	lexer.o \
	expr.o \
	expr-parser.o \
	expr-tostring.o \
	expr-infer.o \
	value.o \
	value-tostring.o \
	type.o \
	type-parser.o \
	type-tostring.o \
	type-apply.o \
	type-unify.o \
	type-getvariables.o \
	type-tollvm.o \
	builtins.o \
	compiler.o

all: xra

clean:
	rm -f *.o *.gch *.d xra

xra: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
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

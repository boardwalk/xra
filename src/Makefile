CXX = g++-4.7

CXXFLAGS = -I/Users/at0m13/homebrew/include \
  -Wall -Werror -pedantic-errors \
  -std=c++11 -g -O0 \
  `llvm-config --cxxflags`
LDFLAGS = -g -O0 `llvm-config --ldflags`
LIBS = `llvm-config --libs core bitwriter`

SOURCES = common.cpp \
	main.cpp \
	lexer.cpp \
	expr.cpp \
	expr-parser.cpp \
	expr-tostring.cpp \
	expr-infer.cpp \
	value.cpp \
	value-tostring.cpp \
	type.cpp \
	type-parser.cpp \
	type-tostring.cpp \
	type-apply.cpp \
	type-unify.cpp \
	type-getvariables.cpp \
	type-tollvm.cpp \
	builtins.cpp \
	compiler.cpp

OBJS = $(patsubst %.cpp,obj/%.o,$(SOURCES))
DEPS = $(patsubst %.cpp,dep/%.d,$(SOURCES))

all: xra

clean:
	rm -rf obj dep xra xra.dSYM

xra: $(OBJS)
	@echo "LINK $@"
	@$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	@dsymutil $@

$(OBJS): obj/common.hpp.gch

obj/%.o: %.cpp | obj
	@echo "COMPILE $<"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

obj/%.gch: % | obj
	@echo "COMPILE HEADER $<"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

dep/%.d: %.cpp | dep
	@echo "DEPENDS $<"
	@$(CXX) -MM $(CXXFLAGS) $< > $@
	@sed -i '' 's%\($*\)\.o[ :]*%\1.o $@ : %g' $@

obj:
	@mkdir -p $@

dep:
	@mkdir -p $@

-include $(DEPS)

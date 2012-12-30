CXX = clang++
CXXFLAGS = -I/Users/at0m13/homebrew/include -I/Users/at0m13/homebrew/include/c++/v1 \
	-Weverything -Wno-c++98-compat -Wno-weak-vtables -Wno-padded \
	-Wno-global-constructors -Wno-exit-time-destructors \
	-g -O0 -std=c++11 -stdlib=libc++ \
	`llvm-config --cxxflags`
LDFLAGS = -g -O0 -L/users/at0m13/homebrew/lib -stdlib=libc++ `llvm-config --ldflags`
LIBS = -lLLVM-3.3svn

SOURCES = common.cpp \
	main.cpp \
	lexer.cpp \
	expr.cpp \
	expr-parser.cpp \
	expr-tostring.cpp \
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
	typechecker.cpp \
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

$(OBJS): obj/common.hpp.pch

obj/%.o: %.cpp | obj
	@echo "COMPILE $<"
	@$(CXX) $(CXXFLAGS) -include-pch obj/common.hpp.pch -c -o $@ $<

obj/%.pch: % | obj
	@echo "COMPILE HEADER $<"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

dep/%.d: %.cpp | dep
	@echo "DEPENDS $<"
	@$(CXX) -MM $(CXXFLAGS) $< > $@
	@sed -i '' 's%\($*\)\.o[ :]*%obj/\1.o $@ : %g' $@

obj:
	@mkdir -p $@

dep:
	@mkdir -p $@

-include $(DEPS)

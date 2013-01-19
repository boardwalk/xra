CXX = clang++
CXXFLAGS = \
	-Weverything -Wno-c++98-compat -Wno-weak-vtables -Wno-padded \
	-Wno-global-constructors -Wno-exit-time-destructors \
	-g -O0 -std=c++11 \
	`llvm-config --cxxflags`
LDFLAGS = -g -O0 -L/users/at0m13/homebrew/lib `llvm-config --ldflags`
LIBS = `llvm-config --libs core bitwriter jit native`

UNAME = $(UNAME -s)
ifeq ($(UNAME),Darwin)
	CXXFLAGS += -I/Users/at0m13/homebrew/include -I/Users/at0m13/homebrew/include/c++/v1
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
endif

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

OBJS = $(patsubst %,obj/%.o,$(SOURCES))
DEPS = $(patsubst %,obj/%.d,$(SOURCES))

all: xra

clean:
	rm -rf obj xra xra.dSYM

xra: $(OBJS)
	@echo "LINK $@"
	@$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJS): obj/common.hpp.pch

obj/%.o: | obj
	@echo "COMPILE $*"
	@$(CXX) $(CXXFLAGS) -include-pch obj/common.hpp.pch -c -o $@ $*

obj/%.pch: % | obj
	@echo "COMPILE HEADER $*"
	@$(CXX) $(CXXFLAGS) -c -o $@ $*

obj/%.d: obj/%.md5 | obj
	@$(CXX) -MM $(CXXFLAGS) $* | perl fixdepends.pl $* > $@

obj/%.md5: HASH = $(shell md5sum $*)
obj/%.md5: dummy | obj
	$(if $(filter-out $(shell cat $@ 2>&1), $(HASH)), $(shell echo $(HASH) > $@))

obj:
	@mkdir -p $@

.SECONDARY:
.PHONY: all clean dummy

-include $(DEPS)

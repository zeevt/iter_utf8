CXXFLAGS := -std=c++0x -Wall -Wextra -Woverloaded-virtual -fno-exceptions -fno-rtti
LDFLAGS := -Wl,-O1 -Wl,--as-needed

ifeq (${DEBUG},yes)
CXXFLAGS := $(CXXFLAGS) -O0 -ggdb -DDEBUG
else
CXXFLAGS := $(CXXFLAGS) -O3 -g -DNDEBUG
endif

ifeq ($(CXX),g++)
	CXXFLAGS := $(CXXFLAGS) -Wsign-promo -pedantic -mtune=native
	LDFLAGS := $(LDFLAGS) -fwhole-program
	ifeq (${PGO_GEN},yes)
		CXXFLAGS := $(CXXFLAGS) -fprofile-generate
	else ifeq (${PGO_USE},yes)
		CXXFLAGS := $(CXXFLAGS) -fprofile-use
	endif
else ifeq ($(CXX),icc)
	CXXFLAGS := $(CXXFLAGS) -D__GXX_EXPERIMENTAL_CXX0X__=1 -xHost
	ifeq (${PGO_GEN},yes)
		CXXFLAGS := $(CXXFLAGS) -prof-gen
	else ifeq (${PGO_USE},yes)
		CXXFLAGS := $(CXXFLAGS) -prof-use -ipo
	else 
		CXXFLAGS := $(CXXFLAGS) -ipo
	endif
endif

PERF = /mnt/backup/home/backup/linux-2.6/tools/perf/perf
TESTFILE = /mnt/backup/home/user1/Downloads/UTF-8-demo.txt

all:

iter_utf8_test.cpp: utf8_foreach_codepoint.hpp
iter_utf8_benchmark.cpp: utf8_foreach_codepoint.hpp
iter_utf8_test: iter_utf8_test.cpp
iter_utf8_benchmark: iter_utf8_benchmark.cpp

run: ${target}
	./${target} $(TESTFILE)

benchmark: ${target}
	$(PERF) stat -e cycles -e instructions -e branches -e branch-misses ./${target} $(TESTFILE)

pgo:
	$(MAKE) clean
	$(MAKE) PGO_GEN=yes run
	$(MAKE) clean
	$(MAKE) PGO_USE=yes benchmark

%_perf: ${target}
	$(PERF) record --output=$*_perf ./$* $(TESTFILE)

profile: ${target}_perf
	$(PERF) report --input=$<
	$(PERF) annotate -l --input=$<

clean: .PHONY
	rm -f iter_utf8_test iter_utf8_benchmark *_perf *_perf.old

distclean: .PHONY clean
	rm -f pgopti.dpi.lock pgopti.dpi *.dyn *.gcda

.PHONY: .REGEN

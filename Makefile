CFLAGS := -std=gnu99 -Wall -Wextra
CXXFLAGS := -std=c++0x -Wall -Wextra -Woverloaded-virtual -fno-exceptions -fno-rtti
LDFLAGS := -Wl,-O1 -Wl,--as-needed

ifeq (${DEBUG},yes)
OPTFLAGS := -O0 -ggdb -DDEBUG
else
OPTFLAGS := -O2 -g -DNDEBUG
endif

ifeq ($(CXX),g++)
	OPTFLAGS := $(OPTFLAGS) -pedantic -mtune=native
	CXXFLAGS := $(CXXFLAGS) -Wsign-promo
	LDFLAGS := $(LDFLAGS) -fwhole-program
	ifeq (${PGO_GEN},yes)
		OPTFLAGS := $(OPTFLAGS) -fprofile-generate
	else ifeq (${PGO_USE},yes)
		OPTFLAGS := $(OPTFLAGS) -fprofile-use
	endif
else ifeq ($(CXX),icc)
	OPTFLAGS := $(OPTFLAGS) -xHost
	CXXFLAGS := $(CXXFLAGS) -D__GXX_EXPERIMENTAL_CXX0X__=1
	ifeq (${PGO_GEN},yes)
		OPTFLAGS := $(OPTFLAGS) -prof-gen
	else ifeq (${PGO_USE},yes)
		OPTFLAGS := $(OPTFLAGS) -prof-use -ipo
	else 
		OPTFLAGS := $(OPTFLAGS) -ipo
	endif
endif

CFLAGS := $(CFLAGS) $(OPTFLAGS)
CXXFLAGS := $(CXXFLAGS) $(OPTFLAGS)

PERF = /mnt/backup/home/backup/linux-2.6/tools/perf/perf
TESTFILE = /mnt/backup/home/user1/Downloads/UTF-8-demo.txt

all:
	rm -f log.txt
	$(MAKE) distclean
	$(MAKE) target=iter_utf8_test benchmark >> log.txt 2>&1
	size -A iter_utf8_test | grep '.text' >> log.txt
	$(MAKE) target=iter_utf8_test pgo >> log.txt 2>&1
	size -A iter_utf8_test | grep '.text' >> log.txt
	$(MAKE) distclean
	$(MAKE) target=iter_utf8_benchmark benchmark >> log.txt 2>&1
	size -A iter_utf8_benchmark | grep '.text' >> log.txt
	$(MAKE) target=iter_utf8_benchmark pgo >> log.txt 2>&1
	size -A iter_utf8_benchmark | grep '.text' >> log.txt
	less log.txt

iter_utf8_test: iter_utf8_test.c utf8_get.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

iter_utf8_benchmark: iter_utf8_benchmark.c utf8_get.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

run: ${target}
	./${target} $(TESTFILE)

benchmark: ${target}
	$(PERF) stat -e cycles -e instructions -e branches -e branch-misses ./${target} $(TESTFILE)

pgo:
	$(MAKE) distclean
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

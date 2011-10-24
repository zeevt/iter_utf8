CXXFLAGS := -std=c++0x -Wall -Wextra -pedantic -fno-exceptions -fno-rtti -flto
LDFLAGS = -Wl,-O1 -Wl,--as-needed -fwhole-program -Wl,-flto
ifeq (${DEBUG},yes)
CXXFLAGS := $(CXXFLAGS) -O0 -ggdb -DDEBUG
else
CXXFLAGS := $(CXXFLAGS) -O2 -g -DNDEBUG
endif

PERF = /mnt/backup/home/backup/linux-2.6/tools/perf/perf
TESTFILE = /mnt/backup/home/user1/Downloads/UTF-8-demo.txt

all: test

iter_utf8.o: iter_utf8.cpp iter_utf8.hpp

iter_utf8_test: iter_utf8_test.cpp iter_utf8.o

iter_utf8_benchmark: iter_utf8_benchmark.cpp iter_utf8.o

test: iter_utf8_test
	./iter_utf8_test

benchmark: iter_utf8_benchmark .PHONY
	$(PERF) stat -e cycles -e instructions -e branches -e branch-misses ./iter_utf8_benchmark $(TESTFILE)

perf.data: iter_utf8_benchmark
	$(PERF) record ./iter_utf8_benchmark $(TESTFILE)

profile: perf.data
	$(PERF) report
	$(PERF) annotate -l

clean: .PHONY
	rm -f iter_utf8.o iter_utf8_test iter_utf8_benchmark perf.data perf.data.old

.PHONY: .REGEN

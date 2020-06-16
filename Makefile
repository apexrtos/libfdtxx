CXXFLAGS := -ggdb -Og -std=c++20 -Wall

check: test/passed

test/passed: test/test
	cd test && ./test
	touch test/passed

test/test: test/test.cpp
	g++ -o $@ $(CXXFLAGS) test/test.cpp libfdt++.cpp -lfdt -lgtest -lgtest_main

clean:
	rm -f test/test test/passed

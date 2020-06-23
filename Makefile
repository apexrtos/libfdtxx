CXXFLAGS := -ggdb -Og -std=c++20 -Wall

check: test/passed

test/passed: test/test
	cd test && ./test
	touch test/passed

test/test: test/fdt.cpp libfdt++.cpp libfdt++.h test/basic.dtb test/path.dtb test/properties.dtb
	$(CXX) -o $@ $(CXXFLAGS) test/fdt.cpp libfdt++.cpp -lfdt -lgtest -lgtest_main

%.dtb: %.dts
	dtc -o $@ $<

clean:
	rm -f test/test test/passed

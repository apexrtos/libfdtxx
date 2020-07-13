CXXFLAGS := -ggdb -std=c++20 -Wall

check: test/passed

test/passed: Makefile \
	     test/test \
	     test/basic.dtb test/path.dtb test/properties.dtb \
	     test/verify.fit test/verify-offset.fit test/verify-position.fit
	cd test && ./test
	touch test/passed

test/test: Makefile \
	   test/fdt.cpp test/fit.cpp \
	   libfit++.cpp libfit++.h libfdt++.cpp libfdt++.h libtomcrypt_init.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(filter %.cpp,$^) -lfdt -lgtest -lgtest_main -ltomcrypt

test/verify.fit: test/rsa2048.key test/rsa4096.key

test/rsa%.key test/rsa%_pub.pem&:
	openssl genpkey -algorithm RSA -out test/rsa$*.key -pkeyopt rsa_keygen_bits:$*
	openssl rsa -in test/rsa$*.key -pubout -outform DER -out test/rsa$*_pub.der

%.dtb: %.dts
	dtc -o $@ $<

%.fit: %.its
	mkimage -k test -f $< $@

%-offset.fit: %.its
	mkimage -E -k test -f $< $@

%-position.fit: %.its
	mkimage -E -p 0x4000 -k test -f $< $@

clean:
	rm -f test/test test/passed

distclean:
	rm -f test/basic.dtb test/path.dtb test/properties.dtb test/verify*.fit
	rm -f test/rsa2048.key test/rsa4096.key

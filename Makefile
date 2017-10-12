.PHONY: build
build:
	./build.py | ${MAKE} build -f -

.PHONY: doxygen
doxygen:
	doxygen

.PHONY: clean
clean:
	rm -rf a.out html latex
	find . -type f -name '*.o' -delete
	find . -type f -name '*.exe' -delete

.PHONY: test
test:
	./build.py | ${MAKE} test -f -

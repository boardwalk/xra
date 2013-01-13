
all: xra

xra:
	$(MAKE) -C src xra

test: xra
	$(CURDIR)/test.sh

clean:
	$(MAKE) -C src clean

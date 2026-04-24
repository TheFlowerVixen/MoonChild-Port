PLATFORM ?= SDL

all:
ifeq ($(PLATFORM),SDL)
	cmake -S. -Bbuild
	cmake --build build
else ifeq ($(PLATFORM),N64)
	make -f Makefile_n64.mk
else ifeq ($(PLATFORM),WII)
	make -f Makefile_wii.mk
else
	echo No platform named $(PLATFORM)!
endif

clean:
	rm -rf build filesystem *.dol *.elf *.z64 *.pak
.PHONY: clean
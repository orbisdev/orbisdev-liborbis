SUBDIRS = orbisdev-libkernelUtil orbisdev-libdebugnet orbisdev-libz orbisdev-liborbisPad orbisdev-liborbisAudio orbisdev-libmod orbisdev-liborbisKeyboard orbisdev-liborbisNfs orbisdev-libSQLite orbisdev-liborbisLink




all:
	$(MAKE) sources
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir && echo $(MAKE) -C $$dir install; \
	done
sources:
	./getSources.sh

install:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir install; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
ifndef ORBISDEV
$(error ORBISDEV, is not set)
endif

target := ps4_lib
OutPath := lib
TargetFile := liborbis
AllTarget = $(OutPath)/$(TargetFile).a 

include $(ORBISDEV)/make/ps4sdklib.mk
CompilerFlags += -DHAVE_CONFIG_H -D_U_="__attribute__((unused))" -DNDEBUG -D__PS4__ -D_BSD_SOURCE -D__ORBIS__
CompilerFlagsCpp += -DHAVE_CONFIG_H -D_U_="__attribute__((unused))" -DNDEBUG -D__PS4__ -D_BSD_SOURCE -D__ORBIS__
IncludePath += -I$(ORBISDEV)/usr/include -Iinclude/nfsc 


$(OutPath)/$(TargetFile).a: $(ObjectFiles)
	$(dirp)
	$(archive)

install:
	@cp $(OutPath)/$(TargetFile).a $(DESTDIR)$(ORBISDEV)/usr/lib
	@cp include/debugnet.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@mkdir -p $(DESTDIR)$(ORBISDEV)/usr/include/orbis/nfsc
	@cp include/nfsc/*.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis/nfsc
	@cp include/orbisNfs.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/orbisPad.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/orbisKeyboard.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/kb.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/orbisAudio.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/modplayer.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/pl_ini.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/orbislink.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/png.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/pngconf.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/pnglibconf.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/pngstruct.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/pnginfo.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/zlib.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/zconf.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/sxmlc.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@cp include/sxmlsearch.h $(DESTDIR)$(ORBISDEV)/usr/include/orbis
	@echo "$(TargetFile) Installed!"


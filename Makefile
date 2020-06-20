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
	@cp $(OutPath)/$(TargetFile).a $(ORBISDEV)/usr/lib
	@cp include/debugnet.h $(ORBISDEV)/usr/include/orbis
	@mkdir -p $(ORBISDEV)/usr/include/orbis/nfsc
	@cp include/nfsc/*.h $(ORBISDEV)/usr/include/orbis/nfsc	
	@cp include/orbisNfs.h $(ORBISDEV)/usr/include/orbis
	@cp include/orbisPad.h $(ORBISDEV)/usr/include/orbis
	@cp include/orbisKeyboard.h $(ORBISDEV)/usr/include/orbis
	@cp include/kb.h $(ORBISDEV)/usr/include/orbis
	@cp include/orbisAudio.h $(ORBISDEV)/usr/include/orbis
	@cp include/modplayer.h $(ORBISDEV)/usr/include/orbis
	@cp include/pl_ini.h $(ORBISDEV)/usr/include/orbis
	@cp include/orbislink.h $(ORBISDEV)/usr/include/orbis
	@cp include/png.h $(ORBISDEV)/usr/include/orbis
	@cp include/pngconf.h $(ORBISDEV)/usr/include/orbis
	@cp include/pnglibconf.h $(ORBISDEV)/usr/include/orbis
	@cp include/pngstruct.h $(ORBISDEV)/usr/include/orbis
	@cp include/pnginfo.h $(ORBISDEV)/usr/include/orbis
	@cp include/zlib.h $(ORBISDEV)/usr/include/orbis
	@cp include/zconf.h $(ORBISDEV)/usr/include/orbis
	@cp include/sxmlc.h $(ORBISDEV)/usr/include/orbis
	@cp include/sxmlsearch.h $(ORBISDEV)/usr/include/orbis
	@echo "$(TargetFile) Installed!"


ifndef PS4SDK
$(error PS4SDK, is not set)
endif

target := ps4_lib
OutPath := lib
TargetFile := liborbis
AllTarget = $(OutPath)/$(TargetFile).a 

include $(PS4SDK)/make/ps4sdklib.mk
CompilerFlags += -DHAVE_CONFIG_H -D_U_="__attribute__((unused))" -DNDEBUG -D__PS4__ -D_BSD_SOURCE -D__ORBIS__
CompilerFlagsCpp += -DHAVE_CONFIG_H -D_U_="__attribute__((unused))" -DNDEBUG -D__PS4__ -D_BSD_SOURCE -D__ORBIS__
IncludePath += -I$(PS4SDK)/include -Iinclude/nfsc 


$(OutPath)/$(TargetFile).a: $(ObjectFiles)
	$(dirp)
	$(archive)

install:
	@cp $(OutPath)/$(TargetFile).a $(PS4SDK)/lib
	@cp include/debugnet.h $(PS4SDK)/include/orbis
	@mkdir -p $(PS4SDK)/include/orbis/nfsc
	@cp include/nfsc/*.h $(PS4SDK)/include/orbis/nfsc	
	@cp include/orbisNfs.h $(PS4SDK)/include/orbis
	@cp include/orbisPad.h $(PS4SDK)/include/orbis
	@cp include/orbisKeyboard.h $(PS4SDK)/include/orbis
	@cp include/kb.h $(PS4SDK)/include/orbis
	@cp include/orbisAudio.h $(PS4SDK)/include/orbis
	@cp include/modplayer.h $(PS4SDK)/include/orbis
	@cp include/pl_ini.h $(PS4SDK)/include/orbis
	@cp include/orbislink.h $(PS4SDK)/include/orbis
	@cp include/png.h $(PS4SDK)/include/orbis
	@cp include/pngconf.h $(PS4SDK)/include/orbis
	@cp include/pnglibconf.h $(PS4SDK)/include/orbis
	@cp include/pngstruct.h $(PS4SDK)/include/orbis
	@cp include/pnginfo.h $(PS4SDK)/include/orbis
	@cp include/zlib.h $(PS4SDK)/include/orbis
	@cp include/zconf.h $(PS4SDK)/include/orbis
	@cp include/sxmlc.h $(PS4SDK)/include/orbis
	@cp include/sxmlsearch.h $(PS4SDK)/include/orbis
	@echo "$(TargetFile) Installed!"


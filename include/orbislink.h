/*
#  ____   ____    ____         ___ ____   ____ _     _
# |    |  ____>   ____>  |    |        | <____  \   /
# |____| |    \   ____>  | ___|    ____| <____   \_/	ORBISDEV Open Source Project.
#------------------------------------------------------------------------------------
# Copyright 2010-2020, orbisdev - http://orbisdev.github.io
# Licenced under MIT license
# Review README & LICENSE files for further details.
*/

#ifndef _ORBIS_ORBISLINK_H_
#define _ORBIS_ORBISLINK_H_

#include <ps4sdk.h>
#include <debugnet.h>
#include <orbisNfs.h>
#include <orbisPad.h>
#include <orbisAudio.h>
#include <orbisKeyboard.h>
#include <pl_ini.h>

typedef struct OrbisGlobalConf
{
	void *conf; //deprecated
	OrbisPadConfig *confPad;
	OrbisAudioConfig *confAudio;
	OrbisKeyboardConfig *confKeyboard;
	void *confLink; //deprecated
	int orbisLinkFlag;
	debugNetConfiguration *confDebug;
	OrbisNfsConfig *confNfs;
}OrbisGlobalConf;


#ifdef __cplusplus
extern "C" {
#endif



int initOrbisLinkAppInternal();
int initOrbisLinkAppVanilla();
int initOrbisLinkAppVanillaGl();
int initOrbisLinkAppVanillaGlMSX();
OrbisGlobalConf * orbisLinkGetGlobalConf();


#ifdef __cplusplus
}
#endif

#endif

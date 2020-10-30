/*
#  ____   ____    ____         ___ ____   ____ _     _
# |    |  ____>   ____>  |    |        | <____  \   /
# |____| |    \   ____>  | ___|    ____| <____   \_/   ORBISDEV Open Source Project.
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

typedef void module_patch_cb_t(void *arg, uint8_t *base, uint64_t size);


#ifdef __cplusplus
extern "C" {
#endif


int initOrbisLinkApp(void);
int initOrbisLinkAppInternal(void);
int initOrbisLinkAppVanilla(void);
int initOrbisLinkAppVanillaGl(void);
int initOrbisLinkAppVanillaGlMSX(void);
OrbisGlobalConf *orbisLinkGetGlobalConf(void);
void finishOrbisLinkApp(void);

bool patch_module(const char *name, module_patch_cb_t *cb, void *arg, int level);
int  loadModulesVanilla(void);

#ifdef __cplusplus
}
#endif

#endif

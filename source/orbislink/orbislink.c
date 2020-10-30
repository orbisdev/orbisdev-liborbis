/*
#  ____   ____    ____         ___ ____   ____ _     _
# |    |  ____>   ____>  |    |        | <____  \   /
# |____| |    \   ____>  | ___|    ____| <____   \_/   ORBISDEV Open Source Project.
#------------------------------------------------------------------------------------
# Copyright 2010-2020, orbisdev - http://orbisdev.github.io
# Licenced under MIT license
# Review README & LICENSE files for further details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <ps4sdk.h>
#include <orbislink.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <orbis/libkernel.h>


OrbisGlobalConf globalConf;
int padEnabled;
int orbisGlEnabled;
int audioEnabled;
int msxEnabled;
int audioSamples;
int audioFrequency;
int audioFormat;
int keyboardEnabled;
int orbisNfsEnabled=0;
//#define SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT (0xffffffffffffffffUL)
//extern size_t sceLibcHeapSize=0xffffffffffffffffUL;
//extern unsigned int sceLibcHeapExtendedAlloc=1;

#define PIGLET_MODULE_NAME "libScePigletv2VSH.sprx"
#define SHCOMP_MODULE_NAME "libSceShaccVSH.sprx"
#define FSELF_NAME "homebrew.self"
#define ORBISLINK_TCP_PORT 18193

//#if 1
//# define MODULE_PATH_PREFIX "/app0/sce_module"
//#else
#define MODULE_PATH_PREFIX "/data/self/system/common/lib"
//#endif

static int s_piglet_module = -1;
static int s_shcomp_module = -1;

const char* g_sandbox_word=NULL;
typedef void module_patch_cb_t(void* arg, uint8_t* base, uint64_t size);

bool get_module_base(const char* name, uint64_t* base, uint64_t* size, int level)
{
    SceKernelModuleInfo moduleInfo;
    int ret;

    ret = sceKernelGetModuleInfoByName(name, &moduleInfo);
    if (ret) {
        debugNetPrintf(level,"[ORBISLINK] %s sceKernelGetModuleInfoByName(%s) failed: 0x%08X\n",__FUNCTION__,name,ret);
        goto err;
    }

    if (base) {
        *base = (uint64_t)moduleInfo.segmentInfo[0].address;
    }
    if (size) {
        *size = moduleInfo.segmentInfo[0].size;
    }

    return true;

err:
    return false;
}

bool patch_module(const char* name, module_patch_cb_t* cb, void* arg,int level) {
    uint64_t base, size;
    int ret;
    
    if (!get_module_base(name, &base, &size,level)) {
        debugNetPrintf(level,"[ORBISLINK] %s get_module_base return error\n",__FUNCTION__);
        goto err;
    }
    debugNetPrintf(level,"[ORBISLINK] %s module base=0x%08X size=%ld\n",__FUNCTION__,base,size);
    
    ret = sceKernelMprotect((void*)base, size, PROT_READ | PROT_WRITE | PROT_EXEC);
    if (ret) {
        debugNetPrintf(level,"[ORBISLINK] %s sceKernelMprotect(%s) failed: 0x%08X\n",__FUNCTION__,name,ret);
        goto err;
    }
    debugNetPrintf(level,"[ORBISLINK] %s patching module\n",__FUNCTION__);
    
    if (cb) {
        (*cb)(arg, (uint8_t*)base, size);
    }

    return true;

err:
    return false;
}

void unloadPigletModules(int level)
{
    int ret;

    if (s_shcomp_module > 0) 
    {
        ret=sceKernelStopUnloadModule(s_shcomp_module,0,NULL,0,NULL,NULL);
        if (ret < 0) 
        {
            debugNetPrintf(level,"[ORBISLINK] %s sceKernelStopUnloadModule(%s) failed: 0x%08X\n",__FUNCTION__,SHCOMP_MODULE_NAME,ret);
        } 
        else 
        {
            s_shcomp_module=-1;
        }
    }

    if (s_piglet_module>0) 
    {
        ret=sceKernelStopUnloadModule(s_piglet_module,0,NULL,0,NULL,NULL);
        if(ret<0) 
        {
            debugNetPrintf(level,"[ORBISLINK] %s sceKernelStopUnloadModule(%s) failed: 0x%08X\n",__FUNCTION__,PIGLET_MODULE_NAME,ret);
        } 
        else 
        {
            s_piglet_module=-1;
        }
    }
}
int loadPigletModules(int flagPiglet,int level)
{
    int ret;
    if(flagPiglet>0)
    {
        ret=sceKernelLoadStartModule(MODULE_PATH_PREFIX "/" PIGLET_MODULE_NAME,0,NULL,0,NULL,NULL);
        if(ret<0) 
        {
            debugNetPrintf(level,"[ORBISLINK] %s sceKernelLoadStartModule(%s) failed: 0x%08X\n",__FUNCTION__,PIGLET_MODULE_NAME,ret);
            goto err;
        }
        s_piglet_module=ret;
        debugNetPrintf(level,"[ORBISLINK] %s sceKernelLoadStartModule(%s) return id: %d\n",__FUNCTION__,PIGLET_MODULE_NAME,ret);
        
        if(flagPiglet>=2)
        {
            ret=sceKernelLoadStartModule(MODULE_PATH_PREFIX "/" SHCOMP_MODULE_NAME,0,NULL,0,NULL,NULL);
            if (ret<0) 
            {
                debugNetPrintf(level,"[ORBISLINK] %s sceKernelLoadStartModule(%s) failed: 0x%08X\n",__FUNCTION__,SHCOMP_MODULE_NAME,ret);
                goto err;
            }
            s_shcomp_module = ret;
            debugNetPrintf(level,"[ORBISLINK] %s sceKernelLoadStartModule(%s) return id: %d\n",__FUNCTION__,SHCOMP_MODULE_NAME,ret);
            return 2;
        }
        return 1;
    }
    err:
    return -1;
}
/* XXX: patches below are given for Piglet module from 4.74 Devkit PUP */
static void pgl_patches_cb(void* arg, uint8_t* base, uint64_t size) 
{
    /* Patch runtime compiler check */
    const uint8_t p_set_eax_to_1[] = {
        0x31, 0xC0, 0xFF, 0xC0, 0x90,
    };
    memcpy(base + 0x5451F, p_set_eax_to_1, sizeof(p_set_eax_to_1));

    /* Tell that runtime compiler exists */
    *(uint8_t*)(base + 0xB2DEC) = 0;
    *(uint8_t*)(base + 0xB2DED) = 0;
    *(uint8_t*)(base + 0xB2DEE) = 1;
    *(uint8_t*)(base + 0xB2E21) = 1;

    /* Inform Piglet that we have shader compiler module loaded */
    *(int32_t*)(base + 0xB2E24) = s_shcomp_module;
}

static bool do_patches(int level) 
{
    if (!patch_module(PIGLET_MODULE_NAME, &pgl_patches_cb, NULL,level)) 
    {
        debugNetPrintf(level,"[ORBISLINK] %s Unable to patch PGL module.\n",__FUNCTION__);
        return false;
    }
    return true;
}

static void cleanup(void) 
{
    unloadPigletModules(DEBUGNET_DEBUG);

    int ret = sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE);
    if (ret) 
    {
        debugNetPrintf(3,"[ORBISLINK] %s sceSysmoduleUnloadModuleInternal(%s) failed: 0x%08X\n",__FUNCTION__,"SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE", ret);
    }
}

void finishOrbisLinkApp(void)
{
    cleanup();

    if(audioEnabled)    orbisAudioFinish();
    if(keyboardEnabled) orbisKeyboardFinish();
    if(padEnabled)      orbisPadFinish();
    if(orbisNfsEnabled) orbisNfsFinish();
}

int orbisLinkCreateSelfDirectories()
{
    struct stat sb;
    int ret;
    ret=sceKernelStat("/data/self/system/common/lib",&sb);
    if(ret!=0)
    {
        ret=sceKernelMkdir("/data/self",0777);
        if(ret!=0 && ret!=0x80020011)
        {
            return -1;
        }
        ret=sceKernelMkdir("/data/self/system",0777);
        if(ret!=0 && ret!=0x80020011)
        {
            return -1;
        }
        ret=sceKernelMkdir("/data/self/system/common",0777);
        if(ret!=0 && ret!=0x80020011)
        {
            return -1;
        }
        ret=sceKernelMkdir("/data/self/system/common/lib",0777);
        if(ret!=0 && ret!=0x80020011)
        {
            return -1;
        }
        
    }
    return 0;
}
int orbisLinkCreateConfigDirectory( const char *path)
{
    int ret;
    struct stat sb;
    ret=sceKernelStat(path, &sb);
    if(ret!=0)
    {
        ret=sceKernelMkdir(path,0777);
        if(ret!=0)
        {
            return -1;
        }   
    }
    return 0;
}
int orbisLinkUploadPigletModules()
{
    struct stat sb;
    int ret;
    int fd;
    int size_piglet;
    int size_shcomp; 
    unsigned char* piglet_mod=NULL;
    unsigned char* shcomp_mod=NULL; 
    ret=sceKernelStat(MODULE_PATH_PREFIX "/" PIGLET_MODULE_NAME,&sb);
    if(ret!=0)
    {
        fd=orbisNfsOpen(PIGLET_MODULE_NAME,O_RDONLY, 0777);
        if(fd<0)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s error opening %s module from your nfs server\n",__FUNCTION__,PIGLET_MODULE_NAME);
            return -1;
        }
        size_piglet=orbisNfsLseek(fd,0,SEEK_END);
        orbisNfsLseek(fd,0,SEEK_SET);
        if(size_piglet<0)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s Failed to read size of file %s\n",__FUNCTION__,PIGLET_MODULE_NAME);
            orbisNfsClose(fd);
            return -1;
        }
        piglet_mod=malloc(sizeof(unsigned char)*size_piglet);
        if(piglet_mod==NULL)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s Failed to allocate %d bytes\n",__FUNCTION__,size_piglet);
            orbisNfsClose(fd);
            return -1;
        }
        ret=orbisNfsRead(fd,piglet_mod,size_piglet);
        if(ret!=size_piglet)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s Failed to read content of file %s\n",__FUNCTION__,PIGLET_MODULE_NAME);
            orbisNfsClose(fd);
            free(piglet_mod);
            return -1;
        }
        orbisNfsClose(fd);
        fd=-1;
        fd=sceKernelOpen(MODULE_PATH_PREFIX "/" PIGLET_MODULE_NAME,O_WRONLY|O_CREAT|O_TRUNC,0777);
        if(fd<0)
        {
            debugNetPrintf(3,"[ORBISLINK %s error sceKernelOpen err 0x%08X opening %s\n",__FUNCTION__,fd,PIGLET_MODULE_NAME);
            return -1;
        }
        ret=sceKernelWrite(fd,piglet_mod,size_piglet);
        if(ret!=size_piglet)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s sceKernelWrite err 0x%08X to write content of file %s\n",__FUNCTION__,ret,PIGLET_MODULE_NAME);
            sceKernelClose(fd);
            free(piglet_mod);
            return -1;
        }
        sceKernelClose(fd);
        sceKernelSync();
        free(piglet_mod);

    }
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s %s module already on PlayStation file system\n",__FUNCTION__,PIGLET_MODULE_NAME);
    fd=-1;
    ret=sceKernelStat(MODULE_PATH_PREFIX "/" SHCOMP_MODULE_NAME,&sb);
    if(ret!=0)
    {
        fd=orbisNfsOpen(SHCOMP_MODULE_NAME,O_RDONLY, 0777);
        if(fd<0)
        {
            debugNetPrintf(3,"[ORBISLINK %s error opening %s module from your nfs server\n",__FUNCTION__,SHCOMP_MODULE_NAME);
        }
        size_shcomp=orbisNfsLseek(fd,0,SEEK_END);
        orbisNfsLseek(fd,0,SEEK_SET);
        if(size_shcomp<0)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s Failed to read size of file %s\n",__FUNCTION__,SHCOMP_MODULE_NAME);
            orbisNfsClose(fd);
            return -1;
        }
        shcomp_mod=malloc(sizeof(unsigned char)*size_shcomp);
        if(shcomp_mod==NULL)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s Failed to allocate %d bytes\n",__FUNCTION__,size_shcomp);
            orbisNfsClose(fd);
            return -1;
        }
        int numread=size_shcomp/(1024*1024);
        int lastread=size_shcomp%(1024*1024);
        int i,j;
        for(j=0;j<numread;j++)
        {
            if(j<numread-1)
            {
                i = orbisNfsRead(fd, shcomp_mod+j*(1024*1024), (1024*1024));
            }
            else
            {
                i = orbisNfsRead(fd, shcomp_mod+j*(1024*1024), (1024*1024)+lastread);
            
            }
            if (i < 0) 
            {
                debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s nfs_read, data read error\n",__FUNCTION__);
                orbisNfsClose(fd);
                free(shcomp_mod);
                return -1;
            }
            debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s orbisNfsRead: chunk %d  read %d\n",__FUNCTION__, j,i);
        }
        orbisNfsClose(fd);
        fd=-1;
        fd=sceKernelOpen(MODULE_PATH_PREFIX "/" SHCOMP_MODULE_NAME,O_WRONLY|O_CREAT|O_TRUNC,0777);
        if(fd<0)
        {
            debugNetPrintf(3,"[ORBISLINK %s error sceKernelOpen err 0x%08X opening %s\n",__FUNCTION__,fd,SHCOMP_MODULE_NAME);
            free(shcomp_mod);
            return -1;
        }
        ret=sceKernelWrite(fd,shcomp_mod,size_shcomp);
        if(ret!=size_shcomp)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s sceKernelWrite err 0x%08X to write content of file %s\n",__FUNCTION__,ret,SHCOMP_MODULE_NAME);
            sceKernelClose(fd);
            free(shcomp_mod);
            return -1;
        }
        sceKernelClose(fd);
        sceKernelSync();
        free(shcomp_mod);
        
    }
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s %s module already on PlayStation file system\n",__FUNCTION__,SHCOMP_MODULE_NAME);
    return 0;
}
int orbisLinkUploadSelf(const char *path)
{
    int ret;
    int size;
    int fd; 
    unsigned char* self_buf=NULL;

    ret=sceKernelChmod("/data/self/system/common/lib/homebrew.self", 0000777);
    fd=orbisNfsOpen(path,O_RDONLY, 0777);
    if(fd<0)
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s error opening %s fself from your nfs server\n",__FUNCTION__,path);
        return -1;
    }
    size=orbisNfsLseek(fd,0,SEEK_END);
    orbisNfsLseek(fd,0,SEEK_SET);
    if(size<0)
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s Failed to read size of file %s\n",__FUNCTION__,path);
        orbisNfsClose(fd);
        return -1;
    }
    self_buf=malloc(sizeof(unsigned char)*size);
    if(self_buf==NULL)
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s Failed to allocate %d bytes\n",__FUNCTION__,size);
        orbisNfsClose(fd);
        return -1;
    }
    if(size>=1024*1024)
    {
        int numread=size/(1024*1024);
        int lastread=size%(1024*1024);
        int i,j;
        for(j=0;j<numread;j++)
        {
            if(j<numread-1)
            {
                i = orbisNfsRead(fd, self_buf+j*(1024*1024), (1024*1024));
            }
            else
            {
                i = orbisNfsRead(fd, self_buf+j*(1024*1024), (1024*1024)+lastread);
            }
            if (i < 0) 
            {
                debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s nfs_read, data read error\n",__FUNCTION__);
                orbisNfsClose(fd);
                free(self_buf);
                return -1;
            }
            debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s orbisNfsRead: chunk %d  read %d\n",__FUNCTION__, j,i);
        }
    }
    else
    {
        ret=orbisNfsRead(fd, self_buf, size);  
        debugNetPrintf(DEBUGNET_DEBUG,"[MSXORBIS] %s nfs read size=%d\n",__FUNCTION__,ret);

    }
    orbisNfsClose(fd);
        
    fd=-1;

    fd=sceKernelOpen(MODULE_PATH_PREFIX "/" FSELF_NAME,O_WRONLY|O_CREAT|O_TRUNC,0777);
    if(fd<0)
    {
        debugNetPrintf(3,"[ORBISLINK %s error sceKernelOpen err 0x%08X opening %s\n",__FUNCTION__,fd,FSELF_NAME);
        return -1;
    }
    ret=sceKernelWrite(fd,self_buf,size);
    if(ret!=size)
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s sceKernelWrite err 0x%08X to write content of file %s\n",__FUNCTION__,ret,FSELF_NAME);
        sceKernelClose(fd);
        free(self_buf);
        return -1;
    }
    sceKernelClose(fd);
    sceKernelSync();
    free(self_buf);
    ret=sceKernelChmod("/data/self/system/common/lib/homebrew.self", 0000777);
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s sceKernelChmod return 0x%08X %s  already on PlayStation file system\n",__FUNCTION__,ret,FSELF_NAME);
    
    return ret;
}
int orbisLinkListenerUp(int port)
{
    int server;
    int client;
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_len=sizeof(address);
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(port);
    server=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int r;
    int ret;
    if(server<0)
    {
        return -1;
    }
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char *)&(int){ 1 }, sizeof(int));
    setsockopt(server, SOL_SOCKET, SO_REUSEPORT, (char *)&(int){ 1 }, sizeof(int));

    if((r = bind(server, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        close(server);
        return -2;
    }

    if((r = listen(server, 10)) < 0)
    {
        close(server);
        return -3;
    }
    //debugNetPrintf(3,"[ORBISLINK] %s listen ready\n",__FUNCTION__);

    client = accept(server, NULL, NULL);
    //debugNetPrintf(3,"[ORBISLINK] %s client ready\n",__FUNCTION__);

    close(server);
    int length = 0;
    int full = 4096;
    uint8_t *data = (void *)malloc(full);
    size_t s = 0;

    
    while((length = read(client, data + s, full - s)) > 0)
    {
        s += length;
        if(s == full)
        {
            void *t;
            full *= 2;
            t = realloc(data, full);
            if(t == NULL)
            {
                free(data);
                return -1;
            }
            data = t;
        }
    }
    //debugNetPrintf(3,"[ORBISLINK] %s read done\n",__FUNCTION__);

    //debugNetPrintf(3,"[ORBISLINK] %s %s\n",__FUNCTION__,data);

    int fd=sceKernelOpen("/data/orbislink/orbislink_config.ini",O_WRONLY|O_CREAT|O_TRUNC,0666);
    if(fd<0)
    {
        //debugNetPrintf(3,"[ORBISLINK] %s error opening for write 0x%08X\n",__FUNCTION__,fd);

        return -1;
    }
    ret=sceKernelWrite(fd,data,s);
    if(ret<s)
    {
        //debugNetPrintf(3,"[ORBISLINK] %s error writing for write 0x%08X\n",__FUNCTION__,ret);

        return -1;
        sceKernelClose(fd);
    }
    sceKernelClose(fd);
    sceKernelSync();
    free(data);
    ret=sceKernelChmod("/data/orbislink/orbislink_config.ini", 0000666);
    fd=-1;
    //debugNetPrintf(3,"[ORBISLINK] %s  sceKernelChmod return  0x%08X\n",__FUNCTION__,ret);

    /*fd=sceKernelOpen("/data/orbislink/orbislink_config.ini",O_RDONLY,0777);
    if(fd<0)
    {
        debugNetPrintf(3,"[ORBISLINK] %s error opening  0x%08X\n",__FUNCTION__,fd);

        
    }*/
    return 0;

}
int loadModulesGl(int compilerFlag, int level)
{
    if(loadPigletModules(compilerFlag,level)<0)
    {
        debugNetPrintf(level,"[ORBISLINK] %s Error loading piglet modules\n",__FUNCTION__);
        return -4;
    }
    debugNetPrintf(level,"[ORBISLINK] %s piglet modules loaded\n",__FUNCTION__);
    
    if(compilerFlag==2)
    {
        if (do_patches(level)<0) 
        {
            debugNetPrintf(level,"[ORBISLINK] %s Unable to patch piglet modules\n",__FUNCTION__);
            return -5;
        }
        debugNetPrintf(level,"[ORBISLINK] %s piglet modules patched\n",__FUNCTION__);
    }
    return 0;
}
int loadModulesVanilla()
{
    int ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE);
    if(ret!=0)
    {
        return -1;
    }
    ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_NET);
    if(ret!=0)
    {
        return -1;
    }
    ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_USER_SERVICE);
    if(ret!=0)
    {
        return -1;
    }
    return 0;
}
//net systemservice userservice debugnet pad gl modules
int initOrbisLinkAppVanillaGl()
{
    int ret;
    pl_ini_file init;
    int debugPort;
    int level;
    char serverIp[16];

    ret=loadModulesVanilla();
    ret=pl_ini_load(&init,"/data/orbislink/orbislink_config.ini");
    debugPort=pl_ini_get_int(&init,"debugnet","debugPort",18194);
    level=pl_ini_get_int(&init,"debugnet","level",3);
    pl_ini_get_string(&init,"debugnet","serverIp","192.168.1.3",serverIp,16);

    debugNetInit(serverIp,debugPort,level);


    ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PAD);
    if(ret) 
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s sceSysmoduleLoadModuleInternal(%s) failed: 0x%08X\n",__FUNCTION__,"SCE_SYSMODULE_INTERNAL_PAD", ret);
        return -1;
    }
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s sceSysmoduleLoadModuleInternal(%s) return: 0x%08X\n",__FUNCTION__,"SCE_SYSMODULE_INTERNAL_PAD", ret);
    ret=orbisPadInit();
    if(ret!=1)
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s orbisPadInit() failed: 0x%08X\n",__FUNCTION__, ret);
        return -2;
    }
    globalConf.confPad=orbisPadGetConf();
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s liborbisPad initialized\n",__FUNCTION__);
    ret=loadModulesGl(2,DEBUGNET_DEBUG);
    if(ret<0)
    {
        return ret;
    }
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s piglet modules patched\n",__FUNCTION__);
    

    return 0;
}
//net systemservice userservice debugnet pad audiomsx gl ime modules
int initOrbisLinkAppVanillaGlMSX()
{
    int ret;
    pl_ini_file init;
    int debugPort;
    int level;
    char serverIp[16];

    ret=loadModulesVanilla();
    ret=pl_ini_load(&init,"/data/orbislink/orbislink_config.ini");
    debugPort=pl_ini_get_int(&init,"debugnet","debugPort",18194);
    level=pl_ini_get_int(&init,"debugnet","level",3);
    pl_ini_get_string(&init,"debugnet","serverIp","192.168.1.3",serverIp,16);

    debugNetInit(serverIp,debugPort,level);


    ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PAD);
    if(ret) 
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s sceSysmoduleLoadModuleInternal(%s) failed: 0x%08X\n",__FUNCTION__,"SCE_SYSMODULE_INTERNAL_PAD", ret);
        return -1;
    }
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s sceSysmoduleLoadModuleInternal(%s) return: 0x%08X\n",__FUNCTION__,"SCE_SYSMODULE_INTERNAL_PAD", ret);
    ret=orbisPadInit();
    if(ret!=1)
    {
        debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s orbisPadInit() failed: 0x%08X\n",__FUNCTION__, ret);
        return -2;
    }
    globalConf.confPad=orbisPadGetConf();
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s liborbisPad initialized\n",__FUNCTION__);
    ret=loadModulesGl(2,DEBUGNET_DEBUG);
    if(ret<0)
    {
        return ret;
    }
    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s piglet modules patched\n",__FUNCTION__);
    ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_AUDIO_OUT);
    if (ret) 
    {
        debugNetPrintf(3,"[ORBISLINK] sceSysmoduleLoadModuleInternal(%s) failed: 0x%08X\n", "SCE_SYSMODULE_INTERNAL_AUDIO_OUT", ret);
        return -1;
    }
    ret=orbisAudioInit();
    if(ret==1)
    {

        ret=orbisAudioInitChannel(ORBISAUDIO_CHANNEL_MAIN,512,48000,ORBISAUDIO_FORMAT_S16_MONO);
        sleep(1);
        debugNetPrintf(3,"[ORBISLINK] orbisAudioInitChannel IME return 0x%08X \n",ret);
        sleep(1);
        if(ret==0)
        {
            globalConf.confAudio=orbisAudioGetConf();
        }
        else
        {
            return -7;
        }

    }
    else
    {
        return -6;
    }
    ret = sceSysmoduleLoadModule(0x0095); //IME
    debugNetPrintf(3,"[ORBISLINK] sceSysmoduleLoadModule return %x \n",ret);

    return 0;
}
//net systemservice userservice modules
int initOrbisLinkAppVanilla()
{
    int ret;
    ret=loadModulesVanilla();
    return ret;
}
//net nfs create directories in data and upload config and gl modules orbislink default first installation
int initOrbisLinkAppInternal()
{
    int ret;
    pl_ini_file init; //, initapp;
    int debugPort;
    int level;
    struct stat sb;

    loadModulesVanilla();
    //debugNetInit("192.168.1.3",18194,3);


    ret=orbisLinkCreateSelfDirectories();
    if(ret!=0)
    {
        return -1;
    }

    ret=orbisLinkCreateConfigDirectory("/data/orbislink");
    if(ret!=0)
    {
        //debugNetPrintf(3,"[ORBISLINK %s error in orbisLinkCreateConfigDirectory\n",__FUNCTION__);

        return -1;
    }   
    /*unsigned char *buffer;
    int fd=sceKernelOpen("/app0/media/config.ini",O_RDONLY,0777);
    ret=sceKernelLseek(fd,0,SEEK_END);
    sceKernelLseek(fd,0,SEEK_SET);
    buffer=malloc(ret+1);
    sceKernelRead(fd,buffer,ret);
    sceKernelClose(fd);
    fd=-1;
    buffer[ret]='\0';
    fd=sceKernelOpen("/data/orbislink/orbislink_config.ini",O_WRONLY|O_CREAT|O_TRUNC,0666);
    sceKernelWrite(fd,buffer,ret+1);
    sceKernelClose(fd);
    sceKernelSync();
    debugNetPrintf(3,"[ORBISLINK] %s %s\n",__FUNCTION__,buffer);
    fd=-1;
    fd=sceKernelOpen("/data/orbislink/orbislink_config.ini",O_RDONLY,0777);
    ret=sceKernelLseek(fd,0,SEEK_END);
    sceKernelLseek(fd,0,SEEK_SET);
    unsigned char *buffer1;

    buffer1=malloc(ret+1);
    sceKernelRead(fd,buffer1,ret+1);
    sceKernelClose(fd);
    debugNetPrintf(3,"[ORBISLINK] %s %s\n",__FUNCTION__,buffer1);*/


    ret=sceKernelStat("/data/orbislink/orbislink_config.ini", &sb);
    if(ret!=0)
    {
        ret=orbisLinkListenerUp(ORBISLINK_TCP_PORT);
        if(ret!=0)
        {
            return -1;
        }
    }
    sleep(1);
    char serverIp[16];
    char nfsurl[256];
    

    


    ret=pl_ini_load(&init,"/data/orbislink/orbislink_config.ini");
    /*debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return %d\n",__FUNCTION__,ret);

    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return orbisNfsEnabled data %d\n",__FUNCTION__,pl_ini_get_int(&init,"orbisNfs","enabled",1));
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return debugPort data %d\n",__FUNCTION__,pl_ini_get_int(&init,"debugnet","debugPort",18191));
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return debugPort data %d %s\n",__FUNCTION__,pl_ini_get_string(&init,"debugnet","serverIp","192.168.1.4",serverIp,16),serverIp);
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return nfsurl data %d %s\n",__FUNCTION__,pl_ini_get_string(&init,"orbisNfs","nfsurl",NULL,nfsurl,256),nfsurl);




    
    ret=pl_ini_load(&initapp,"/app0/media/config.ini");
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return %d\n",__FUNCTION__,ret);
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return debugPort app0 %d\n",__FUNCTION__,pl_ini_get_int(&initapp,"debugnet","debugPort",18192));
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return orbisNfsEnabled app0 %d\n",__FUNCTION__,pl_ini_get_int(&initapp,"orbisNfs","enabled",1));
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return ipaddress app0 %d %s\n",__FUNCTION__,pl_ini_get_string(&initapp,"debugnet","serverIp","192.168.1.4",serverIp,16),serverIp);
    debugNetPrintf(3,"[ORBISLINK] %s pl_ini_load return nfsurl app0 %d %s\n",__FUNCTION__,pl_ini_get_string(&initapp,"orbisNfs","nfsurl",NULL,nfsurl,256),nfsurl);

*/
    
    

    
    orbisNfsEnabled=pl_ini_get_int(&init,"orbisNfs","enabled",1);
    debugPort=pl_ini_get_int(&init,"debugnet","debugPort",18194);
    level=pl_ini_get_int(&init,"debugnet","level",3);
    pl_ini_get_string(&init,"debugnet","serverIp","192.168.1.3",serverIp,16);

    debugNetInit(serverIp,debugPort,level);


    debugNetPrintf(3,"[ORBISLINK %s config ready placed on /data/orbislink/orbiskink_config.ini\n",__FUNCTION__);
    
    globalConf.confDebug=debugNetGetConf();
    //debugNetPrintf(3,"[ORBISLINK] %s before get nfsurl from config.ini\n",__FUNCTION__);
    ret=pl_ini_get_string(&init,"orbisNfs","nfsurl",NULL,nfsurl,256);
    if(ret==0)
    {
        debugNetPrintf(3,"[ORBISLINK] %s no nfsurl from orbislink_config.ini\n",__FUNCTION__);
        return -1;
    } 
    debugNetPrintf(3,"[ORBISLINK] %s nfsurl %s\n",__FUNCTION__,nfsurl);
    ret=orbisNfsInit(nfsurl);
    if(ret)
    {
        ret=orbisLinkUploadPigletModules();
        if(ret!=0)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s error uploading piglet modules\n",__FUNCTION__);
            return -1;
        }
        
    }
    return 0;
    
}
int initOrbisLinkApp()
{

    int ret;

    int debugPort;
    int level;

    pl_ini_file init;
    pl_ini_file initNfs;
    
    //atexit(&cleanup);
    
    loadModulesVanilla();
    //debugNetInit("192.168.1.3",18194,3);

    pl_ini_load(&init,"/data/orbislink/orbislink_config.ini");
    char serverIp[16];
    char nfsurl[256];
    orbisNfsEnabled=pl_ini_get_int(&init,"orbisNfs","enabled",1);
    debugPort=pl_ini_get_int(&init,"debugnet","debugPort",18194);
    level=pl_ini_get_int(&init,"debugnet","level",3);
    pl_ini_get_string(&init,"debugnet","serverIp","192.168.1.3",serverIp,16);
    
    debugNetInit(serverIp,debugPort,level);
    
    globalConf.orbisLinkFlag=0;
    globalConf.confDebug=debugNetGetConf();
    

    if(orbisNfsEnabled)
    {
        debugNetPrintf(3,"[ORBISLINK] %s before get nfsurl from config.ini\n",__FUNCTION__);
        pl_ini_get_string(&init,"orbisNfs","nfsurl","nfs://192.168.1.3/System/Volumes/Data/usr/local/orbisdev/git/ps4sh/bin/hostapp",nfsurl,256); 
        debugNetPrintf(3,"[ORBISLINK] %s nfsurl %s\n",__FUNCTION__,nfsurl);
        ret=orbisNfsInit(nfsurl);
        if(ret)
        {
            globalConf.confNfs=orbisNfsGetConf();
            ret=pl_ini_load_from_nfs(&initNfs,"config.ini");
            if(ret)
            {
                padEnabled=pl_ini_get_int(&initNfs,"orbisPad","enabled",1);
                orbisGlEnabled=pl_ini_get_int(&initNfs,"orbisGl","enabled",1);
                audioEnabled=pl_ini_get_int(&initNfs,"orbisAudio","enabled",0);
                msxEnabled=pl_ini_get_int(&initNfs,"fmsx","enabled",0);
                audioSamples=pl_ini_get_int(&initNfs,"orbisAudio","samples",1024);
                audioFrequency=pl_ini_get_int(&initNfs,"orbisAudio","frequency",48000);
                audioFormat=pl_ini_get_int(&initNfs,"orbisAudio","format",ORBISAUDIO_FORMAT_S16_STEREO);
                keyboardEnabled=pl_ini_get_int(&initNfs,"orbisKeyboard","enabled",0);

                if(msxEnabled)
                {
                    debugNetPrintf(3,"[ORBISLINK] %s local msx audio config enabled\n",__FUNCTION__);
                    audioEnabled=1;
                    audioSamples=500;
                    audioFrequency=48000;
                    audioFormat=ORBISAUDIO_FORMAT_S16_MONO;
                }
                ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PAD);
                if(ret) 
                {
                    debugNetPrintf(3,"[ORBISLINK] %s sceSysmoduleLoadModuleInternal(%s) failed: 0x%08X\n",__FUNCTION__,"SCE_SYSMODULE_INTERNAL_PAD", ret);
                    return -1;
                }
                debugNetPrintf(3,"[ORBISLINK] %s sceSysmoduleLoadModuleInternal(%s) return: 0x%08X\n",__FUNCTION__,"SCE_SYSMODULE_INTERNAL_PAD", ret);

                if(padEnabled==1)
                {
                    ret=orbisPadInit();
                }
                else
                {
                    return -2;
                }

                globalConf.confPad=orbisPadGetConf();
                debugNetPrintf(3,"[ORBISLINK] %s liborbisPad initialized\n",__FUNCTION__);

                if(orbisGlEnabled==1)
                {
                    //g_sandbox_word = sceKernelGetFsSandboxRandomWord();
                    //if (!g_sandbox_word) 
                    //{
                    //  debugNetPrintf(DEBUGNET_ERROR,"[ORBISLINK] %s sceKernelGetFsSandboxRandomWord failed. 0x%08X\n",__FUNCTION__,g_sandbox_word);
                    //  return -3;
                    //}
                    //debugNetPrintf(3,"[ORBISLINK] %s sandbox %s\n",__FUNCTION__,g_sandbox_word);
                    ret=loadModulesGl(2,DEBUGNET_DEBUG);
                    if(ret<0)
                    {
                        return ret;
                    }
                    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s piglet modules patched\n",__FUNCTION__);
                }   
                if(audioEnabled==1)
                {
                    ret=sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_AUDIO_OUT);
                    if (ret) 
                    {
                        debugNetPrintf(3,"[ORBISLINK] sceSysmoduleLoadModuleInternal(%s) failed: 0x%08X\n", "SCE_SYSMODULE_INTERNAL_AUDIO_OUT", ret);
                        return -1;
                    }
                    ret=orbisAudioInit();
                    if(ret==1)
                    {

                        ret=orbisAudioInitChannel(ORBISAUDIO_CHANNEL_MAIN,512,48000,ORBISAUDIO_FORMAT_S16_MONO);
                        sleep(1);
                        debugNetPrintf(3,"[ORBISLINK] orbisAudioInitChannel return %x \n",ret);
                        sleep(1);
                        if(ret==0)
                        {
                            globalConf.confAudio=orbisAudioGetConf();
                        }
                        else
                        {
                            return -7;
                        }

                    }
                    else
                    {
                        return -6;
                    }
                }
                if(keyboardEnabled)
                {
                    int ret1 = sceSysmoduleLoadModule(0x0095);
                    
                    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] sceSysmoduleLoadModule(%s) return: 0x%08X\n", "SCE_SYSMODULE_LIBIME", ret1);
                    
                    ret=orbisKeyboardInit();
                    debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s orbisKeyboardInit return %d\n",__FUNCTION__,ret);
                    if(ret==1)
                    {
                        globalConf.confKeyboard=(OrbisKeyboardConfig *)orbisKeyboardGetConf();
                        ret=orbisKeyboardOpen();
                        debugNetPrintf(DEBUGNET_DEBUG,"[ORBISLINK] %s orbisKeyboardOpen return %d\n",__FUNCTION__,ret);
                    }
                    else
                    {
                        return -8;
                    }
                }
            }
            else
            {
                debugNetPrintf(3,"[ORBISLINK] %s no config.ini file on nfsurl %s\n",__FUNCTION__,nfsurl);
                return -9;
            }
        }
        else
        {
            return -10;
        }
        
    }
    else
    {
        debugNetPrintf(3,"[ORBISLINK] %s no nfs enabled on config.ini in app0/media\n",__FUNCTION__);
        return -11;
    }
    return 0;
}
OrbisGlobalConf *orbisLinkGetGlobalConf()
{
    return &globalConf;
}
/*int main()
{
    
    int ret=initOrbisLinkApp();
    
    if(ret>=0)
    {
        
        

        debugNetPrintf(3,"[ORBISLINK] %s ready tp loading homebrew.elf\n",__FUNCTION__);
        //orbisExecUserElf();
    
        while(!orbisLinkGetGlobalConf()->orbisLinkFlag)
        {
        
        }
    }
    else
    {
        debugNetPrintf(3,"[ORBISLINK] %s something wrong happen initOrbisLinkApp return 0x%8x %d \n",__FUNCTION__,ret,ret);
        debugNetPrintf(3,"[ORBISLINK] %s Exiting\n",__FUNCTION__);
        
    }
    
    finishOrbisLinkApp();

    exit(0);

    return 0;
}
void catchReturnFromMain(int exit_code) {
}*/


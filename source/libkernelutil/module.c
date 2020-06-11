#include <stdlib.h>
#include <string.h>
#include <ps4sdk.h>
#include <prerequisites.h>
int mysyscall(int num, ...);

int sceKernelGetModuleInfo(SceKernelModule handle, SceKernelModuleInfo* info) {
	int ret;

	if (!info) {
		ret =  0x8002000E;
		goto err;
	}

	memset(info, 0, sizeof(*info));
	{
		info->size = sizeof(*info);
	}

	ret = mysyscall(593, handle, info); /* TODO: make proper error code */

err:
	return ret;
}

int sceKernelGetModuleInfoByName(const char* name, SceKernelModuleInfo* info) {
	SceKernelModuleInfo tmpInfo;
	SceKernelModule handles[256];
	size_t numModules;
	size_t i;
	int ret;

	if (!name) {
		ret =  0x8002000E;
		goto err;
	}
	if (!info) {
		ret =  0x8002000E;
		goto err;
	}

	memset(handles, 0, sizeof(handles));

	ret = sceKernelGetModuleList(handles, ARRAY_SIZE(handles), &numModules);
	if (ret) {
		goto err;
	}

	for (i = 0; i < numModules; ++i) {
		ret = sceKernelGetModuleInfo(handles[i], &tmpInfo);
		if (ret) {
			goto err;
		}

		if (strcmp(tmpInfo.name, name) == 0) {
			memcpy(info, &tmpInfo, sizeof(tmpInfo));
			ret = 0;
			goto err;
		}
	}

	ret = 0x80020002;

err:
	return ret;
}

int sceKernelGetModuleInfoEx(SceKernelModule handle, SceKernelModuleInfoEx* info) {
	int ret;

	if (!info) {
		ret =  0x8002000E;
		goto err;
	}

	memset(info, 0, sizeof(*info));
	{
		info->size = sizeof(*info);
	}

	ret = mysyscall(608, handle, info); /* TODO: make proper error code */

err:
	return ret;
}

int sceKernelGetModuleInfoExByName(const char* name, SceKernelModuleInfoEx* info) {
	SceKernelModuleInfoEx tmpInfo;
	SceKernelModule handles[256];
	size_t numModules;
	size_t i;
	int ret;

	if (!name) {
		ret =  0x8002000E;
		goto err;
	}
	if (!info) {
		ret =  0x8002000E;
		goto err;
	}

	memset(handles, 0, sizeof(handles));

	ret = sceKernelGetModuleList(handles, ARRAY_SIZE(handles), &numModules);
	if (ret) {
		goto err;
	}

	for (i = 0; i < numModules; ++i) {
		ret = sceKernelGetModuleInfoEx(handles[i], &tmpInfo);
		if (ret) {
			goto err;
		}

		if (strcmp(tmpInfo.name, name) == 0) {
			memcpy(info, &tmpInfo, sizeof(tmpInfo));
			ret = 0;
			goto err;
		}
	}

	ret = 0x80020002;

err:
	return ret;
}



#include <unistd.h>

#include "WINGsP.h"


extern void W_InitNotificationCenter(void);


struct W_Application WMApplication;


char *_WINGS_progname = NULL;



Bool 
W_ApplicationInitialized(void)
{
    return _WINGS_progname!=NULL;
}


void
WMInitializeApplication(char *applicationName, int *argc, char **argv)
{
    int i;
    
    assert(argc!=NULL);
    assert(argv!=NULL);
    assert(applicationName!=NULL);

    _WINGS_progname = argv[0];
    
    WMApplication.applicationName = wstrdup(applicationName);
    WMApplication.argc = *argc;

    WMApplication.argv = wmalloc((*argc+1)*sizeof(char*));
    for (i=0; i<*argc; i++) {
	WMApplication.argv[i] = wstrdup(argv[i]);
    }
    WMApplication.argv[i] = NULL;
    
    /* initialize notification center */
    W_InitNotificationCenter();
}


void
WMSetResourcePath(char *path)
{
    if (WMApplication.resourcePath)
	free(WMApplication.resourcePath);
    WMApplication.resourcePath = wstrdup(path);
}


char*
WMGetApplicationName()
{
    return WMApplication.applicationName;
}


static char*
checkFile(char *path, char *folder, char *ext, char *resource)
{
    char *ret;
    int extralen;
    
    extralen = (ext ? strlen(ext) : 0) + (folder ? strlen(folder) : 0) + 4; 
    ret = wmalloc(strlen(path)+strlen(resource)+extralen+8);
    strcpy(ret, path);
    if (folder) {
	strcat(ret, "/");
	strcat(ret, folder);
    }
    if (ext) {
	strcat(ret, "/");
	strcat(ret, ext);
    }
    strcat(ret, "/");
    strcat(ret, resource);
    
    if (access(ret, F_OK)!=0) {
	free(ret);
	ret = NULL;
    }

    return ret;
}


char*
WMPathForResourceOfType(char *resource, char *ext)
{
    char *path = NULL;
    char *tmp, *appdir;
    int i;
    
    /* 
     * Paths are searched in this order:
     * - resourcePath/ext
     * - argv[0]/ext
     * - GNUSTEP_USER_ROOT/Apps/ApplicationName.app/ext 
     * - ~/GNUstep/Apps/ApplicationName.app/ext
     * - GNUSTEP_LOCAL_ROOT/Apps/ApplicationName.app/ext
     * - /usr/local/GNUstep/Apps/ApplicationName.app/ext
     * - GNUSTEP_SYSTEM_ROOT/Apps/ApplicationName.app/ext
     * - /usr/GNUstep/Apps/ApplicationName.app/ext
     */
    
    if (WMApplication.resourcePath) {
	path = checkFile(WMApplication.resourcePath, NULL, ext, resource);
	if (path)
	    return path;
    }

    if (WMApplication.argv[0]) {
	tmp = wstrdup(WMApplication.argv[0]);
	i = strlen(tmp);
	while (i > 0 && tmp[i]!='/')
	    i--;
	tmp[i] = 0;
	if (i>0) {
	    path = checkFile(tmp, NULL, ext, resource);
	} else {
	    path = NULL;
	}
	free(tmp);
	if (path)
	    return path;
    }
    
    appdir = wmalloc(strlen(WMApplication.applicationName)+10);
    sprintf(appdir, "Apps/%s.app", WMApplication.applicationName);

    if (getenv("GNUSTEP_USER_ROOT")) {
	path = checkFile(getenv("GNUSTEP_USER_ROOT"), appdir, ext, resource);
	if (path) {
	    free(appdir);
	    return path;
	}
    }
    
    tmp = wusergnusteppath();
    if (tmp) {
	path = checkFile(tmp, appdir, ext, resource);
	if (path) {
	    free(appdir);
	    return path;
	}
    }
    
    if (getenv("GNUSTEP_LOCAL_ROOT")) {
	path = checkFile(getenv("GNUSTEP_LOCAL_ROOT"), appdir, ext, resource);
	if (path) {
	    free(appdir);
	    return path;
	}
    }
    
    path = checkFile("/usr/local/GNUstep", appdir, ext, resource);
    if (path) {
	free(appdir);
	return path;
    }

    
    if (getenv("GNUSTEP_SYSTEM_ROOT")) {
	path = checkFile(getenv("GNUSTEP_SYSTEM_ROOT"), appdir, ext, resource);
	if (path) {
	    free(appdir);
	    return path;
	}
    }

    path = checkFile("/usr/GNUstep", appdir, ext, resource);
    if (path) {
	free(appdir);
	return path;
    }
    
    return NULL;
}



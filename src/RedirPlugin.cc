#include <sys/stat.h>
#include <unistd.h>
#include <XrdVersion.hh>
#include <cstdlib>
#include <iostream>
#include "RedirPlugin.hh"
//------------------------------------------------------------------------------
//! Necessary implementation for XRootD to get the Plug-in
//------------------------------------------------------------------------------
extern "C" XrdCmsClient *XrdCmsGetClient(XrdSysLogger *Logger,
                                         int opMode,
                                         int myPort,
                                         XrdOss *theSS)
{
    RedirPlugin *plugin = new RedirPlugin(Logger, opMode, myPort, theSS);
    return plugin;
}

//------------------------------------------------------------------------------
//! Constructor
//------------------------------------------------------------------------------
RedirPlugin::RedirPlugin(XrdSysLogger *Logger,
                         int opMode,
                         int myPort,
                         XrdOss *theSS)
    : XrdCmsClient(amLocal)
{
    nativeCmsFinder = new XrdCmsFinderRMT(Logger, opMode, myPort);
    this->theSS = theSS;
}
//------------------------------------------------------------------------------
//! Destructor
//------------------------------------------------------------------------------
RedirPlugin::~RedirPlugin()
{
    delete nativeCmsFinder;
}

//------------------------------------------------------------------------------
//! Configure the nativeCmsFinder
//------------------------------------------------------------------------------
int RedirPlugin::Configure(const char *cfn, char *Parms, XrdOucEnv *EnvInfo)
{
    if(nativeCmsFinder)
        return nativeCmsFinder->Configure(cfn, Parms, EnvInfo);
    return 0;
}

//------------------------------------------------------------------------------
//! Locate the file, get Client IP and target IP.
//! 1) If both are private, redirect to local does apply.
//!    set ErrInfo of param Resp and return SFS_REDIRECT.
//! 2) Not both are private, redirect to local does NOT apply.
//!    return nativeCmsFinder->Locate, for normal redirection procedure
//!
//! @Param Resp: Either set manually here or passed to nativeCmsFinder->Locate
//! @Param path: The path of the file, passed to nativeCmsFinder->Locate
//! @Param flags: The open flags, passed to nativeCmsFinder->Locate
//! @Param EnvInfo: Contains the secEnv, which contains the addressInfo of the
//!                 Client. Checked to see if redirect to local conditions apply
//------------------------------------------------------------------------------
int RedirPlugin::Locate(XrdOucErrInfo &Resp,
                        const char *path,
                        int flags,
                        XrdOucEnv *EnvInfo)
{
    XrdCl::URL *target = new XrdCl::URL(path);
    std::cout << "path is: " << path << std::endl;
    std::string pathLookup = target->GetPath();
    std::cout << pathLookup << std::endl;
    if(const char *proxyprefix = std::getenv("PROXYPREFIX")) {
        std::cout << "Your PROXYPREFIX is: " << proxyprefix << '\n';
        int rc = 0;                // figure out exact meaning
        int maxPathLength = 4096;  // total acceptable buffer length,
        // which must be longer than localroot and filename concatenated
        char *buff = new char[maxPathLength];
        const char *ppath =
            theSS->Lfn2Pfn(pathLookup.c_str(), buff, maxPathLength, rc);
        std::cout << ppath << std::endl;

        if(access(ppath, F_OK)) {  // File found
            std::cout << "File exists, redirect to local" << std::endl;
            Resp.setErrInfo(-1,
                            ppath);  // set info which will be sent to client
            return SFS_REDIRECT;
        } else {
            char proxypath[1000];            // array to hold the result.
            strcpy(proxypath, proxyprefix);  // copy string one into the result.
            strcat(proxypath, path);
            std::cout << "File doesn't exist, redirect to proxy" << std::endl;
            Resp.setErrInfo(
                target->GetPort(),//TO DO: proxy port needed, not target port
                proxypath);  // set info which will be sent to client
            return SFS_REDIRECT;
        }
    }
    return SFS_ERROR;
}

//------------------------------------------------------------------------------
//! Space
//! Calls nativeCmsFinder->Space
//------------------------------------------------------------------------------
int RedirPlugin::Space(XrdOucErrInfo &Resp,
                       const char *path,
                       XrdOucEnv *EnvInfo)
{
    if(nativeCmsFinder)
        return nativeCmsFinder->Space(Resp, path, EnvInfo);
    return 0;
}

XrdVERSIONINFO(XrdCmsGetClient, RedirPlugin);

#include "RedirPlugin.hh"
#include <XrdOuc/XrdOucStream.hh>
#include <XrdOuc/XrdOucString.hh>
#include <XrdVersion.hh>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>

//------------------------------------------------------------------------------
//! Necessary implementation for XRootD to get the Plug-in
//------------------------------------------------------------------------------
extern "C" XrdCmsClient *XrdCmsGetClient(XrdSysLogger *Logger, int opMode,
                                         int myPort, XrdOss *theSS) {
  RedirPlugin *plugin = new RedirPlugin(Logger, opMode, myPort, theSS);
  return plugin;
}

//------------------------------------------------------------------------------
//! Constructor
//------------------------------------------------------------------------------
RedirPlugin::RedirPlugin(XrdSysLogger *Logger, int opMode, int myPort,
                         XrdOss *theSS)
    : XrdCmsClient(amLocal) {
  nativeCmsFinder = new XrdCmsFinderRMT(Logger, opMode, myPort);
  this->theSS = theSS;
  readOnlyredirect = false;
}
//------------------------------------------------------------------------------
//! Destructor
//------------------------------------------------------------------------------
RedirPlugin::~RedirPlugin() { delete nativeCmsFinder; }

//------------------------------------------------------------------------------
//! Configure the nativeCmsFinder
//------------------------------------------------------------------------------
int RedirPlugin::Configure(const char *cfn, char *Parms, XrdOucEnv *EnvInfo) {
  loadConfig(cfn);
  if (nativeCmsFinder)
    return nativeCmsFinder->Configure(cfn, Parms, EnvInfo);
  return 0;
}

void RedirPlugin::loadConfig(const char *filename) {

  XrdOucStream Config;
  int cfgFD;
  char *word;

  if ((cfgFD = open(filename, O_RDONLY, 0)) < 0) {
    return;
  }
  Config.Attach(cfgFD);
  while ((word = Config.GetFirstWord(true))) {//get word in lower case
    if (strcmp(word, "redirplugin.localroot") == 0) {
      localroot = std::string(Config.GetWord());
    }
    //search for readonlyredirect, 
    //which only allows read calls to be redirected to local
    else if(strcmp(word, "redirplugin.readonlyredirect") == 0){
        // get next word in lower case
      std::string readWord = std::string(Config.GetWord(true));//to lower case
      if(readWord.find("true")!=string::npos)
          readOnlyredirect = true;
    }
  }
  if (localroot.empty())
    throw std::runtime_error(
        "Redirplugin.Localroot not set in configuration file");
  Config.Close();
}

//------------------------------------------------------------------------------
//! Preconditions:
//! Client Protocol Version is >= 784
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
int RedirPlugin::Locate(XrdOucErrInfo &Resp, const char *path, int flags,
                        XrdOucEnv *EnvInfo) {
  int rcode = 0;
  if (nativeCmsFinder) {
    bool privClient = EnvInfo->secEnv()->addrInfo->isPrivate();
    // get regular target host
    rcode = nativeCmsFinder->Locate(Resp, path, flags, EnvInfo); 

    int pversion = Resp.getUCap();
    pversion &= 0x0000ffff; // get protocol version
    
    if (pversion < 784)// native redirect in case of old protocol version
        return rcode;
    if (flags & SFS_O_STAT)
        return rcode;     // always use native function if you want to do stat

    if(readOnlyredirect){
        if (!(flags == SFS_O_RDONLY))
            return rcode;
    }

    int rc = 0;               // figure out exact meaning
    int maxPathLength = 4096; // total acceptable buffer length,
    // which must be longer than localroot and filename concatenated
    char *buff = new char[maxPathLength];
    const char *ppath = theSS->Lfn2Pfn(path, buff, maxPathLength, rc);
    XrdNetAddr target(-1); // port is necessary, but can be any
    target.Set(Resp.getErrText());
    bool privTarget = target.isPrivate();
    if (privClient && privTarget) {
      // set info which will be sent to client
      Resp.setErrInfo(-1, (localroot + std::string(ppath)).c_str());
      return SFS_REDIRECT;
    }
  }
  return rcode;
}

//------------------------------------------------------------------------------
//! Space
//! Calls nativeCmsFinder->Space
//------------------------------------------------------------------------------
int RedirPlugin::Space(XrdOucErrInfo &Resp, const char *path,
                       XrdOucEnv *EnvInfo) {
  if (nativeCmsFinder)
    return nativeCmsFinder->Space(Resp, path, EnvInfo);
  return 0;
}

XrdVERSIONINFO(XrdCmsGetClient, RedirPlugin);

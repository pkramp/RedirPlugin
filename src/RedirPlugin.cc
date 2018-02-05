#include "RedirPlugin.hh"
#include <XrdOuc/XrdOucStream.hh>
#include <XrdOuc/XrdOucString.hh>
#include <XrdVersion.hh>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>

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
  char *var;

  if ((cfgFD = open(filename, O_RDONLY, 0)) < 0) {
    return;
  }

  Config.Attach(cfgFD);
  while ((var = Config.GetMyFirstWord())) {
    if (strcmp(var, "RedirPlugin.Localroot") == 0) {
      var += 21;
      localroot = std::string(Config.GetWord());
      break;
    }
  }
  std::cout << "RedirPlugin.LocalRoot set to:" << localroot << std::endl;
  if (localroot.empty())
    throw std::runtime_error(
        "Redirplugin.Localroot not set in configuration file");
  Config.Close();
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
int RedirPlugin::Locate(XrdOucErrInfo &Resp, const char *path, int flags,
                        XrdOucEnv *EnvInfo) {
  int rcode = 0;
  if (nativeCmsFinder) {
    bool privClient = EnvInfo->secEnv()->addrInfo->isPrivate();
    rcode = nativeCmsFinder->Locate(Resp, path, flags,
                                    EnvInfo); // get regular target host
    if (flags & SFS_O_STAT)
      return rcode; // always use native function if you want to do stat

    int rc = 0;               // figure out exact meaning
    int maxPathLength = 4096; // total acceptable buffer length,
    // which must be longer than localroot and filename concatenated
    char *buff = new char[maxPathLength];
    const char *ppath = theSS->Lfn2Pfn(path, buff, maxPathLength, rc);
    XrdNetAddr target(-1); // port is necessary, but can be any
    target.Set(Resp.getErrText());
    bool privTarget = target.isPrivate();
    if (privClient && privTarget) {
      Resp.setErrInfo(-1,
                      (localroot + std::string(ppath))
                          .c_str()); // set info which will be sent to client
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

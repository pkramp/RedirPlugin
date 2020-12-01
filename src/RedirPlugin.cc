#include "RedirPlugin.hh"
#include <XrdVersion.hh>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
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

  if (nativeCmsFinder) {
    loadConfig(cfn);
    return nativeCmsFinder->Configure(cfn, Parms, EnvInfo);
  }
  return 0;
}

void RedirPlugin::loadConfig(const char *cfn) {

  XrdOucStream Config;
  int cfgFD;
  char *var;

  if ((cfgFD = open(cfn, O_RDONLY, 0)) < 0) {
    return;
  }

  Config.Attach(cfgFD);
  while ((var = Config.GetMyFirstWord())) {
    if (strcmp(var, "Redir.prefix") == 0) {
      var += 13;
      prefix = std::string(Config.GetWord());
      break;
    }
  }
  if (prefix.empty())
    throw std::runtime_error("Redir.prefix not set in configuration file");
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
  std::string rpath(path);
  rpath.erase(0, 1);
  XrdCl::URL target(rpath);
  std::string pathLookup = target.GetURL();
  size_t splitpos = pathLookup.find("/");
  if (splitpos == string::npos) 
    return SFS_ERROR;

  //size_t slen = pathLookup.size();
  XrdCl::URL proxyTarget(std::string("root://") + prefix + std::string("/x") +
                         pathLookup.substr(0, splitpos));
  int rc = 0;               // figure out exact meaning
  int maxPathLength = 4096; // total acceptable buffer length,
  // which must be longer than localroot and filename concatenated
  char *buff = new char[maxPathLength];
  const char *ppath =
      theSS->Lfn2Pfn(path, buff,
                     maxPathLength, rc);

  if (access(ppath, F_OK) == 0) { // File found
    std::cerr << "File exists, redirect to local" << std::endl;
    Resp.setErrInfo(-1, ppath); // set info which will be sent to client
    return SFS_REDIRECT;
  } else {
    std::cerr << "File doesn't exist, redirect to proxy" << std::endl;
    Resp.setErrInfo(proxyTarget.GetPort(),
                    proxyTarget.GetHostName()
                        .c_str()); // set info which will be sent to client
    return SFS_REDIRECT;
  }
  return SFS_ERROR;
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

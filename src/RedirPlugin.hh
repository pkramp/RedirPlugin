#ifndef REDIRPLUGIN_HH_
#define REDIRPLUGIN_HH_
#include "in/XrdCmsFinder.hh"
#include <XrdCl/XrdClURL.hh>
#include <XrdCms/XrdCmsClient.hh>
#include <XrdNet/XrdNetAddr.hh>
#include <XrdOss/XrdOss.hh>
#include <XrdOuc/XrdOucEnv.hh>
#include <XrdOuc/XrdOucStream.hh>
#include <XrdOuc/XrdOucString.hh>
#include <XrdSec/XrdSecEntity.hh>
#include <XrdSfs/XrdSfsInterface.hh>
#include <XrdSys/XrdSysLogger.hh>
#include <fcntl.h>

class RedirPlugin : public XrdCmsClient {
public:
  RedirPlugin(XrdSysLogger *Logger, int opMode, int myPort, XrdOss *theSS);
  ~RedirPlugin();
  int Configure(const char *cfn, char *Parms, XrdOucEnv *EnvInfo);

  int Locate(XrdOucErrInfo &Resp, const char *path, int flags,
             XrdOucEnv *EnvInfo);

  int Space(XrdOucErrInfo &Resp, const char *path, XrdOucEnv *EnvInfo);
  void loadConfig(const char *cfn);

  //---------------------------------------------------------------------------
  //! used to forward requests to CmsFinder with regular implementation
  //---------------------------------------------------------------------------
  XrdCmsClient *nativeCmsFinder;
  XrdOss *theSS;
  std::string prefix;
};

#endif // REDIRPLUGIN_HH_

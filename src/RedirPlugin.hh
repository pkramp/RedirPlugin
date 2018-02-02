#ifndef REDIRPLUGIN_HH_
#define REDIRPLUGIN_HH_
#include "in/XrdCmsFinder.hh"
#include <XrdCms/XrdCmsClient.hh>
#include <XrdNet/XrdNetAddr.hh>
#include <XrdOss/XrdOss.hh>
#include <XrdOuc/XrdOucEnv.hh>
#include <XrdSec/XrdSecEntity.hh>
#include <XrdSfs/XrdSfsInterface.hh>
#include <XrdSys/XrdSysLogger.hh>
#include <string>

class RedirPlugin : public XrdCmsClient {
public:
  RedirPlugin(XrdSysLogger *Logger, int opMode, int myPort, XrdOss *theSS);
  ~RedirPlugin();
  int Configure(const char *cfn, char *Parms, XrdOucEnv *EnvInfo);
  void loadConfig(const char *filename);
  int Locate(XrdOucErrInfo &Resp, const char *path, int flags,
             XrdOucEnv *EnvInfo);

  int Space(XrdOucErrInfo &Resp, const char *path, XrdOucEnv *EnvInfo);
  void Added(const char *path, int Pend = 0) {
    nativeCmsFinder->Added(path, Pend);
  }
  int Forward(XrdOucErrInfo &Resp, const char *cmd, const char *arg1 = 0,
              const char *arg2 = 0, XrdOucEnv *Env1 = 0, XrdOucEnv *Env2 = 0) {
    return nativeCmsFinder->Forward(Resp, cmd, arg1, arg2, Env1, Env2);
  }
  int isRemote() { return nativeCmsFinder->isRemote(); }
  XrdOucTList *Managers() { return nativeCmsFinder->Managers(); }
  int Prepare(XrdOucErrInfo &Resp, XrdSfsPrep &pargs, XrdOucEnv *Info = 0) {
    return nativeCmsFinder->Prepare(Resp, pargs, Info);
  }
  void Removed(const char *path) { return nativeCmsFinder->Removed(path); }
  void Resume(int Perm = 1) { nativeCmsFinder->Resume(Perm); }
  void Suspend(int Perm = 1) { nativeCmsFinder->Suspend(Perm); }
  int Resource(int n) { return nativeCmsFinder->Resource(n); }
  int Reserve(int n = 1) { return nativeCmsFinder->Reserve(n); }
  int Release(int n = 1) { return nativeCmsFinder->Release(n); }

  //---------------------------------------------------------------------------
  //! used to forward requests to CmsFinder with regular implementation
  //---------------------------------------------------------------------------
  XrdCmsClient *nativeCmsFinder;
  XrdOss *theSS;
  std::string localroot;
};

#endif // REDIRPLUGIN_HH_

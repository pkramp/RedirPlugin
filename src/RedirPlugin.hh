#ifndef REDIRPLUGIN_HH_
#define REDIRPLUGIN_HH_
#include <XrdOuc/XrdOucEnv.hh>
#include <XrdSec/XrdSecEntity.hh>
#include <XrdSys/XrdSysLogger.hh>
#include <XrdCms/XrdCmsClient.hh>
#include <XrdSfs/XrdSfsInterface.hh>
#include <XrdNet/XrdNetAddr.hh>
#include <XrdOss/XrdOss.hh>
#include "in/XrdCmsFinder.hh"
#include <XrdCl/XrdClURL.hh>

class RedirPlugin : public XrdCmsClient
{
public:
   RedirPlugin( XrdSysLogger *Logger, int opMode, int myPort, XrdOss *theSS );
   ~RedirPlugin();
   int Configure( const char *cfn, char *Parms, XrdOucEnv *EnvInfo );

   int Locate( XrdOucErrInfo &Resp, const char *path, int flags, XrdOucEnv *EnvInfo );

   int Space( XrdOucErrInfo &Resp, const char *path, XrdOucEnv *EnvInfo );

   //---------------------------------------------------------------------------
   //! used to forward requests to CmsFinder with regular implementation
   //---------------------------------------------------------------------------
   XrdCmsClient *nativeCmsFinder;
   XrdOss *theSS;
};

#endif // REDIRPLUGIN_HH_

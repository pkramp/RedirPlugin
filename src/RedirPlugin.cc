#include "RedirPlugin.hh"
#include <XrdVersion.hh>
//------------------------------------------------------------------------------
//! Necessary implementation for XRootD to get the Plug-in
//------------------------------------------------------------------------------
extern "C" XrdCmsClient*
XrdCmsGetClient(XrdSysLogger* Logger, int opMode, int myPort, XrdOss* theSS)
{
   RedirPlugin* plugin = new RedirPlugin(Logger, opMode, myPort, theSS);
   return plugin;
}

//------------------------------------------------------------------------------
//! Constructor
//------------------------------------------------------------------------------
RedirPlugin::RedirPlugin(XrdSysLogger* Logger,
                         int opMode,
                         int myPort,
                         XrdOss* theSS)
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
int RedirPlugin::Configure(const char* cfn, char* Parms, XrdOucEnv* EnvInfo)
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
int RedirPlugin::Locate(XrdOucErrInfo& Resp,
                        const char* path,
                        int flags,
                        XrdOucEnv* EnvInfo)
{
   if(nativeCmsFinder) {
      if(EnvInfo->secEnv()->addrInfo->isPrivate()) { // Client is private
         nativeCmsFinder->Locate(
             Resp, path, flags, EnvInfo); // get regular target host
         int rc = 0;                      // figure out exact meaning
         int maxPathLength = 4096;        // total acceptable buffer length,
         // which must be longer than localroot and filename concatenated
         char* buff = new char[maxPathLength];
         const char* ppath = theSS->Lfn2Pfn(path, buff, maxPathLength, rc);
         XrdNetAddr target(-1); // port is necessary, but can be any
         target.Set(Resp.getErrText());
         if(target.Name() && target.isPrivate()) { // Name must exist
            // now both client and target are private
            Resp.setErrInfo(-1, ppath); // set info which will be sent to client
            return SFS_REDIRECT;
         }
         // target is not private, do regular locate
         return nativeCmsFinder->Locate(Resp, path, flags, EnvInfo);
      } else {
         // client is not private, do regular locate
         return nativeCmsFinder->Locate(Resp, path, flags, EnvInfo);
      }
   } else
      return 0;
}

//------------------------------------------------------------------------------
//! Space
//! Calls nativeCmsFinder->Space
//------------------------------------------------------------------------------
int
RedirPlugin::Space(XrdOucErrInfo& Resp, const char* path, XrdOucEnv* EnvInfo)
{
   if(nativeCmsFinder)
      return nativeCmsFinder->Space(Resp, path, EnvInfo);
   return 0;
}

XrdVERSIONINFO(XrdCmsGetClient, RedirPlugin);

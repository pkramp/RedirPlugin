#include "RedirPlugin.hh"
#include <XrdVersion.hh>
#include <iostream>
//------------------------------------------------------------------------------
//! Necessary implementation for XRootD to get the Plug-in
//------------------------------------------------------------------------------
extern "C" XrdCmsClient *XrdCmsGetClient(XrdSysLogger *Logger,
                                             int           opMode,
                                             int           myPort,
                                             XrdOss       *theSS)
{
   //std::cout << "New CmsClient" << std::endl;
   RedirPlugin *plugin = new RedirPlugin( Logger, opMode, myPort, theSS );
   return plugin;
}

//------------------------------------------------------------------------------
//! Constructor
//------------------------------------------------------------------------------
RedirPlugin::RedirPlugin( XrdSysLogger *Logger, int opMode, int myPort, XrdOss *theSS ) 
                                                      : XrdCmsClient( amLocal ){
    nativeCmsFinder = new XrdCmsFinderRMT( Logger, opMode, myPort );
    //nativeCmsFinder->setSS( theSS );
}
//------------------------------------------------------------------------------
//! Destructor
//------------------------------------------------------------------------------
RedirPlugin::~RedirPlugin(){
   delete nativeCmsFinder;
}

//------------------------------------------------------------------------------
//! Configure the nativeCmsFinder
//------------------------------------------------------------------------------
int RedirPlugin::Configure(const char *cfn, char *Parms, XrdOucEnv *EnvInfo){
    if( nativeCmsFinder )
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
int RedirPlugin::Locate( XrdOucErrInfo &Resp, 
                         const char    *path, 
                         int           flags, 
                         XrdOucEnv  *EnvInfo ) { 
    if( EnvInfo->secEnv()->addrInfo->isPrivate() ){ //Client is private
        //std::cout << "Client is: " << EnvInfo->secEnv()->addrInfo->Name() << std::endl;
        nativeCmsFinder->Locate( Resp, path, flags, EnvInfo );//get regular target host
        XrdNetAddr X( 1094 );//port is necessary, but can be any
        X.Set( Resp.getErrText() );
        if( X.Name() ){ //Name must exist
            //std::cout << "Target is: " << X.Name() << std::endl;
            //std::cout << "Path is: " << path << std::endl;
            if( X.isPrivate() ) {//now both client and target are private
            	//std::cout << "Both are private" << std::endl;
                Resp.setErrInfo( -1, path );
                return SFS_REDIRECT;
            }
        }
        return nativeCmsFinder->Locate( Resp, path, flags, EnvInfo );
    }
    else {
        if( nativeCmsFinder )//client is not private, do regular locate
            return nativeCmsFinder->Locate( Resp, path, flags, EnvInfo );
        else return 0;//failure
    }
}

//------------------------------------------------------------------------------
//! Space
//! Calls nativeCmsFinder->Space
//------------------------------------------------------------------------------
int RedirPlugin::Space( XrdOucErrInfo &Resp, const char *path, XrdOucEnv *EnvInfo ){
   return nativeCmsFinder->Space( Resp, path, EnvInfo );
}

XrdVERSIONINFO( XrdCmsGetClient, RedirPlugin );

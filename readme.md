#XRootDRedirPlugin 
An XRootD server(cms) plug-in to "redirect" XRootD calls to a locally mounted file system.
This only applies if both client and the normally used dataserver have private IP-Adresses/Hostnames
# Warning
The source tree includes private header files of the XrdCms source tree(namely XrdCmsFinder.hh) and needs to be kept in sync with the XRootD version source used!

# Plug-in configuration
This plug-in currently has no configuration

# Install and tests
To compile the plug-in, you need to set the XROOTD_PATH environmental variable to the toplevel of your XRootD installation.

You can compile the plug-in library with :
```shell
cmake3 .
make
```
# Usage
When using this plug-in, all high level XRootD calls (xrdcp, from TNetXNGFile in ROOT, etc.) should instead be "redirected" to a file available in the local file system.

# License
The XrdOpenLocal plug-in is distributed under the terms of the GNU Lesser Public Licence version 3 (LGPLv3)

# ToDo's
Configurable address range in which local redirection applies



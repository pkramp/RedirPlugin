# XRootDRedirPlugin 
An XRootD server(cms) plug-in to "redirect" XRootD calls to a locally mounted file system.
This only applies if both client and the normally used dataserver have private IP-Adresses/Hostnames
# Warning
The source tree includes private header files of the XrdCms source tree(namely XrdCmsFinder.hh) and needs to be kept in sync with the XRootD version source used!

# Plug-in configuration
Optional:
redirplugin.readonlyredirect true/false/True/False

# Install and tests
To compile the plug-in, you need to set the XRD_PATH environmental variable to the toplevel of your XRootD installation.

You can compile the plug-in library with :
```shell
make
```
# Potential improvements
Configurable ip-address range in which local redirection applies



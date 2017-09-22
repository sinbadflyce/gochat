ClearKeep - (Client on Windows of GoChat)
===================================================

https://github.com/sinbadflyce/gochat/tree/Windows/windows

I. Introduce
-----------------------

- ClearKeep is a client that based on Themis library(https://github.com/cossacklabs/themis).

II. How to build
-----------------------
To build project from source, the following tools are needed:

  * MS Visual Studio 2015
  
Note: Libraries need to include:

  * Boost.1.65.1.0
  * OpenSSL
  
  All have been included with NUGET, so use packages.config to get all needed src and lib.
{
	Boost will needed to get .lib files by run commands as guide: 
	"You add the Boost headers only NuGet package to your project.
	Depending on what are you using and build configuration, Boost may require additional libraries (lib files). In this case, you will receive a compilation/linking error.
	Depending on the compilation/linking error, you add to your project additional NuGet packages with precompiled Boost libraries from here
	I agree, the process is not obvious. To improve it, Boost should finalize their modularization first. I mean, when Boost libraries (headers and binaries) can be shipped separately.
	https://github.com/sergey-shandar/getboost/blob/master/releases/1.65.md#precompiled-libraries"
}

III. Test:
----------------------------
The folder "windows\ClearKeep\Themis\x64\Debug" includes executable files that had built for Windows 10.
Visual C++ Redistributable for Visual Studio 2015 will needed.
https://www.microsoft.com/en-us/download/details.aspx?id=48145


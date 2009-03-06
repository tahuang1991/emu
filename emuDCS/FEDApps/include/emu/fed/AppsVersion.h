/*****************************************************************************\
* $Id: AppsVersion.h,v 1.4 2009/03/05 22:04:09 paste Exp $
*
* $Log: AppsVersion.h,v $
* Revision 1.4  2009/03/05 22:04:09  paste
* * Fixed a minor bug involving DCC Expert commands
*
* Revision 1.1  2009/03/05 16:18:24  paste
* * Shuffled FEDCrate libraries to new locations
* * Updated libraries for XDAQ7
* * Added RPM building and installing
* * Various bug fixes
* * Added ForPageOne functionality to the Manager
*
\*****************************************************************************/

#ifndef __EMU_FED_APPSVERSION_H__
#define __EMU_FED_APPSVERSION_H__

#include "config/PackageInfo.h"

namespace emufedapps {
	const std::string package     = "emufedapps";
	const std::string versions    = "07.00.03";
	const std::string description = "Emu (CSC) FED crate XDAq applications";

	const std::string summary     = "A collection of XDAq applications for controlling end-cap muon CSC FED VME hardware.";
	const std::string authors     = "Phillip Killewald, Stan Durkin, Jason Gilmore, Jianhui Gu";
	const std::string link        = "http://www.physics.ohio-state.edu/~cms";

	config::PackageInfo getPackageInfo();
	
	void checkPackageDependencies()
	throw (config::PackageInfo::VersionException);
	
	std::set<std::string, std::less<std::string> > getPackageDependencies();
}

#endif 

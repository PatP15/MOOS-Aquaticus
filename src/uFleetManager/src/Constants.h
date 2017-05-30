/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: Constants.h 	                             */
/*    DESC: Constants definitions for the MOOS Fleet Manager 		*/
/*    DATE: May 10th 2017                                       */
/*****************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace Status {
	// Good
	static std::string GREAT = "GREAT";
	static std::string GOOD = "OK"; // can't be OK, conflicts with ncurses macro

	// No expected result
	static std::string NODATA = "-";

	// Bad
	static std::string NOCONN = "!";
	static std::string NOPUB = "NOPUB";

	// Very bad (nonsensical)
	static std::string ERROR = "ERR";

	// Test not applicable
	static std::string NOTAPPLIC = "NA";
	static std::string ISLOCAL = "LOC";

	static std::string NOIMPL = "TODO";
}

#endif
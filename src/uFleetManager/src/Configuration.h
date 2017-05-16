/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: Configuration.h 																		 */
/*    DATE: March 10th 2017                                   	 */
/*****************************************************************/

#ifndef DEPLOYMENTMANAGERCONFIGURATOR
#define DEPLOYMENTMANAGERCONFIGURATOR

#include <string>
#include <vector>
#include <map>
#include "ManagedMOOSMachine.h"

class Configuration {
public: // getters and setter
	// set once
	Configuration(int, char*[]);
	// copy constructor, DO NOT USE
	Configuration() {};
	// getters
	std::vector<ManagedMoosMachine> getMachines();
	bool hasParameter(std::string);
	std::string lookup(std::string);
	bool m_filter_by_liveness;
protected: // helper methods
	std::map<std::string, ManagedMoosMachine> allDefaultMachines();
private: // member variables
	std::vector<ManagedMoosMachine> m_machines;
	std::map<std::string, std::string> m_other_configs;
};



#endif
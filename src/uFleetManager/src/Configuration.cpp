/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: Configuration.cpp 	                     					   */
/*    DESC: Configuration of the MOOS Deployment Manager		 		 */
/*    DATE: March 10th 2017                                      */
/*****************************************************************/

#include "Configuration.h"
#include <regex>

using namespace std;


//--------------------------------------------------------------------
// Procedure: allDefaultMachines()
//   Purpose:	generate the default machines so that the ones being used can be
//						chosen
//   Returns: map of machines by their name
//      Note: capitalization of normal machines: Evan, Felix, ...
//						capitalization of unique machines: shore, master, ...

map<string, ManagedMoosMachine> Configuration::allDefaultMachines()
{
	map<string, ManagedMoosMachine> default_machines;

	default_machines["Evan"] = ManagedMoosMachine("Evan", "192.168.5.100");
	default_machines["Felix"] = ManagedMoosMachine("Felix", "192.168.6.100");
	default_machines["Gus"] = ManagedMoosMachine("Gus", "192.168.7.100");
	default_machines["Hal"] = ManagedMoosMachine("Hal", "192.168.8.100");
	default_machines["Ida"] = ManagedMoosMachine("Ida", "192.168.9.100");
	default_machines["Jing"] = ManagedMoosMachine("Jing", "192.168.10.100");
	default_machines["Kirk"] = ManagedMoosMachine("Kirk", "192.168.11.100");
	default_machines["manual"] = ManagedMoosMachine("Manual", "192.168.2.30");

	string gen_m200_loc = "~/moos-ivp-aquaticus/missions/aquaticus1.1/m200";
	string gen_m200_launch_file = "launch_m200.sh";
	string gen_m200_args = "-s";

	map<string, ManagedMoosMachine>::iterator m;
	for(m=default_machines.begin(); m!=default_machines.end(); m++) {
		m->second.setUsername("student2680");
		m->second.setTargetScripts(gen_m200_loc,
														   gen_m200_launch_file,
														   gen_m200_args);
	}

	// a local shoreside
	default_machines["shore"] = ManagedMoosMachine("Local", "localhost");
	default_machines["shore"].setTargetScripts(
		"~/moos-ivp-aquaticus/trunk/missions/aquaticus1.1/shoreside",
		"launch_shoreside.sh",
		"");

	// pablo-master, which lives in Mike's office in CSAIL
	// for testing only
	default_machines["master"] = ManagedMoosMachine(
		"Master", // TODO - fix this evil hack
		"pablo-master.csail.mit.edu");
	default_machines["master"].setUsername("student2680");
	default_machines["master"].setTargetScripts(gen_m200_loc,
												  										gen_m200_launch_file,
								   				  									gen_m200_args);

	return(default_machines);
}

//--------------------------------------------------------------------
// Procedure: Configuration()
//   Purpose:
//   Returns:
//      Note: Arguments assumed to be passed from main(). Configs only saved if
//			  they have a value, so a lookup on FOOBAR="" that searches for
//			  FOOBAR should give {""}, but a lookup of the nonexistant parameter
//			  BARFOO should give {}.

Configuration::Configuration(int argc, char* argv[]) {
	map<string, ManagedMoosMachine> default_machines = allDefaultMachines();

	m_other_configs["mailbox_dir"] = "/tmp";
	vector<string> machine_names;
	m_filter_by_liveness = false;

	for(int i=1; i<argc; i++) {
		string s = string(argv[i]);
		// TODO: tolower is not unicode compliant
		transform(s.begin(), s.end(), s.begin(), ::tolower);
		if (s=="--all") {
			map<string, ManagedMoosMachine>::iterator i;
			for(i=default_machines.begin(); i!=default_machines.end(); i++) {
				string name = i->first;
				if ((name!="master")&&(name!="manual")) machine_names.push_back(name);
			}
			break;
		}
		else if (s=="--up") {
			m_filter_by_liveness = true;
			map<string, ManagedMoosMachine>::iterator i;
			for(i=default_machines.begin(); i!=default_machines.end(); i++) {
				string name = i->first;
				if ((name!="master")&&(name!="manual")) machine_names.push_back(name);
			}
			break;
		}
		else if ((s=="-e")||(s=="--evan")) {
			machine_names.push_back("Evan");
		}
		else if ((s=="-f")||(s=="--felix")) {
			machine_names.push_back("Felix");
		}
		else if ((s=="-g")||(s=="--gus")) {
			machine_names.push_back("Gus");
		}
		else if ((s=="-h")||(s=="--hal")) {
			machine_names.push_back("Hal");
		}
		else if ((s=="-i")||(s=="--ida")) {
			machine_names.push_back("Ida");
		}
		else if ((s=="-j")||(s=="--jing")) {
			machine_names.push_back("Jing");
		}
		else if ((s=="-k")||(s=="--kirk")) {
			machine_names.push_back("Kirk");
		}
		else if (s=="--shore") {
			machine_names.push_back("shore");
		}
		else if ((s=="-pm")||(s=="--master")) {
			machine_names.push_back("master");
		}
		// TODO teams
		// TODO script locations?
		else {
			// TODO default behavior
			// crash?
			// pause?
		}
	}

	// if(machine_names.size()==0) {

	// 	machine_names.push_back("shore");
	// }
	machine_names.push_back("master");
	machine_names.push_back("manual");

	sort(machine_names.begin(), machine_names.end());

	vector<string>::iterator i;
	for(i=machine_names.begin(); i!=machine_names.end(); i++) {
		string name = *i;
		ManagedMoosMachine machine = default_machines[name];
		machine.setTeam("blue"); // TODO fix
		m_machines.push_back(machine);
	}
}

//--------------------------------------------------------------------
// Procedure: getMachines()
//   Purpose:
//   Returns:
//      Note:

vector<ManagedMoosMachine> Configuration::getMachines() {
	return(m_machines);
}

//--------------------------------------------------------------------
// Procedure: hasParamater()
//   Purpose: returns if lookup() would find config values for param
//   Returns:
//      Note: Has map.find()'s case sensitivity

bool Configuration::hasParameter(string param) {
	return(m_other_configs.find(param)!=m_other_configs.end());
}

//--------------------------------------------------------------------
// Procedure: lookup()
//   Purpose: Look up and return configs by their parameter name
//   Returns:
//      Note: Returns an empty list if no value is found

string Configuration::lookup(string param) {
	string config;
	map<string, string>::iterator i = m_other_configs.find(param);
	if (i != m_other_configs.end()) {
		config = i->second; // m_other_configs[param] has side effects
	}

	return(config);
}
/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: Configuration.cpp 	                     					   */
/*    DESC: Configuration of the MOOS Deployment Manager		 		 */
/*    DATE: March 10th 2017                                      */
/*****************************************************************/

#include "Configuration.h"
#include "MBUtils.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "utils.h"
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

	default_machines["evan"] = ManagedMoosMachine("Evan", "192.168.5.100");
	default_machines["felix"] = ManagedMoosMachine("Felix", "192.168.6.100");
	default_machines["gus"] = ManagedMoosMachine("Gus", "192.168.7.100");
	default_machines["hal"] = ManagedMoosMachine("Hal", "192.168.8.100");
	default_machines["ida"] = ManagedMoosMachine("Ida", "192.168.9.100");
	default_machines["jing"] = ManagedMoosMachine("Jing", "192.168.10.100");
	default_machines["kirk"] = ManagedMoosMachine("Kirk", "192.168.11.100");
	default_machines["mokai1"] = ManagedMoosMachine("Aqua1", "192.168.1.191");
	default_machines["mokai2"] = ManagedMoosMachine("Aqua2", "192.168.1.192");
	default_machines["master"] = ManagedMoosMachine(
		"Master",
		"pablo-master.csail.mit.edu");

	map<string, ManagedMoosMachine>::iterator m;
	for(m=default_machines.begin(); m!=default_machines.end(); m++) {
		m->second.setUsername("student2680");
	}
	default_machines["mokai1"].setUsername("student");
	default_machines["mokai2"].setUsername("student");

	// a local shoreside
	default_machines["shore"] = ManagedMoosMachine("Local", "localhost");

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
	bool loaded_config = false;

	for(int i=1; i<argc; i++) {
		string s = string(argv[i]);
		s = lowercase(s);

		if (s=="--file") {
			assert(i<argc-1); // need to actually specify the file!
			// const regex moos_files (".*\\.moos$");
			string filename = argv[++i];
			// printf("%d\t%d\n", regex_match(filename, moos_files), regex_match("foo", moos_files));

			// open the config file and read it in
			map<string, string> key_value_pairs;
			vector<string> lines;
			ifstream config_file(filename);
			if (config_file.is_open()) {
				string line;
				while (getline(config_file, line)) lines.push_back(line);
			}
			if (lines.size() > 0) loaded_config = true;

			vector<string>::iterator line;
			for(line=lines.begin(); line!=lines.end(); line++) {
				string key = biteStringX(*line, '=');
				string val = *line;
				if ((key!="")&&(val!="")) {
					if (key=="ProcessConfig") assert(val=="uFleetManager");
					else key_value_pairs[key] = val;
				}
			}

			string mission;
			if (key_value_pairs.find("mission")!=key_value_pairs.end()) {
				mission = key_value_pairs["mission"];
			}
			vector<string> machines;
			if (key_value_pairs.find("machines")!=key_value_pairs.end()) {
				machines = parseString(key_value_pairs["machines"], ' ');
			}
			vector<string>::iterator m_name;
			for(m_name=machines.begin(); m_name!=machines.end(); m_name++) {
				string name = *m_name;
				// name[0] = toupper(name[0]);
				machine_names.push_back(name);

				if (key_value_pairs.find(name)!=key_value_pairs.end()) {
					string launch, args;
					launch = tokStringParse(key_value_pairs[name], "launch", ',', ':');
					args = tokStringParse(key_value_pairs[name], "args", ',', ':');

					if (key_value_pairs.find("blue")!=key_value_pairs.end()) {
						vector<string> blue_team;
						blue_team = parseString(key_value_pairs["blue"], ' ');
						vector<string>::iterator i;
						for(i=blue_team.begin(); i!=blue_team.end(); i++) {
							if(name==*i) default_machines[name].setTeam("blue");
						}
					}
					if (key_value_pairs.find("red")!=key_value_pairs.end()) {
						vector<string> red_team;
						red_team = parseString(key_value_pairs["red"], ' ');
						vector<string>::iterator i;
						for(i=red_team.begin(); i!=red_team.end(); i++) {
							if(name==*i) default_machines[name].setTeam("red");
						}
					}

					default_machines[name].setTargetScripts(mission, launch, args);

				}
			}
			break;
		}

		// else if (s=="--all") {
		// 	map<string, ManagedMoosMachine>::iterator i;
		// 	for(i=default_machines.begin(); i!=default_machines.end(); i++) {
		// 		string name = i->first;
		// 		if ((name!="master")&&(name!="manual")) machine_names.push_back(name);
		// 	}
		// 	break;
		// }
		// else if (s=="--up") {
		// 	m_filter_by_liveness = true;
		// 	map<string, ManagedMoosMachine>::iterator i;
		// 	for(i=default_machines.begin(); i!=default_machines.end(); i++) {
		// 		string name = i->first;
		// 		if ((name!="master")&&(name!="manual")) machine_names.push_back(name);
		// 	}
		// 	break;
		// }
		else {
			printf("Parameter %s is not recognized!\n", s.c_str());
		}
	}

	// default behavior:
	// load everyone
	// make no attempt to set MOOS mission; this limits the MOOS commands that can
	// be run, but there are no "safe" default assumptions.
	if (!loaded_config) {
		map<string, ManagedMoosMachine>::iterator def;
		for(def=default_machines.begin(); def!=default_machines.end(); def++) {
			string name = def->first;
			machine_names.push_back(name);
		}
	}

	sort(machine_names.begin(), machine_names.end());

	vector<string>::iterator i;
	for(i=machine_names.begin(); i!=machine_names.end(); i++) {
		string name = *i;
		ManagedMoosMachine machine = default_machines[name];
		// machine.setTeam("blue"); // TODO fix
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
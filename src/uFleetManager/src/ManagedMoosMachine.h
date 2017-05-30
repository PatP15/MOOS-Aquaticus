/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: ManagedMoosMachine.h                                 */
/*    DATE: March 8th 2017                                       */
/*****************************************************************/

#ifndef MANAGEDMOOSMACHINE
#define MANAGEDMOOSMACHINE

#include <vector>
#include <string>
#include "utils.h"

class ManagedMoosMachine {

public: // Constructors and Destructors
		ManagedMoosMachine(std::string, std::string="");
		ManagedMoosMachine() {}; // implicit copy constructor.
		~ManagedMoosMachine() {};

public: // Public methods (getting statuses)
	// state-determining tests

	// back seat, the machine itself
	std::pair<std::string, std::string> dispatchPing(int=1);
	std::pair<std::string, std::string> dispatchSsh(int=3);
	// front seat, the vehicle
	std::pair<std::string, std::string> dispatchVehiclePing(int=1);
	std::pair<std::string, std::string> dispatchVehicleSsh(int=3);

	std::pair<std::string, std::string> dispatchMoosdbCheck();
	std::pair<std::string, std::string> dispatchCompassStatus();
	std::pair<std::string, std::string> dispatchGpsPdop();

	std::pair<std::string, std::string> dispatchAquaticusSvnRevision();
	std::pair<std::string, std::string> dispatchMoosIvpSvnRevision();
	std::pair<std::string, std::string> dispatchPabloSvnRevision();

	// move mail from files to variables
	void checkPingMail();
	void checkSshMail();
	void checkVehiclePingMail();
	void checkVehicleSshMail();
	void checkMoosdbMail();
	void checkCompassStatusMail();
	void checkGpsPdopStatusMail();

	void checkMoosIvpSvnRevisionMail();
	void checkAquaticusSvnRevisionMail();
	void checkPabloSvnRevisionMail();

	// get mail from variables
	std::string readPingMail();
	std::string readSshMail();
	std::string readVehiclePingMail();
	std::string readVehicleSshMail();
	std::string readMoosdbMail();
	std::string readCompassStatusMail();
	std::string readGpsPdopStatusMail();

	std::string readMoosIvpSvnRevisionMail();
	std::string readAquaticusSvnRevisionMail();
	std::string readPabloSvnRevisionMail();

public: // Public methods (sending commands)
	// deployment management
	std::pair<std::string, std::string> startMOOS(int=2);
	std::pair<std::string, std::string> stopMOOS();
	std::pair<std::string, std::string> restartMOOS(int=2);

	std::pair<std::string, std::string> restartHardware();
	std::pair<std::string, std::string> stopHardware();

	std::pair<std::string, std::string> stopMOOSCommunity() {return(std::make_pair("",""));};

	// restart hardware and MOOS
	// TODO is restartMachine() really a good name?
	std::pair<std::string, std::string> restartMachine() {return(std::make_pair("",""));};

public: // Public methods (local commands)
	std::pair<std::string, std::string> clearCache();

	// getters and setters
	std::string getFullAddress();
	std::string getIp();
	std::string getUsername();
	std::string getFrontSeatAddress();
	std::string getFrontSeatIp();
	std::string getFrontSeatUsername();
	std::string getName();
	std::string getTeam();
	std::string getId();

	void setUsername(std::string);
	void setIP(std::string);
	void setTargetScripts(std::string, std::string, std::string);
	void setTeam(std::string);

protected: // Helper methods
	bool targetIsLocal();
	std::vector<std::string> readServiceMailbox(std::string);
	index_t grabIndex(std::vector<std::string>&);
	bool receiveUpdate(StampedData&, index_t);
	std::string prepareUpdate(StampedData&);
	std::string serviceMailboxName(std::string);
	std::string sshTrustPrefix();

protected: // Variables
	std::string m_name;
	std::string m_id_number;
	std::string m_machine_username;
	std::string m_machine_ip_address;
	std::string m_front_seat_ip_address;
	std::string m_team;

	std::string m_target_script_dir;
	std::string m_target_script_name;
	std::string m_target_script_args;

	// buffering variables
	std::vector<std::string> m_mailboxes;

	StampedData m_ping_results;
	StampedData m_ssh_results;
	StampedData m_front_seat_ping_results;
	StampedData m_front_seat_ssh_results;
	StampedData m_moosdb_results;
	StampedData m_compass_results;
	StampedData m_gps_pdop_results;
	StampedData m_aquaticus_svn_revision_results;
	StampedData m_moos_ivp_svn_revision_results;
	StampedData m_pablo_svn_revision_results;
	StampedData m_restart_hardware_results;
	StampedData m_shutdown_hardware_results;
};

#endif
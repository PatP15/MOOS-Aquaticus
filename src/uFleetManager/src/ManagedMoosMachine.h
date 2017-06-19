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
#include <map>
#include "utils.h"

class ManagedMoosMachine {

public: // Constructors and Destructors
		ManagedMoosMachine(std::string, std::string="");
		ManagedMoosMachine() {}; // implicit copy constructor.
		~ManagedMoosMachine() {};

public: // Public methods (getting statuses)
	// state-determining tests

	// back seat, the machine itself
	CommandSummary dispatchPing(int=1);
	CommandSummary dispatchSsh(int=3);
	// front seat, the vehicle
	CommandSummary dispatchVehiclePing(int=1);
	CommandSummary dispatchVehicleSsh(int=3);

	CommandSummary dispatchMoosdbCheck();
	CommandSummary dispatchCompassStatus();
	CommandSummary dispatchGpsPdop();

	CommandSummary dispatchSvnRevisionCheck(std::string);
	// CommandSummary dispatchAquaticusSvnRevision();
	// CommandSummary dispatchMoosIvpSvnRevision();
	// CommandSummary dispatchPabloSvnRevision();

	// get mail, cache it, and return a status string
	std::string checkPingMail(bool=true);
	std::string checkSshMail(bool=true);
	std::string checkVehiclePingMail(bool=true);
	std::string checkVehicleSshMail(bool=true);
	std::string checkMoosdbMail(bool=true);
	std::string checkCompassStatusMail(bool=true);
	std::string checkGpsPdopStatusMail(bool=true);

	std::string checkSvnRevisionMail(std::string, bool=true);
	// std::string checkMoosIvpSvnRevisionMail();
	// std::string checkAquaticusSvnRevisionMail();
	// std::string checkPabloSvnRevisionMail();

public: // Public methods (sending commands)
	// deployment management
	CommandSummary startMOOS(int=2);
	CommandSummary stopMOOS();
	CommandSummary restartMOOS(int=2);

	CommandSummary reboot();
	CommandSummary shutdown();
	CommandSummary rebootVehicle();
	CommandSummary shutdownVehicle();

	CommandSummary stopMOOSCommunity() {return(std::make_pair("",""));};

	// restart hardware and MOOS
	// TODO is restartMachine() really a good name?
	CommandSummary restartMachine() {return(std::make_pair("",""));};

public: // Public methods (local commands)
	CommandSummary clearCache();

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
	std::string getFullMission();
	std::string getLaunchFile();

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
	CommandSummary _dispatchPavCmd(std::string,
																 std::string,
																 std::string,
																 bool,
																 bool,
																 bool,
																 std::string,
																 std::string="");
	std::string getFullMissionPath();

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
	std::map<std::string, std::string> m_mailboxes;
	std::map<std::string, StampedData> m_mail;

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
	StampedData m_colregs_svn_revision_results;
	StampedData m_restart_hardware_results;
	StampedData m_shutdown_hardware_results;
};

#endif
/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: ManagedMoosMachine.cpp 	                             */
/*    DESC: Class implementation of ManagedMoosMachine           */
/*					ManagedMoosMachine represents a computer, frequently */
/*					but not always a PABLO, running MOOS and managed by  */
/*					a delegator 																				 */
/*    DATE: March 8th 2017                                       */
/*****************************************************************/

#include "system_call.h"
#include "ManagedMoosMachine.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <unistd.h>
#include <cmath>
#include <regex>
#include "Constants.h"

using namespace std;

//--------------------------------------------------------------------
// Status Tests
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Procedure: dispatchPing()
//   Purpose: Test connection to the robot with ping
//   Returns: Command name and exact command
//      Note: ssh might succeed even if ping fails. "pinging" localhost
//			  instantly returns.

CommandSummary ManagedMoosMachine::dispatchPing(int t)
{
	string summary = m_name + "back seat ping";
	string command = Status::ISLOCAL;
	if (targetIsLocal()) return(make_pair(summary, command));

	if (!ready_to_dispatch(m_mail["bs_ping"].cache))
		return(make_pair(summary, Status::STALE));

	// ping flags; for more information, see the man page
	// -o  : return after the first successful packet is received
	// -t X: timeout after X seconds
	command = "ping -o -t " + to_string(t) + " " + m_machine_ip_address;
	string index = prepareUpdate(m_mail["bs_ping"].cache);

	system_call_dispatch_return(command, m_mail["bs_ping"].mailbox, index);

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchSsh()
//   Purpose: Test sending, remotely executing, and getting a result
//   Returns: Command name and exact command
//      Note: Immediately returns if target is localhost

CommandSummary ManagedMoosMachine::dispatchSsh(int t)
{
	string desc = "back seat ssh";
	string name = "bs_ssh";
	string cmd = "pav_test_ssh.sh";
	string return_type = "return";

	bool allowed_local = false;
	bool front_seat = false;
	bool clear = false;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}

//--------------------------------------------------------------------
// Procedure: dispatchVehiclePing()
//   Purpose: Test connection to the front seat of the robot with ping
//   Returns: Command name and exact command
//      Note: ssh might succeed even if ping fails. "pinging" localhost
//			  instantly returns.

CommandSummary ManagedMoosMachine::dispatchVehiclePing(int t)
{
	string summary = m_name + "front seat ping";
	string command = Status::NOTAPPLIC;
	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	if (!ready_to_dispatch(m_mail["fs_ping"].cache))
		return(make_pair(summary, Status::STALE));

	// ping flags; for more information, see the man page
	// -o  : return after the first successful packet is received
	// -t X: timeout after X seconds
	command = "ping -o -t " + to_string(t) + " " + m_front_seat_ip_address;

	string index = prepareUpdate(m_mail["fs_ping"].cache);

	system_call_dispatch_return(command, m_mail["fs_ping"].mailbox, index);

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchVehicleSsh()
//   Purpose: Test sending, remotely executing, and getting a result on the
//						front seat
//   Returns: Command name and exact command
//      Note: Immediately returns if target doesn't have a front seat

CommandSummary ManagedMoosMachine::dispatchVehicleSsh(int t)
{
	string desc = "front seat ssh";
	string name = "fs_ssh";
	string cmd = "exit 42";
	string return_type = "return";

	bool allowed_local = false;
	bool front_seat = true;
	bool clear = false;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}

//--------------------------------------------------------------------
// Procedure: dispatchMoosdbCheck()
//   Purpose: Test that the MOOSDB is up
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::dispatchMoosdbCheck()
{
	string desc = "moosdb";
	string name = "moosdb";
	string cmd = "pav_count_moosdb_running.sh";
	string return_type = "pipe";

	bool allowed_local = true;
	bool front_seat = false;
	bool clear = false;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}


//--------------------------------------------------------------------
// Procedure: dispatchSvnRevisionCheck()
//   Purpose: Get the SVN revision of the specified tree
//   Returns: Command name and exact command
//      Note: Intentionally does not parse the "Revision: " out of the line,
//			  		so that getAquaticusSvnRevisionMail() can tell the difference
//						between errors and revision numbers.
//
//						Only accepts "moos", "aqua", or "pablo".

CommandSummary ManagedMoosMachine::dispatchSvnRevisionCheck(string tree)
{
	string cmd;
	string desc = "svn rev";
	string return_type = "pipe";

	bool allowed_local = true;
	bool front_seat = false;
	bool clear = false;

	if (tree=="aqua") {
		cmd = "pav_get_svn_rev_aqua.sh 2>/dev/null";
		desc = "aquaticus " + desc;
	}
	else if (tree=="moos") {
		cmd = "pav_get_svn_rev_moos.sh 2>/dev/null";
		desc = "moos-ivp " + desc;
	}
	else if (tree=="pablo") {
		cmd = "pav_get_svn_rev_pablo.sh 2>/dev/null";
		desc = "pablo-common " + desc;
	}
	else if (tree=="colregs") {
		cmd = "pav_get_svn_rev_colregs.sh 2>/dev/null";
		desc = "colregs " + desc;
	}
	else if (tree=="mokai") {
		cmd = "pav_get_svn_rev_mokai.sh 2>/dev/null";
		desc = "mokai " + desc;
	}

	return(_dispatchPavCmd(tree, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}

//--------------------------------------------------------------------
// Procedure: dispatchCompassStatus()
//   Purpose: Test if the compass is on and broadcasting
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::dispatchCompassStatus()
{
	string desc = "compass";
	string name = "compass";

	string cmd = "pav_get_compass_heading.sh " + getFrontSeatAddress();

	string return_type = "pipe";

	bool allowed_local = false;
	bool front_seat = false;
	bool clear = false;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}

//--------------------------------------------------------------------
// Procedure: dispatchgps_pdop()
//   Purpose: Test if the GPS is on and publish its PDOP
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::dispatchGpsPdop()
{
	string desc = "gps_pdop";
	string name = "gps_pdop";

	string cmd = "pav_get_gps_pdop.sh " + getFrontSeatAddress();

	string return_type = "pipe";

	bool allowed_local = false;
	bool front_seat = false;
	bool clear = false;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}


//--------------------------------------------------------------------
// Procedure: checkPingMail()
//   Purpose: Perform the check of the ping mailbox
//   Returns:
//      Note: Sets m_ping

string ManagedMoosMachine::checkPingMail(bool check_cache)
{
	if (targetIsLocal()) {
		m_mail["bs_ping"].cache.data = Status::ISLOCAL;
		return(get_data_and_staleness(m_mail["bs_ping"].cache));
	}

	if (! check_cache) return(m_mail["bs_ping"].cache.data);

	vector<string> mail_list = readServiceMailbox(m_mail["bs_ping"].mailbox);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["bs_ping"].cache, index)) {
		if (mail_list.size() > 0) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_mail["bs_ping"].cache.data = Status::NODATA;
				return(get_data_and_staleness(m_mail["bs_ping"].cache));
			}
			try	{
				if (0==stoi(mail)) m_mail["bs_ping"].cache.data = Status::GOOD;
				else if (2==stoi(mail)) m_mail["bs_ping"].cache.data = Status::NOCONN;
				else m_mail["bs_ping"].cache.data = Status::ERROR;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["bs_ping"].cache.data = Status::ERROR; // error
			}
		}
		else m_mail["bs_ping"].cache.data = Status::NODATA; // No data yet
	}

	return(get_data_and_staleness(m_mail["bs_ping"].cache));
}

//--------------------------------------------------------------------
// Procedure: checkSshMail()
//   Purpose: Perform the check of the ssh mailbox
//   Returns:
//      Note: Sets m_ssh

string ManagedMoosMachine::checkSshMail(bool check_cache)
{
	if (targetIsLocal()) {
		m_mail["bs_ssh"].cache.data = Status::ISLOCAL;
		return(get_data_and_staleness(m_mail["bs_ssh"].cache));
	}
	if (! check_cache) return(m_mail["bs_ssh"].cache.data);

	vector<string> mail_list = readServiceMailbox(m_mail["bs_ssh"].mailbox);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["bs_ssh"].cache, index)) {
		if (mail_list.size() > 0)
		{
			string mail = mail_list[0];
			if (mail.size()==0) {
				m_mail["bs_ssh"].cache.data = Status::NODATA;
				return(get_data_and_staleness(m_mail["bs_ssh"].cache));
			}
			try {
				// TODO: 42 is a magic number
				if (mail=="42") m_mail["bs_ssh"].cache.data = Status::GOOD;
				else m_mail["bs_ssh"].cache.data = Status::NOCONN;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["bs_ssh"].cache.data = Status::ERROR;
			}
		}
		else m_mail["bs_ssh"].cache.data = Status::NODATA; // no data yet
	}

	return(get_data_and_staleness(m_mail["bs_ssh"].cache));
}

//--------------------------------------------------------------------
// Procedure: checkPingMail()
//   Purpose: Perform the check of the front seat ping mailbox
//   Returns:
//      Note: Sets m_fs_ping

string ManagedMoosMachine::checkVehiclePingMail(bool check_cache)
{
	if (m_front_seat_ip_address=="") {
		m_mail["fs_ping"].cache.data = Status::NOTAPPLIC;
		return(get_data_and_staleness(m_mail["fs_ping"].cache));
	}
	if (! check_cache) return(m_mail["fs_ping"].cache.data);

	vector<string> mail_list = readServiceMailbox(m_mail["fs_ping"].mailbox);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["fs_ping"].cache, index)) {
		if (mail_list.size() > 0) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_mail["fs_ping"].cache.data = Status::NODATA;
				return(get_data_and_staleness(m_mail["fs_ping"].cache));
			}
			try	{
				if (0==stoi(mail))
					m_mail["fs_ping"].cache.data = Status::GOOD;
				else if (2==stoi(mail))
					m_mail["fs_ping"].cache.data = Status::NOCONN;
				else
					m_mail["fs_ping"].cache.data = Status::ERROR;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["fs_ping"].cache.data = Status::ERROR; // error
			}
		}
		else m_mail["fs_ping"].cache.data = Status::NODATA; // No data yet
	}
	return(get_data_and_staleness(m_mail["fs_ping"].cache));
}

//--------------------------------------------------------------------
// Procedure: checkVehicleSshMail()
//   Purpose: Perform the check of the front seat ssh mailbox
//   Returns:
//      Note: Sets m_fs_ssh

string ManagedMoosMachine::checkVehicleSshMail(bool check_cache)
{
	if (m_front_seat_ip_address=="") {
		m_mail["fs_ssh"].cache.data = Status::NOTAPPLIC;
		return(get_data_and_staleness(m_mail["fs_ssh"].cache));
	}
	if (! check_cache) return(m_mail["fs_ssh"].cache.data);

	vector<string> mail_list = readServiceMailbox(m_mail["fs_ssh"].mailbox);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["fs_ssh"].cache, index)) {
		if (mail_list.size() > 0)
		{
			string mail = mail_list[0];
			if (mail.size()==0) {
				m_mail["fs_ssh"].cache.data = Status::NODATA;
				return(get_data_and_staleness(m_mail["fs_ssh"].cache));
			}
			try {
				// TODO: 42 is a magic number
				if (mail=="42") m_mail["fs_ssh"].cache.data = Status::GOOD;
				else m_mail["fs_ssh"].cache.data = Status::NOCONN;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["fs_ssh"].cache.data = Status::ERROR;
			}
		}
		else m_mail["fs_ssh"].cache.data = Status::NODATA; // no data yet
	}
	return(get_data_and_staleness(m_mail["fs_ssh"].cache));
}

//--------------------------------------------------------------------
// Procedure: checkMoosdbMail()
//   Purpose: Perform the check of the MOOSDB mailbox
//   Returns:
//      Note: Sets m_moosdb

string ManagedMoosMachine::checkMoosdbMail(bool check_cache)
{
	if (! check_cache) return(m_mail["moosdb"].cache.data);

	vector<string> mail_list = readServiceMailbox(m_mail["moosdb"].mailbox);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["moosdb"].cache, index)) {
		if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) m_mail["moosdb"].cache.data = Status::NODATA;
			else m_mail["moosdb"].cache.data = Status::GOOD;
		}
		else if (mail_list.size()>0) m_mail["moosdb"].cache.data = "TOO MANY";
		else m_mail["moosdb"].cache.data = Status::NODATA;
	}
	return(get_data_and_staleness(m_mail["moosdb"].cache));
}


//--------------------------------------------------------------------
// Procedure: checkSvnRevisionMail()
//   Purpose: Perform the check of the svn mailbox
//   Returns:
//      Note:

string ManagedMoosMachine::checkSvnRevisionMail(string tree, bool check_cache)
{
	if (! check_cache) return(m_mail[tree].cache.data);

	vector<string> mail_list = readServiceMailbox(m_mail[tree].mailbox);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail[tree].cache, index)) {
		if (mail_list.size()==0) {
			m_mail[tree].cache.data = Status::NODATA;
		}
		else if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_mail[tree].cache.data = Status::NODATA;
				return(get_data_and_staleness(m_mail[tree].cache));
			}

			string delim = ": ";
			size_t loc = mail.find(delim);

			if (loc==string::npos) { // badly formated
				m_mail[tree].cache.data = Status::ERROR;
			}
			else {
				if (loc+delim.size()>mail.size()) {
					m_mail[tree].cache.data = Status::ERROR;
				}
				try {
					size_t point = loc + delim.size();
					m_mail[tree].cache.data = mail.substr(point);
				}
				catch (...) { // exception &e / type error only? / TODO?
					// badly formatted
					m_mail[tree].cache.data = Status::ERROR;
				}
			}
		}
		// very badly formatted
		else m_mail[tree].cache.data = Status::ERROR;
	}
	return(get_data_and_staleness(m_mail[tree].cache));
}

//--------------------------------------------------------------------
// Procedure: checkCompassStatusMail()
//   Purpose: Perform the check of the compass mailbox
//   Returns:
//      Note: Sets m_compass

string ManagedMoosMachine::checkCompassStatusMail(bool check_cache)
{
	string nopub = "WARNING: topic [/imu/compass_heading] does not appear to be published yet";
	string hsplit = "---";
	string delim = ": ";

	string mokai = "OS5000";

	if (! check_cache) return(m_mail["compass"].cache.data);

	vector<string> mail = readServiceMailbox(m_mail["compass"].mailbox);
	index_t index = grabIndex(mail);

	if (receiveUpdate(m_mail["compass"].cache, index)) {
		int expected_results = 5;
		int observed_results = 0;

		if (mail.size()==0) {
			m_mail["compass"].cache.data = Status::NODATA; // no data yet
		}
		else if (mail[0]==nopub) { // not publishing; likely hardware related
			m_mail["compass"].cache.data = Status::NOPUB;
		}
		else if (mail[0]==mokai) {
			m_mail["compass"].cache.data = Status::GOOD;
		}
		else {
			//expected format is repetitions of
			//data: FLOAT
			//---
			//but if the FLOAT is NaN, regard that as a hardware error
			const regex hline_pattern ("^---$");
			const regex nan_pattern ("^data: nan$");
			const regex data_pattern ("^data: -?\\d{1,3}\\.\\d*$");

			vector<string>::iterator i;
			for (i=mail.begin(); i!=mail.end(); i++) {
				string line = *i;
				if (line.size()==0) continue;
				else if (regex_match(line, hline_pattern)) continue;
				else if (regex_match(line, nan_pattern)) {
					m_mail["compass"].cache.data = "NaN";
					return(get_data_and_staleness(m_mail["compass"].cache));
				}
				else if (regex_match(line, data_pattern)) observed_results++;
				else {
					m_mail["compass"].cache.data = Status::ERROR; // badly formatted
					return(get_data_and_staleness(m_mail["compass"].cache));
				}
			}
			if (observed_results<expected_results) {
				m_mail["compass"].cache.data = Status::NODATA;
			}
			else if (observed_results==expected_results) {
				m_mail["compass"].cache.data = Status::GOOD;
			}
			else {
				m_mail["compass"].cache.data = Status::ERROR;
			}
		}
	}
	return(get_data_and_staleness(m_mail["compass"].cache));
}

//--------------------------------------------------------------------
// Procedure: checkgps_pdopStatusMail()
//   Purpose: Perform the check of the compass mailbox
//   Returns:
//      Note: Sets m_compass

string ManagedMoosMachine::checkGpsPdopStatusMail(bool check_cache)
{
	string nopub = "WARNING: topic [/navsat/rx] does not appear to be published yet";
	string hsplit = "---";
	string delim = ": ";

	string mokai = "GPSUBLOX";
	int GPGSA_PDOP_index = 15; // per GPS spec

	if (! check_cache) return(m_mail["gps_pdop"].cache.data);


	vector<string> mail = readServiceMailbox(m_mail["gps_pdop"].mailbox);
	index_t index = grabIndex(mail);

	if (receiveUpdate(m_mail["gps_pdop"].cache, index)) {
		if (mail.size()==0) {
			m_mail["gps_pdop"].cache.data = Status::NODATA; // no data yet
		}
		else if (mail[0]==nopub) { // not publishing; likely hardware related
			m_mail["gps_pdop"].cache.data = Status::NOPUB;
		}
		else if (mail[0]==mokai) {
			m_mail["gps_pdop"].cache.data = Status::GOOD;
		}
		else {
			// no messages seen
			m_mail["gps_pdop"].cache.data = Status::NODATA;

			// look for the GPGSA message, and return its PDOP
			const regex gpgsa_pattern ("^.*GPGSA.*$");
			const regex msg_pattern ("^.*GP.*$");

			vector<string>::iterator i;
			for (i=mail.begin(); i!=mail.end(); i++) {
				string line = *i;
				if (line.size()==0) continue;
				else if (regex_match(line, gpgsa_pattern)) {
					stringstream parse (line);
					vector<string> gpgsa_msg;
					string s;
					while (parse.good()) {
						getline(parse, s, ',');
						gpgsa_msg.push_back(s);
					}

					if (gpgsa_msg.size()<GPGSA_PDOP_index+1) {
						m_mail["gps_pdop"].cache.data = Status::ERROR;
					}
					else {
						m_mail["gps_pdop"].cache.data = gpgsa_msg[GPGSA_PDOP_index];
					}
					break;
				}
				else if ((regex_match(line, msg_pattern))&&
								 (m_mail["gps_pdop"].cache.data==Status::NODATA)) {
					// seen some messages; if no GPGSA messages have been seen, this will
					// return error, otherwise it will be overwritten and returned
					m_mail["gps_pdop"].cache.data = Status::ERROR;
				}
			}
		}
	}
	return(get_data_and_staleness(m_mail["gps_pdop"].cache));
}

//--------------------------------------------------------------------
// Management actions
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Procedure: startMOOS()
//   Purpose: Run a MOOS instance
//   Returns: Command name and exact command
//      Note: Doesn't check any dependencies, such as pinging or ssh testing.

CommandSummary ManagedMoosMachine::startMOOS(int t)
{
	string desc = "start MOOS";
	string name = "start_moos";
	string cmd = "cd " + m_target_script_dir + "\nbash " \
						 + m_target_script_name + " " + m_target_script_args;
	string return_type = "pipe";

	bool allowed_local = true;
	bool front_seat = false;
	bool clear = false;

	return(_dispatchPavCmd(name, cmd, desc,
											 allowed_local, front_seat, clear,
											 return_type));
}

//--------------------------------------------------------------------
// Procedure: stopMOOS()
//   Purpose: Stops MOOS on the machine, without shutting down the machine
//   Returns: Command name and exact command
//      Note: A thin wrapper around ktm

CommandSummary ManagedMoosMachine::stopMOOS()
{
	string desc = "stop MOOS";
	string name = "stop_moos";
	string cmd = "pav_stop_moos.sh";
	string return_type = "return";

	bool allowed_local = true;
	bool front_seat = false;
	bool clear = false;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}

//--------------------------------------------------------------------
// Procedure: restartMOOS()
//   Purpose: Stops running MOOS and restarts it
//   Returns: Command name and exact command
//      Note: Thin, passes arguments through. Same verbosity for starting and
// 			  stopping MOOS

CommandSummary ManagedMoosMachine::restartMOOS(int t)
{
	string summary = m_name + " restart MOOS";
	CommandSummary stop_res = stopMOOS();
	CommandSummary start_res = startMOOS(t);

	return(make_pair(summary, stop_res.second + "\n\n" + start_res.second));
}

//--------------------------------------------------------------------
// Procedure: reboot()
//   Purpose: Reboot the designated machine
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::reboot()
{
	string desc = "reboot back seat";
	string name = "reboot";
	string cmd = "pav_reboot_computer.sh";
	string return_type = "return";

	bool allowed_local = false;
	bool front_seat = false;
	bool clear = true;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}

//--------------------------------------------------------------------
// Procedure: shutdownHardware()
//   Purpose: Shutdown the designated machine
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::shutdown() {
	string desc = "shutdown back seat";
	string name = "shutdown";
	string cmd = "pav_shutdown_computer.sh";
	string return_type = "return";

	bool allowed_local = false;
	bool front_seat = false;
	bool clear = true;

	return(_dispatchPavCmd(name, cmd, desc,
												 allowed_local, front_seat, clear,
												 return_type));
}

//--------------------------------------------------------------------
// Procedure: rebootVehicle()
//   Purpose: Reboot the (front seat) machine
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::rebootVehicle()
{
	string summary = m_name + " reboot front seat";
	string command = Status::NOTAPPLIC;
	string mailbox = m_mail["reboot_vehicle"].mailbox;

	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	command = sshTrustPrefix() + getFrontSeatAddress() + " -t " + \
					+ " \"source ~/.profile; sudo reboot\"";

	string index = prepareUpdate(m_mail["reboot"].cache);

	system_call_dispatch_pipe(command, mailbox, index);

	clearCache();
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: shutdownVehicle()
//   Purpose: Shutdown the designated machine
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::shutdownVehicle() {
	string summary = m_name + " shutdown front seat";
	string command = Status::NOTAPPLIC;
	string mailbox = m_mail["shutdown_vehicle"].mailbox;

	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	command = sshTrustPrefix() + getFrontSeatAddress() + " -t " + \
					+ " \"source ~/.profile; sudo shutdown now\"";

	string index = prepareUpdate(m_mail["shutdown_vehicle"].cache);

	system_call_dispatch_pipe(command, mailbox, index);

	clearCache();
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: _dispatchPavCmd()
//   Purpose: Remotely call generic pavlab commands
// Arguments: name  						: the name of the function, as used to look up
//																its variables
//						cmd   						: the full name of the function or alias, for
//																running remotely
//						desc  						: a human readable description of cmd
//						allowed_local 		: this function may run on the user's own
//																computer
//						runs_on_fron_seat : this function is sent to a secondary address,
//																e.g. the M300 front seat
//						clear 						: clear the cache after running this function
//						args  						: any extra arguments that cmd needs
//						clear 						: if true, clear the cache after sending cmd
//																(e.g. reboot)
//      Note:

CommandSummary ManagedMoosMachine::_dispatchPavCmd(string name,
																										string cmd,
																										string desc,
																									  bool allowed_local,
																									  bool runs_on_front_seat,
																									  bool clear,
																									  string return_type,
																									  string args)
{
	string summary = m_name + " " + desc;
	string command = Status::NOTAPPLIC;

	//--------------------------------------------------------------------
	// Don't spam; wait until the old message is received
	//--------------------------------------------------------------------
	if (!ready_to_dispatch(m_mail[name].cache))
		return(make_pair(summary, Status::STALE));

	//--------------------------------------------------------------------
	// Handle the case of running commands against your own machine
	//--------------------------------------------------------------------
	if (targetIsLocal()) {
		if (allowed_local) {
			command = "source ~/.profile\n" + cmd + " " + args;
		}
		else {
			return(make_pair(summary, Status::ISLOCAL));
		}
	}
	//--------------------------------------------------------------------
	// Handle the case of running commands against other machines
	//--------------------------------------------------------------------
	else {
		//--------------------------------------------------------------------
		// Handle a secondary computer, e.g. the M300 Heron front seat
		//--------------------------------------------------------------------
		if (runs_on_front_seat) {
			if (getFrontSeatAddress()=="")
				return(make_pair(summary, Status::NOTFOUND));
			else command = sshTrustPrefix() + getFrontSeatAddress();
		}
		//--------------------------------------------------------------------
		// The "normal" case
		//--------------------------------------------------------------------
		else {
			if (getFullAddress()=="") return(make_pair(summary, Status::NOTFOUND));
			else command = sshTrustPrefix() + getFullAddress();
		}

		command += " -o ConnectTimeout=3 \"source ~/.profile\n" \
						 + cmd + " " + args + "\"";
	}
	string index = prepareUpdate(m_mail[name].cache);

	//--------------------------------------------------------------------
	// Select the correct return type; a piped string or a returned integer
	//--------------------------------------------------------------------
	if (return_type=="pipe")
		system_call_dispatch_pipe(command, m_mail[name].mailbox, index);
	else if (return_type=="return")
		system_call_dispatch_return(command, m_mail[name].mailbox, index);

	//--------------------------------------------------------------------
	// Clear the cache for commands that reset the state (e.g. reboots and
	// shutdowns)
	//--------------------------------------------------------------------
	if(clear) clearCache();

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Getters and Setters
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Procedure: getFullAddress()
//   Purpose: Returns "localhost" or "USER@IP"
//   Returns:
//      Note:

string ManagedMoosMachine::getFullAddress()
{
	if (targetIsLocal()) return("localhost");
	else return(m_machine_username + "@" + m_machine_ip_address);
}

//--------------------------------------------------------------------
// Procedure: getIp()

string ManagedMoosMachine::getIp()
{
	return(m_machine_ip_address);
}

//--------------------------------------------------------------------
// Procedure: getUsername()

string ManagedMoosMachine::getUsername()
{
	return(m_machine_username);
}

//--------------------------------------------------------------------
// Procedure: getFrontSeatAddress()
//   Purpose: Returns "" or "student@IP"
//   Returns:
//      Note:

string ManagedMoosMachine::getFrontSeatAddress()
{
	if (m_front_seat_ip_address=="") return("");
	else return("student@" + m_front_seat_ip_address);
}

//--------------------------------------------------------------------
// Procedure: getFrontSeatIp()

string ManagedMoosMachine::getFrontSeatIp()
{
	return(m_front_seat_ip_address);
}

//--------------------------------------------------------------------
// Procedure: getFrontSeatUsername()

string ManagedMoosMachine::getFrontSeatUsername()
{
	if (m_front_seat_ip_address=="") return("");
	else return("student");
}

//--------------------------------------------------------------------
// Procedure: getName()

string ManagedMoosMachine::getName()
{
	return m_name;
}

//--------------------------------------------------------------------
// Procedure: getTeam()

string ManagedMoosMachine::getTeam()
{
	return m_team;
}

//--------------------------------------------------------------------
// Procedure: getId()

string ManagedMoosMachine::getId()
{
	string id = "";
	const regex pavlab_m300_back_seats ("^192\\.168\\.\\d{1,3}\\.100$");
	const regex pavlab_mokai_latops ("^192\\.168\\.1\\.19\\d$");
	if (regex_match(m_machine_ip_address, pavlab_m300_back_seats)) {
		stringstream ip_stream (m_machine_ip_address);

		vector<string> ip_parts;
		string s;
		while (ip_stream.good()) {
			getline(ip_stream, s, '.');
			ip_parts.push_back(s);
		}
		id = ip_parts[2]; // 192.168.VEH_ID.100
	}
	else if (regex_match(m_machine_ip_address, pavlab_mokai_latops)) {
		int size = m_machine_ip_address.size();
		string last_char = m_machine_ip_address.substr(size - 1); // 192.168.1.19i
		id = "19" + last_char;
	}
	return(id);
}


//--------------------------------------------------------------------
// Procedure: getFullMission()
//   Purpose: return full command as dispatched by startMOOS

string ManagedMoosMachine::getFullMission()
{
	if ((getFullMissionPath()!="")&&(m_target_script_args!="")) {
		return(getFullMissionPath() + " " + m_target_script_args);
	}
	else return("");
}


//--------------------------------------------------------------------
// Procedure: getLaunchFile()
//   Purpose: returns launch file name, without the full path
//			Note: a more terse companion to getFullMission()
string ManagedMoosMachine::getLaunchFile()
{
	if ((getFullMissionPath()!="")&&(m_target_script_args!="")) {
		return(m_target_script_name + " " + m_target_script_args);
	}
	else return("");
}


//--------------------------------------------------------------------
// Procedure: setUsername()

void ManagedMoosMachine::setUsername(string name)
{
	m_machine_username = name;
}

//--------------------------------------------------------------------
// Procedure: setIP()
// 			Note: lowercases the address

void ManagedMoosMachine::setIP(string addr)
{
	// strip and lowercase addr
	// assuming a well-formatted IPv4 or IPv6 address, this shouldn't have an
	// effect. However, if the input is "Localhost " we should be able to match
	// it against "localhost"

	// TODO: strip whitespace. MOOS String parsing?

	// TODO: tolower is not unicode compliant
	std::transform(addr.begin(), addr.end(), addr.begin(), ::tolower);
	m_machine_ip_address = addr;

	// if we're a PABLO, set our front seat's addr
	m_front_seat_ip_address="";
	const regex back_seat_addrs ("^192\\.168\\.\\d{1,3}\\.100$");
	if (regex_match(m_machine_ip_address, back_seat_addrs)) {
		// to got from ...100 to ...1, just pop the last two characters
		m_front_seat_ip_address = string(m_machine_ip_address); // copy
		m_front_seat_ip_address.pop_back();
		m_front_seat_ip_address.pop_back();
	}
}

//--------------------------------------------------------------------
// Procedure: setTargetScript()
// 	 Purpose: set the exact script to run MOOS with on this machine

void ManagedMoosMachine::setTargetScripts(string dir,
										  string script_name,
										  string args="")
{
	m_target_script_dir = dir;
	m_target_script_name = script_name;
	m_target_script_args = args;

	if (! targetIsLocal()) {
		if (m_team!="") {
			m_target_script_args += " --" + m_team;
		}

		// add vehicle-based arguments
		if (getId()!="") {
			char vehicle_letter = tolower(m_name[0]);
			// most vehicle letters are lowercase. Jing's, however, is uppercase
			// because -j was already claimed by the --just_build flag
			if (vehicle_letter == 'j') vehicle_letter = 'J';
			m_target_script_args += " -";
			m_target_script_args += vehicle_letter;
		}
	}
}

//--------------------------------------------------------------------
// Procedure: setTeam()

void ManagedMoosMachine::setTeam(string team)
{
	m_team = team;
	if (m_target_script_args!="") {
		m_target_script_args += " --" + m_team;
	}
}

//--------------------------------------------------------------------
// Procedure: clearCache()
//   Purpose: Clear all cache variables
//   Returns: Command name and exact command
//      Note: Clears both the C++ variable buffer and the UNIX file buffer

CommandSummary ManagedMoosMachine::clearCache()
{
	string summary = m_name + " clear cache";

	map<string, Mail>::iterator mail;
	for(mail=m_mail.begin(); mail!=m_mail.end(); mail++) {
		clear_stamped_data(mail->second.cache);
		system_call("echo \"\" > " + mail->second.mailbox);
	}
	return(make_pair(summary, Status::NOTAPPLIC));
}

//--------------------------------------------------------------------
// Procedure: ManagedMoosMachine()
//   Purpose:
//   Returns:
//      Note: Touch each mailbox to make sure they exist

ManagedMoosMachine::ManagedMoosMachine(string name, string ip)
{
	m_name=name;
	if(ip!="") setIP(ip);

	m_mail["bs_ping"].mailbox = serviceMailboxName("backseat_ping");
	m_mail["bs_ssh"].mailbox = serviceMailboxName("backseat_ssh");
	m_mail["fs_ping"].mailbox = serviceMailboxName("frontseat_ping");
	m_mail["fs_ssh"].mailbox = serviceMailboxName("frontseat_ssh");
	m_mail["moosdb"].mailbox = serviceMailboxName("moosdb");
	m_mail["compass"].mailbox = serviceMailboxName("compass");
	m_mail["gps_pdop"].mailbox = serviceMailboxName("gpsPdop");
	m_mail["start_moos"].mailbox = serviceMailboxName("startMOOS");
	m_mail["stop_moos"].mailbox = serviceMailboxName("stopMOOS");
	m_mail["aqua"].mailbox = serviceMailboxName("aquaSvnRev");
	m_mail["moos"].mailbox = serviceMailboxName("moosSvnRev");
	m_mail["pablo"].mailbox = serviceMailboxName("pabloSvnRev");
	m_mail["colregs"].mailbox = serviceMailboxName("colregsSvnRev");
	m_mail["mokai"].mailbox = serviceMailboxName("mokaiSvnRev");
	m_mail["reboot"].mailbox = serviceMailboxName("reboot");
	m_mail["shutdown"].mailbox = serviceMailboxName("shutdown");
	m_mail["reboot_vehicle"].mailbox = serviceMailboxName("rebootVehicle");
	m_mail["shutdown_vehicle"].mailbox = serviceMailboxName("shutdownVehicle");

	StampedData blank;
	map<string, Mail>::iterator m;
	for(m=m_mail.begin(); m!=m_mail.end(); m++) {
		m->second.cache = blank;
		system_call("touch " + m->second.mailbox);
	}

	clearCache();
}

//--------------------------------------------------------------------
// Helper methods
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Procedure: targetIsLocal()
//   Purpose: Returns whether the robot's IP points to the local machine.
//   Returns:
//      Note: Accepts capitalizations of 'localhost', 127.0.0.1, ::1
//			TODO: accept the manager's own IP

bool ManagedMoosMachine::targetIsLocal()
{
	vector<string> local_aliases;
	local_aliases.push_back("localhost"); // literal
	local_aliases.push_back("127.0.0.1"); // IPv4
	local_aliases.push_back("::1"); // IPv6

	// TODO local_aliases.push_back(""); // effective IP (as found in ifconfig)

	vector<string>::iterator alias;
	for(alias=local_aliases.begin(); alias!=local_aliases.end(); alias++) {
		if (! alias->compare(m_machine_ip_address)) return(true);
	}
	return(false);
}

//--------------------------------------------------------------------
// Procedure: readServiceMailbox()
//   Purpose: Read the file buffer
//   Returns: A vector strings, each string is one line of the file
//      Note: Lines may be empty

vector<string> ManagedMoosMachine::readServiceMailbox(string service)
{
	vector<string> lines;

	ifstream mailbox(service);

	if (mailbox.is_open()) {
		string line;

		while (getline(mailbox, line)) lines.push_back(line);
	}

	return(lines);
}

//--------------------------------------------------------------------
// Procedure: grabIndex()
//   Purpose: Get the message index number
//   Returns:
//      Note: Removes the index line.

index_t ManagedMoosMachine::grabIndex(vector<string> & lines)
{
	index_t index = -1;
	// fairly specific; collisions unlikely
	const regex index_pattern ("MOOS_MANAGER_MESSAGE_INDEX:\\d+");
	for(vector<string>::iterator l=lines.begin(); l!=lines.end(); l++) {
		string index_line = *l;
		if (regex_match(index_line, index_pattern)) {
			stringstream linestream (index_line);
			string s;
			while (linestream.good()) {
				getline(linestream, s, ':');
			}
			index = stoi(s);
			lines.erase(l);
			break;
		}
	}
	return(index);
}

//--------------------------------------------------------------------
// Procedure: receiveUpdate()
//   Purpose: Determine if mail is a new update, and update the cache if needed
//   Returns: Whether mail is new
//      Note: Updates received time if mail is new

bool ManagedMoosMachine::receiveUpdate(StampedData & s, index_t i)
{
	if (i==s.i+1) {
		// update index
		s.i = i;
		// update timestamp
		s.received = time(0);
		return(true);
	}
	else return(false);
}

//--------------------------------------------------------------------
// Procedure: prepareUpdate()
//   Purpose: Update cache, and prepare index string
//   Returns: The string to write to the cache file
//      Note: Updates sent time

string ManagedMoosMachine::prepareUpdate(StampedData & s)
{
	// update timestamp
	s.sent = time(0);
	// prepare writeback string
	string tag = "MOOS_MANAGER_MESSAGE_INDEX:";
	string index = to_string(s.i+1);
	return(tag + index);
}

//--------------------------------------------------------------------
// Procedure: serviceMailboxName()
//   Purpose:
//   Returns: The fully qualified name of this machine's mailbox for a given
//						service
//      Note:

string ManagedMoosMachine::serviceMailboxName(string service)
{
	return("/tmp/MOOSMAIL/" + m_name + "_" + service + ".mailbox");
}


//--------------------------------------------------------------------
// Procedure: sshTrustPrefix()
//   Purpose: Use appropriate levels of ssh security based on where the machine
//						is. Effectively, trust machines on the local network.
//   Returns:
//      Note: Be paranoid with this function. Add IPs to match against VERY
//						CONSERVATIVELY
//
// 			What: The "trusting" version ignores the old ssh host key and doesn't
// 						store a new one. This prevents warnings about a potential
//						man-in-the-middle attack every time we reflash a PABLO. However,
//						it actually does expose us to those attacks, so only use it when
//						you're confident that you're safe.

string ManagedMoosMachine::sshTrustPrefix() {
	vector<regex> valid_ip_patterns;
	// front seats
	const regex pavlab_front_seats ("^192\\.168\\.\\d{1,3}\\.1$");
	// back seats
	const regex pavlab_back_seats ("^192\\.168\\.\\d{1,3}\\.100$");
	// PABLOs plugged directly into computer
	const regex pavlab_direct_to_pablos ("^192\\.168\\.2\\.\\d{1,3}$");
	// PABLO Master
	const regex pavlab_pablo_master ("^128\\.30\\.28\\.55$");
	// Mokai laptops
	const regex pavlab_mokai_computers ("^192\\.168\\.1\\.19\\d$");

	valid_ip_patterns.push_back(pavlab_front_seats);
	valid_ip_patterns.push_back(pavlab_back_seats);
	valid_ip_patterns.push_back(pavlab_direct_to_pablos);
	valid_ip_patterns.push_back(pavlab_pablo_master);
	valid_ip_patterns.push_back(pavlab_mokai_computers);

	string trusting_ssh = string("ssh -q") \
						 					+ string(" -o UserKnownHostsFile=/dev/null") \
						 					+ string(" -o StrictHostKeyChecking=no ");
	string unstrusting_ssh = "ssh ";

	vector<regex>::iterator i;
	for(i=valid_ip_patterns.begin(); i!=valid_ip_patterns.end(); i++) {
		const regex pattern = *i;
		if (regex_match(m_machine_ip_address, pattern)) return(trusting_ssh);
	}
	// IP isn't in the safe set, use the normal ssh
	return(unstrusting_ssh);
}


//--------------------------------------------------------------------
// Procedure: getFullMissionPath()

string ManagedMoosMachine::getFullMissionPath()
{
	return(m_target_script_dir + "/" + m_target_script_name);
}

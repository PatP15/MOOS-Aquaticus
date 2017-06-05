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

#define ROS_SOURCE "source /etc/ros/setup.bash;"
#define BASH_SOURCE "source ~/.bashrc"

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

	if (!ready_to_dispatch(m_mail["ping"]))
		return(make_pair(summary, Status::STALE));

	// ping flags; for more information, see the man page
	// -o  : return after the first successful packet is received
	// -t X: timeout after X seconds
	command = "ping -o -t " + to_string(t) + " " + m_machine_ip_address;
	string index = prepareUpdate(m_mail["ping"]);

	system_call_dispatch_return(command, m_mailboxes["bs_ping"], index);

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchSsh()
//   Purpose: Test sending, remotely executing, and getting a result
//   Returns: Command name and exact command
//      Note: Immediately returns if target is localhost

CommandSummary ManagedMoosMachine::dispatchSsh(int t)
{
	string summary = m_name + "back seat ssh";
	string command = Status::ISLOCAL;
	if (targetIsLocal()) return(make_pair(summary, command));
	if (!ready_to_dispatch(m_mail["ssh"]))
		return(make_pair(summary, Status::STALE));

	// ssh flags; for more information, see the man page
	// -o ConnectTimeout=X 			  : -o lets you specify an OPTION=VALUE pair
	// the string after the user@addr : executes the string as a command

	// send a payload with a random number that isn't likely to be a false
	// positive. If ssh exits with that number, the whole process is a success.

	// TODO : actually random?
	// see http://support.attachmate.com/techdocs/2487.html for allowed codes
	int payload = 42; // guaranteed to be random: https://xkcd.com/221/

	command = sshTrustPrefix() + "-o ConnectTimeout=" + to_string(t) + " " \
					+ getFullAddress() + " \"exit " + to_string(payload) + "\"";

	string index = prepareUpdate(m_mail["ssh"]);

	system_call_dispatch_return(command, m_mailboxes["bs_ssh"], index);

	return(make_pair(summary, command));
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

	if (!ready_to_dispatch(m_mail["front_seat_ping"]))
		return(make_pair(summary, Status::STALE));

	// ping flags; for more information, see the man page
	// -o  : return after the first successful packet is received
	// -t X: timeout after X seconds
	command = "ping -o -t " + to_string(t) + " " + m_front_seat_ip_address;

	string index = prepareUpdate(m_mail["front_seat_ping"]);

	system_call_dispatch_return(command, m_mailboxes["fs_ping"], index);

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
	string summary = m_name + "front seat ssh";
	string command = Status::NOTAPPLIC;

	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	if (!ready_to_dispatch(m_mail["front_seat_ssh"]))
	 return(make_pair(summary, Status::STALE));

	// ssh flags; for more information, see the man page
	// -o ConnectTimeout=X 			  : -o lets you specify an OPTION=VALUE pair
	// the string after the user@addr : executes the string as a command

	// send a payload with a random number that isn't likely to be a false
	// positive. If ssh exits with that number, the whole process is a success.

	// TODO : actually random?
	// see http://support.attachmate.com/techdocs/2487.html for allowed codes
	int payload = 42; // guaranteed to be random: https://xkcd.com/221/

	command = sshTrustPrefix() + "-o ConnectTimeout=" + to_string(t) + " " \
					+ getFrontSeatAddress() + " \"exit " + to_string(payload) + "\"";

	string index = prepareUpdate(m_mail["front_seat_ssh"]);

	system_call_dispatch_return(command, m_mailboxes["fs_ssh"], index);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchMoosdbCheck()
//   Purpose: Test that the MOOSDB is up
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::dispatchMoosdbCheck()
{
	string summary = m_name + " moosdb";
	string command;
	string mailbox = m_mailboxes["moosdb"];

	if (!ready_to_dispatch(m_mail["moosdb"]))
		return(make_pair(summary, Status::STALE));

	if (targetIsLocal()) {
		command = "ps aux | grep '[0-9] MOOSDB'";
	}
	else {
		command = sshTrustPrefix() + getFullAddress() \
					   + " \"ps aux\" | grep \"[0-9] MOOSDB\" 2>/dev/null";
	}

	string index = prepareUpdate(m_mail["moosdb"]);

	system_call_dispatch_pipe(command, mailbox, index, 10);
	return(make_pair(summary, command));
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

	bool allowed_local = true;
	bool front_seat = false;
	bool clear = false;

	if (tree=="aqua") {
		cmd = "pav-get-svn_rev_aqua";
		desc = "aquaticus " + desc;
	}
	else if (tree=="moos") {
		cmd = "pav-get-svn_rev_moos";
		desc = "moos-ivp " + desc;
	}
	else if (tree=="pablo") {
		cmd = "pav-get-svn_rev_pablo";
		desc = "pablo-common " + desc;
	}

	return(_dispatchGeneric(tree, cmd, desc, allowed_local, front_seat, clear));
}

//--------------------------------------------------------------------
// Procedure: dispatchCompassStatus()
//   Purpose: Test if the compass is on and broadcasting
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::dispatchCompassStatus()
{
	// TODO make actually robust
	string summary = m_name + " compass";
	string mailbox = m_mailboxes["compass"];
	string command = Status::NOTAPPLIC;

	if (!ready_to_dispatch(m_mail["compass"]))
		return(make_pair(summary, Status::STALE));

	if (m_front_seat_ip_address!="") {
		string ssh_wrapper = sshTrustPrefix() + " " + getFrontSeatAddress();
		string src = ROS_SOURCE;
		string payload = "\"" + src + "\nrostopic echo /imu/compass_heading -n 5 \"";
		string suppress_errors = "2>/dev/null";
		string command = ssh_wrapper + " " + payload + " " + suppress_errors;
		string index = prepareUpdate(m_mail["compass"]);

		system_call_dispatch_pipe(command, mailbox, index, 10);
	}

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchGpsPdop()
//   Purpose: Test if the GPS is on and publish its PDOP
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::dispatchGpsPdop()
{
	// TODO make actually robust
	string summary = m_name + " GPS PDOP";
	string mailbox = m_mailboxes["gpspdop"];
	string command = Status::NOTAPPLIC;

	if (!ready_to_dispatch(m_mail["gps_pdop"]))
		return(make_pair(summary, Status::STALE));

	if (m_front_seat_ip_address!="") {
		string ssh_wrapper = sshTrustPrefix() + " " + getFrontSeatAddress();
		string src = ROS_SOURCE;
		string payload = "\"" + src + "\nrostopic echo /navsat/rx -n 5 \"";
		string suppress_errors = "2>/dev/null";
		string command = ssh_wrapper + " " + payload + " " + suppress_errors;
		string index = prepareUpdate(m_mail["gps_pdop"]);

		system_call_dispatch_pipe(command, mailbox, index, 10);
	}

	return(make_pair(summary, command));
}


//--------------------------------------------------------------------
// Procedure: checkPingMail()
//   Purpose: Perform the check of the ping mailbox
//   Returns:
//      Note: Sets m_ping

string ManagedMoosMachine::checkPingMail()
{
	if (targetIsLocal()) {
		m_mail["ping"].data = Status::ISLOCAL;
		return(get_data_and_staleness(m_mail["ping"]));
	}

	vector<string> mail_list = readServiceMailbox(m_mailboxes["bs_ping"]);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["ping"], index)) {
		if (mail_list.size() > 0) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_mail["ping"].data = Status::NODATA;
				return(get_data_and_staleness(m_mail["ping"]));
			}
			try	{
				if (0==stoi(mail)) m_mail["ping"].data = Status::GOOD;
				else if (2==stoi(mail)) m_mail["ping"].data = Status::NOCONN;
				else m_mail["ping"].data = Status::ERROR;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["ping"].data = Status::ERROR; // error
			}
		}
		else m_mail["ping"].data = Status::NODATA; // No data yet
	}

	return(get_data_and_staleness(m_mail["ping"]));
}

//--------------------------------------------------------------------
// Procedure: checkSshMail()
//   Purpose: Perform the check of the ssh mailbox
//   Returns:
//      Note: Sets m_ssh

string ManagedMoosMachine::checkSshMail()
{
	if (targetIsLocal()) {
		m_mail["ssh"].data = Status::ISLOCAL;
		return(get_data_and_staleness(m_mail["ssh"]));
	}

	vector<string> mail_list = readServiceMailbox(m_mailboxes["bs_ssh"]);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["ssh"], index)) {
		if (mail_list.size() > 0)
		{
			string mail = mail_list[0];
			if (mail.size()==0) {
				m_mail["ssh"].data = Status::NODATA;
				return(get_data_and_staleness(m_mail["ssh"]));
			}
			try {
				// TODO: 42 is a magic number
				if (mail=="42") m_mail["ssh"].data = Status::GOOD;
				else m_mail["ssh"].data = Status::NOCONN;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["ssh"].data = Status::ERROR;
			}
		}
		else m_mail["ssh"].data = Status::NODATA; // no data yet
	}

	return(get_data_and_staleness(m_mail["ssh"]));
}

//--------------------------------------------------------------------
// Procedure: checkPingMail()
//   Purpose: Perform the check of the front seat ping mailbox
//   Returns:
//      Note: Sets m_front_seat_ping

string ManagedMoosMachine::checkVehiclePingMail()
{
	if (m_front_seat_ip_address=="") {
		m_mail["front_seat_ping"].data = Status::NOTAPPLIC;
		return(get_data_and_staleness(m_mail["front_seat_ping"]));
	}

	vector<string> mail_list = readServiceMailbox(m_mailboxes["fs_ping"]);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["front_seat_ping"], index)) {
		if (mail_list.size() > 0) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_mail["front_seat_ping"].data = Status::NODATA;
				return(get_data_and_staleness(m_mail["front_seat_ping"]));
			}
			try	{
				if (0==stoi(mail))
					m_mail["front_seat_ping"].data = Status::GOOD;
				else if (2==stoi(mail))
					m_mail["front_seat_ping"].data = Status::NOCONN;
				else
					m_mail["front_seat_ping"].data = Status::ERROR;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["front_seat_ping"].data = Status::ERROR; // error
			}
		}
		else m_mail["front_seat_ping"].data = Status::NODATA; // No data yet
	}
	return(get_data_and_staleness(m_mail["front_seat_ping"]));
}

//--------------------------------------------------------------------
// Procedure: checkVehicleSshMail()
//   Purpose: Perform the check of the front seat ssh mailbox
//   Returns:
//      Note: Sets m_front_seat_ssh

string ManagedMoosMachine::checkVehicleSshMail()
{
	if (m_front_seat_ip_address=="") {
		m_mail["front_seat_ssh"].data = Status::NOTAPPLIC;
		return(get_data_and_staleness(m_mail["front_seat_ssh"]));
	}

	vector<string> mail_list = readServiceMailbox(m_mailboxes["fs_ssh"]);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["front_seat_ssh"], index)) {
		if (mail_list.size() > 0)
		{
			string mail = mail_list[0];
			if (mail.size()==0) {
				m_mail["front_seat_ssh"].data = Status::NODATA;
				return(get_data_and_staleness(m_mail["front_seat_ssh"]));
			}
			try {
				// TODO: 42 is a magic number
				if (mail=="42") m_mail["front_seat_ssh"].data = Status::GOOD;
				else m_mail["front_seat_ssh"].data = Status::NOCONN;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_mail["front_seat_ssh"].data = Status::ERROR;
			}
		}
		else m_mail["front_seat_ssh"].data = Status::NODATA; // no data yet
	}
	return(get_data_and_staleness(m_mail["front_seat_ssh"]));
}

//--------------------------------------------------------------------
// Procedure: checkMoosdbMail()
//   Purpose: Perform the check of the MOOSDB mailbox
//   Returns:
//      Note: Sets m_moosdb

string ManagedMoosMachine::checkMoosdbMail()
{
	vector<string> mail_list = readServiceMailbox(m_mailboxes["moosdb"]);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail["moosdb"], index)) {
		if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) m_mail["moosdb"].data = Status::NODATA;
			else m_mail["moosdb"].data = Status::GOOD;
		}
		else if (mail_list.size()>0) m_mail["moosdb"].data = "TOO MANY";
		else m_mail["moosdb"].data = Status::NODATA;
	}
	return(get_data_and_staleness(m_mail["moosdb"]));
}


//--------------------------------------------------------------------
// Procedure: checkSvnRevisionMail()
//   Purpose: Perform the check of the svn mailbox
//   Returns:
//      Note:

string ManagedMoosMachine::checkSvnRevisionMail(string tree) {
	vector<string> mail_list = readServiceMailbox(m_mailboxes[tree]);
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_mail[tree], index)) {
		if (mail_list.size()==0) {
			m_mail[tree].data = Status::NODATA;
		}
		else if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_mail[tree].data = Status::NODATA;
				return(get_data_and_staleness(m_mail[tree]));
			}

			string delim = ": ";
			size_t loc = mail.find(delim);

			if (loc==string::npos) { // badly formated
				m_mail[tree].data = Status::ERROR;
			}
			else {
				if (loc+delim.size()>mail.size()) {
					m_mail[tree].data = Status::ERROR;
				}
				try {
					size_t point = loc + delim.size();
					m_mail[tree].data = mail.substr(point);
				}
				catch (...) { // exception &e / type error only? / TODO?
					// badly formatted
					m_mail[tree].data = Status::ERROR;
				}
			}
		}
		// very badly formatted
		else m_mail[tree].data = Status::ERROR;
	}
	return(get_data_and_staleness(m_mail[tree]));
}

//--------------------------------------------------------------------
// Procedure: checkCompassStatusMail()
//   Purpose: Perform the check of the compass mailbox
//   Returns:
//      Note: Sets m_compass

string ManagedMoosMachine::checkCompassStatusMail()
{
	string nopub = "WARNING: topic [/imu/compass_heading] does not appear to be published yet";
	string hsplit = "---";
	string delim = ": ";

	vector<string> mail = readServiceMailbox(m_mailboxes["compass"]);
	index_t index = grabIndex(mail);

	if (receiveUpdate(m_mail["compass"], index)) {
		int expected_results = 5;
		int observed_results = 0;

		if (getFrontSeatAddress()=="") {
			m_mail["compass"].data = Status::NODATA;
		}
		else if (mail.size()==0) {
			m_mail["compass"].data = Status::NODATA; // no data yet
		}
		else if (mail[0]==nopub) { // not publishing; likely hardware related
			m_mail["compass"].data = Status::NOPUB;
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
					m_mail["compass"].data = "NaN";
					return(get_data_and_staleness(m_mail["compass"]));
				}
				else if (regex_match(line, data_pattern)) observed_results++;
				else {
					m_mail["compass"].data = Status::ERROR; // badly formatted
					return(get_data_and_staleness(m_mail["compass"]));
				}
			}
			if (observed_results<expected_results) {
				m_mail["compass"].data = Status::NODATA;
			}
			else if (observed_results==expected_results) {
				m_mail["compass"].data = Status::GOOD;
			}
			else {
				m_mail["compass"].data = Status::ERROR;
			}
		}
	}
	return(get_data_and_staleness(m_mail["compass"]));
}

//--------------------------------------------------------------------
// Procedure: checkGpsPdopStatusMail()
//   Purpose: Perform the check of the compass mailbox
//   Returns:
//      Note: Sets m_compass

string ManagedMoosMachine::checkGpsPdopStatusMail()
{
	string nopub = "WARNING: topic [/navsat/rx] does not appear to be published yet";
	string hsplit = "---";
	string delim = ": ";

	int GPGSA_PDOP_index = 15; // per GPS spec
	vector<string> mail = readServiceMailbox(m_mailboxes["gpspdop"]);
	index_t index = grabIndex(mail);

	if (receiveUpdate(m_mail["gps_pdop"], index)) {
		if (getFrontSeatAddress()=="") {
			m_mail["gps_pdop"].data = Status::NODATA;
		}
		else if (mail.size()==0) {
			m_mail["gps_pdop"].data = Status::NODATA; // no data yet
		}
		else if (mail[0]==nopub) { // not publishing; likely hardware related
			m_mail["gps_pdop"].data = Status::NOPUB;
		}
		else {
			// no messages seen
			m_mail["gps_pdop"].data = Status::NODATA;

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
						m_mail["gps_pdop"].data = Status::ERROR;
					}
					else {
						m_mail["gps_pdop"].data = gpgsa_msg[GPGSA_PDOP_index];
					}
					break;
				}
				else if (regex_match(line, msg_pattern)) {
					// seen some messages; if no GPGSA messages have been seen, this will
					// return error, otherwise it will be overwritten and returned
					m_mail["gps_pdop"].data = Status::ERROR;
				}
			}
		}
	}
	return(get_data_and_staleness(m_mail["gps_pdop"]));
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
	string summary = m_name + " start MOOS";
	string mailbox = serviceMailboxName(m_mailboxes["startMOOS"]);
	string command;

	if (targetIsLocal()) {
		// line by line explanation

		// save the current directory and change to the specified one
		// execute the named command, with any arguments
		// save the exit code
		// change back to the old directory, return the result

		command = "cd " + m_target_script_dir + "\n" + \
						  "bash " + m_target_script_name + " " + \
						  m_target_script_args + " &>/dev/null &\n" + \
						  "RESULT=$( echo $? )\nexit $RESULT;";
	}
	else { // target is not local, send commands via ssh

		// ssh flags; for more information, see the man page
		// -o OPTION=VALUE 		  : named options and their values
		// string after user@addr : remotely executes the string as a command
		string ssh_prefix = sshTrustPrefix() + "-o ConnectTimeout=" + to_string(t) \
											+ " " + getFullAddress();

		string remote = "source ~/.profile\ncd " + m_target_script_dir + "\n" + \
							  	  "bash " + m_target_script_name + " " + \
							  	  m_target_script_args + " &>/dev/null\n" + \
							      "exit $?";

		command = ssh_prefix + " \"" + remote + "\"";
	}

	string index = prepareUpdate(m_mail["moosdb"]);

	system_call_dispatch_return(command, mailbox, index);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: stopMOOS()
//   Purpose: Stops MOOS on the machine, without shutting down the machine
//   Returns: Command name and exact command
//      Note: A thin wrapper around ktm

CommandSummary ManagedMoosMachine::stopMOOS()
{
	string summary = m_name + " stop MOOS";
	string mailbox = serviceMailboxName(m_mailboxes["stopMOOS"]);
	string command;

	if (targetIsLocal()) {
		command = "source ~/.bashrc\nktm";
	}
	else { // remote
		string ssh_with_args = sshTrustPrefix() + "-o ConnectTimeout=2";
		string payload = "\"source ~/.profile\nktm\"";
		string suppress_output = "&>/dev/null";
		string addr = getFullAddress();

		command = ssh_with_args + " " + addr + " " + payload; + \
							" " + suppress_output;
	}

	string index = prepareUpdate(m_mail["moosdb"]);
	m_mail["moosdb"].data = Status::NODATA;

	system_call_dispatch_return(command, mailbox, index);

	return(make_pair(summary, command));
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
	string summary = m_name + " restart hardware";
	string command = Status::ISLOCAL;
	string mailbox = m_mailboxes["reboot"];

	if (targetIsLocal()) return(make_pair(summary, command));

	command = sshTrustPrefix() + getFullAddress() + \
					+ " \"source ~/.profile; reboot\"";

	string index = prepareUpdate(m_mail["reboot"]);

	system_call_dispatch_pipe(command, mailbox, index);

	clearCache();
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: shutdownHardware()
//   Purpose: Shutdown the designated machine
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::shutdown() {
	string summary = m_name + " shutdown hardware";
	string command = Status::ISLOCAL;
	string mailbox = m_mailboxes["shutdown"];

	if (targetIsLocal()) return(make_pair(summary, command));

	command = sshTrustPrefix() + getFullAddress() + \
					+ " \"source ~/.profile; shutdown now\"";

	string index = prepareUpdate(m_mail["shutdown"]);

	system_call_dispatch_pipe(command, mailbox, index);

	clearCache();
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: rebootVehicle()
//   Purpose: Reboot the (front seat) machine
//   Returns: Command name and exact command
//      Note:

CommandSummary ManagedMoosMachine::rebootVehicle()
{
	string summary = m_name + " restart vehicle";
	string command = Status::NOTAPPLIC;
	string mailbox = m_mailboxes["reboot_vehicle"];

	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	command = sshTrustPrefix() + getFrontSeatAddress() + " -t " + \
					+ " \"source ~/.profile; sudo reboot\"";

	string index = prepareUpdate(m_mail["reboot"]);

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
	string summary = m_name + " shutdown vehicle";
	string command = Status::NOTAPPLIC;
	string mailbox = m_mailboxes["shutdown_vehicle"];

	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	command = sshTrustPrefix() + getFrontSeatAddress() + " -t " + \
					+ " \"source ~/.profile; shutdown now\"";

	string index = prepareUpdate(m_mail["shutdown"]);

	system_call_dispatch_pipe(command, mailbox, index);

	clearCache();
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchGeneric()
//   Purpose: Remotely call generic pavlab commands
// Arguments: name  : the common name of the function
//						cmd   : the full name of the function or alias
//						desc  : a human readable description of cmd
//						ready :
//						args  : any extra arguments that cmd needs
//						clear : if true, clear the cache after sending cmd (e.g. reboot)
//   Returns:
//      Note:

CommandSummary ManagedMoosMachine::_dispatchGeneric(string name,
																										string cmd,
																										string desc,
																									  bool allowed_local,
																									  bool runs_on_front_seat,
																									  bool clear,
																									  string args)
{
	string summary = m_name + " " + desc;
	string command = Status::NOTAPPLIC;

	if (targetIsLocal()) {
		if (allowed_local) {
			command = "source ~/.profile\n" + cmd + " " + args;
			string index = prepareUpdate(m_mail[name]);
			system_call_dispatch_pipe(command, m_mailboxes[name], index);
			if(clear) clearCache();
		}
		else {
			command = Status::ISLOCAL;
		}
	}
	else if (runs_on_front_seat) {
		if (getFrontSeatAddress()=="") return(make_pair(summary, Status::NOTFOUND));
		else command = sshTrustPrefix() + getFrontSeatAddress();
	}
	else {
		command = sshTrustPrefix() + getFullAddress();

		if (!ready_to_dispatch(m_mail[name]))
			return(make_pair(summary, Status::STALE));

		command = sshTrustPrefix() + getFullAddress() \
					+ " \"source ~/.profile\n" + cmd + " " + args + "\"";

		string index = prepareUpdate(m_mail[name]);
		system_call_dispatch_pipe(command, m_mailboxes[name], index);
		if(clear) clearCache();
	}
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
	const regex pavlab_back_seats ("^192\\.168\\.\\d{1,3}\\.100$");
	if (regex_match(m_machine_ip_address, pavlab_back_seats)) {
		stringstream ip_stream (m_machine_ip_address);

		vector<string> ip_parts;
		string s;
		while (ip_stream.good()) {
			getline(ip_stream, s, '.');
			ip_parts.push_back(s);
		}
		id = ip_parts[2]; // 192.168.VEH_ID.100
	}
	return(id);
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

	map<string, StampedData>::iterator mail;
	for(mail=m_mail.begin(); mail!=m_mail.end(); mail++) {
		clear_stamped_data(mail->second);
	}

	map<string, string>::iterator mailbox;
	for(mailbox=m_mailboxes.begin(); mailbox!=m_mailboxes.end(); mailbox++) {
		system_call("echo \"\" > " + mailbox->second);
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

	StampedData blank;
	m_mail["ping"] = blank;
	m_mail["ssh"] = blank;
	m_mail["moosdb"] = blank;
	m_mail["aqua"] = blank;
	m_mail["moos"] = blank;
	m_mail["pablo"] = blank;
	m_mail["compass"] = blank;
	m_mail["gps_pdop"] = blank;
	m_mail["front_seat_ping"] = blank;
	m_mail["front_seat_ssh"] = blank;
	m_mail["reboot"] = blank;
	m_mail["shutdown"] = blank;
	m_mail["reboot_vehicle"] = blank;
	m_mail["shutdown_vehicle"] = blank;

	m_mailboxes["bs_ping"] = serviceMailboxName("backseat_ping");
	m_mailboxes["bs_ssh"] = serviceMailboxName("backseat_ssh");
	m_mailboxes["fs_ping"] = serviceMailboxName("frontseat_ping");
	m_mailboxes["fs_ssh"] = serviceMailboxName("frontseat_ssh");
	m_mailboxes["moosdb"] = serviceMailboxName("moosdb");
	m_mailboxes["compass"] = serviceMailboxName("compass");
	m_mailboxes["gpspdop"] = serviceMailboxName("GpsPdop");
	m_mailboxes["startMOOS"] = serviceMailboxName("startMOOS");
	m_mailboxes["stopMOOS"] = serviceMailboxName("stopMOOS");
	m_mailboxes["aqua"] = serviceMailboxName("aquaSvnRev");
	m_mailboxes["moos"] = serviceMailboxName("moosSvnRev");
	m_mailboxes["pablo"] = serviceMailboxName("pabloSvnRev");
	m_mailboxes["reboot"] = serviceMailboxName("reboot");
	m_mailboxes["shutdown"] = serviceMailboxName("shutdown");
	m_mailboxes["reboot_vehicle"] = serviceMailboxName("rebootVehicle");
	m_mailboxes["shutdown_vehicle"] = serviceMailboxName("shutdownVehicle");

	map<string, string>::iterator m;
	for(m=m_mailboxes.begin(); m!=m_mailboxes.end(); m++) {
		system_call("touch " + m->second);
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
// 			What: The "trusting" version ignores the old ssh host key and don't
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

	valid_ip_patterns.push_back(pavlab_front_seats);
	valid_ip_patterns.push_back(pavlab_back_seats);
	valid_ip_patterns.push_back(pavlab_direct_to_pablos);
	valid_ip_patterns.push_back(pavlab_pablo_master);

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

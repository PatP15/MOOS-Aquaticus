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

pair<string, string> ManagedMoosMachine::dispatchPing(int t)
{
	string summary = m_name + "back seat ping";
	string command = Status::ISLOCAL;
	if (targetIsLocal()) return(make_pair(summary, command));

	if (!ready_to_dispatch(m_ping_results)) return(make_pair(summary, "STALE"));

	// ping flags; for more information, see the man page
	// -o  : return after the first successful packet is received
	// -t X: timeout after X seconds
	command = "ping -o -t " + to_string(t) + " " + m_machine_ip_address;
	string index = prepareUpdate(m_ping_results);

	system_call_dispatch_return(command,
															serviceMailboxName("backseat_ping"),
															index);

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchSsh()
//   Purpose: Test sending, remotely executing, and getting a result
//   Returns: Command name and exact command
//      Note: Immediately returns if target is localhost

pair<string, string> ManagedMoosMachine::dispatchSsh(int t)
{
	string summary = m_name + "back seat ssh";
	string command = Status::ISLOCAL;
	if (targetIsLocal()) return(make_pair(summary, command));
	if (!ready_to_dispatch(m_ssh_results)) return(make_pair(summary, "STALE"));

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

	string index = prepareUpdate(m_ssh_results);

	system_call_dispatch_return(command,
															serviceMailboxName("backseat_ssh"),
															index);

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchVehiclePing()
//   Purpose: Test connection to the front seat of the robot with ping
//   Returns: Command name and exact command
//      Note: ssh might succeed even if ping fails. "pinging" localhost
//			  instantly returns.

pair<string, string> ManagedMoosMachine::dispatchVehiclePing(int t)
{
	string summary = m_name + "front seat ping";
	string command = Status::NOTAPPLIC;
	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	if (!ready_to_dispatch(m_front_seat_ping_results))
		return(make_pair(summary, "STALE"));

	// ping flags; for more information, see the man page
	// -o  : return after the first successful packet is received
	// -t X: timeout after X seconds
	command = "ping -o -t " + to_string(t) + " " + m_front_seat_ip_address;

	string index = prepareUpdate(m_front_seat_ping_results);

	system_call_dispatch_return(command,
															serviceMailboxName("frontseat_ping"),
															index);

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchVehicleSsh()
//   Purpose: Test sending, remotely executing, and getting a result on the
//						front seat
//   Returns: Command name and exact command
//      Note: Immediately returns if target doesn't have a front seat

pair<string, string> ManagedMoosMachine::dispatchVehicleSsh(int t)
{
	string summary = m_name + "front seat ssh";
	string command = Status::NOTAPPLIC;

	if (getFrontSeatAddress()=="") return(make_pair(summary, command));

	if (!ready_to_dispatch(m_front_seat_ssh_results))
	 return(make_pair(summary, "STALE"));

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

	string index = prepareUpdate(m_front_seat_ssh_results);

	system_call_dispatch_return(command,
															serviceMailboxName("frontseat_ssh"),
															index);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchMoosdbCheck()
//   Purpose: Test that the MOOSDB is up
//   Returns: Command name and exact command
//      Note:

pair<string, string> ManagedMoosMachine::dispatchMoosdbCheck()
{
	string summary = m_name + " moosdb";
	string command;
	string mailbox = serviceMailboxName("moosdb");

	if (!ready_to_dispatch(m_moosdb_results)) return(make_pair(summary, "STALE"));

	if (targetIsLocal()) {
		command = "ps aux | grep '[0-9] MOOSDB'";
	}
	else {
		command = sshTrustPrefix() + getFullAddress() \
					   + " \"ps aux\" | grep \"[0-9] MOOSDB\" 2>/dev/null";
	}

	string index = prepareUpdate(m_moosdb_results);

	system_call_dispatch_pipe(command, mailbox, index, 10);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchAquaticusSvnRevision()
//   Purpose: Get the SVN revision of the moos-ivp-aquaticus tree
//   Returns: Command name and exact command
//      Note: Intentionally does not parse the "Revision: " out of the line,
//			  so that getAquaticusSvnRevisionMail() can tell the difference between
//			  errors and revision numbers.

pair<string, string> ManagedMoosMachine::dispatchAquaticusSvnRevision()
{
	string summary = m_name + " aquaticus svn rev";
	string command;
	string mailbox = serviceMailboxName("aquaSvnRev");

	if (!ready_to_dispatch(m_aquaticus_svn_revision_results))
		return(make_pair(summary, "STALE"));

	if (targetIsLocal()) {
		command = "cd ~/moos-ivp-aquaticus\nsvn info | grep Revision";
	}
	else {
		command = sshTrustPrefix() + getFullAddress() \
				+ " \"cd ~/moos-ivp-aquaticus\nsvn info | grep Revision\" 2>/dev/null";
	}

	string index = prepareUpdate(m_aquaticus_svn_revision_results);

	system_call_dispatch_pipe(command, mailbox, index, 10);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchMoosIvpSvnRevision()
//   Purpose: Get the SVN revision of the moos-ivp tree
//   Returns: Command name and exact command
//      Note: Intentionally does not parse the "Revision: " out of the line,
//			  so that getMoosIvpSvnRevisionMail() can tell the difference between
//			  errors and revision numbers.

pair<string, string> ManagedMoosMachine::dispatchMoosIvpSvnRevision()
{
	string summary = m_name + " moos-ivp svn rev";
	string command;
	string mailbox = serviceMailboxName("moosSvnRev");

	if (!ready_to_dispatch(m_moos_ivp_svn_revision_results))
		return(make_pair(summary, "STALE"));

	if (targetIsLocal()) {
		command = "cd ~/moos-ivp\nsvn info | grep Revision";
	}
	else {
		command = sshTrustPrefix() + getFullAddress() \
					+ " \"cd ~/moos-ivp\nsvn info | grep Revision\" 2>/dev/null";
	}

	string index = prepareUpdate(m_moos_ivp_svn_revision_results);

	system_call_dispatch_pipe(command, mailbox, index, 10);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchPabloSvnRevision()
//   Purpose: Get the SVN revision of the pablo-common/pablo-common-aro tree
//   Returns: Command name and exact command
//      Note: Intentionally does not parse the "Revision: " out of the line,
//			  so that getPabloSvnRevisionMail() can tell the difference between
//			  errors and revision numbers.

pair<string, string> ManagedMoosMachine::dispatchPabloSvnRevision()
{
	string summary = m_name + " pablo-common svn rev";
	string command;
	string mailbox = serviceMailboxName("pabloSvnRev");
	string get_dir = string("cd ~/pablo-common-aro 2>/dev/null") \
								 + string(" || ") \
								 + string(" cd ~/pablo-common 2>/dev/null");

	if (!ready_to_dispatch(m_pablo_svn_revision_results))
		return(make_pair(summary, "STALE"));

	if (targetIsLocal()) {
		command = get_dir + "\nsvn info 2>/dev/null | grep Revision";
	}
	else {
		command = sshTrustPrefix() + getFullAddress() \
					+ " \"" + get_dir + "\nsvn info | grep Revision\" 2>/dev/null";
	}

	string index = prepareUpdate(m_pablo_svn_revision_results);

	system_call_dispatch_pipe(command, mailbox, index, 10);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchCompassStatus()
//   Purpose: Test if the compass is on and broadcasting
//   Returns: Command name and exact command
//      Note:

pair<string, string> ManagedMoosMachine::dispatchCompassStatus()
{
	// TODO make actually robust
	string summary = m_name + " compass";
	string mailbox = serviceMailboxName("compass");
	string command = Status::NOTAPPLIC;

	if (!ready_to_dispatch(m_compass_results))
		return(make_pair(summary, "STALE"));

	if (m_front_seat_ip_address!="") {
		string ssh_wrapper = sshTrustPrefix() + " " + getFrontSeatAddress();
		string src = ROS_SOURCE;
		string payload = "\"" + src + "\nrostopic echo /imu/compass_heading -n 5 \"";
		string suppress_errors = "2>/dev/null";
		string command = ssh_wrapper + " " + payload + " " + suppress_errors;
		string index = prepareUpdate(m_compass_results);

		system_call_dispatch_pipe(command, mailbox, index, 10);
	}

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: dispatchGpsPdop()
//   Purpose: Test if the GPS is on and publish its PDOP
//   Returns: Command name and exact command
//      Note:

pair<string, string> ManagedMoosMachine::dispatchGpsPdop()
{
	// TODO make actually robust
	string summary = m_name + " GPS PDOP";
	string mailbox = serviceMailboxName("GpsPdop");
	string command = Status::NOTAPPLIC;

	if (!ready_to_dispatch(m_gps_pdop_results))
		return(make_pair(summary, "STALE"));

	if (m_front_seat_ip_address!="") {
		string ssh_wrapper = sshTrustPrefix() + " " + getFrontSeatAddress();
		string src = ROS_SOURCE;
		string payload = "\"" + src + "\nrostopic echo /navsat/rx -n 5 \"";
		string suppress_errors = "2>/dev/null";
		string command = ssh_wrapper + " " + payload + " " + suppress_errors;
		string index = prepareUpdate(m_gps_pdop_results);

		system_call_dispatch_pipe(command, mailbox, index, 10);
	}

	return(make_pair(summary, command));
}


//--------------------------------------------------------------------
// Procedure: checkPingMail()
//   Purpose: Perform the check of the ping mailbox
//   Returns:
//      Note: Sets m_ping_results

void ManagedMoosMachine::checkPingMail()
{
	if (targetIsLocal()) {
		m_ping_results.data = Status::ISLOCAL;
		return;
	}

	vector<string> mail_list = readServiceMailbox("backseat_ping");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_ping_results, index)) {
		if (mail_list.size() > 0) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_ping_results.data = Status::NODATA;
				return;
			}
			try	{
				if (0==stoi(mail)) m_ping_results.data = Status::GOOD;
				else if (2==stoi(mail)) m_ping_results.data = Status::NOCONN;
				else m_ping_results.data = Status::ERROR;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_ping_results.data = Status::ERROR; // error
			}
		}
		else m_ping_results.data = Status::NODATA; // No data yet
	}
}

//--------------------------------------------------------------------
// Procedure: checkSshMail()
//   Purpose: Perform the check of the ssh mailbox
//   Returns:
//      Note: Sets m_ssh_results

void ManagedMoosMachine::checkSshMail()
{
	if (targetIsLocal()) {
		m_ssh_results.data = Status::ISLOCAL;
		return;
	}

	vector<string> mail_list = readServiceMailbox("backseat_ssh");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_ssh_results, index)) {
		if (mail_list.size() > 0)
		{
			string mail = mail_list[0];
			if (mail.size()==0) {
				m_ssh_results.data = Status::NODATA;
				return;
			}
			try {
				// TODO: 42 is a magic number
				if (mail=="42") m_ssh_results.data = Status::GOOD;
				else m_ssh_results.data = Status::NOCONN;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_ssh_results.data = Status::ERROR;
			}
		}
		else m_ssh_results.data = Status::NODATA; // no data yet
	}

}

//--------------------------------------------------------------------
// Procedure: checkPingMail()
//   Purpose: Perform the check of the front seat ping mailbox
//   Returns:
//      Note: Sets m_front_seat_ping_results

void ManagedMoosMachine::checkVehiclePingMail()
{
	if (m_front_seat_ip_address=="") {
		m_front_seat_ping_results.data = Status::NOTAPPLIC;
		return;
	}

	vector<string> mail_list = readServiceMailbox("frontseat_ping");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_front_seat_ping_results, index)) {
		if (mail_list.size() > 0) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_front_seat_ping_results.data = Status::NODATA;
				return;
			}
			try	{
				if (0==stoi(mail)) m_front_seat_ping_results.data = Status::GOOD;
				else if (2==stoi(mail)) m_front_seat_ping_results.data = Status::NOCONN;
				else m_front_seat_ping_results.data = Status::ERROR;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_front_seat_ping_results.data = Status::ERROR; // error
			}
		}
		else m_front_seat_ping_results.data = Status::NODATA; // No data yet
	}
}

//--------------------------------------------------------------------
// Procedure: checkVehicleSshMail()
//   Purpose: Perform the check of the front seat ssh mailbox
//   Returns:
//      Note: Sets m_front_seat_ssh_results

void ManagedMoosMachine::checkVehicleSshMail()
{
	if (m_front_seat_ip_address=="") {
		m_front_seat_ssh_results.data = Status::NOTAPPLIC;
		return;
	}

	vector<string> mail_list = readServiceMailbox("frontseat_ssh");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_front_seat_ssh_results, index)) {
		if (mail_list.size() > 0)
		{
			string mail = mail_list[0];
			if (mail.size()==0) {
				m_front_seat_ssh_results.data = Status::NODATA;
				return;
			}
			try {
				// TODO: 42 is a magic number
				if (mail=="42") m_front_seat_ssh_results.data = Status::GOOD;
				else m_front_seat_ssh_results.data = Status::NOCONN;
			}
			catch (...) { // exception &e / type error only? / TODO?
				m_front_seat_ssh_results.data = Status::ERROR;
			}
		}
		else m_front_seat_ssh_results.data = Status::NODATA; // no data yet
	}
}

//--------------------------------------------------------------------
// Procedure: checkMoosdbMail()
//   Purpose: Perform the check of the MOOSDB mailbox
//   Returns:
//      Note: Sets m_moosdb_results

void ManagedMoosMachine::checkMoosdbMail()
{
	vector<string> mail_list = readServiceMailbox("moosdb");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_moosdb_results, index)) {
		if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) m_moosdb_results.data = Status::NODATA;
			else m_moosdb_results.data = Status::GOOD;
		}
		else if (mail_list.size()>0) m_moosdb_results.data = "TOO MANY";
		else m_moosdb_results.data = Status::NODATA;
	}
}

//--------------------------------------------------------------------
// Procedure: checkAquaticusSvnRevisionMail()
//   Purpose: Perform the check of the svn mailbox
//   Returns:
//      Note: Sets m_aquaticus_svn_revision_results

void ManagedMoosMachine::checkAquaticusSvnRevisionMail()
{
	vector<string> mail_list = readServiceMailbox("aquaSvnRev");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_aquaticus_svn_revision_results, index)) {
		if (mail_list.size()==0) {
			m_aquaticus_svn_revision_results.data = Status::NODATA;
		}
		else if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_aquaticus_svn_revision_results.data = Status::NODATA;
				return;
			}

			string delim = ": ";
			size_t loc = mail.find(delim);

			if (loc==string::npos) { // badly formated
				m_aquaticus_svn_revision_results.data = Status::ERROR;
			}
			else {
				if (loc+delim.size()>mail.size()) {
					m_aquaticus_svn_revision_results.data = Status::ERROR;
				}
				try {
					size_t point = loc + delim.size();
					m_aquaticus_svn_revision_results.data = mail.substr(point);
				}
				catch (...) { // exception &e / type error only? / TODO?
					// badly formatted
					m_aquaticus_svn_revision_results.data = Status::ERROR;
				}
			}
		}
		// very badly formatted
		else m_aquaticus_svn_revision_results.data = Status::ERROR;
	}
}
//--------------------------------------------------------------------
// Procedure: checkMoosIvpSvnRevisionMail()
//   Purpose: Perform the check of the svn mailbox
//   Returns:
//      Note: Sets m_moos_ivp_svn_revision_results

void ManagedMoosMachine::checkMoosIvpSvnRevisionMail()
{
	vector<string> mail_list = readServiceMailbox("moosSvnRev");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_moos_ivp_svn_revision_results, index)) {
		if (mail_list.size()==0) {
			m_moos_ivp_svn_revision_results.data = Status::NODATA;
		}
		else if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_moos_ivp_svn_revision_results.data = Status::NODATA;
				return;
			}

			string delim = ": ";
			size_t loc = mail.find(delim);

			if (loc==string::npos) { // badly formated
				m_moos_ivp_svn_revision_results.data = Status::ERROR;
			}
			else {
				if (loc+delim.size()>mail.size()) {
					m_moos_ivp_svn_revision_results.data = Status::ERROR;
				}
				try {
					size_t point = loc + delim.size();
					m_moos_ivp_svn_revision_results.data = mail.substr(point);
				}
				catch (...) { // exception &e / type error only? / TODO?
					// badly formatted
					m_moos_ivp_svn_revision_results.data = Status::ERROR;
				}
			}
		}
		// very badly formatted
		else m_moos_ivp_svn_revision_results.data = Status::ERROR;
	}
}
//--------------------------------------------------------------------
// Procedure: checkPabloSvnRevisionMail()
//   Purpose: Perform the check of the svn mailbox
//   Returns:
//      Note: Sets m_pablo_svn_revision_results

void ManagedMoosMachine::checkPabloSvnRevisionMail()
{
	vector<string> mail_list = readServiceMailbox("pabloSvnRev");
	index_t index = grabIndex(mail_list);

	if (receiveUpdate(m_pablo_svn_revision_results, index)) {
		if (mail_list.size()==0) m_pablo_svn_revision_results.data = Status::NODATA;
		else if (mail_list.size()==1) {
			string mail = mail_list[0];
			if(mail.size()==0) {
				m_pablo_svn_revision_results.data = Status::NODATA;
				return;
			}

			string delim = ": ";
			size_t loc = mail.find(delim);

			if (loc==string::npos) {
				m_pablo_svn_revision_results.data = Status::ERROR; // badly formated
			}
			else {
				if (loc+delim.size()>mail.size()) {
					m_pablo_svn_revision_results.data = Status::ERROR;
				}
				try {
					size_t point = loc + delim.size();
					m_pablo_svn_revision_results.data = mail.substr(point);
				}
				catch (...) { // exception &e / type error only? / TODO?
					// badly formatted
					m_pablo_svn_revision_results.data = Status::ERROR;
				}
			}
		}
		// very badly formatted
		else m_pablo_svn_revision_results.data = Status::ERROR;
	}
}

//--------------------------------------------------------------------
// Procedure: checkCompassStatusMail()
//   Purpose: Perform the check of the compass mailbox
//   Returns:
//      Note: Sets m_compass_results

void ManagedMoosMachine::checkCompassStatusMail()
{
	string nopub = "WARNING: topic [/imu/compass_heading] does not appear to be published yet";
	string hsplit = "---";
	string delim = ": ";

	vector<string> mail = readServiceMailbox("compass");
	index_t index = grabIndex(mail);

	if (receiveUpdate(m_compass_results, index)) {
		int expected_results = 5;
		int observed_results = 0;

		if (getFrontSeatAddress()=="") {
			m_compass_results.data = Status::NODATA;
		}
		else if (mail.size()==0) {
			m_compass_results.data = Status::NODATA; // no data yet
		}
		else if (mail[0]==nopub) { // not publishing; likely hardware related
			m_compass_results.data = Status::NOPUB;
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
					m_compass_results.data = "NaN";
					return;
				}
				else if (regex_match(line, data_pattern)) observed_results++;
				else {
					m_compass_results.data = Status::ERROR; // badly formatted
					return;
				}
			}
			if (observed_results<expected_results) {
				m_compass_results.data = Status::NODATA;
			}
			else if (observed_results==expected_results) {
				m_compass_results.data = Status::GOOD;
			}
			else {
				m_compass_results.data = Status::ERROR;
			}
		}
	}
}

//--------------------------------------------------------------------
// Procedure: checkGpsPdopStatusMail()
//   Purpose: Perform the check of the compass mailbox
//   Returns:
//      Note: Sets m_compass_results

void ManagedMoosMachine::checkGpsPdopStatusMail()
{
	string nopub = "WARNING: topic [/navsat/rx] does not appear to be published yet";
	string hsplit = "---";
	string delim = ": ";

	int GPGSA_PDOP_index = 15; // per GPS spec
	vector<string> mail = readServiceMailbox("GpsPdop");
	index_t index = grabIndex(mail);

	if (receiveUpdate(m_gps_pdop_results, index)) {
		if (getFrontSeatAddress()=="") {
			m_gps_pdop_results.data = Status::NODATA;
		}
		else if (mail.size()==0) {
			m_gps_pdop_results.data = Status::NODATA; // no data yet
		}
		else if (mail[0]==nopub) { // not publishing; likely hardware related
			m_gps_pdop_results.data = Status::NOPUB;
		}
		else {
			// no messages seen
			m_gps_pdop_results.data = Status::NODATA;

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
						m_gps_pdop_results.data = Status::ERROR;
					}
					else {
						m_gps_pdop_results.data = gpgsa_msg[GPGSA_PDOP_index];
					}
					break;
				}
				else if (regex_match(line, msg_pattern)) {
					// seen some messages; if no GPGSA messages have been seen, this will
					// return error, otherwise it will be overwritten and returned
					m_gps_pdop_results.data = Status::ERROR;
				}
			}
		}
	}
}

//--------------------------------------------------------------------
// Procedure: readPingMail()
//   Purpose: Read the internal status variable, m_ping_results
//   Returns:
//      Note:

string ManagedMoosMachine::readPingMail()
{
	return(get_data_and_staleness(m_ping_results));
}

//--------------------------------------------------------------------
// Procedure: readSshMail()
//   Purpose: Read the internal status variable, m_ssh_results
//   Returns:
//      Note:

string ManagedMoosMachine::readSshMail()
{
	return(get_data_and_staleness(m_ssh_results));
}

//--------------------------------------------------------------------
// Procedure: readVehiclePingMail()
//   Purpose: Read the internal status variable, m_ping_results
//   Returns:
//      Note:

string ManagedMoosMachine::readVehiclePingMail()
{
	return(get_data_and_staleness(m_front_seat_ping_results));
}

//--------------------------------------------------------------------
// Procedure: readVehicleSshMail()
//   Purpose: Read the internal status variable, m_ssh_results
//   Returns:
//      Note:

string ManagedMoosMachine::readVehicleSshMail()
{
	return(get_data_and_staleness(m_front_seat_ssh_results));
}

//--------------------------------------------------------------------
// Procedure: readMoosdbMail()
//   Purpose: Read the internal status variable, m_moosdb_results
//   Returns:
//      Note:

string ManagedMoosMachine::readMoosdbMail()
{
	return(get_data_and_staleness(m_moosdb_results));
}

//--------------------------------------------------------------------
// Procedure: readAquaticusSvnRevisionMail()
//   Purpose: Read the internal status variable, m_svn_revision_results
//   Returns:
//      Note:

string ManagedMoosMachine::readAquaticusSvnRevisionMail()
{
	return(get_data_and_staleness(m_aquaticus_svn_revision_results));
}

//--------------------------------------------------------------------
// Procedure: readMoosIvpSvnRevisionMail()
//   Purpose: Read the internal status variable, m_moos_ivp_svn_revision_results
//   Returns:
//      Note:

string ManagedMoosMachine::readMoosIvpSvnRevisionMail()
{
	return(get_data_and_staleness(m_moos_ivp_svn_revision_results));
}

//--------------------------------------------------------------------
// Procedure: readPabloRevisionMail()
//   Purpose: Read the internal status variable, m_pablo_svn_revision_results
//   Returns:
//      Note:

string ManagedMoosMachine::readPabloSvnRevisionMail()
{
	return(get_data_and_staleness(m_pablo_svn_revision_results));
}

//--------------------------------------------------------------------
// Procedure: readCompassStatusMail()
//   Purpose: Read the internal status variable, m_compass_results
//   Returns:
//      Note:

string ManagedMoosMachine::readCompassStatusMail()
{
	return(get_data_and_staleness(m_compass_results));
}

//--------------------------------------------------------------------
// Procedure: readGpsPdopStatusMail()
//   Purpose: Read the internal status variable, m_gps_pdop_results
//   Returns:
//      Note:

string ManagedMoosMachine::readGpsPdopStatusMail()
{
	return(get_data_and_staleness(m_gps_pdop_results));
}


//--------------------------------------------------------------------
// Management actions
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Procedure: startMOOS()
//   Purpose: Run a MOOS instance
//   Returns: Command name and exact command
//      Note: Doesn't check any dependencies, such as pinging or ssh testing.

pair<string, string> ManagedMoosMachine::startMOOS(int t)
{
	string summary = m_name + " start MOOS";
	string mailbox = serviceMailboxName("startMOOS");
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

	string index = prepareUpdate(m_moosdb_results);

	system_call_dispatch_return(command, mailbox, index);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: stopMOOS()
//   Purpose: Stops MOOS on the machine, without shutting down the machine
//   Returns: Command name and exact command
//      Note: A thin wrapper around ktm

pair<string, string> ManagedMoosMachine::stopMOOS()
{
	string summary = m_name + " stop MOOS";
	string mailbox = serviceMailboxName("stopMOOS");
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

	string index = prepareUpdate(m_moosdb_results);
	m_moosdb_results.data = Status::NODATA;

	system_call_dispatch_return(command, mailbox, index);

	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: restartMOOS()
//   Purpose: Stops running MOOS and restarts it
//   Returns: Command name and exact command
//      Note: Thin, passes arguments through. Same verbosity for starting and
// 			  stopping MOOS

pair<string, string> ManagedMoosMachine::restartMOOS(int t)
{
	string summary = m_name + " restart MOOS";
	pair<string, string> stop_res = stopMOOS();
	pair<string, string> start_res = startMOOS(t);

	return(make_pair(summary, stop_res.second + "\n\n" + start_res.second));
}

//--------------------------------------------------------------------
// Procedure: restartHardwareBlocking()
//   Purpose: Restart the designated machine
//   Returns: Command name and exact command
//      Note: Immediately returns 0 if run against the localhost. Blocks.

pair<string, string> ManagedMoosMachine::restartHardwareBlocking()
{
	string summary = m_name + " restart hardware";
	string command = Status::ISLOCAL;
	if (targetIsLocal()) return(make_pair(summary, command));

	// TODO - figure out how password handling works in curses
	// NOTE: decision on to-do - de-sudo-protect shutdown and restart on PABLOs

	//
	// Magic bash script ahead
	// Line-by-line explanations included
	//

	// # reboot; sudo password required. -t allows sending password (pseudo-tty)
	// ssh -t MACHINE@pIP 'source ~/.profile; sudo reboot now'

	// # wait until RasPi actually shuts down
	// # from five trials, never exceeded 2 seconds.
	// # Margin for maximum confidence; we shouldn't be debugging THIS code
	// sleep 5

	// # -o exits 0 after the first successful packet
	// ping -o IP

	// # wait until ssh server starts
	// # from five trials, never exceeded 3 seconds.
	// # Margin for maximum confidence; we shouldn't be debugging THIS code
	// sleep 5

	// # we livin
	// ssh MACHINE@IP 'exit 42'
	// echo $?

	command = sshTrustPrefix() + "-t " + getFullAddress() +
					  string(" 'source ~/.profile; sudo reboot now'\n") +
					  string("sleep 5\n") +
					  string("ping -o ") + m_machine_ip_address + "\n" +
					  string("sleep 5\n") +
					  sshTrustPrefix() + getFullAddress() + " 'exit 42'";

	system_call(command);
	return(make_pair(summary, command));
}

//--------------------------------------------------------------------
// Procedure: restartHardware()
//   Purpose: Restart the designated machine
//   Returns: Command name and exact command
//      Note:

pair<string, string> ManagedMoosMachine::restartHardware()
{
	string summary = m_name + " restart hardware";
	string command = Status::ISLOCAL;
	string mailbox = serviceMailboxName("restartHardware");

	if (targetIsLocal()) return(make_pair(summary, command));

	// // TODO - figure out how password handling works in curses

	// //
	// // Magic bash script ahead
	// // Line-by-line explanations included
	// //

	// // # reboot; sudo password required. -t allows sending password (pseudo-tty)
	// // ssh -t MACHINE@pIP 'source ~/.profile; sudo reboot now'

	// // # wait until RasPi actually shuts down
	// // # from five trials, never exceeded 2 seconds.
	// // # Margin for maximum confidence; we shouldn't be debugging THIS code
	// // sleep 5

	// // # -o exits 0 after the first successful packet
	// // ping -o IP

	// // # wait until ssh server starts
	// // # from five trials, never exceeded 3 seconds.
	// // # Margin for maximum confidence; we shouldn't be debugging THIS code
	// // sleep 5

	// // # we livin
	// // ssh MACHINE@IP 'exit 42'
	// // echo $?

	// command = sshTrustPrefix() + "-t " + getFullAddress() +
	// 					string(" 'source ~/.profile; sudo reboot now'\n") +
	// 					string("sleep 5\n") +
	// 					string("ping -o ") + m_machine_ip_address + "\n" +
	// 					string("sleep 5\n") +
	// 					sshTrustPrefix() + getFullAddress() + " 'exit 42'";

	// TODO: add a mailbox, and then get an index from it for below
	// system_call_dispatch_return(command, mailbox, index);
	// clearCache();
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
// Procedure: getFrontSetAddress()
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
// Procedure: getId()

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

pair<string, string> ManagedMoosMachine::clearCache()
{
	string summary = m_name + " clear cache";
	clear_stamped_data(m_ping_results);
	clear_stamped_data(m_ssh_results);
	clear_stamped_data(m_moosdb_results);
	clear_stamped_data(m_aquaticus_svn_revision_results);
	clear_stamped_data(m_moos_ivp_svn_revision_results);
	clear_stamped_data(m_pablo_svn_revision_results);
	clear_stamped_data(m_compass_results);
	clear_stamped_data(m_gps_pdop_results);
	clear_stamped_data(m_front_seat_ping_results);
	clear_stamped_data(m_front_seat_ssh_results);

	vector<string>::iterator m;
	for(m=m_mailboxes.begin(); m!=m_mailboxes.end(); m++) {
		system_call("echo \"\" > " + *m);
	}
	return(make_pair(summary, "--"));
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

	m_mailboxes.push_back(serviceMailboxName("backseat_ping"));
	m_mailboxes.push_back(serviceMailboxName("backseat_ssh"));
	m_mailboxes.push_back(serviceMailboxName("frontseat_ping"));
	m_mailboxes.push_back(serviceMailboxName("frontseat_ssh"));
	m_mailboxes.push_back(serviceMailboxName("moosdb"));
	m_mailboxes.push_back(serviceMailboxName("compass"));
	m_mailboxes.push_back(serviceMailboxName("GpsPdop"));
	m_mailboxes.push_back(serviceMailboxName("startMOOS"));
	m_mailboxes.push_back(serviceMailboxName("stopMOOS"));
	m_mailboxes.push_back(serviceMailboxName("aquaSvnRev"));
	m_mailboxes.push_back(serviceMailboxName("moosSvnRev"));
	m_mailboxes.push_back(serviceMailboxName("pabloSvnRev"));
	vector<string>::iterator m;
	for(m=m_mailboxes.begin(); m!=m_mailboxes.end(); m++) {
		system_call("touch " + *m);
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
	string mailbox_name = serviceMailboxName(service);
	vector<string> lines;

	ifstream mailbox(mailbox_name);

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
	vector<const regex> valid_ip_patterns;
	// front seats
	const regex pavlab_front_seats ("^192\\.168\\.\\d{1,3}\\.1$");
	// back seats
	const regex pavlab_back_seats ("^192\\.168\\.\\d{1,3}\\.100$");
	// PABLOs plugged directly into computer
	const regex pavlab_direct_to_pablos ("^192\\.168\\.2\\.\\d{1,3}$");

	valid_ip_patterns.push_back(pavlab_front_seats);
	valid_ip_patterns.push_back(pavlab_back_seats);
	valid_ip_patterns.push_back(pavlab_direct_to_pablos);

	string trusting_ssh = string("ssh -q") \
						 					+ string(" -o UserKnownHostsFile=/dev/null") \
						 					+ string(" -o StrictHostKeyChecking=no ");
	string unstrusting_ssh = "ssh ";

	vector<const regex>::iterator i;
	for(i=valid_ip_patterns.begin(); i!=valid_ip_patterns.end(); i++) {
		const regex pattern = *i;
		if (regex_match(m_machine_ip_address, pattern)) return(trusting_ssh);
	}
	// IP isn't in the safe set, use the normal ssh
	return(unstrusting_ssh);
}

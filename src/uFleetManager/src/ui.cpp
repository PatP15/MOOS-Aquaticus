/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: ui.cpp 	                                					 	 */
/*    DESC: main UI for the MOOS Fleet Manager                   */
/*    DATE: March 8th 2017                                       */
/*****************************************************************/

#include <unistd.h>
#include <ncurses.h>
#include "system_call.h"
#include "ManagedMoosMachine.h" // definitely included from other headers...
#include "utils.h"
#include "MBUtils.h"
#include <ACTable.h>
#include "ui.h"
#include <regex>
#include "utils.h"
#include "Constants.h"
#include <fstream>

using namespace std;

// matches keyboard modifier 'control', as in ctrl-c.
#define CTRL(X) (X & 037)
#define SECOND_IN_MICROS 1000000 // 1,000,000 microseconds == 1 second

//--------------------------------------------------------------------
// Procedure: setTableFormats()
//   Purpose: set up the formating for each window view (to be read by
//						ACTables in UI::loop)
//   Returns:
//      Note:

void UI::setTableFormats()
{
	vector<string> main1, main2;

	// The following code is NOT FORMATTED ACCORDING TO NORMAL LAB CONVENTIONS
	// I believe it is dramatically more readable and debuggable this way, but
	// that is my opinion and can be overruled

	main1.push_back(""); main2.push_back("M#");
	main1.push_back(""); main2.push_back("NAME");
	main1.push_back(""); main2.push_back("ID");
	main1.push_back("F"); main2.push_back("");
	main1.push_back(""); main2.push_back("NET");
	main1.push_back(""); main2.push_back("COMPASS");
	main1.push_back(""); main2.push_back("GPS PDOP");
	// main1.push_back(""); main2.push_back("BATT (A)");
	main1.push_back("B"); main2.push_back("");
	main1.push_back(""); main2.push_back("NET");
	main1.push_back(""); main2.push_back("MOOSDB");
	main1.push_back(""); main2.push_back("SVN");
	main1.push_back(""); main2.push_back("TEAM");
	m_headers["main"].push_back(main1);
	m_headers["main"].push_back(main2);

	vector<string> hist1;
	hist1.push_back("EXEC SUMMARY");
	hist1.push_back("TIME");
	hist1.push_back("FULL COMMAND");
	m_headers["cmd_hist"].push_back(hist1);

	vector<string> net1, net2;
	net1.push_back(""); net2.push_back("M#");
	net1.push_back(""); net2.push_back("NAME");
	net1.push_back(""); net2.push_back("ID");
	net1.push_back("F"); net2.push_back("");
	net1.push_back(""); net2.push_back("PING");
	net1.push_back(""); net2.push_back("SSH");
	net1.push_back(""); net2.push_back("USER");
	net1.push_back(""); net2.push_back("ADDR");
	net1.push_back("B"); net2.push_back("");
	net1.push_back(""); net2.push_back("PING");
	net1.push_back(""); net2.push_back("SSH");
	net1.push_back(""); net2.push_back("USER");
	net1.push_back(""); net2.push_back("ADDR");
	m_headers["net"].push_back(net1);
	m_headers["net"].push_back(net2);

	vector<string> svn1, svn2;
	svn1.push_back(""); svn2.push_back("#");
	svn1.push_back(""); svn2.push_back("NAME");
	svn1.push_back("\\"); svn2.push_back("\\");
	svn1.push_back("MOOS-IVP"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	svn1.push_back("AQUATICUS"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	svn1.push_back("PABLO"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	m_headers["svn"].push_back(svn1);
	m_headers["svn"].push_back(svn2);

	m_help_headers.push_back("TOPIC");
	m_help_headers.push_back("CMD");
	m_help_headers.push_back("DESCRIPTION");

	// a command is a pair of strings; the command itself and a description
	// each window (keys of the map) has a list (the vector) of commands
	m_help["all"].push_back(make_pair("h", "Toggle full help tooltips"));
	m_help["all"].push_back(make_pair("ctrl-a", "Toggle commanding mode"));
	m_help["all"].push_back(make_pair("ctrl-c", "Quit"));
	m_help["nav"].push_back(make_pair("m", "Main window"));
	m_help["nav"].push_back(make_pair("H", "Command history window"));
	m_help["nav"].push_back(make_pair("v", "SVN revisions window"));
	m_help["nav"].push_back(make_pair("n", "Network communications window"));
	m_help["cmd_all"].push_back(make_pair("S/s#", "Start MOOS        (all/machine #)"));
	m_help["cmd_all"].push_back(make_pair("K/k#", "ktm               (all/machine #)"));
	m_help["cmd_all"].push_back(make_pair("R/r#", "Restart MOOS      (all/machine #)"));
	m_help["cmd_all"].push_back(make_pair("W/w#", "Restart hardware  (all/machine #)"));
	m_help["cmd_all"].push_back(make_pair("D/d#", "Turn off hardware (all/machine #)"));
	m_help["cmd_all"].push_back(make_pair("G/g#", "Restart vehicle   (all/machine #)"));
	m_help["cmd_all"].push_back(make_pair("F/f#", "Turn off vehicle  (all/machine #)"));
	m_help["main"].push_back(make_pair("C",
																		 "Clear app's cache for all machines"));
	m_help["main"].push_back(make_pair("c#",
																		 "Clear app's cache for machines #"));
}

//--------------------------------------------------------------------
// Procedure: timeStampCommand()
//   Purpose: Add a timestamp to the command that the machine reports
//   Returns:
//      Note:

TimestampedCommand UI::timeStampCommand(CommandSummary raw_command)
{
	TimestampedCommand tc;
	tc.summary = raw_command.first;
	tc.command = raw_command.second;
	tc.timestamp = time(0);

	return(tc);
}

//--------------------------------------------------------------------
// Procedure: formatCommandTime()
//   Purpose: format a time for printing in the command history
//   Returns:
//      Note: time should be human-readable, with at least H/M/S info.


string UI::formatCommandTime(time_t time)
{
	string formated_time = "";
	tm * local_time = localtime(&time);

	formated_time += to_string(local_time->tm_hour) + ":";
	formated_time += to_string(local_time->tm_min) + ":";
	formated_time += to_string(local_time->tm_sec);

	return(formated_time);
}

//--------------------------------------------------------------------
// Procedure: accumulateStatus()
//   Purpose: Given multiple component statuses, give a summary
//   Returns: Summary string
//      Note: good values match with AND, bad values match with OR, else default

string UI::accumulateStatus(vector<string> statuses,
														vector<string> good_values,
														vector<string> bad_values,
														string default_value)
{
	vector<string>::iterator value;
	vector<string>::iterator status;

	// if S1 == good_value AND S2 == good_value AND ...
	for(value=good_values.begin(); value!=good_values.end(); value++) {
		bool all_equal = true;
		for(status=statuses.begin(); status!=statuses.end(); status++) {
			if (*status!=*value) {
				all_equal = false;
				break;
			}
		}
		if (all_equal) return(*value);
	}
	// if S1 == bad_value OR S2 == bad_value OR ...
	for(value=bad_values.begin(); value!=bad_values.end(); value++) {
		for(status=statuses.begin(); status!=statuses.end(); status++) {
			if (*status==*value) {
				return(*value);
			}
		}
	}
	return(default_value);
}

//--------------------------------------------------------------------
// Procedure: machineIsFiltered()
//   Purpose: Determines whether a machine should be displayed
//   Returns: true if the machine should be filtered
//      Note:

// bool UI::machineIsFiltered(vector<bool> statuses)
// {
// 	return(false);
// }


//--------------------------------------------------------------------
// Procedure: batchRecords()
//   Purpose: Coalesce similar command records into a single summary
//   Returns:
//      Note: Assumed that all commands have a summary of the form
// 								So-and-so DID THING
// 						and the batch has the form
//								ALL DID THING
//						and the commands are concatenated together

pair<string, string> UI::batchRecords(vector<CommandSummary> records)
{
	string summary, commands;
	if (records.size() > 0) {
		summary = records.front().first;
		biteString(summary, ' ');
		summary = "All " + summary;

		vector<CommandSummary>::iterator record;
		for(record=records.begin(); record!=records.end(); record++) {
			string command = record->second;
			if (command!=Status::NOTAPPLIC) {
				commands += command + "\n";
			}
		}
		if (commands=="") commands = "--";
	}
	return(make_pair(summary, commands));
}

//--------------------------------------------------------------------
// Procedure: sendIPCheck()
//   Purpose: Dispatch IP request to own computer
//      Note: IP may change during operations, so a single up-front check is not
// 						sufficient.

void UI::sendIPCheck()
{
	string ip_block = "( ifconfig en0 || ifconfig eth0 )";
	string get_line = " | grep 'inet '";
	string parse_line = " | awk '{split($0,a,\" \"); print a[2]}'";
	string write_back = " > /tmp/MOOSMAIL/ui_ip.mailbox";
	string ip_command = ip_block + get_line + parse_line + write_back;
	system_call(ip_command);
}


//--------------------------------------------------------------------
// Procedure: readIPCheck()
//   Purpose: Read IP mailbox and return a string
//   Returns: an IP if one is found, ERROR otherwise

string UI::readIPCheck()
{
	vector<string> lines;

	ifstream mailbox("/tmp/MOOSMAIL/ui_ip.mailbox");
	if (mailbox.is_open()) {
		string line;
		while (getline(mailbox, line)) lines.push_back(line);
	}

	const regex ip_format ("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");

	vector<string>::iterator i;
	for (i=lines.begin(); i!=lines.end(); i++) {
		string line = *i;
		if (regex_match(line, ip_format)) return(line);
	}
	return(Status::ERROR);
}

//--------------------------------------------------------------------
// Procedure: checkMachineMail()
//   Purpose: call on the machines to check mail
//   Returns:
//      Note: throttles for the CPU's sake

void UI::checkMachineMail()
{
	time_t new_time = time(0);
	int time_between_status_requests = 3; // seconds
	int time_between_mailbox_checks = 1; // seconds

	vector<ManagedMoosMachine>::iterator m;
	int elapsed_time_dispatch = new_time - m_last_status_request;
	if (time_between_status_requests < elapsed_time_dispatch) {
		sendIPCheck();
		for (m = m_machines.begin(); m != m_machines.end(); m++) {
			m->dispatchPing();
			m->dispatchSsh();
			m->dispatchVehiclePing();
			m->dispatchVehicleSsh();

			// machine (back seat)
			if ((m->checkSshMail()==Status::GOOD)||
					(m->checkSshMail()==Status::ISLOCAL)) {
				m->dispatchMoosdbCheck();
				// svn
				m->dispatchSvnRevisionCheck("moos");
				m->dispatchSvnRevisionCheck("aqua");
				m->dispatchSvnRevisionCheck("pablo");
			}

			// front seat, if applicable and up
			if ((m->checkVehicleSshMail()==Status::GOOD)) {
				m->dispatchCompassStatus();
				m->dispatchGpsPdop();
			}
		}
		m_last_status_request = new_time;
	}

	// int elapsed_time_check_mail = new_time - m_last_mail_check_request;
	// if (time_between_mailbox_checks < elapsed_time_check_mail) { // TODO was I filtering here?
	// 	for (m = m_machines.begin(); m != m_machines.end(); m++) {
	// 		m->checkPingMail();
	// 		m->checkSshMail();
	// 		m->checkVehiclePingMail();
	// 		m->checkVehicleSshMail();
	// 		m->checkMoosdbMail();
	// 		m->checkCompassStatusMail();
	// 		m->checkGpsPdopStatusMail();

	// 		// svn
	// 		m->checkMoosIvpSvnRevisionMail();
	// 		m->checkAquaticusSvnRevisionMail();
	// 		m->checkPabloSvnRevisionMail();
	// 	}
	// }
}
//--------------------------------------------------------------------
// Procedure: actOnKeyPress()
//   Purpose: parse key inputs, and respond if they match a known command
//   Returns:
//      Note: modifies m_key_feed and m_view

void UI::actOnKeyPress(int c)
{
	CommandSummary record;
	vector<CommandSummary> records;
	vector<ManagedMoosMachine>::iterator m;
	// conveniently, \\d+ only matches positive integers
	const regex restart_one ("r\\d+");
	const regex ktm_one ("k\\d+");
	const regex start_one ("s\\d+");
	const regex clear_one ("c\\d+");
	const regex hardware_restart_one ("w\\d+");
	const regex hardware_shutdown_one ("d\\d+");
	const regex vehicle_restart_one ("g\\d+");
	const regex vehicle_shutdown_one ("f\\d+");

	if(c!=ERR) {
		bool command_match = false;
		//--------------------------------------------------------------------
		// match special characters, such as ctrl-c
		//--------------------------------------------------------------------
		if (c==27||c==127) { // BACKSPACE, DELETE keys
			if (m_key_feed.size() > 0) m_key_feed = "";
		}
		else if (c==CTRL('c')) {
			m_keep_alive = false;
			return;
		}
		else if (c==CTRL('a')) {
			m_is_commanding ^= true; // toggle
		}
		else if (c==KEY_RESIZE) {
			// do nothing, don't add to key feed
		}
		//--------------------------------------------------------------------
		// add character to command, see if it matches
		//--------------------------------------------------------------------
		else {
			m_key_feed += c;
		}
		//--------------------------------------------------------------------
		// match special UI commands
		//--------------------------------------------------------------------
		if (m_key_feed=="h") {
			m_view_full_help ^= true; // toggle
			command_match = true;
		}
		else if (m_key_feed=="m") {
			m_view = "main";
			command_match = true;
		}
		else if (m_key_feed=="H") {
			m_view = "cmd_hist";
			command_match = true;
		}
		else if (m_key_feed=="v") {
			m_view = "svn";
			command_match = true;
		}
		else if (m_key_feed=="n") {
			m_view = "net";
			command_match = true;
		}
		//--------------------------------------------------------------------
		// match exact machine commands
		//--------------------------------------------------------------------
		if (m_is_commanding) {
			if (m_key_feed=="S") {
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					// if is connected or is local
					if ((m->checkSshMail()==Status::GOOD)||
							(m->checkSshMail()==Status::ISLOCAL))
					{
						// don't start MOOS twice...
						if (m->checkMoosdbMail()==Status::NODATA)
							record = m->startMOOS();
					}
				}
				command_match = true;
			}
			else if (regex_match(m_key_feed, start_one)) {
				int start_index = stoi(m_key_feed.substr(1));
				record = m_machines[start_index].startMOOS();
				command_match = true;
			}
			else if (m_key_feed=="K") {
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					records.push_back(m->stopMOOS());
				}
				record = batchRecords(records);
				command_match = true;
			}
			else if (regex_match(m_key_feed, ktm_one)) {
				int ktm_index = stoi(m_key_feed.substr(1));
				record = m_machines[ktm_index].stopMOOS();
				command_match = true;
			}
			else if (m_key_feed=="R") {
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					records.push_back(m->restartMOOS());
				}
				record = batchRecords(records);
				command_match = true;
			}
			else if (regex_match(m_key_feed, restart_one)) {
				int restart_index = stoi(m_key_feed.substr(1));
				// machines[restart_index].restartHardware();
				record = m_machines[restart_index].restartMOOS();
				command_match = true;
			}
			else if (m_key_feed=="W") {
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					records.push_back(m->reboot());
				}
				record = batchRecords(records);
				command_match = true;
			}
			else if (regex_match(m_key_feed, hardware_restart_one)) {
				int clear_index = stoi(m_key_feed.substr(1));
				record = m_machines[clear_index].reboot();
				command_match = true;
			}
			else if (m_key_feed=="D") {
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					records.push_back(m->shutdown());
				}
				record = batchRecords(records);
				command_match = true;
			}
			else if (regex_match(m_key_feed, hardware_shutdown_one)) {
				int clear_index = stoi(m_key_feed.substr(1));
				record = m_machines[clear_index].shutdown();
				command_match = true;
			}
			else if (m_key_feed=="G") {
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					records.push_back(m->rebootVehicle());
				}
				record = batchRecords(records);
				command_match = true;
			}
			else if (regex_match(m_key_feed, vehicle_restart_one)) {
				int clear_index = stoi(m_key_feed.substr(1));
				record = m_machines[clear_index].rebootVehicle();
				command_match = true;
			}
			else if (m_key_feed=="F") {
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					records.push_back(m->shutdownVehicle());
				}
				record = batchRecords(records);
				command_match = true;
			}
			else if (regex_match(m_key_feed, vehicle_shutdown_one)) {
				int clear_index = stoi(m_key_feed.substr(1));
				record = m_machines[clear_index].shutdownVehicle();
				command_match = true;
			}
		}
		if (m_key_feed=="C") {
			for(m=m_machines.begin(); m!=m_machines.end(); m++) {
				records.push_back(m->clearCache());
			}
			record = batchRecords(records);
			command_match = true;
		}
		else if (regex_match(m_key_feed, clear_one)) {
			int clear_index = stoi(m_key_feed.substr(1));
			record = m_machines[clear_index].clearCache();
			command_match = true;
		}
		if (command_match) {
			m_key_feed = "";
			c = -1;
			if ((record.first!="")&&(record.second!="")) {
				m_command_history.push_back(timeStampCommand(record));
				record.first = "";
				record.second = "";
			}
			records.clear();
		}
	}
}

//--------------------------------------------------------------------
// Procedure: printWindow()
//   Purpose: print the current window
//   Returns:
//      Note:

int UI::printWindow(int line_number)
{
	//--------------------------------------------------------------------
	// Print the window name
	//--------------------------------------------------------------------
	string window_name = "Window: " + m_view;
	mvprintw(line_number++, 0, window_name.c_str());

	vector<ManagedMoosMachine>::iterator m;

	//--------------------------------------------------------------------
	// Make a copy of the window's headers. The copied table will be printed.
	//--------------------------------------------------------------------
	ACTable view_table = ACTable(m_headers[m_view][0].size(), m_padding_size);
	vector<vector<string> >::iterator hi;
	vector<string>::iterator ti;
	for (hi=m_headers[m_view].begin(); hi!=m_headers[m_view].end(); hi++) {
		for(ti=hi->begin(); ti!=hi->end(); ti++) {
			view_table << *ti;
		}
	}

	//--------------------------------------------------------------------
	// Fill in view table contents
	//--------------------------------------------------------------------

	// horizontal break above content
	view_table.addHeaderLines();

	// gather all svn revisions; report which machine(s) are relatively newest
	vector<int> moos_revisions, aqua_revisions, pablo_revisions;
	for (m = m_machines.begin(); m != m_machines.end(); m++) {
		string raw_svn_rev;
		raw_svn_rev = m->checkSvnRevisionMail("moos");
		if (raw_svn_rev==Status::NODATA) moos_revisions.push_back(-1);
		else {
			try {
			 moos_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				moos_revisions.push_back(-2);
			}
		}
		raw_svn_rev = m->checkSvnRevisionMail("aqua");
		if (raw_svn_rev==Status::NODATA) aqua_revisions.push_back(-1);
		else {
			try {
				 aqua_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				aqua_revisions.push_back(-2);
			}
		}
		raw_svn_rev = m->checkSvnRevisionMail("pablo");
		if (raw_svn_rev==Status::NODATA) pablo_revisions.push_back(-1);
		else {
			try {
				 pablo_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				pablo_revisions.push_back(-2);
			}
		}
	}
	int machine_i = 0;

	if (m_view=="main") {
		vector<string> comms_good;
		comms_good.push_back(Status::GOOD);
		comms_good.push_back(Status::ISLOCAL);
		comms_good.push_back(Status::NOTAPPLIC);

		vector<string> comms_bad;
		comms_bad.push_back(Status::ERROR);
		comms_bad.push_back(Status::NOCONN);
		comms_bad.push_back(Status::NODATA);

		vector<string> svn_good;
		svn_good.push_back("NEW");

		vector<string> svn_bad;
		svn_bad.push_back("OOB"); // Out Of Bounds
		svn_bad.push_back(Status::ERROR);
		svn_bad.push_back(Status::NODATA);
		svn_bad.push_back("OLD");

		string default_err = "UI ERR";

		for (m = m_machines.begin(); m != m_machines.end(); m++) {
			int this_machine_i = machine_i++;
			vector<string> fs_comms;
			fs_comms.push_back(m->checkVehiclePingMail());
			fs_comms.push_back(m->checkVehicleSshMail());
			string fs_comm_status = accumulateStatus(fs_comms,
																							 comms_good,
																							 comms_bad,
																							 default_err);

			vector<string> bs_comms;
			bs_comms.push_back(m->checkPingMail());
			bs_comms.push_back(m->checkSshMail());

			string bs_comm_status = accumulateStatus(bs_comms,
																							 comms_good,
																							 comms_bad,
																							 default_err);

			// if the user requested "up" mode, and a machine (and its front seat, if
			// it has one) hasn't come up, then don't display it
			bool fs_down = ((fs_comm_status==Status::NODATA)||
					 						(fs_comm_status==Status::NOCONN)||
					 						(fs_comm_status==Status::NOTAPPLIC));

			bool bs_down = ((bs_comm_status==Status::NODATA)||
					 						(bs_comm_status==Status::NOCONN));

			if ((m_config.m_filter_by_liveness)&&(fs_down&&bs_down)) continue;

			view_table << to_string(this_machine_i);
			view_table << m->getName();
			view_table << m->getId();
			view_table << "/"; // Front

			view_table << fs_comm_status;

			view_table << m->checkCompassStatusMail();
			view_table << m->checkGpsPdopStatusMail();
			// view_table << Status::NOIMPL;
			view_table << "/"; // Back

			view_table << bs_comm_status;

			view_table << m->checkMoosdbMail();

			vector<string> svn_sum;
			svn_sum.push_back(compare_to_newest(this_machine_i, aqua_revisions));
			svn_sum.push_back(compare_to_newest(this_machine_i, moos_revisions));
			svn_sum.push_back(compare_to_newest(this_machine_i, pablo_revisions));

			view_table << accumulateStatus(svn_sum, svn_good, svn_bad, default_err);

			if (m->getTeam() == "") view_table << "-";
			else view_table << m->getTeam();
		}
	}
	else if (m_view=="cmd_hist") {
		int total_commands = m_command_history.size();
		if (total_commands>0) {
			int commands_to_view = 10; // TODO make this setable?
			int first_command;
			if (total_commands>commands_to_view) {
				first_command = total_commands - commands_to_view - 1;
			}
			else {
				first_command = 0;
			}

			for(int hist_i=first_command; hist_i<total_commands; hist_i++) {
				view_table << m_command_history[hist_i].summary;

				time_t raw_time = m_command_history[hist_i].timestamp;
				view_table << formatCommandTime(raw_time);

				string full_command = m_command_history[hist_i].command;
				vector<string> splitlines = parseString(full_command, '\n');
				if (splitlines.size()==1) {
					view_table << full_command;
				}
				else {
					vector<string>::iterator line;
					for(line=splitlines.begin(); line!=splitlines.end(); line++) {
						if (*line!="") {
							if (line!=splitlines.begin()) {
								view_table << "";
								view_table << "";
							}
							view_table << *line;
						}
					}
				}
			}
		}
	}
	else if (m_view=="svn") {
		int machine_i = 0;
		for (m = m_machines.begin(); m != m_machines.end(); m++) {
			int this_machine_i = machine_i++;
			view_table << to_string(this_machine_i);
			view_table << m->getName();

			view_table << "\\";
			view_table << m->checkSvnRevisionMail("moos");
			view_table << compare_to_newest(this_machine_i, moos_revisions);
			view_table << "/";
			view_table << m->checkSvnRevisionMail("aqua");
			view_table << compare_to_newest(this_machine_i, aqua_revisions);
			view_table << "/";
			view_table << m->checkSvnRevisionMail("pablo");
			view_table << compare_to_newest(this_machine_i, pablo_revisions);
			view_table << "/";

		}
	}
	else if (m_view=="net") {
		int machine_i = 0;
		for (m = m_machines.begin(); m != m_machines.end(); m++) {
			view_table << to_string(machine_i++);
			view_table << m->getName();
			view_table << m->getId();
			view_table << "/"; // Front
			view_table << m->checkVehiclePingMail();
			view_table << m->checkVehicleSshMail();
			view_table << m->getFrontSeatUsername();
			view_table << m->getFrontSeatIp();
			view_table << "/"; // Back
			view_table << m->checkPingMail();
			view_table << m->checkSshMail();
			view_table << m->getUsername();
			view_table << m->getIp();
		}
	}

	// horizontal break below content
	view_table.addHeaderLines();

	//--------------------------------------------------------------------
	// print the fully composed view
	//--------------------------------------------------------------------
	vector<string> printable_table = view_table.getTableOutput();
	for(ti=printable_table.begin(); ti!=printable_table.end(); ti++) {
		mvprintw(line_number++, 0, ti->c_str());
	}
	// add one line of padding
	line_number++;

	return(line_number);
}

//--------------------------------------------------------------------
// Procedure: printKeyFeed()
//   Purpose: print the keyboard input feed
//   Returns:
//      Note:

int UI::printKeyFeed(int key, int line_number)
{
	string prompt = "Input Stream:";
	mvprintw(line_number, 0, prompt.c_str());
	if (m_is_commanding) mvprintw(line_number, prompt.size()+1, ": COMMAND MODE");
	line_number++;
	attron(A_BOLD);
	mvprintw(line_number, 0, "%s", m_key_feed.c_str());

	int size = m_key_feed.size();
	if (m_key_feed.size() > 0) {
		if (key > 0) mvprintw(line_number, size, " (%3d)", key);
	}
	attroff(A_BOLD);
	// move cursor to the end of the key feed
	mvprintw(line_number, m_key_feed.size(), "");


	// move past key feed, and add one line of padding
	return(line_number+2);
}

//--------------------------------------------------------------------
// Procedure: printHelpText()
//   Purpose: print the help text
//   Returns:
//      Note:

int UI::printHelpText(int line_number)
{
	//--------------------------------------------------------------------
	// Make a copy of the view's help headers. The copied table will be printed.
	//--------------------------------------------------------------------
	vector<string>::iterator ti;
	ACTable help_view = ACTable(m_help_headers.size(), m_padding_size + 1);
	for(ti=m_help_headers.begin(); ti!=m_help_headers.end(); ti++) {
		help_view << *ti;
	}

	// Horizontal break above content
	help_view.addHeaderLines();

	// Add only the help topics that are relevant (all, and nav + current view
	// if the user asks to see the entire help sheet)
	vector<string> help_topics;
	help_topics.push_back("all");
	if (m_view_full_help) {
		help_topics.push_back("nav");

		// add this window as a section, if it has any commands
		if (m_help[m_view].size()>0) help_topics.push_back(m_view);

		// add commands if in the right windows and commanding mode
		if ((m_view=="main")&&(m_is_commanding)) help_topics.push_back("cmd_all");
	}

	//--------------------------------------------------------------------
	// Add each help topic
	//--------------------------------------------------------------------
	vector<string>::iterator hti;
	for(hti=help_topics.begin(); hti!=help_topics.end(); hti++) {
		string topic = *hti;

		// topic header
		help_view << topic;
		help_view << "";
		help_view << "";

		// add the commands
		vector<pair<string, string> > commands = m_help[topic];
		vector<pair<string, string> >::iterator hi;
		for(hi=commands.begin(); hi!=commands.end(); hi++) {
			//TODO// if((topic=="nav")&&(hi->fi==m_view)) continue;
			help_view << "";
			help_view << hi->first;
			help_view << hi->second;
		}
	}

	// Horizontal break below content
	help_view.addHeaderLines();

	//--------------------------------------------------------------------
	// Print out the table
	//--------------------------------------------------------------------
	mvprintw(line_number++, 0, "Commands (case sensitive):");
	vector<string> printable_help = help_view.getTableOutput();
	for(ti=printable_help.begin(); ti!=printable_help.end(); ti++) {
		mvprintw(line_number++, 2, ti->c_str());
	}

	// add one line of padding
	line_number++;
	return(line_number);
}

//--------------------------------------------------------------------
// Procedure: printComputerInfo()
//   Purpose: print info about the caller's computer
//   Returns:
//      Note:

int UI::printComputerInfo(int line_number) {

	// TODO
	// Own IP
	// Network name (e.g. is it Kayak-Local-5/2?)
	// Own CPU/RAM/Network traffic

	mvprintw(line_number++, 0, "-----------------------------------------------");
	string current_time = "Time: " + formatCommandTime(time(0));
	mvprintw(line_number++, 0, current_time.c_str());
	string ip = "My IP: " + readIPCheck();
	mvprintw(line_number++, 0, ip.c_str());
	mvprintw(line_number++, 0, "-----------------------------------------------");

	// add one line of padding
	line_number++;
	return(line_number);
}


//--------------------------------------------------------------------
// Procedure: printLastCommand()
//   Purpose: print the executive summary of the last command
//   Returns:
//      Note:

int UI::printLastCommand(int line_number)
{
	string last_command;
	if (m_command_history.size() > 0) {
		last_command = m_command_history.back().summary;
	}
	else {
		last_command = "none yet";
	}

	string prefix = "Last issued command: " + last_command;
	mvprintw(line_number++, 0, prefix.c_str());
	return(line_number);
}

//--------------------------------------------------------------------
// Procedure: buildUp()
//   Purpose: Prepare the UI app.
//   Returns:
//      Note: Should be kept separate from UI instantiation. This is the "start
//						the UI" method, which you may want to do some time after you
//						instantiate it

void UI::buildUp ()
{
	initscr();
	raw();
	cbreak();

	unsigned int input_milliseconds = 8; // hand tuned for responsiveness
	halfdelay(input_milliseconds);
	noecho();

	vector<ManagedMoosMachine>::iterator m;
	for(m=m_machines.begin(); m!=m_machines.end(); m++) {
		m->dispatchPing();
		m->dispatchSsh();
		m->dispatchVehiclePing();
		m->dispatchVehicleSsh();
	}

	sendIPCheck();
	setTableFormats();
	clear();
}

//--------------------------------------------------------------------
// Procedure: tearDown()
//   Purpose: Cleanly close the UI app.
//   Returns:
//      Note:

void UI::tearDown ()
{
	clear();
	endwin();
	printf("That was the Managed MOOS!\n"); // TODO - why don't I see this?
}

//--------------------------------------------------------------------
// Procedure: loop()
//   Purpose: Main rendering loop
//   Returns:
//      Note:

void UI::loop()
{
	m_last_status_request = time(0);
	m_last_mail_check_request = time(0) + 1;

	int i = 0; // iteration counter, for rate limiting
	int key_press = -1; // input character
	int print_i; // current line. Some prints, but not all, increment print_i
	while(m_keep_alive) {
		raw();
		cbreak();

		unsigned int input_milliseconds = 8; // hand tuned for responsiveness
		halfdelay(input_milliseconds);
		noecho();
		clear();
		// initialize variables
		print_i = 0;

		// header
		string header = "MOOS Fleet Manager";
		mvprintw(print_i, 0, header.c_str());
		if (m_is_commanding) mvprintw(print_i, header.size()+1, ": COMMAND MODE");
		print_i+=2;

		checkMachineMail();
		actOnKeyPress(key_press);

		print_i = printWindow(print_i);
		print_i = printHelpText(print_i);
		print_i = printComputerInfo(print_i);
		// get input characters below (it's the blocking step, it goes last)
		print_i = printLastCommand(print_i);
		print_i = printKeyFeed(key_press, print_i);

		refresh();
		i++;
		// get input characters
		key_press = getch();
		// sleep(2); //debugging hook
	}
}

//--------------------------------------------------------------------
// Procedure: UI()

UI::UI(Configuration config) {
	m_config = config;
	m_machines = m_config.getMachines();
	m_filtering_by_liveness = config.m_filter_by_liveness;
	m_view = "main";
	m_padding_size = 2;
	m_view_full_help = false;
	m_keep_alive = true;
	m_is_commanding = false;
}
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
	main1.push_back(""); main2.push_back("SVN");
	main1.push_back(""); main2.push_back("");
	main1.push_back("F"); main2.push_back("NET");
	main1.push_back("B"); main2.push_back("NET");
	main1.push_back(""); main2.push_back("");

	main1.push_back(""); main2.push_back("COMPASS");
	main1.push_back(""); main2.push_back("GPS");
	// main1.push_back(""); main2.push_back("BATT (A)");
	main1.push_back(""); main2.push_back("MOOSDB");

	m_headers["main"].push_back(main1);
	m_headers["main"].push_back(main2);

	vector<string> hist1;
	hist1.push_back("EXEC SUMMARY");
	hist1.push_back("TIME");
	hist1.push_back("FULL COMMAND*");
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
	svn1.push_back(""); svn2.push_back("M#");
	svn1.push_back(""); svn2.push_back("NAME");
	svn1.push_back("\\"); svn2.push_back("\\");
	svn1.push_back("MOOS-IVP"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	svn1.push_back("AQUATICUS"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	svn1.push_back("COLREGS"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	svn1.push_back("PABLO"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	svn1.push_back("MOKAI"); svn2.push_back("REV");
	svn1.push_back(""); svn2.push_back("CMP");
	svn1.push_back("/"); svn2.push_back("/");
	m_headers["svn"].push_back(svn1);
	m_headers["svn"].push_back(svn2);

	vector<string> moos1, moos2;
	moos1.push_back(""); moos2.push_back("M#");
	moos1.push_back(""); moos2.push_back("NAME");
	moos1.push_back(""); moos2.push_back("ID");
	moos1.push_back("A"); moos2.push_back("");
	moos1.push_back(""); moos2.push_back("MOOSDB");
	// moos1.push_back(""); moos2.push_back("TEAM");
	// moos1.push_back(""); moos2.push_back("MISSION*");
	moos1.push_back("E"); moos2.push_back("");
	moos1.push_back(""); moos2.push_back("TEAM");
	moos1.push_back(""); moos2.push_back("MISSION*");
	m_headers["MOOS"].push_back(moos1);
	m_headers["MOOS"].push_back(moos2);

	m_help_headers.push_back("TOPIC");
	m_help_headers.push_back("CMD");
	m_help_headers.push_back("DESCRIPTION");

	// a command is a pair of strings; the command itself and a description
	// each window (keys of the map) has a list (the vector) of
	HelpEntry help = {"", "h", "Toggle full help tooltips"};
	HelpEntry verb = {"", "V", "Toggle UI verbosity"};
	HelpEntry cmding = {"", "ctrl-a", "Toggle commanding mode"};
	HelpEntry quit = {"", "ctrl-c", "Quit"};
	HelpEntry clear = {"", "Backspace", "Clear input stream"};
	HelpEntry cache = 			{"", "C/c#", "Clear uFleetManager's cache (all/machine #)"};
	HelpEntry nav_main = {"main", "m", "Main window"};
	HelpEntry nav_hist = {"cmd_hist", "H", "Command history window"};
	HelpEntry nav_svn = {"svn", "v", "SVN revisions window"};
	HelpEntry nav_net = {"net", "n", "Network communications window"};
	HelpEntry nav_moos = {"MOOS", "M", "MOOS window"};
	HelpEntry nav_prev = {"prev", "p", "Previous view"};
	HelpEntry start = 			{"", "S/s#", "Start MOOS                  (all/machine #)"};
	HelpEntry stop = 				{"", "K/k#", "Stop MOOS                   (all/machine #)"};
	HelpEntry restart = 		{"", "R/r#", "Restart MOOS                (all/machine #)"};
	HelpEntry reboot_m = 		{"", "W/w#", "Reboot back seat            (all/machine #)"};
	HelpEntry shutdown_m = 	{"", "D/d#", "Shutdown back seat          (all/machine #)"};
	HelpEntry reboot_v = 		{"", "G/g#", "Reboot front seat           (all/machine #)"};
	HelpEntry shutdown_v = 	{"", "F/f#", "Shutdown front seat         (all/machine #)"};

	m_help["all"].push_back(help);
	m_help["common"].push_back(verb);
	m_help["common"].push_back(cmding);
	m_help["common"].push_back(quit);
	m_help["common"].push_back(clear);
	m_help["common"].push_back(cache);
	m_help["nav"].push_back(nav_main);
	m_help["nav"].push_back(nav_hist);
	m_help["nav"].push_back(nav_svn);
	m_help["nav"].push_back(nav_net);
	m_help["nav"].push_back(nav_moos);
	m_help["nav"].push_back(nav_prev);
	m_help["cmd_all"].push_back(start);
	m_help["cmd_all"].push_back(stop);
	m_help["cmd_all"].push_back(restart);
	m_help["cmd_all"].push_back(reboot_m);
	m_help["cmd_all"].push_back(shutdown_m);
	m_help["cmd_all"].push_back(reboot_v);
	m_help["cmd_all"].push_back(shutdown_v);
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
// Procedure: charFromIndex()
//   Purpose: Transforms the range 0-35 to the range 0-9a-z

string UI::charFromIndex(int i)
{
	char a[10]; // seems large enough
	if ((0<=i)&&(i<10)) sprintf(a, "%c", '0' + i);// ('0' /*+ i*/); // '0'-'9'
	else if ((10<=i)&&(i<36)) sprintf(a, "%c", 'a' + (i-10)); //c = ('a' /*+ (i-10)*/); // 'a'-'z'
	else sprintf(a, "%c", -1);
	string s (a);
	return(s.substr(0));
}

//--------------------------------------------------------------------
// Procedure: indexFromChar()
//   Purpose: Transforms the range 0-9a-z to the range 0-35

int UI::indexFromChar(char c)
{
	if (isdigit(c)) return(c - '0'); // '0'-'9' -> 0-9
	else return(c - 'a' + 10); // 'a'-'z' -> 10-35
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
				m->dispatchSvnRevisionCheck("colregs");
				m->dispatchSvnRevisionCheck("mokai");

				m->dispatchCompassStatus();
				m->dispatchGpsPdop();
			}

			// front seat, if applicable and up
			if ((m->checkVehicleSshMail()==Status::GOOD)) {
				// commands sent directly to the front seat go here
			}
		}
		m_last_status_request = new_time;
	}
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
	const regex restart_one ("r\\w+");
	const regex ktm_one ("k\\w+");
	const regex start_one ("s\\w+");
	const regex clear_one ("c\\w+");
	const regex hardware_restart_one ("w\\w+");
	const regex hardware_shutdown_one ("d\\w+");
	const regex vehicle_restart_one ("g\\w+");
	const regex vehicle_shutdown_one ("f\\w+");

	if(c!=ERR) {
		bool command_match = false;
		bool clear_key_feed = false;
		bool m_confirmed_previous = false;
		//--------------------------------------------------------------------
		// match special characters, such as ctrl-c
		//--------------------------------------------------------------------
		// match quit
		if (c==CTRL('c')) {
			m_keep_alive = false;
			return;
		}
		// if buffered a command that requires a confirmation, wait for yes or no
		else if (m_confirming_previous_command) {
			if ((c=='y')||(c=='Y')) {
				m_confirmed_previous = true;
				m_confirming_previous_command = false;
			}
			else if ((c=='n')||(c=='N')) {
				m_confirming_previous_command = false;
				m_buffered_command = "";
			}
			clear_key_feed = true;
		}
		// match backspace and delete
		else if (c==27||c==127) {
			clear_key_feed = true;
		}
		// match commanding toggle
		else if (c==CTRL('a')) {
			m_is_commanding ^= true; // toggle
			clear_key_feed = true;
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
		// match state changes
		//--------------------------------------------------------------------
		if (m_key_feed=="h") {
			m_view_full_help ^= true; // toggle
			clear_key_feed = true;
		}
		else if (m_key_feed=="V") {
			m_verbose ^= true; // toggle
			clear_key_feed = true;
		}
		//--------------------------------------------------------------------
		// match view changes
		//--------------------------------------------------------------------
		else if (m_key_feed=="m") {
			m_view_prev = m_view;
			m_view = "main";
			clear_key_feed = true;
		}
		else if (m_key_feed=="H") {
			m_view_prev = m_view;
			m_view = "cmd_hist";
			clear_key_feed = true;
		}
		else if (m_key_feed=="v") {
			m_view_prev = m_view;
			m_view = "svn";
			clear_key_feed = true;
		}
		else if (m_key_feed=="n") {
			m_view_prev = m_view;
			m_view = "net";
			clear_key_feed = true;
		}
		else if (m_key_feed=="M") {
			m_view_prev = m_view;
			m_view = "MOOS";
			clear_key_feed = true;
		}
		else if (m_key_feed=="p") {
			string view_prev = m_view_prev;
			m_view_prev = m_view;
			m_view = view_prev;
			clear_key_feed = true;
		}
		//--------------------------------------------------------------------
		// match exact machine commands
		//--------------------------------------------------------------------
		if (m_is_commanding) {
			string command;
			if (m_confirmed_previous) command = m_buffered_command;
			else command = m_key_feed;

			if (command=="S") {
				command_match = true;
				for(m=m_machines.begin(); m!=m_machines.end(); m++) {
					// if is connected or is local
					if ((m->checkSshMail()==Status::GOOD)||
							(m->checkSshMail()==Status::ISLOCAL))
					{
						// don't start MOOS twice...
						if (m->checkMoosdbMail()==Status::NODATA)
							records.push_back(m->startMOOS());
					}
				}
				record = batchRecords(records);
			}
			else if (regex_match(command, start_one)) {
				command_match = true;
				int index = stoi(command.substr(1));
				if ((0<=index)&&(index<m_machines.size())) {
					record = m_machines[index].startMOOS();
				}
			}
			else if (command=="K") {
				command_match = true;
				if (m_confirmed_previous) {
					for(m=m_machines.begin(); m!=m_machines.end(); m++) {
						records.push_back(m->stopMOOS());
					}
					record = batchRecords(records);
				}
				else {
					m_buffered_command = command;
					m_confirming_previous_command = true;
				}
			}
			else if (regex_match(command, ktm_one)) {
				command_match = true;
				int index = indexFromChar(command[1]);
				if ((0<=index)&&(index<m_machines.size())) {
					if (m_confirmed_previous) {

							record = m_machines[index].stopMOOS();
						}
					else {
						m_buffered_command = command;
						m_confirming_previous_command = true;
					}
				}
			}
			else if (command=="R") {
				command_match = true;
				if (m_confirmed_previous) {
					for(m=m_machines.begin(); m!=m_machines.end(); m++) {
						records.push_back(m->restartMOOS());
					}
					record = batchRecords(records);
				}
				else {
					m_buffered_command = command;
					m_confirming_previous_command = true;
				}
			}
			else if (regex_match(command, restart_one)) {
				command_match = true;
				int index = indexFromChar(command[1]);
				if ((0<=index)&&(index<m_machines.size())) {
					if (m_confirmed_previous) {
						record = m_machines[index].restartMOOS();
					}
					else {
						m_buffered_command = command;
						m_confirming_previous_command = true;
					}
				}
			}
			else if (command=="W") {
				command_match = true;
				if (m_confirmed_previous) {
					for(m=m_machines.begin(); m!=m_machines.end(); m++) {
						records.push_back(m->reboot());
					}
					record = batchRecords(records);
				}
				else {
					m_buffered_command = command;
					m_confirming_previous_command = true;
				}
			}
			else if (regex_match(command, hardware_restart_one)) {
				command_match = true;
				int index = indexFromChar(command[1]);
				if ((0<=index)&&(index<m_machines.size())) {
					if (m_confirmed_previous) {
						record = m_machines[index].reboot();
					}
					else {
						m_buffered_command = command;
						m_confirming_previous_command = true;
					}
				}
			}
			else if (command=="D") {
				command_match = true;
				if (m_confirmed_previous) {
					for(m=m_machines.begin(); m!=m_machines.end(); m++) {
						records.push_back(m->shutdown());
					}
					record = batchRecords(records);
				}
				else {
					m_buffered_command = command;
					m_confirming_previous_command = true;
				}
			}
			else if (regex_match(command, hardware_shutdown_one)) {
				command_match = true;
				int index = indexFromChar(command[1]);
				if ((0<=index)&&(index<m_machines.size())) {
					if (m_confirmed_previous) {
						record = m_machines[index].shutdown();
					}
					else {
						m_buffered_command = command;
						m_confirming_previous_command = true;
					}
				}
			}
			else if (command=="G") {
				command_match = true;
				if (m_confirmed_previous) {
					for(m=m_machines.begin(); m!=m_machines.end(); m++) {
						records.push_back(m->rebootVehicle());
					}
					record = batchRecords(records);
				}
				else {
					m_buffered_command = command;
					m_confirming_previous_command = true;
				}
			}
			else if (regex_match(command, vehicle_restart_one)) {
				command_match = true;
				int index = indexFromChar(command[1]);
				if ((0<=index)&&(index<m_machines.size())) {
					if (m_confirmed_previous) {
						record = m_machines[index].rebootVehicle();
					}
					else {
						m_buffered_command = command;
						m_confirming_previous_command = true;
					}
				}
			}
			else if (command=="F") {
				command_match = true;
				if (m_confirmed_previous) {
					for(m=m_machines.begin(); m!=m_machines.end(); m++) {
						records.push_back(m->shutdownVehicle());
					}
					record = batchRecords(records);
				}
				else {
					m_buffered_command = command;
					m_confirming_previous_command = true;
				}
			}
			else if (regex_match(command, vehicle_shutdown_one)) {
				command_match = true;
				int index = indexFromChar(command[1]);
				if ((0<=index)&&(index<m_machines.size())) {
					if (m_confirmed_previous) {
						record = m_machines[index].shutdownVehicle();
					}
					else {
						m_buffered_command = command;
						m_confirming_previous_command = true;
					}
				}
			}
		}
		if (m_key_feed=="C") {
			command_match = true;
			for(m=m_machines.begin(); m!=m_machines.end(); m++) {
				records.push_back(m->clearCache());
			}
			record = batchRecords(records);
		}
		else if (regex_match(m_key_feed, clear_one)) {
			command_match = true;
			int index = indexFromChar(m_key_feed[1]);
			if ((0<=index)&&(index<m_machines.size())) {
				record = m_machines[index].clearCache();
			}
		}
		if (command_match) {
			if ((record.first!="")&&(record.second!="")) {
				m_command_history.push_back(timeStampCommand(record));
				record.first = "";
				record.second = "";
			}
			if (m_confirmed_previous) {
				m_confirmed_previous = false;
				m_confirming_previous_command = false;
				m_buffered_command = "";
			}
			records.clear();
			clear_key_feed = true;
		}
		if (clear_key_feed) {
			m_key_feed = "";
			c = -1;
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
	// Print the state variables
	//--------------------------------------------------------------------

	string window_name = "Window: " + m_view;
	string is_verbose = "Verbose: ";
	string is_commanding = "Commanding: ";
	if (m_verbose) is_verbose += "Y";
	else is_verbose += "N";
	if (m_is_commanding) is_commanding += "Y";
	else is_commanding += "N";
	int buffer = 4;
	int position = 0;
	mvprintw(line_number, position, window_name.c_str());
	position += window_name.size() + buffer;
	mvprintw(line_number, position, is_verbose.c_str());
	position += is_verbose.size() + buffer;
	mvprintw(line_number++, position, is_commanding.c_str());
	mvprintw(line_number++, 0, "-----------------------------------------------");
	line_number++;

	//--------------------------------------------------------------------
	// Select which statuses to check
	//--------------------------------------------------------------------
	// this block could be used to stagger file checks in order to increase UI
	// responsiveness. It does not seem necessary to do so at this time, but this
	// block is being left as a hook in to do so.
	bool check_net, check_svn, check_sensors, check_moos;
	check_net = true;
	check_svn = true; //bool (m_mailbox_check_staggering_index % 3);
	check_sensors = true; //bool ((m_mailbox_check_staggering_index+1) % 3);
	check_moos = true; //bool ((m_mailbox_check_staggering_index+2) % 3);
	m_mailbox_check_staggering_index++;

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
	vector<int> moos_revisions, aqua_revisions, pablo_revisions,
							colregs_revisions, mokai_revisions;
	vector<ManagedMoosMachine>::iterator m;
	for (m = m_machines.begin(); m != m_machines.end(); m++) {
		string raw_svn_rev;
		raw_svn_rev = m->checkSvnRevisionMail("moos", check_svn);
		if (raw_svn_rev==Status::NODATA) moos_revisions.push_back(-1);
		else {
			try {
			 moos_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				moos_revisions.push_back(-2);
			}
		}
		raw_svn_rev = m->checkSvnRevisionMail("aqua", check_svn);
		if (raw_svn_rev==Status::NODATA) aqua_revisions.push_back(-1);
		else {
			try {
				 aqua_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				aqua_revisions.push_back(-2);
			}
		}
		raw_svn_rev = m->checkSvnRevisionMail("pablo", check_svn);
		if (raw_svn_rev==Status::NODATA) pablo_revisions.push_back(-1);
		else {
			try {
				 pablo_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				pablo_revisions.push_back(-2);
			}
		}
		raw_svn_rev = m->checkSvnRevisionMail("colregs", check_svn);
		if (raw_svn_rev==Status::NODATA) colregs_revisions.push_back(-1);
		else {
			try {
				 colregs_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				colregs_revisions.push_back(-2);
			}
		}
		raw_svn_rev = m->checkSvnRevisionMail("mokai", check_svn);
		if (raw_svn_rev==Status::NODATA) mokai_revisions.push_back(-1);
		else {
			try {
				 mokai_revisions.push_back(stoi(raw_svn_rev));
			} catch (...) {
				mokai_revisions.push_back(-2);
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
			fs_comms.push_back(m->checkVehiclePingMail(check_net));
			fs_comms.push_back(m->checkVehicleSshMail(check_net));
			string fs_comm_status = accumulateStatus(fs_comms,
																							 comms_good,
																							 comms_bad,
																							 default_err);

			vector<string> bs_comms;
			bs_comms.push_back(m->checkPingMail(check_net));
			bs_comms.push_back(m->checkSshMail(check_net));

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

			//--------------------------------------------------------------------
			// Static info
			//--------------------------------------------------------------------
			view_table << charFromIndex(this_machine_i);
			view_table << m->getName();
			view_table << m->getId();

			//--------------------------------------------------------------------
			// SVN - changes few times per mission
			//--------------------------------------------------------------------
			vector<string> svn_sum;
			string pablo_svn = compare_to_newest(this_machine_i, pablo_revisions);
			string mokai_svn = compare_to_newest(this_machine_i, mokai_revisions);
			bool pablo_has_svn_results = ((pablo_svn!=Status::NODATA)&&
																		(pablo_svn!=Status::ERROR));
			bool mokai_has_svn_results = ((mokai_svn!=Status::NODATA)&&
																		(mokai_svn!=Status::ERROR));
			if (pablo_has_svn_results) svn_sum.push_back(pablo_svn);
			if (mokai_has_svn_results) svn_sum.push_back(mokai_svn);
			if ((! pablo_has_svn_results)&&(! mokai_has_svn_results)) {
				if ((pablo_svn==Status::ERROR)||(mokai_svn==Status::ERROR))
					svn_sum.push_back(Status::ERROR);
				else
					svn_sum.push_back(Status::NODATA);
			}

			svn_sum.push_back(compare_to_newest(this_machine_i, aqua_revisions));
			svn_sum.push_back(compare_to_newest(this_machine_i, moos_revisions));
			svn_sum.push_back(compare_to_newest(this_machine_i, colregs_revisions));
			view_table << accumulateStatus(svn_sum, svn_good, svn_bad, default_err);

			//--------------------------------------------------------------------
			// Network status - may change frequently within a mission
			//--------------------------------------------------------------------
			view_table << "\\";
			view_table << fs_comm_status;
			view_table << bs_comm_status;
			view_table << "/";

			//--------------------------------------------------------------------
			// Sensor status
			//--------------------------------------------------------------------
			view_table << m->checkCompassStatusMail(check_sensors);
			view_table << m->checkGpsPdopStatusMail(check_sensors);
			// view_table << Status::NOIMPL; // battery

			//--------------------------------------------------------------------
			// Mission status
			//--------------------------------------------------------------------
			string moosdb = m->checkMoosdbMail(check_moos);
			if (m->getTeam() != "") moosdb += " (" + m->getTeam() + ")";
			view_table << moosdb;
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

				if (m_verbose) { // verbose mode
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
				else { // non verbose mode
					view_table << "<toggle verbosity>";
				}
			}
		}
	}
	else if (m_view=="svn") {
		int machine_i = 0;
		for (m = m_machines.begin(); m != m_machines.end(); m++) {
			int this_machine_i = machine_i++;
			view_table << charFromIndex(this_machine_i);
			view_table << m->getName();

			view_table << "\\";
			view_table << m->checkSvnRevisionMail("moos", check_svn);
			view_table << compare_to_newest(this_machine_i, moos_revisions);
			view_table << "/";
			view_table << m->checkSvnRevisionMail("aqua", check_svn);
			view_table << compare_to_newest(this_machine_i, aqua_revisions);
			view_table << "/";
			view_table << m->checkSvnRevisionMail("colregs", check_svn);
			view_table << compare_to_newest(this_machine_i, colregs_revisions);
			view_table << "/";
			view_table << m->checkSvnRevisionMail("pablo", check_svn);
			view_table << compare_to_newest(this_machine_i, pablo_revisions);
			view_table << "/";
			view_table << m->checkSvnRevisionMail("mokai", check_svn);
			view_table << compare_to_newest(this_machine_i, mokai_revisions);
			view_table << "/";
		}
	}
	else if (m_view=="net") {
		int machine_i = 0;
		for (m = m_machines.begin(); m != m_machines.end(); m++) {
			int this_machine_i = machine_i++;
			view_table << charFromIndex(this_machine_i);//to_string(machine_i++);
			view_table << m->getName();
			view_table << m->getId();
			view_table << "/"; // Front
			view_table << m->checkVehiclePingMail(check_net);
			view_table << m->checkVehicleSshMail(check_net);
			view_table << m->getFrontSeatUsername();
			view_table << m->getFrontSeatIp();
			view_table << "/"; // Back
			view_table << m->checkPingMail(check_net);
			view_table << m->checkSshMail(check_net);
			view_table << m->getUsername();
			view_table << m->getIp();
		}
	}
	else if (m_view=="MOOS") {
		for (m = m_machines.begin(); m != m_machines.end(); m++) {
			int this_machine_i = machine_i++;
			view_table << charFromIndex(this_machine_i);//to_string(this_machine_i);
			view_table << m->getName();
			view_table << m->getId();
			view_table << "\\"; // Actual
			view_table << m->checkMoosdbMail(check_moos);
			view_table << "/"; // Expected
			view_table << m->getTeam();
			string mission;
			if (m_verbose) mission = m->getFullMission();
			else mission = m->getLaunchFile();
			view_table << mission;
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
	string prompt;
	if (m_confirming_previous_command)
		prompt = "Confirm Command " + m_buffered_command + " [y/n]:";
	else {
		prompt = "Listening";
		if (m_is_commanding) prompt += " [COMMAND MODE]:";
		else prompt += ":";
	}
	// display prompt
	mvprintw(line_number++, 0, prompt.c_str());
	// display current key feed
	mvprintw(line_number, 0, "%s", m_key_feed.c_str());
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
		help_topics.push_back("common");


		// add this window as a section, if it has any commands
		if (m_help[m_view].size()>0) help_topics.push_back(m_view);

		// add commands if in the right windows and commanding mode
		if (m_is_commanding) help_topics.push_back("cmd_all");
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
		vector<HelpEntry> commands = m_help[topic];
		vector<HelpEntry>::iterator hi;
		for(hi=commands.begin(); hi!=commands.end(); hi++) {
			if((topic=="nav")&&(hi->view==m_view)) help_view << "=====>";
			else help_view << "";

			help_view << hi->key;
			help_view << hi->desc;
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
	// raw();
	// cbreak();

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
//      Note: Seems to only run on seg fault. See main.cpp

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
		mvprintw(print_i++, 0, header.c_str());
		print_i++;

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
	m_view_prev = "main";
	m_padding_size = 2;
	m_view_full_help = false;
	m_keep_alive = true;
	m_is_commanding = false;
	m_verbose = false;
	m_mailbox_check_staggering_index = 0;
	m_confirmed_previous = false;
	m_confirming_previous_command = false;
	m_buffered_command = "";
}
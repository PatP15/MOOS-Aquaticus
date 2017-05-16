/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: utils.cpp 	                     					         */
/*    DESC: A collection of utilities used by the MOOS 					 */
/*					Deployment Manager           												 */
/*    DATE: April 20th 2017                                      */
/*****************************************************************/

#include "utils.h"
#include <ctime>
#include "system_call.h"
#include <string>
#include "Constants.h"
#include <sys/types.h>
#include <ifaddrs.h>

using namespace std;

//--------------------------------------------------------------------
// Procedure: sanitizeFileMailboxes()
//   Purpose: Clear out the directory containing status mailboxes
//   Returns:
//      Note:
// 			  Originally written 03/15/17. Collected into dm_utils 04/20/17.

int sanitizeFileMailboxes() {
	string loc = "/tmp/MOOSMAIL";

	char test_format[] = "test %s %s";
	char exists_test [100];
	sprintf(exists_test, test_format, "-e", loc.c_str());

	// if the mailbox directory exists (test returns 0) then clear it out
	// otherwise, make the directory and return
	if (0==system_call(exists_test)) {
		char dir_or_link_test [100];
		char link_test [100];

		sprintf(dir_or_link_test, test_format, "-d", loc.c_str());
		sprintf(link_test, test_format, "-h", loc.c_str());

		// confirm that the mailbox is a directory (as opposed to a file or a
		// symlink) before treating it like a directory.
		//
		// note that testing with -d matches directories and links, so we need
		// to check that it matches -d and that it doesn't match -h
		if((0==system_call(dir_or_link_test))&&(1==system_call(link_test))) {
			system_call("rm " + loc + "/*.mailbox");
		}
		else {
			// if the file exists and is not a directory, something is very
			// wrong. Abort!
			return(-1);
		}
	}
	else {
		system_call("mkdir " + loc);
	}

	return(0);
}


//--------------------------------------------------------------------
// Procedure: all_equal()
//   Purpose: determines if all elements of an array are equal
//   Returns:
//      Note: in an empty array, logically all elements are equal. This is
// 						equaivalent to !(are some elements unequal)

bool all_equal(vector<int> v) {
	for(vector<int>::iterator i=v.begin(); i!=v.end(); i++) {
		vector<int>::iterator next = i+1;
		if(next!=v.end()) {
			int a = *i;
			int b = *next;
			if (a!=b) return(false);
		}
	}
	return(true);
}

//--------------------------------------------------------------------
// Procedure: compare_to_largest()
//   Purpose: determine if v[index] is at least as large as the maximum element
//						in v
//   Returns:
//      Note: ignores negative values as bad

string compare_to_largest(unsigned int index, vector<int> v) {
	if (index>=v.size()) return("OOB");
	else if (v[index]==-1) return(Status::NODATA);
	else if (v[index]<0) return(Status::ERROR);
	for(vector<int>::iterator i=v.begin(); i!=v.end(); i++) {
		int x = *i;
		if (x>v[index]) return("SMALL");
	}
	return("LARGE");
}


//--------------------------------------------------------------------
// Procedure: compare_to_newest()
//   Purpose: wrapper around compare_to_largest, renaming to make more sense for
//						revisions
//   Returns:
//      Note: LARGE->NEW, SMALL->OLD

string compare_to_newest(unsigned int index, vector<int> v) {
	string raw = compare_to_largest(index, v);
	if (raw=="LARGE") return("NEW");
	else if (raw=="SMALL") return("OLD");
	else return(raw);
}


//--------------------------------------------------------------------
// Procedure: clear_stamped_data()
//   Purpose: Set default values for the StampedData type

void clear_stamped_data(StampedData & s) {
	s.data = Status::NODATA;
	s.i = 0;
	s.sent = time(0);
	s.received = time(0);
}


//--------------------------------------------------------------------
// Procedure: get_data_and_staleness()
//   Purpose: Appends staleness to data if
//   Returns:
//      Note:

string get_data_and_staleness(StampedData s, time_t staleness_time)
{
	time_t current_time = time(0);
	time_t elapsed_time = current_time - s.received;
	string data = s.data;
	// if ((data!=Status.NODATA)&&
	// 		(data!=Status.NOTAPPLIC)&&
	// 		(data!=Status.ISLOCAL)&&
	// 		(elapsed_time > staleness_time)) {
	// 	string staleness_suffix = " (" + to_string(elapsed_time) + ")";
	// 	data += staleness_suffix;
	// }
	return(data);
}

//--------------------------------------------------------------------
// Procedure: ready_to_dispatch()
//   Purpose: Allow a new packet out if the old one has been received
//   Returns:
//      Note: Times out after timeout seconds, default 30

bool ready_to_dispatch(StampedData & s, time_t timeout)
{
	bool received_latest = (s.sent <= s.received);
	if (received_latest) return(true);
	else {
		time_t current_time = time(0);
		time_t elapsed_time = current_time - s.sent;
		if (elapsed_time > timeout) {
			s.i++;
			s.sent = 0;
			return(true);
		}
		else return(false);
	}
}

string get_my_ip() {
	string my_ip;
	struct ifaddrs * ifaces;
	int iface_count = getifaddrs(&ifaces);

 	freeifaddrs(ifaces);
 	return(my_ip);
}

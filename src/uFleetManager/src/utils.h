/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: utils.h                                					 */
/*    DATE: April 20th 2017                                      */
/*****************************************************************/

#ifndef DEPLOYMENTMANAGERUTILS
#define DEPLOYMENTMANAGERUTILS

#include <vector>
#include <string>

typedef long index_t;

struct StampedData {
	std::string data; // payload
	index_t i; // index, only unique per mailbox
	time_t sent;
	time_t received;
};

int sanitizeFileMailboxes();
bool all_equal(std::vector<int>);
std::string compare_to_largest(unsigned int, std::vector<int>);
std::string compare_to_newest(unsigned int, std::vector<int>);

std::string get_my_ip();

void clear_stamped_data(StampedData&);
std::string get_data_and_staleness(StampedData, time_t=15);
bool ready_to_dispatch(StampedData&, time_t=45);

#endif
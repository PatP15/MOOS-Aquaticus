/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: main.cpp 	                     					         		 */
/*    DESC: main file for Fleet Commander							 					 */
/*    DATE: March 30th 2017                                   	 */
/*****************************************************************/

#include "ui.h"
#include "Configuration.h"
#include <exception>
#include <iostream>
#include "utils.h"

using namespace std;

//--------------------------------------------------------------------
// Procedure: main()

int main(int argc, char* argv[]) {

	sanitizeFileMailboxes();
	Configuration config = Configuration(argc, argv);
	UI ui = UI(config);

	// the UI identifies machines by their number. To support enter-key-less
	// commands, the total number of machines is capped at 10.
	//
	// To expand this in the future, either an explicit confirm key (e.g. 'enter'
	// to send commands) needs to be reimplemented, or the workflow needs to be
	// redesigned (e.g. go onto the machine's page, execute the command, go back
	// to the main page. That frees up lots of keys on the main page.)

	int max_machines = 10;
	int machine_count = config.getMachines().size();
	if (machine_count>max_machines) {
		printf("Selected %d machines, but the UI can only support %d.\n",
					 machine_count, max_machines);
		printf("This limitation is driven by the UI design; see main.cpp.\n");
		printf("Aborting.\n");
		return(1);
	}

	try {
		ui.buildUp();
		ui.loop();
	} catch (exception &e) {
		cout << "Caught exception:\t" << e.what() << endl;
	}
	ui.tearDown();
	return(0);
}
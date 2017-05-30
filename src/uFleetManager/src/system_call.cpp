 /*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: system_call.cpp 	                                 	 */
/*    DESC: Various wrapped C++ 'system' calls                   */
/*    DATE: March 8th 2017                                       */
/*****************************************************************/

#include "system_call.h"
#include <cstdlib>

using namespace std;

//--------------------------------------------------------------------
// Procedure: system_call()
//   Purpose: Wrapped bash script call
//   Returns:
//      Note: This method has real security concerns
// 			  Do not use it if you don't know what you're doing
// 			  Never use it to call command line scripts of unknown pedigree
//
//			  Where you would use semicolons between commands on the command
// 			  line or newlines in an actual script, use newlines

int system_call(string command) {
	return(system(command.c_str()) / 256);
}

//--------------------------------------------------------------------
// Procedure: _dispatch()
//   Purpose:
//   Returns:
//      Note: runs in the background and returns immediately
// 			  nohup will sustain the command after the shell closes, and &
//			  pushes it to the background. As a result, the return value is
//			  meaningless; it is the result of nohup delegating, not the script
//			  result

void _dispatch(string command, string timeout) {
	string dispatched = "nohup `bash -c \'" + command + "\'` &>/dev/null &";
	system_call(dispatched + " " + timeout);
}

//--------------------------------------------------------------------
// Procedure: system_call_dispatch_return()
//   Purpose: Wrapped dispatch call that saves the returned value
//   Returns:
//      Note: It is crucial to write the index after the data; this is the app's
//						signal that the data is new. Doing it in the other order will
//						cause long gaps where useful data is not displayed in the app

void system_call_dispatch_return(string command, string mailbox, string index) {
	_dispatch(command + " &>/dev/null\necho $? > " + mailbox + \
						"\necho " + index + " >> " + mailbox);
}

//--------------------------------------------------------------------
// Procedure: system_call_dispatch_pipe()
//   Purpose: Wrapped dispatch call that saves the piped results
//   Returns:
//      Note: A bash script such as "A > B" will clear B before A begins and
//			  		write to it after A ends. The buffer file will therefore be empty
//			  		for a nontrivial period of time. By saving the data to an array
//			  		as the call goes and then only writing to the file once the call
//			  		returns, the time that the file is empty for is greatly reduced
//
//						It is crucial to write the index after the data; this is the app's
//						signal that the data is new. Doing it in the other order will
//						cause long gaps where useful data is not displayed in the app//
//
// 						Code is commented out because decision was made to buffer in
//						variables instead of bash files. It seems like buffering in those
//						bash files *should* be simpler, since we get to leverage UNIX
//						infrastructure, but that wasn't my experience.
//
// 						I think it may be a fruitful line of inquiry at some point in the
//						future, so I have left it as-is.

void system_call_dispatch_pipe(string command,
															 string mailbox,
															 string index,
															 int timeout) {
	// string prepare = "declare -a BUF\n";

	// string timer = "";
	// if (timeout > 0) timer = string("\nTASK=$!\n") + \
	// 						 string("sleep ") + to_string(timeout) + "\n" + \
	// 						 string("kill $TASK");

	// // buffer the new data
	// string do_buffer = string("while IFS= read -r -a B\n") + \
	// 				   string("do BUF[${#BUF[@]}]=B\n") + \
	// 				   string("done\n <<< " + command + "\n");

	// // now with all data in hand, clear old file and write to it
	// string do_write = string("print \"\" > " + mailbox + "\n") + \
	// 				  string("for LINE in ${BUF[@]}\n") + \
	// 				  string("do echo $LINE >> " + mailbox + "\n") + \
	// 				  string("done\n");

	// string cleanup = "unset buffer";

	// _dispatch(" " + prepare + do_buffer + do_write + cleanup + " "/*, timer*/);
	_dispatch(command + " > " + mailbox + "\necho " + index + " >> " + mailbox);
}

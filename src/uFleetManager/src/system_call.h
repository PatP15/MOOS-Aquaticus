/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: system_call.h                               				 */
/*    DATE: March 8th 2017                                       */
/*****************************************************************/

#ifndef WRAPPEDSYSTEMCALL
#define WRAPPEDSYSTEMCALL

#include <string>

int system_call(std::string);
void _dispatch(std::string, std::string="");

void system_call_dispatch_return(std::string, std::string, std::string);
void system_call_dispatch_pipe(std::string, std::string, std::string, int=-1);

#endif
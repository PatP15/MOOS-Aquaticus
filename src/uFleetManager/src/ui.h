/*****************************************************************/
/*    NAME: Raphael Segal	                                       */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: ui.h                                 								 */
/*    DATE: March 30th 2017                                      */
/*****************************************************************/

#include "Configuration.h" // definitely included from other headers...
#include <string>
#include <vector>
#include <map>
#include <ctime>

struct TimestampedCommand {
	std::string summary;
	std::string command;
	time_t timestamp;
};

class UI {
public: // Constructor and Destructor
	UI(Configuration);
	~UI() {};
public: // Public methods
	void buildUp ();
	void tearDown ();
	void loop();
protected: // Helper methods
	void setTableFormats();
	TimestampedCommand timeStampCommand(std::pair<std::string, std::string>);
	std::string formatCommandTime(time_t);
	std::string accumulateStatus(std::vector<std::string>,
															 std::vector<std::string>,
															 std::vector<std::string>,
															 std::string);
	bool machineIsFiltered(std::vector<bool>);
protected: // Action methods
	void checkMachineMail();
	void actOnKeyPress(int);
protected: // Render methods
	int printWindow(int);
	int printKeyFeed(int, int);
	int printHelpText(int);
	int printComputerInfo(int);
protected: // Member variables (general)
	Configuration m_config;
	std::vector<ManagedMoosMachine> m_machines;
	bool m_keep_alive;
protected: // Member variables (input)
	std::string m_key_feed; // accumulation of keystrokes, for matching commands
protected: // Member variables (output)
	std::map<std::string, std::vector<std::vector<std::string> > > m_headers;
	std::vector<std::string> m_help_headers;
	std::string m_view;
	int m_padding_size;
	bool m_view_full_help;
	bool m_is_commanding;
	std::vector<TimestampedCommand> m_command_history;
	std::map<std::string,
				 std::vector<std::pair<std::string, std::string> > > m_help;
	time_t m_last_status_request;
	time_t m_last_mail_check_request;
	bool m_filtering_by_liveness;
};
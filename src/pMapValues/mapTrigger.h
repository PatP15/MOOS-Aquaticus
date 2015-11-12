/*
 * mapTrigger.h
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#include <iostream>
#include "MOOS/libMOOS/MOOSLib.h"

#ifndef MAPTRIGGER_H_
#define MAPTRIGGER_H_

#ifndef BAD_DOUBLE
#define BAD_DOUBLE -99999.99
#endif

class mapTrigger {
public:
    mapTrigger();
	mapTrigger(MOOS::MOOSAsyncCommClient* pComms, std::string sDef);
	~mapTrigger() {}


	bool StoreValueThenPublish(std::string sVal);
	bool StoreValueThenPublish(double dVal);
	unsigned int CountPublished() { return m_countNotified; }
	std::string GetAppCastMsg();
	bool IsValid() { return m_error.empty(); }
	std::string GetError() { return m_error; }
	std::string GetKey() { return m_inName; }
	std::string GetLastValue() { return m_lastVal; }
	std::string GetAppCastStatusString();

private:
	void        PrepAppCastMsg();
    bool ayTokParse(const std::string& str, const std::string& left, char gsep, char lsep, std::string& rstr);

	std::string m_lastVal;
	std::string m_inName;
	std::string m_triggerValue;
	std::string m_outName;
	std::string m_outString;
	double      m_outDouble;
	bool        m_bOutIsDouble;
	unsigned int m_countNotified;
	std::string m_appCastPrep;
	std::string m_error;

	MOOS::MOOSAsyncCommClient* m_pComms;

};







#endif















//

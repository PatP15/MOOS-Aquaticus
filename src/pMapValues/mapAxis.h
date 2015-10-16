/*
 * mapAxis.h
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#ifndef PMAPVALUES_MAPAXIS_H_
#define PMAPVALUES_MAPAXIS_H_

#include <iostream>
#include "MOOS/libMOOS/MOOSLib.h"

#ifndef BAD_DOUBLE
#define BAD_DOUBLE -99999.99
#endif

class mapAxis {
public:
	mapAxis();
	mapAxis(MOOS::MOOSAsyncCommClient* pComms, std::string sDef);
	~mapAxis() {}

	bool        MapAndNotify(double dIn);
	double      MapInToOut(double dIn);
	bool        AxisNotify(double dMappedAlready);
	std::string	PublishAs() { return m_outName; }
	unsigned int CountPublished() { return m_countNotified; }
	std::string GetAppCastMsg();
	bool        IsValid() { return m_error.empty(); }
	std::string	GetError() { return m_error; }
	std::string GetKey() { return m_inName; }

private:
	void        SetConstraints();
	void        PrepAppCastMsg();

	unsigned int m_countNotified;

	std::string m_inName;
	double		m_inMin;
	double		m_inMax;
	double		m_inConstrainMin;
	double		m_inConstrainMax;

	std::string	m_outName;
	double		m_outMin;
	double		m_outMax;
	std::string m_appCastPrep;
	std::string	m_error;

	MOOS::MOOSAsyncCommClient* m_pComms;
};

#endif










//

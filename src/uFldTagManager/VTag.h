/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: DepthCharge.h                                        */
/*    DATE: Sep 22nd, 2012                                       */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/

#ifndef UFLD_DEPTH_CHARGE_RECORD_HEADER
#define UFLD_DEPTH_CHARGE_RECORD_HEADER

#include <vector>
#include <string>
#include <map>

class DepthCharge 
{
 public:
  DepthCharge();
  virtual ~DepthCharge() {};

  void  setVName(std::string str) {m_vname=str;};
  void  setX(double x)            {m_x=x; m_x_set=true;};
  void  setY(double y)            {m_y=y; m_y_set=true;};
  void  setXY(double x, double y) {m_x=x; m_y=y; m_x_set=true; m_y_set=true;};
  void  setTimeLaunched(double t) {m_time_launched=t;};
  void  setTimeDelay(double t)    {m_time_delay=t;};
  void  setRange(double r)        {m_range=r;};

  std::string  getVName() const         {return(m_vname);};
  double       getX() const             {return(m_x);};
  double       getY() const             {return(m_y);};
  double       getTimeLaunched() const  {return(m_time_launched);};
  double       getTimeDelay() const     {return(m_time_delay);};
  double       getRange() const         {return(m_range);};

  bool  valid() const;

 protected: // State variables

  std::string  m_vname;
  double       m_x;
  double       m_y;
  double       m_time_launched;
  double       m_time_delay;
  double       m_range;

  bool         m_x_set;
  bool         m_y_set;
};

#endif 


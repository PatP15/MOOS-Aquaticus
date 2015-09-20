/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: DepthCharge.cpp                                      */
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

#include "DepthCharge.h"

using namespace std;

//------------------------------------------------------------
// Constructor

DepthCharge::DepthCharge()
{
  m_x = 0;
  m_y = 0;
  m_time_launched = 0;
  m_time_delay    = 0;
  m_range         = -1;

  m_x_set = false;
  m_y_set = false;
}

//------------------------------------------------------------
// Procedure: valid()

bool DepthCharge::valid() const
{
  if(!m_x_set || !m_y_set)
    return(false);
  if(m_time_launched <= 0)
    return(false);
  if(m_time_delay <= 0)
    return(false);
  if(m_range <= 0)
    return(false);
  if(m_vname == "")
    return(false);
  return(true);
}

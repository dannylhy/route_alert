/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Georgia Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Qi Zhang <qzhang90@gatech.edu>
 */

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "bs-routing-table.h"
#include <vector>
#include <boost/lexical_cast.hpp>

#define EXPIRETIME 10

using namespace std;

NS_LOG_COMPONENT_DEFINE ("BSRoutingTable");

namespace ns3{
 namespace bs{
	TypeId BufferAndSwitchRoutingTable::GetTypeId ()
	{
		static TypeId tid = TypeId ("ns3::bs::BufferAndSwitchRoutingTable")
			            .SetParent<Object> ()
				    .AddConstructor<BufferAndSwitchRoutingTable> ()
				    ;	
		return tid;
	}

	BufferAndSwitchRoutingTable::BufferAndSwitchRoutingTable ()
	{
		m_entryExpireTime = EXPIRETIME;
	}

	BufferAndSwitchRoutingTable::~BufferAndSwitchRoutingTable ()
	{
	}

	void BufferAndSwitchRoutingTable::UpdateMyCurrentPos (uint64_t posx, uint64_t posy)
	{
		m_myCurrentPosx = posx;
		m_myCurrentPosy = posy;
	}

        void BufferAndSwitchRoutingTable::UpdateRoute (Ipv4Address addr, uint64_t posx, uint64_t posy, std::vector<char> currentRoad)
	{
		
		for (std::vector<BSRoutingTableEntry>::iterator it = m_bsTable.begin (); it != m_bsTable.end (); it++)
		{
			time_t currentTime;
			time (&currentTime);
			NS_LOG_FUNCTION (currentTime);
			//delete legancy entry
			/*
			if ( (currentTime - it->timeStamp) > m_entryExpireTime )
			{
				m_bsTable.erase (it);
				return;
			}
			*/	
			//if find, update routing table
			if ((it->addr).IsEqual (addr))
			{
				it->posx = posx;
				it->posy = posy;
				it->currentRoad = currentRoad;
				time (&(it->timeStamp));
				return;
			}
		}
		
		//if not find, then add
		BSRoutingTableEntry bsrEntry;
		bsrEntry.addr = addr;
		bsrEntry.posx = posx;
		bsrEntry.posy = posy;
		bsrEntry.currentRoad = currentRoad;
		time (&(bsrEntry.timeStamp));
		
		m_bsTable.push_back (bsrEntry);
		
	}
       
	Ipv4Address BufferAndSwitchRoutingTable::LookupRoute (std::vector<char> currentRoad)
	{
		std::vector<Ipv4Address> ipv4AddressVector;
		Ipv4Address addr;

		double minDistance = std::numeric_limits<double>::max();

		for (uint32_t i = 0; i < m_bsTable.size (); i++)
		{
			if ( (m_bsTable [i].currentRoad [0] == currentRoad [0]) && ( m_bsTable [i].currentRoad [1] == currentRoad [1]) )
			{
				double dist = GetDistance (m_myCurrentPosx, m_bsTable [i].posx, m_myCurrentPosy, m_bsTable [i].posy);
				if (dist < minDistance)
				{
					minDistance = dist;
					addr = m_bsTable [i] .addr;
				}
			}
			
		}
	
		return addr;		
	}
 }
}

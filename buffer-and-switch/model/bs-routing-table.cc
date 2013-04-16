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

#define EXPIRETIME 5

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


	/*bool BufferAndSwitchRoutingTable::Compare (std::vector<char> vector1, std::vector<char> vector2)
	{
		if (vector1.size () != vector2.size ())
		{
			return false;
		}
	
		for (uint32_t i = 0; i < vector1.size (); i++)
		{
			if (vector1[i] != vector2[i])
			{
				return false;
			}
		}

		return true;
	}*/

        void BufferAndSwitchRoutingTable::UpdateRoute (Ipv4Address addr, uint64_t posx, uint64_t posy, std::string currentRoad, uint32_t id)
	{
		for (std::vector<BSRoutingTableEntry>::iterator it = m_bsTable.begin (); it != m_bsTable.end (); it++)
		{
			//delete legancy entry
			
			if ( (ns3::Simulator::Now().GetSeconds () - it->timeStamp.GetSeconds ()) > m_entryExpireTime )
			{
				m_bsTable.erase (it);
				NS_LOG_WARN ("delete legancy entry");
				return;
			}
				
			//if find, update routing table
			if ((it->addr).IsEqual (addr))
			{
				it->posx = posx;
				it->posy = posy;
				it->currentRoad = currentRoad;
				it->timeStamp = ns3::Simulator::Now();
				it->id = id;
				NS_LOG_WARN ("update existing entry, ip = " << addr << " currentRoad = " << currentRoad << " time = " << ns3::Simulator::Now().GetSeconds ());
				return;
			}
		}
		
		//if not find, then add
		BSRoutingTableEntry bsrEntry;
		bsrEntry.addr = addr;
		bsrEntry.posx = posx;
		bsrEntry.posy = posy;
		bsrEntry.currentRoad = currentRoad;
		bsrEntry.timeStamp = ns3::Simulator::Now();
		bsrEntry.id = id;
		
		m_bsTable.push_back (bsrEntry);
		NS_LOG_WARN ("insert new entry, ip = " << addr << " currentRoad = " << currentRoad << " time = " << ns3::Simulator::Now().GetSeconds ());
	}
       
	Ipv4Address BufferAndSwitchRoutingTable::LookupRoute (std::string currentRoad, uint64_t myCurrentPosx, uint64_t myCurrentPosy,uint32_t id)
	{
		Ipv4Address addr("0.0.0.0");

		double minDistance = std::numeric_limits<double>::max();
		int num = 0;
		for (uint32_t i = 0; i < m_bsTable.size (); i++)
		{
			if (m_bsTable[i].id == id)
			{
				num++;
			}
		}
		std::cout << "LookupRoute : routing table has " << num << " entries" << std::endl; 
		for (uint32_t i = 0; i < m_bsTable.size (); i++)
		{
			
			if ( (m_bsTable[i].id == id) && (m_bsTable[i].currentRoad.compare (currentRoad)) )
			{
				double dist = GetDistance (myCurrentPosx, m_bsTable [i].posx, myCurrentPosy, m_bsTable [i].posy);
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

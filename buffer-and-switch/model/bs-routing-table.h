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
 * Author: Qi Zhang <qzhang90@gatech.edu>>
 */

#ifndef BUFFER_AND_SWITCH_ROUTING_TABLE_H
#define BUFFER_AND_SWITCH_ROUTING_TABLE_H

#include "ns3/node.h"
#include "ns3/ipv4-route.h"
#include "ns3/output-stream-wrapper.h"
#include <list>
#include <vector>
#include <time.h>
#include "ns3/simulator.h"

namespace ns3{
 namespace bs{
	class BufferAndSwitchRoutingTable : public Object
	{
	private:	
		typedef struct
		{
			Ipv4Address addr;
			uint64_t posx;
			uint64_t posy;
			std::string currentRoad;
			Time timeStamp;
		}BSRoutingTableEntry; 
		
		std::vector<BSRoutingTableEntry> m_bsTable;
		int64_t m_entryExpireTime;
		bool m_isEmergencyV;
		uint64_t m_myCurrentPosx;
		uint64_t m_myCurrentPosy;;
	public:
		BufferAndSwitchRoutingTable ();
		virtual ~BufferAndSwitchRoutingTable ();
		
		static TypeId GetTypeId();
		void UpdateRoute (Ipv4Address addr, uint64_t posx, uint64_t posy, std::string currentRoad);
		Ipv4Address LookupRoute (std::string currentRoad);
		void UpdateMyCurrentPos (uint64_t posx, uint64_t posy);

		void SetMyCurrentPosx (uint64_t posx)
		{
			m_myCurrentPosx = posx;
		}

		uint64_t GetMyCurrentPosx (void)
		{
			return m_myCurrentPosx;
		}

		void SetMyCurrentPosy (uint64_t posy)
		{
			m_myCurrentPosy = posy;
		}

		uint64_t GetMyCurrentPosy (void)
		{
			return m_myCurrentPosy;
		}

		void SetIsEmergencyV (bool isEmergencyV)
		{
			m_isEmergencyV = isEmergencyV;
		}
		
		bool GetIsEmergencyV (void)
		{
			return m_isEmergencyV;
		}
	private:
		double GetDistance (uint64_t posx1, uint64_t posx2, uint64_t posy1, uint64_t posy2)
		{
			double i = pow ((posx1 - posx2), 2) + pow ((posy1 - posy2), 2);
			return sqrt (i);
		}
		
		
	};	
 }
}

#endif //BUFFER_AND_SWITCH_ROUTING_TABLE_H

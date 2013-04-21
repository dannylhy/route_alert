/* * -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef BUFFER_AND_SWITCH_ROUTING_H
#define BUFFER_AND_SWITCH_ROUTING_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/nstime.h"
#include "bs-routing-table.h"
#include "bs-routing-packet.h"
#include <list>

namespace ns3{
 namespace bs{
	class BufferAndSwitchRouting: public Ipv4RoutingProtocol
	{
		public:
			static TypeId GetTypeId(void);
			static const uint32_t BS_PORT;

			BufferAndSwitchRouting ();
			virtual ~BufferAndSwitchRouting();

			//Below are from Ipv4RoutingProtocol
	  		virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);

  			virtual bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                           		 	 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                           	 	 	 LocalDeliverCallback lcb, ErrorCallback ecb);
  			virtual void NotifyInterfaceUp (uint32_t interface);
  			virtual void NotifyInterfaceDown (uint32_t interface);
  			virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  			virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  			virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  			virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const;	
			
			void SetRtable (Ptr<BufferAndSwitchRoutingTable> ptr);
			
			void SetDownTarget (IpL4Protocol::DownTargetCallback cb);
			void HelloTimerExpire (void);
			IpL4Protocol::DownTargetCallback GetDownTarget (void) const;
		private:
			Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr) const;
			void SendPkt (Ipv4Address dst, MessageType type);
			void BSRecv (Ptr<Socket> socket);
			uint32_t GetNodeId ();
			bool OnThePath (std::string currentRoad, std::string path);
			void SendAlertPacket ();
			std::string GetNextRoad (std::string path);

			int GetAngle (double x, double y)
			{
				if (x == 0 && y == 0)
				{
					return 0;
				}
				double result = atan2 (y,x) * 180 / (3.14159265);
				if (result >= -45 && result <= 45)
				{
					return 1;
				}else if (result <= -45 && result >= -135)
				{
					return 2;
				}else if (result >= 45 && result <= 135)
				{
					return 4;
				}else {
					return 3;
				}

			}
			std::string GetPreviousIntersection (std::string currentRoad)
			{
				return currentRoad.substr (1, 2);
			}
			std::string GetNextIntersection (std::string currentRoad)
			{
				return currentRoad.substr (3, 2);
			}
		private:
			Ptr<BufferAndSwitchRoutingTable> m_rtable;
			Ipv4Address m_address;
			Ptr<Ipv4> m_ipv4;
			uint32_t m_ifaceId;
			std::map< Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
			Ipv4InterfaceAddress m_iface;
			Timer m_helloIntervalTimer;
			Time m_helloInterval;
			IpL4Protocol::DownTargetCallback m_downTarget;
			int m_alertSent;
			std::string m_currentRoad;
			uint32_t m_velocityx;
			uint32_t m_velocityy;
			uint32_t m_last_alert;
	};
 }
}

#endif  //BUFFER_AND_SWITCH_ROUTING_H


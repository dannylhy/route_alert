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

#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/pointer.h"
#include "ns3/double.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/mobility-model.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable.h"
#include "bs-routing.h"
#include "bs-routing-table.h"
#include <iostream>
#include <fstream>

NS_LOG_COMPONENT_DEFINE ("BSRouting");

namespace ns3{
 namespace bs{
	#define FIRST_JITTER (Seconds (1))
 
	NS_OBJECT_ENSURE_REGISTERED (BufferAndSwitchRouting);
	
	//UDP Port for BS control traffic
	const uint32_t BufferAndSwitchRouting::BS_PORT = 888;

	TypeId BufferAndSwitchRouting::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::bs::BufferAndSwitchRouting")
				    .SetParent<Ipv4RoutingProtocol>()
				    .AddConstructor<BufferAndSwitchRouting>()
			    	    .AddAttribute ("RoutingTable", "Pointer to Routing Table",
						  PointerValue (),
						  MakePointerAccessor (&BufferAndSwitchRouting::SetRtable),
					 	  MakePointerChecker<BufferAndSwitchRoutingTable>())
					  	  ;
		return tid;
	}


	BufferAndSwitchRouting::BufferAndSwitchRouting ()
	{
		NS_LOG_FUNCTION_NOARGS ();
		m_helloIntervalTimer = Timer::CANCEL_ON_DESTROY;
		m_helloInterval = Seconds (1);	
	}
		
	BufferAndSwitchRouting::~BufferAndSwitchRouting ()
	{
		NS_LOG_FUNCTION_NOARGS ();
	}

	void BufferAndSwitchRouting::SetRtable (Ptr<BufferAndSwitchRoutingTable> ptr)
	{
		NS_LOG_FUNCTION (ptr);
		m_rtable = ptr;	
	}

	void BufferAndSwitchRouting::HelloTimerExpire ()
	{
		Ipv4Address dst = m_iface.GetBroadcast ();
		SendPkt (dst, MSG_HELLO);
		m_helloIntervalTimer.Cancel ();
		m_helloIntervalTimer.Schedule (m_helloInterval);
	}

	Ptr<Socket> BufferAndSwitchRouting::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr) const
	{
		NS_LOG_FUNCTION (addr);
		for (std::map< Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
		     j != m_socketAddresses.end (); j++)
		{
			Ptr<Socket> socket = j->first;
			Ipv4InterfaceAddress iface = j->second;
			if (iface == addr)
			{
				return socket;
			}
		}
		Ptr<Socket> socket;
		return socket;
	}

	void BufferAndSwitchRouting::BSRecv (Ptr<Socket> socket)
	{
		
		NS_LOG_FUNCTION (socket);
		
		Address sourceAddress;
		Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
		InetSocketAddress inetSourceAddress = InetSocketAddress::ConvertFrom (sourceAddress);
		Ipv4Address src = inetSourceAddress.GetIpv4 ();
	
		TypeHeader tHeader (MSG_INIT);
		packet->RemoveHeader (tHeader);
	
		switch (tHeader.GetType ())
		{
			case MSG_HELLO:
			{
				NS_LOG_FUNCTION ("receive an hello pkt");
				BSHeader bHeader;
				packet->RemoveHeader (bHeader);
				m_rtable->UpdateRoute (src, bHeader.GetPosx (), bHeader.GetPosy (), bHeader.GetCurrentRoad ());
				break;
			}
			case MSG_RREQ:
			{
				NS_LOG_FUNCTION ("receive an rreq pkt");
	
				//To be added;
	
				break;
			}
			case MSG_ALERT:
			{
				NS_LOG_FUNCTION ("receive an alert pkt");

				
				Ptr<Packet> tmpPacket = packet->Copy ();
				BSHeader bHeader;
				tmpPacket->RemoveHeader (bHeader);
				Ipv4Address dst = m_rtable->LookupRoute (bHeader.GetCurrentRoad ());

				if (!Ipv4Address ("102.102.102.102").IsEqual (dst))
				{
					SendPkt (dst, MSG_ALERT);	
				}	


				
				break;
			}
			default:
			{
			}
		}
	}

	void BufferAndSwitchRouting::SendPkt (Ipv4Address dst, MessageType type)
	{
		NS_LOG_FUNCTION (dst << type);
		double posx, posy;
		Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
		
		NS_LOG_DEBUG ("m_socketAddresses.size () = " << m_socketAddresses.size ());
		switch (type)
		{
			case MSG_ALERT:
			{
				
				posx = MM->GetPosition ().x;
				posy = MM->GetPosition ().y;
				
				//posx = 100;
				//posy = 100;

				std::vector<char> currentRoad; //Should get from map info;
				
				for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::const_iterator j = m_socketAddresses.begin ();
		     		j != m_socketAddresses.end (); j++)
				{
					Ptr<Socket> socket = j->first;
					Ipv4InterfaceAddress iface = j->second;
						
					BSHeader bHeader;
					bHeader.SetPosx ((uint64_t) posx);
					bHeader.SetPosy ((uint64_t) posy);
					bHeader.SetCurrentRoad (currentRoad);
					
					TypeHeader tHeader (MSG_ALERT);
					
					Ptr<Packet> packet = Create<Packet> ();
					packet->AddHeader (bHeader);
					packet->AddHeader (tHeader);
					
	
					int ret = socket->SendTo (packet, 0, InetSocketAddress (dst, BS_PORT));	
					NS_LOG_DEBUG ("SendTo ret " << ret);
				}
				break;
			}	
			case MSG_HELLO:
			{
				
				posx = MM->GetPosition ().x;
				posy = MM->GetPosition ().y;
				
				//posx = 100;
				//posy = 100;

				std::vector<char> currentRoad; //Should get from map info;
				//std::string road = GetRoad (GetNodeId ());
				std::string road;
				if (road.length () != 0)
				{
					currentRoad.reserve(currentRoad.size() + road.size()); // may want to avoid unneeded resizes
					std::copy(road.begin(), road.end(), back_inserter(currentRoad));
				}				

				for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::const_iterator j = m_socketAddresses.begin ();
		     		j != m_socketAddresses.end (); j++)
				{
					Ptr<Socket> socket = j->first;
					Ipv4InterfaceAddress iface = j->second;
						
					BSHeader bHeader;
					bHeader.SetPosx ((uint64_t) posx);
					bHeader.SetPosy ((uint64_t) posy);
					
					bHeader.SetCurrentRoad (currentRoad);
					
					Ptr<Packet> packet = Create<Packet> ();
					packet->AddHeader (bHeader);
				
			
					TypeHeader tHeader (MSG_HELLO);
					packet->AddHeader (tHeader);
				
	
					m_rtable->UpdateMyCurrentPos (bHeader.GetPosx (), bHeader.GetPosy ());	
					int ret = socket->SendTo (packet, 0, InetSocketAddress (dst, BS_PORT));	
					NS_LOG_DEBUG ("SendTo ret " << ret);
				}
				break;
			}
			case MSG_RREQ://RREQ packet only has tHeader
			{
				for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::const_iterator j = m_socketAddresses.begin ();
		     		j != m_socketAddresses.end (); j++)
				{
					Ptr<Socket> socket = j->first;
					Ipv4InterfaceAddress iface = j->second;
					
					Ptr<Packet> packet = Create<Packet> ();					

					TypeHeader tHeader (MSG_RREQ);
					packet->AddHeader (tHeader);
					
					socket->SendTo (packet, 0, InetSocketAddress (dst, BS_PORT));
				}	
				break;
			}
			default:
			{	
			}
		}
	}

	void BufferAndSwitchRouting::SetIpv4 (Ptr<Ipv4> ipv4)
	{
                NS_LOG_FUNCTION (ipv4);
		m_ipv4 = ipv4;
		m_helloIntervalTimer.SetFunction (&BufferAndSwitchRouting::HelloTimerExpire, this);
		m_helloIntervalTimer.Schedule (FIRST_JITTER);
	}

	void BufferAndSwitchRouting::NotifyInterfaceUp (uint32_t interface)
	{
                NS_LOG_FUNCTION (this << interface);
		return;
	}
	
	void BufferAndSwitchRouting::NotifyInterfaceDown (uint32_t interface)
	{
  		NS_LOG_FUNCTION (this << interface);
		return;
	}
	
	void BufferAndSwitchRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
	{
                NS_LOG_FUNCTION (this << interface << address);
		Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
		m_address = address.GetLocal ();
		NS_LOG_DEBUG ("m_address = " << m_address);
		NS_LOG_DEBUG ("l3->GetNAddresses (interface) = " << l3->GetNAddresses (interface));
		if (l3->GetNAddresses (interface) == 1)
		{
			Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
			if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
			{
				return;
			}

			Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
			NS_ASSERT (socket != 0);
			socket->SetRecvCallback (MakeCallback (&BufferAndSwitchRouting::BSRecv, this));
			socket->BindToNetDevice (l3->GetNetDevice (interface));
			socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), BS_PORT)); 
			socket->SetAllowBroadcast (true);
			socket->SetAttribute ("IpTtl", UintegerValue (128));
			m_socketAddresses.insert (std::make_pair (socket, iface));

			m_ifaceId = interface;
			m_iface = iface;
			
		}else{
			NS_LOG_LOGIC ("Buffer-And-Switch does not work with more than one address per each interface. Ignore added address");
		}
	}

	void BufferAndSwitchRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
	{
		NS_LOG_FUNCTION (this << interface << address);
		Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
		NS_ASSERT (socket);
		m_socketAddresses.erase (socket);
	}
	
	void BufferAndSwitchRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
	{
		NS_LOG_FUNCTION (this << stream);
		return;
	}

	Ptr<Ipv4Route> BufferAndSwitchRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
	{
		NS_LOG_FUNCTION (this << m_address << " " << header.GetSource ()<< "->" << header.GetDestination ());
		
		Ptr<Packet> packet = p->Copy ();
                TypeHeader tHeader (MSG_INIT);
		packet->RemoveHeader (tHeader);
	
		if (tHeader.GetType () == MSG_HELLO)
		{
			//Hello Packets are boradcast, do not need route->SetGateway(...)
			NS_LOG_DEBUG ("Output: route a hello pkt");
			Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  			route->SetSource (m_address);
  			route->SetDestination (header.GetDestination ());
  			route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));

  			sockerr = Socket::ERROR_NOTERROR;

  			return route;
			
		}else if (tHeader.GetType () == MSG_RREQ)
		{
			NS_LOG_DEBUG ("Output: route a rreq pkt");

			// To be added;
	
		}else if (tHeader.GetType () == MSG_ALERT)
		{
			NS_LOG_DEBUG ("Output: route an alert pkt");
			GetNodeId ();
			Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  			route->SetGateway (header.GetDestination ());
  			route->SetSource (m_address);
  			route->SetDestination (header.GetDestination ());
  			route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));

  			sockerr = Socket::ERROR_NOTERROR;

  			return route;
		}else if (m_address.IsEqual ("10.1.1.1")){
			//Trigger the Alert Packet
			NS_LOG_DEBUG ("Emergency Vehicle starts: sending an ALERT pkt");
			Ipv4Address dst = header.GetDestination ();
			SendPkt (dst, MSG_ALERT);
		}

		return 0;
	}
	
	bool BufferAndSwitchRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                                 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                                 LocalDeliverCallback lcb, ErrorCallback ecb)
	{
		NS_LOG_FUNCTION (this << header.GetSource () << "->" << header.GetDestination ());
		NS_ASSERT (m_ipv4 != 0);
		NS_ASSERT (p != 0);
		NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);

		int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
		Ipv4Address dst = header.GetDestination ();
		Ipv4Address src = header.GetSource ();

		/*			
		NS_LOG_FUNCTION ("packet size = " << p->GetSize ());
		Ptr<Packet> packet = p->Copy ();	
		char type;
		type =*(reinterpret_cast<char *>(PeekPointer(packet)) + 8);
		NS_LOG_FUNCTION (type);
		std::cout << std::hex << type << std::endl;
		std::filebuf fb;
		fb.open ("test.txt", std::ios::out);
		std::ostream os (&fb);
		p->Print (os);
		fb.close ();
		*/

		if (m_ipv4->IsDestinationAddress (dst, iif))
		{
			NS_LOG_FUNCTION (this << "I'm the destination");
			lcb (p, header, iif);
			return true;
		}else{
			NS_LOG_FUNCTION (this << " I'm not the destination");
			return true;
		}	
		return true;
	}

	void BufferAndSwitchRouting::SetDownTarget (IpL4Protocol::DownTargetCallback cb)
	{
		m_downTarget = cb;
	}

        
	IpL4Protocol::DownTargetCallback BufferAndSwitchRouting::GetDownTarget (void) const
	{
		return m_downTarget;
	}

	uint32_t BufferAndSwitchRouting::GetNodeId ()
	{
		uint8_t buf[4];
		m_address.Serialize (buf);
		return buf[3];
	}
 }
}

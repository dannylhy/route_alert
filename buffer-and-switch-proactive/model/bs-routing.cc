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
#include "ns3/random-variable-stream.h"
#include "bs-routing.h"
#include "bs-routing-table.h"
#include "location.h"
#include <iostream>
#include <fstream>

std::string ambPath = "L191314151617181206"; //Should get from map info;
ns3::Ipv4Address destination = ns3::Ipv4Address ("10.1.1.252");
ns3::Ipv4Address ambIP = ns3::Ipv4Address ("10.1.1.39");
uint32_t ambID = 38;
uint32_t pkt_send = 0;
uint32_t pkt_drop = 0;
uint32_t pkt_recv = 0;

#define EXPIRE_HELLO_PKT 100

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
		//NS_LOG_WARN_NOARGS ();
		m_helloIntervalTimer = Timer::CANCEL_ON_DESTROY;
		m_helloInterval = Seconds (1);
		m_alertSent = 0;
		m_velocityx = 0;
		m_velocityy = 0;
		//m_rtable = PointerValue (CreateObject<BufferAndSwitchRoutingTable> ());
	}
		
	BufferAndSwitchRouting::~BufferAndSwitchRouting ()
	{
		//NS_LOG_WARN_NOARGS ();
	}

	void BufferAndSwitchRouting::SetRtable (Ptr<BufferAndSwitchRoutingTable> ptr)
	{
		//NS_LOG_WARN (ptr);
		m_rtable = ptr;	
	}

	
	
	void BufferAndSwitchRouting::HelloTimerExpire ()
	{
		Ipv4Address dst = m_iface.GetBroadcast ();
		SendPkt (dst, MSG_HELLO);
		//If it is an activated amb, send alert pkt periodically
		if ( (GetNodeId () == ambID) && (m_alertSent != 0) )
		{
			SendAlertPacket ();
		}else if (-1 == m_alertSent)
		{
			//If there is buffered alert pkt && vehicle is still on the path, try to send it periodically
			std::string tmp = GetRoad (GetNodeId ());
			if (tmp.length() == 0) {
				//NS_LOG_WARN("No road returned. Not active. Skipping.");
				return;
			}
			if (tmp.at (0) != ':')
			{
				m_currentRoad = tmp;
			}
			
			SendAlertPacket ();
		}
		//std::cout << "ID's " << GetNodeId() << " HelloTimer expired at " << ns3::Simulator::Now().GetSeconds() << " Delay Left: " << m_helloIntervalTimer.GetDelayLeft().GetMilliSeconds() << endl;
		m_helloIntervalTimer.Cancel ();
		m_helloIntervalTimer.Schedule (m_helloInterval);
	}

	Ptr<Socket> BufferAndSwitchRouting::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr) const
	{
		//NS_LOG_WARN (addr);
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
	
	std::string BufferAndSwitchRouting::GetNextRoad (std::string path)
	{
		std::string tmp = GetRoad (GetNodeId ());

		if (tmp.length () == 0)
		{
			return "";
		}

		if (tmp.at (0) != ':')
		{
			m_currentRoad = tmp;
		}
		std::string nextIntersection = GetNextIntersection (m_currentRoad);
		
		uint32_t index = path.find (nextIntersection);
		if ((path.length () - index) > 3)
		{
			return ("L" + path.substr (index, 4));
		}else{
			return "last";
		}
	}

	bool BufferAndSwitchRouting::OnThePath (std::string currentRoad, std::string path)
	{
		if (path.find (currentRoad) == (size_t)(-1))
		{	
			return false;
		}else{
			return true;
		}
	}

	void BufferAndSwitchRouting::SendAlertPacket ()
	{
		Ptr<MobilityModel> MM1 = m_ipv4->GetObject<MobilityModel> ();
		NS_LOG_WARN (m_address << " posx = " << MM1->GetPosition ().x << " posy = " << MM1->GetPosition ().y);	
		if (!OnThePath (m_currentRoad.substr(1), ambPath))
		{
			m_alertSent = 1;
			pkt_drop ++;
		/*	if (GetNodeId () == ambID)
			{
				std::cout << "pkt drop : " << pkt_drop << std::endl;
				std::cout << "pkt send : " << pkt_send << std::endl;
			}*/
			return;
		}
		NS_LOG_WARN (m_address << " SendAlertPacket()");
		std::string nextRoad = GetNextRoad (ambPath);
		//NS_LOG_WARN ( "Got next road " << nextRoad);
		if (nextRoad.length () == 0)
		{
			return;
		}
		double posx, posy, velocityx, velocityy;
		Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();	
		posx = MM->GetPosition ().x;
	        posy = MM->GetPosition ().y;	
		velocityx = MM->GetVelocity ().x;
		velocityy = MM->GetVelocity ().y;
		if ((velocityx != 0) || (velocityy != 0))
		{
			m_velocityx = velocityx;
			m_velocityy = velocityy;
		}
		NS_LOG_WARN ("LookupRoute: currentRoad = " << m_currentRoad);	
		//NS_LOG_WARN("Lookuproute: velocityx = " << m_velocityx << " velocityy = " << m_velocityy);
		Ipv4Address dst = m_rtable->LookupRoute (m_currentRoad, (uint64_t)posx, (uint64_t)posy, GetNodeId (), GetAngle (m_velocityx, m_velocityy));
		if (!dst.IsEqual (Ipv4Address ("0.0.0.0")))
		{
			NS_LOG_WARN (m_address << " is on " << m_currentRoad << " forward alert pkt on current road dst ip = " << dst << " time = " << ns3::Simulator::Now().GetSeconds ());
			SendPkt (dst, MSG_ALERT);
			m_alertSent = 1;
		}
		else if (0 == nextRoad.compare ("last"))
		{
			NS_LOG_WARN (m_address << " hold the alert pkt on road " << m_currentRoad << " time = " << ns3::Simulator::Now().GetSeconds ());
			m_alertSent = -1;
		} 
		else
		{
			Ipv4Address dst = m_rtable->LookupRoute (nextRoad, (uint64_t)posx, (uint64_t)posy, GetNodeId (), -1);
			if (dst.IsEqual (Ipv4Address ("0.0.0.0")))
			{
				NS_LOG_WARN (m_address << " hold the alert pkt on road " << m_currentRoad << " time = " << ns3::Simulator::Now().GetSeconds ());
				m_alertSent = -1;
			}else{
				NS_LOG_WARN (m_address << " is on " << m_currentRoad << " forward alert pkt to nextroad " << nextRoad << " dst ip = " << dst << " time = " << ns3::Simulator::Now().GetSeconds ());
				SendPkt (dst, MSG_ALERT);
				m_alertSent = 1;
			}
		}
	}

	void BufferAndSwitchRouting::BSRecv (Ptr<Socket> socket)
	{
		
		//NS_LOG_WARN (socket);
		std::string tmp = GetRoad (GetNodeId ());
		//NS_LOG_WARN (m_address << " BSRecv : GetRoad = " << tmp << " ID = " << GetNodeId () << " time = " << ns3::Simulator::Now().GetSeconds());
		if (tmp.length () == 0)
		{
			return;
		}else{
			m_currentRoad = tmp;
		}
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
				/*if (GetNodeId() == ambID)
				{
					NS_LOG_WARN (m_address << " receive a hello pkt from " << src);
				}*/
				BSHeader bHeader;
				packet->RemoveHeader (bHeader);
				if ((ns3::Simulator::Now().GetSeconds() - bHeader.GetTime ()) > EXPIRE_HELLO_PKT)
				{
					NS_LOG_WARN ("************************************fuck************************");
					return;
				}
				if (!src.IsEqual (m_address))
				{
					//NS_LOG_WARN (m_address << "id = " << GetNodeId () << " update route entry ");
					/*if (GetNodeId() == ambID) 
					{ 
						NS_LOG_WARN (m_address << "id = " << GetNodeId () << " src: " << src << " CurrentRoad " << bHeader.GetCurrentRoad());
					}*/
					m_rtable->UpdateRoute (src, bHeader.GetPosx (), bHeader.GetPosy (), bHeader.GetCurrentRoad (), GetNodeId ());
				}
				break;
			}
			case MSG_RREQ:
			{
				NS_LOG_WARN ("receive a rreq pkt");
	
				//To be added;
	
				break;
			}
			case MSG_ALERT:
			{
				pkt_recv++;
				if (m_address.IsEqual (destination)) //destinatin of Amb
				{
					NS_LOG_WARN ("Alert packet reaches destination, stop forwarding");
					std::cout << "pkt drop : " << pkt_drop << std::endl;
					std::cout << "pkt send : " << pkt_send << std::endl;
					std::cout << "pkt recv : " << pkt_recv << std::endl;
					return;
				}
				
				NS_LOG_WARN (m_address <<" receive an alert pkt on road " << m_currentRoad);
				Ipv4Address dst;
				Ptr<Packet> tmpPacket = packet->Copy ();
				BSHeader bHeader;
				tmpPacket->RemoveHeader (bHeader);				
					
				/*if (!Ipv4Address ("102.102.102.102").IsEqual (dst))
				{
					NS_LOG_WARN ("Alert packet get destination****************");
					return;
				}*/

				SendAlertPacket ();
				break;
			}
			default:
			{
			}
		}
	}

	void BufferAndSwitchRouting::SendPkt (Ipv4Address dst, MessageType type)
	{
		double posx, posy;
		Ptr<MobilityModel> MM = m_ipv4->GetObject<MobilityModel> ();
		
		switch (type)
		{
			case MSG_ALERT:
			{
                                NS_LOG_WARN(m_address << " sends an alert packet " << " at time " << ns3::Simulator::Now().GetSeconds() << " to " << dst);

				posx = MM->GetPosition ().x;
				posy = MM->GetPosition ().y;
				
				for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::const_iterator j = m_socketAddresses.begin ();
		     		j != m_socketAddresses.end (); j++)
				{
					Ptr<Socket> socket = j->first;
					Ipv4InterfaceAddress iface = j->second;
						
					BSHeader bHeader;
					bHeader.SetPosx ((uint64_t) posx);
					bHeader.SetPosy ((uint64_t) posy);
					bHeader.SetCurrentRoad (ambPath);
					
					TypeHeader tHeader (MSG_ALERT);
					
					Ptr<Packet> packet = Create<Packet> ();
					packet->AddHeader (bHeader);
					packet->AddHeader (tHeader);
					
					socket->SendTo (packet, 0, InetSocketAddress (dst, BS_PORT));	
					pkt_send++;
					//NS_LOG_WARN ("ret of send alert = " << ret);
				}
				break;
			}	
			case MSG_HELLO:
			{
				posx = MM->GetPosition ().x;
				posy = MM->GetPosition ().y;
				
				std::string tmp = GetRoad (GetNodeId ());
                                if (tmp.length() == 0) {
                                    return;
                                }
				if (tmp.at (0) != ':')
				{
					m_currentRoad = tmp;
				}

				/*if (GetNodeId() == 251)
				{
					NS_LOG_WARN (m_address << " sends hello pkt, current road = " << m_currentRoad << " time = " << ns3::Simulator::Now ().GetSeconds ());
				}*/
				for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::const_iterator j = m_socketAddresses.begin ();
		     		j != m_socketAddresses.end (); j++)
				{
					Ptr<Socket> socket = j->first;
					Ipv4InterfaceAddress iface = j->second;
						
					BSHeader bHeader;
					bHeader.SetPosx ((uint64_t) posx);
					bHeader.SetPosy ((uint64_t) posy);
					
					bHeader.SetCurrentRoad (m_currentRoad);
					bHeader.SetTime ((uint64_t)(ns3::Simulator::Now().GetSeconds()));
	
					Ptr<Packet> packet = Create<Packet> ();
					packet->AddHeader (bHeader);
				
			
					TypeHeader tHeader (MSG_HELLO);
					packet->AddHeader (tHeader);
				
	
					m_rtable->UpdateMyCurrentPos (bHeader.GetPosx (), bHeader.GetPosy ());	
					socket->SendTo (packet, 0, InetSocketAddress (dst, BS_PORT));	
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
                NS_LOG_WARN (ipv4);
		m_ipv4 = ipv4;
		m_helloIntervalTimer.SetFunction (&BufferAndSwitchRouting::HelloTimerExpire, this);

		double minRand = 1;
		double maxRand = 999;
		Ptr<UniformRandomVariable> randStartTime= CreateObject<UniformRandomVariable>();
		randStartTime->SetAttribute("Min", DoubleValue(minRand));
		randStartTime->SetAttribute("Max", DoubleValue(maxRand));
		Time start_time = Seconds(1.0 + (randStartTime->GetValue()/1000.0));
		//m_helloIntervalTimer.Schedule (FIRST_JITTER);
		m_helloIntervalTimer.Schedule (start_time);

		//std::cout << "final Delay: " << m_helloIntervalTimer.GetDelayLeft().GetMilliSeconds() << endl;
	}

	void BufferAndSwitchRouting::NotifyInterfaceUp (uint32_t interface)
	{
                //NS_LOG_WARN (this << interface);
		return;
	}
	
	void BufferAndSwitchRouting::NotifyInterfaceDown (uint32_t interface)
	{
  		//NS_LOG_WARN (this << interface);
		return;
	}
	
	void BufferAndSwitchRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
	{
                //NS_LOG_WARN (this << interface << address);
		Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
		m_address = address.GetLocal ();
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
		//NS_LOG_WARN (this << interface << address);
		Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
		NS_ASSERT (socket);
		m_socketAddresses.erase (socket);
	}
	
	void BufferAndSwitchRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
	{
		//NS_LOG_WARN (this << stream);
		return;
	}

	Ptr<Ipv4Route> BufferAndSwitchRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
	{
		//NS_LOG_WARN (m_address << " " << header.GetSource ()<< "->" << header.GetDestination ());
		
		Ptr<Packet> packet = p->Copy ();
                TypeHeader tHeader (MSG_INIT);
		packet->RemoveHeader (tHeader);
	
		if (tHeader.GetType () == MSG_HELLO)
		{
			//Hello Packets are boradcast, do not need route->SetGateway(...)
			//NS_LOG_WARN ("Output: route a hello pkt");
			Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  			route->SetSource (m_address);
  			route->SetDestination (header.GetDestination ());
  			route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));

  			sockerr = Socket::ERROR_NOTERROR;

  			return route;
			
		}else if (tHeader.GetType () == MSG_RREQ)
		{
			//NS_LOG_WARN ("Output: route a rreq pkt");

			// To be added;
	
		}else if (tHeader.GetType () == MSG_ALERT)
		{
			//NS_LOG_WARN ("RouteOutput: route an alert pkt");
			Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  			route->SetGateway (header.GetDestination ());
			//NS_LOG_WARN ("RouteOutput: set next hop = " << header.GetDestination ());
  			route->SetSource (m_address);
			//NS_LOG_WARN ("RouteOutput: set source = " << m_address);
  			route->SetDestination (header.GetDestination ());
			//NS_LOG_WARN ("RouteOutput: set destination = " << header.GetDestination ());
  			route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));

  			sockerr = Socket::ERROR_NOTERROR;

  			return route;
		}else if (GetNodeId () == ambID){
			//Trigger the Alert Packet
			
			NS_LOG_WARN ("Emergency Vehicle starts: sending an ALERT pkt");
			std::string tmp = GetRoad (GetNodeId ());
			if (tmp.length () == 0)
			{
				NS_LOG_WARN ("Ams has not started yet");
				return 0;
			}else{
				m_currentRoad = tmp;
			}
 
			SendAlertPacket ();			
		}

		return 0;
	}
	
	bool BufferAndSwitchRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                                 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                                 LocalDeliverCallback lcb, ErrorCallback ecb)
	{
		//NS_LOG_WARN (m_address << " receives a packet, source = " << header.GetSource () << " destination = " << header.GetDestination () << " time = " << ns3::Simulator::Now().GetSeconds ());
		NS_ASSERT (m_ipv4 != 0);
		NS_ASSERT (p != 0);
		NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);

		int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
		Ipv4Address dst = header.GetDestination ();
		Ipv4Address src = header.GetSource ();

		/*			
		NS_LOG_WARN ("packet size = " << p->GetSize ());
		Ptr<Packet> packet = p->Copy ();	
		char type;
		type =*(reinterpret_cast<char *>(PeekPointer(packet)) + 8);
		NS_LOG_WARN (type);
		std::cout << std::hex << type << std::endl;
		std::filebuf fb;
		fb.open ("test.txt", std::ios::out);
		std::ostream os (&fb);
		p->Print (os);
		fb.close ();
		*/
		
		if (m_ipv4->IsDestinationAddress (dst, iif) || dst.IsEqual (Ipv4Address ("10.1.1.255")))
		{
			lcb (p, header, iif);
			return true;
		}else{
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
		return (buf[3] - 1); // IP Address starts from 1, which is node 0
	}
 }
}

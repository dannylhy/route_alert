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
#include "bs-routing.h"
#include "bs-routing-table.h"

NS_LOG_COMPONENT_DEFINE ("BSRouting");

namespace ns3{
   namespace bs{
        NS_OBJECT_ENSURE_REGISTERED (BufferAndSwitchRouting);

        //UDP Port for BS control traffic
        const uint32_t BufferAndSwitchRouting::BS_PORT = 888;

        TypeId BufferAndSwitchRouting::GetTypeId(void)
        {
                static TypeId tid = TypeId("ns3::BufferAndSwitchRouting")
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
        }

        BufferAndSwitchRouting::~BufferAndSwitchRouting ()
        {
        }

        void BufferAndSwitchRouting::SetRtable (Ptr<BufferAndSwitchRoutingTable> ptr)
        {
                m_rtable = ptr;
        }

        Ptr<Socket> BufferAndSwitchRouting::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr) const
        {
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
                Address sourceAddress;
                Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
                Ipv4Address src;
                src = src.ConvertFrom (sourceAddress);
                TypeHeader tHeader (MSG_HELLO);
                packet->RemoveHeader (tHeader);

                switch (tHeader.GetType ())
                {
                        case MSG_HELLO:
                        {
                                if (tHeader.GetType () == MSG_HELLO)
                                {
                                        BSHeader bHeader;
                                        packet->RemoveHeader (bHeader);
                                        m_rtable->UpdateRoute (src, bHeader.GetPosx (), bHeader.GetPosy (), bHeader.GetCurrentRoad ());
                                }
                                break;
                        }
                        case MSG_RREQ:
                        {
                                SendPkt (src, MSG_HELLO);
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
                        case MSG_HELLO:
                        {
                                posx = MM->GetPosition ().x;
                                posy = MM->GetPosition ().y;

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

                                        Ptr<Packet> packet = Create<Packet> ();
                                        packet->AddHeader (bHeader);


                                        TypeHeader tHeader (MSG_HELLO);
                                        packet->AddHeader (tHeader);

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
                m_ipv4 = ipv4;
        }

        void BufferAndSwitchRouting::NotifyInterfaceUp (uint32_t interface)
        {
                /*
                Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
                if (l3->GetNAddresses (interface) > 1)
                {
                        NS_LOG_WARN ("Buffer-And-Switch does not work with more than one address per each interface");
                }

                Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
                if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
                {
                        return;
                }

                Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node>, UdpSocketFactory::GetTypeId ());
                NS_ASSERT (socket != 0);
                socket->SetRecvCallback (MakeCallback (&BufferAndSwitchRouting::BSRecv, this));
                socket->BindToNetDevice (l3->GetNetDevice (interface));
                socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), BS_PORT)); 
                socket->SetAllowBroadcast (true);
                socket->SetAttribute ("IpTtl", UintegerValue (1));
                m_socketAddresses.insert (std::make_pair (socket, iface));
                */
                return;
        }

        void BufferAndSwitchRouting::NotifyInterfaceDown (uint32_t interface)
        {
                /*
                Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (interface, 0));
                NS_ASSERT (socket);
                m_socketAddresses.erase (socket);
                */
        }

        void BufferAndSwitchRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
        {
                Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
                if (!l3->IsUp (interface))
                {
                        return;
                }
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
                        socket->SetAttribute ("IpTtl", UintegerValue (1));
                        m_socketAddresses.insert (std::make_pair (socket, iface));

                        m_ifaceId = interface;
                        m_iface = iface;

                }else{
                        NS_LOG_LOGIC ("Buffer-And-Switch does not work with more than one address per each interface. Ignore added address");
                }
        }

        void BufferAndSwitchRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
        {
                Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
                NS_ASSERT (socket);
                m_socketAddresses.erase (socket);
        }

        void BufferAndSwitchRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream) const
        {
                return;
        }

        Ptr<Ipv4Route> BufferAndSwitchRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
        {
                Ptr<Packet> packet = p->Copy ();
                TypeHeader tHeader (MSG_HELLO);
                packet->RemoveHeader (tHeader);

                if (tHeader.GetType () == MSG_ALERT)
                {
                        BSHeader bHeader;
                        packet->RemoveHeader (bHeader);

                        Ipv4Address dst = m_iface.GetBroadcast ();
                        SendPkt (dst, MSG_RREQ);

                        //How to make sure that routing table has been updated? 
                        std::vector<Ipv4Address> relays = m_rtable->LookupRoute (bHeader.GetCurrentRoad ());
                        for (uint32_t i = 0; i < relays.size(); i++)
                        {
                                //return the route to the nearest node in relays?
                        }


                }
                /*Ipv4Address relay = m_rtable->LookupRoute (this->GetNextIntersection ());
                if (0 == relay)
                {
                        NS_LOG_DEBUG("Can't find route!!");
                }
                
                Ptr<Ipv4Route> route = Create<Ipv4Route> ();
                route->SetGateway (relay);
                route->SetSource (m_address);
                route->SetDestination (header.GetDestination ());
                route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));
                
                sockerr = Socket::ERROR_NOTERROR;
                
                return route;*/
                return 0;
        }

        bool BufferAndSwitchRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                                 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                                 LocalDeliverCallback lcb, ErrorCallback ecb)
        {
                NS_ASSERT (m_ipv4 != 0);
                NS_ASSERT (p != 0);
                NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);

                int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
                Ipv4Address dst = header.GetDestination ();
                Ipv4Address src = header.GetSource ();
                Ptr<Packet> packet = p->Copy ();
                TypeHeader tHeader (MSG_HELLO);
                packet->RemoveHeader (tHeader);

                if (m_ipv4->IsDestinationAddress (dst, iif))
                {
                        lcb (packet, header, iif);
                }else{
                        if (tHeader.GetType () == MSG_ALERT)
                        {
                                BSHeader bHeader;
                                packet->RemoveHeader (bHeader);
                                std::vector<char> currentRoad = bHeader.GetCurrentRoad ();
                                std::vector<Ipv4Address> relays = m_rtable->LookupRoute (currentRoad);
                                for (uint32_t i = 0; i < relays.size(); i++)
                                {
                                        Ptr<Packet> pkt = p->Copy ();
                                        Ptr<Ipv4Route> route = Create<Ipv4Route> ();
                                        route->SetGateway (relays[i]);
                                        route->SetSource (header.GetSource ());
                                        route->SetDestination (header.GetDestination ());
                                        route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));
                                        ucb (route, pkt, header);
                                }
                        }

                }
                return true;
        }
   }
}

/* * -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Arizona
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
#include "bs-routing-table.h"
#include <list>

namespace ns3{

        class BufferAndSwitchRouting: public Ipv4RoutingProtocol
        {
                public:
                        static TypeId GetTypeId(void);
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
                private:
                        char GetNextIntersection();
                private:
                        Ptr<BufferAndSwitchRoutingTable> m_rtable;
                        Ipv4Address m_address;
                        Ptr<Ipv4> m_ipv4;
                        uint32_t m_ifaceId;
        };
}

#endif  //BUFFER_AND_SWITCH_ROUTING_H

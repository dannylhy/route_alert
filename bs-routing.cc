/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Arizona
 *
 * This program is free software; you can redistribute it and/or modify
/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/pointer.h"
#include "ns3/double.h"
#include "ns3/ipv4-static-routing.h"
#include "bs-routing.h"
#include "bs-routing-table.h"

NS_LOG_COMPONENT_DEFINE ("BSRouting");

namespace ns3{

        NS_OBJECT_ENSURE_REGISTERED (BufferAndSwitchRouting);

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

        char BufferAndSwitchRouting::GetNextIntersection ()
        {
                return 'X'; //to be modified
        }

        Ptr<Ipv4Route> BufferAndSwitchRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
        {
                Ipv4Address relay = m_rtable->LookupRoute (this->GetNextIntersection ());
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

                return route;
                return 0;
        }



}

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

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "bs-routing-table.h"
#include "ns3/mobility-model.h"
#include <vector>
#include <boost/lexical_cast.hpp>

#define EXPIRETIME 10

using namespace std;

NS_LOG_COMPONENT_DEFINE ("BSRoutingTable");

namespace ns3{

        TypeId BufferAndSwitchRoutingTable::GetTypeId ()
        {
                static TypeId tid = TypeId ("ns3::BufferAndSwitchRoutingTable")
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

        void BufferAndSwitchRoutingTable::AddRoute (BSRoutingTableEntry entry)
        {
                m_bsTable.push_front (entry);
        }

        void BufferAndSwitchRoutingTable::UpdateRoute (Ipv4Address ip, char nextIntersection)
        {
                for (std::list<BSRoutingTableEntry>::iterator it = m_bsTable.begin (); it != m_bsTable.end (); it++)
                {
                        //if find, update routing table
                        if ((it->addr).IsEqual (ip))
                        {
                                it->nextIntersection = nextIntersection;
                                it->timeStamp = 0; //to be modified
                                return;
                        }
                }

                //if not find, then add
                BSRoutingTableEntry bsr;
                bsr.addr = ip;
                bsr.nextIntersection = nextIntersection;
                bsr.timeStamp = 0;  //to be modified

                m_bsTable.push_front (bsr);
        }

        Ipv4Address BufferAndSwitchRoutingTable::LookupRoute (char nextIntersection)
        {
                for(std::list<BSRoutingTableEntry>::iterator it = m_bsTable.begin ();
                    it != m_bsTable.end(); it++)
                {
                        if (it->nextIntersection == nextIntersection)
                        {
                                return it->addr;
                        }
                }

                return 0;
        }
}

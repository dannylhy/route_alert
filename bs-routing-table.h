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
 * Author: Qi Zhang <qzhang90@gatech.edu>>
 */

#ifndef BUFFER_AND_SWITCH_ROUTING_TABLE_H
#define BUFFER_AND_SWITCH_ROUTING_TABLE_H

#include "ns3/node.h"
#include "ns3/ipv4-route.h"
#include "ns3/output-stream-wrapper.h"
#include <list>
#include <vector>

namespace ns3{

        class BufferAndSwitchRoutingTable : public Object
        {
        private:
                typedef struct
                {
                        Ipv4Address addr;
                        char nextIntersection;
                        uint32_t timeStamp;
                }BSRoutingTableEntry;

                std::list<BSRoutingTableEntry> m_bsTable;
                std::vector<char> m_myPathToDst; //assume no loop in the path

                uint32_t m_entryExpireTime;
        public:
                BufferAndSwitchRoutingTable ();
                virtual ~BufferAndSwitchRoutingTable ();

                static TypeId GetTypeId();
                void AddRoute (BSRoutingTableEntry entry);
                void UpdateRoute (Ipv4Address ip, char nextIntersection);
                Ipv4Address LookupRoute (char nextIntersection);

        };
}

#endif //BUFFER_AND_SWITCH_ROUTING_TABLE_H

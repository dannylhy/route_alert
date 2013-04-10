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


#include "bs-routing-helper.h"
#include "bs-routing.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3 {

        BSRoutingHelper::BSRoutingHelper ()
                        : Ipv4RoutingHelper ()
        {
                m_agentFactory.SetTypeId ("ns3::bs::RoutingProtocol");
        }

        BSRoutingHelper* BSRoutingHelper::Copy (void) const
        {
                return new BSRoutingHelper (*this);
        }

        Ptr<Ipv4RoutingProtocol> BSRoutingHelper:: Create (Ptr<Node> node) const
        {
                Ptr<bs::BufferAndSwitchRouting> agent = m_agentFactory.Create<bs::BufferAndSwitchRouting> ();
                node->AggregateObject (agent);
                return agent;
        }

        void BSRoutingHelper::Set (std::string name, const AttributeValue &value)
        {
                m_agentFactory.Set (name, value);
        }

}

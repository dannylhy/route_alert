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

#include "bs-routing-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"

namespace ns3{
   namespace bs{
        //******************TyperHeader**********************
        TypeHeader::TypeHeader (MessageType t = MSG_HELLO)
        :m_type(t),
         m_valid(true)
        {

        }

        TypeId TypeHeader::GetTypeId (void)
        {
                static TypeId tid = TypeId ("ns3::bs::TypeHeader")
                                    .SetParent<Header> ()
                                    .AddConstructor<TypeHeader> ()
                                    ;
                return tid;
        }
        TypeId TypeHeader::GetInstanceTypeId () const
        {
                return GetTypeId ();
        }
        uint32_t TypeHeader::GetSerializedSize (void) const
        {
                return 1;
        }

        void TypeHeader::Serialize (Buffer::Iterator i) const
        {
                i.WriteU8 ((uint8_t) m_type);
        }

        uint32_t TypeHeader::Deserialize (Buffer::Iterator start)
        {
                Buffer::Iterator i = start;
                uint8_t type = i.ReadU8 ();
                m_valid = true;

                switch (type)
                {
                        case MSG_HELLO:
                        case MSG_ALERT:
                        {
                                m_type = (MessageType) type;
                                break;
                        }
                        default:
                        {
                                m_valid = false;
                        }
                }

                uint32_t dist = i.GetDistanceFrom (start);
                NS_ASSERT (dist == GetSerializedSize ());
                return dist;
        }

        void TypeHeader::Print (std::ostream &os) const
        {
                switch (m_type)
                {
                        case MSG_HELLO:
                        {
                                os << "MSG_HELLO";
                                break;
                        }
                        case MSG_ALERT:
                        {
                                os << "MSG_ALERT";
                                break;
                        }
                        default:
                        {
                                os << "UNKNOWN_TYPE";
                        }
                }
        }

        //******************BSHeader**********************
        //BSHeader::BSHeader (std::vector<char> currentRoad, uint64_t posx, uint64_t posy)
        BSHeader::BSHeader ()
        {
                m_posx = 0;
                m_posy = 0;
        }

        TypeId BSHeader::GetTypeId ()
        {
                static TypeId tid = TypeId ("ns3::bs::BSHeader")
                                    .SetParent<Header> ()
                                    .AddConstructor<BSHeader> ()
                                    ;
                return tid;
        }

        TypeId BSHeader::GetInstanceTypeId () const
        {
                return GetTypeId ();
        }

        uint32_t BSHeader::GetSerializedSize (void) const
        {
                uint32_t size = 0;
                size += 4;  //length of currentRoad vector (uint32_t). Limitation of number of junctions: 2^32
                size += 1*m_currentRoad.size(); //size of contents in currentRoad vector
                size += 16; //posx, posy
                return size;
        }

        void BSHeader::Serialize (Buffer::Iterator start) const
        {
                uint32_t size;
                uint32_t i;

                size = m_currentRoad.size ();

                start.WriteHtonU64 (m_posx);
                start.WriteHtonU64 (m_posy);
                start.WriteHtonU32 (size);
                for (i=0; i<size; i++)
                {
                        start.WriteU8 (m_currentRoad [i]);
                }
        }

        uint32_t BSHeader::Deserialize (Buffer::Iterator start)
        {
                Buffer::Iterator i = start;
                m_posx = i.ReadNtohU64 ();
                m_posy = i.ReadNtohU64 ();

                uint32_t size = i.ReadNtohU32 ();
                if (0 != size)
                {
                        m_currentRoad.push_back ((char)i.ReadU8 ());
                }
                uint32_t dist = i.GetDistanceFrom (start);
                NS_ASSERT (dist == GetSerializedSize ());
                return dist;
        }

        void BSHeader::Print (std::ostream &os) const
        {
                os << " Position X : " << m_posx
                   << " Position Y : " << m_posy;
                uint32_t size = m_currentRoad.size ();
                os << " currentRoad: ";
                for (uint32_t i=0; i<size; i++)
                {
                        os << " " << m_currentRoad [i];
                }
        }


  }
}

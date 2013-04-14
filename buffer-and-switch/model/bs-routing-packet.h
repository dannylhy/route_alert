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

#ifndef BUFFER_AND_SWITCH_ROUTING_PACKET_H
#define BUFFER_AND_SWITCH_ROUTING_PACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"

namespace ns3{
 namespace bs{
        enum MessageType
        {
		MSG_INIT = 0,//Used in initialization
                MSG_HELLO = 1, //Hello Message
                MSG_ALERT = 2, //Alert Message
		MSG_RREQ = 3,  //Routing Request Message
        };
	
	class TypeHeader : public Header
	{
		public:	
			TypeHeader (MessageType t);
                        // from Header
                        static TypeId GetTypeId ();
                        uint32_t GetSerializedSize (void) const;
                        void Serialize (Buffer::Iterator start) const;
                        uint32_t Deserialize (Buffer::Iterator start);
                        void Print (std::ostream &os) const;
			
			TypeId GetInstanceTypeId () const;
			
			MessageType GetType (void)
			{
				return m_type;
			}
			
			bool IsValid (void)
			{
				return m_valid;
			}

		private:
			MessageType m_type;
			bool m_valid;
	};

        class BSHeader : public Header
        {
                public:
                        BSHeader();

                        // from Header
                        static TypeId GetTypeId (void);
                        uint32_t GetSerializedSize (void) const;
                        void Serialize (Buffer::Iterator start) const;
                        uint32_t Deserialize (Buffer::Iterator start);
                        void Print (std::ostream &os) const;
			
			TypeId GetInstanceTypeId () const;

                        void SetCurrentRoad(std::string currentRoad)
                        {
                                this->m_currentRoad = currentRoad;
                        }

                        std::string GetCurrentRoad(void)
                        {
                                return this->m_currentRoad;
                        }

			void SetPosx (uint64_t posx)
			{
				this->m_posx = posx;
			}

			uint64_t GetPosx (void)
			{
				return this->m_posx;	
			}

			void SetPosy (uint64_t posy)
			{
				this->m_posy = posy;
			}

			uint64_t GetPosy (void)
			{
				return this->m_posy;
			}
                private:
                        std::string m_currentRoad; //if it is a HelloPacket, m_currentRoad is current road
							 //if it is a AlertPacket, m_currentRoad is current path to destination
			uint64_t m_posx; //X coordinate
			uint64_t m_posy; //Y coordinate
        };
 }
}

#endif /*BUFFER_AND_SWITCH_ROUTING_PACKET_H*/

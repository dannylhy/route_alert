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
 * Author: Qi Zhang <qzhang90@gatech.edu> Danny Lee <dannylee@gatech.edu>
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/internet-module.h"
#include "ns3/bs-routing-helper.h"
#include "ns3/bs-routing-table.h"
#include "ns3/bs-routing.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define NUM_NODE 2

NS_LOG_COMPONENT_DEFINE ("BSExample");

using namespace ns3;

int main (int argc, char *argv[])
{
  uint16_t numNodes = 221;

  LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_DEBUG);
  LogComponentEnable("MacLow", LogLevel(LOG_WARN) );
  LogComponentEnable("BSRouting", LogLevel(LOG_WARN) );
  LogComponentEnable("BSRoutingTable", LogLevel(LOG_WARN) ); 
  
  std::cout<< "************1" << std::endl;
  ns3::Packet::EnablePrinting ();
  NodeContainer nodes;
  nodes.Create (numNodes);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue ("DsssRate1Mbps"), "ControlMode",StringValue ("DsssRate1Mbps"));
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);
  
  std::cout << "***********2" << std::endl;
  Ns2MobilityHelper mobility ("/home/cs6250/proj/repos/ns-3-allinone/ns-3.16/scratch/mobility_1s.tcl");
  mobility.Install ();
  /*MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (10.0, 0.0, 0.0));
  positionAlloc->Add (Vector (20.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);*/

  std::cout << "***********3" << std::endl;
  BufferAndSwitchRoutingHelper br;
  Ptr<bs::BufferAndSwitchRoutingTable> brt = CreateObject<bs::BufferAndSwitchRoutingTable> ();
  br.Set ("RoutingTable", PointerValue(brt));

  InternetStackHelper internet;
  internet.SetRoutingHelper (br);
  internet.Install (nodes);
  
  /*
  ShortestPathRoutingHelper sr;
  Ptr<ShortestPathRoutingTable> srt = CreateObject<ShortestPathRoutingTable> ();
  sr.Set ("RoutingTable", PointerValue (srt));

  InternetStackHelper internet;
  internet.SetRoutingHelper (sr);
  internet.Install (nodes);
  */
  
  std::cout << "***********4" << std::endl;
  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iface = ipv4.Assign (devices);
 
  
  /*
  for (uint16_t i = 0; i < numNodes; i++)
    {
      srt->AddNode (nodes.Get (i), iface.GetAddress (i));
    }
  srt->UpdateRoute (100);
  */
  
  std::cout << "***********5" << std::endl;
  /*UdpServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (numNodes-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (2.0));

  UdpClientHelper client (iface.GetAddress (156), 9);
  client.SetAttribute ("MaxPackets", UintegerValue (2));
  client.SetAttribute ("Interval", TimeValue (Seconds (1)));
  client.SetAttribute ("PacketSize", UintegerValue (1000));

  ApplicationContainer clientApps;
  clientApps = client.Install (nodes.Get (0));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (2.0));
*/
  
  std::cout << "}}}}}}}}}}}}}}}}}}}}}}" << nodes.Get(0)->GetId () << "  " << nodes.Get (0)->GetDevice (0)->GetAddress () << std::endl;
  std::cout << "}}}}}}}}}}}}}}}}}}}}}}" << NodeList::GetNode (0)->GetId () << "  " << NodeList::GetNode (0)->GetDevice (0)->GetAddress () << std::endl;
  Simulator::Stop (Seconds (60));

  wifiPhy.EnablePcapAll (std::string ("bs"));
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}


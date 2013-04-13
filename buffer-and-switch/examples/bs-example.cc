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
#include "ns3/internet-module.h"
#include "ns3/bs-routing-helper.h"
#include "ns3/bs-routing-table.h"
#include "ns3/bs-routing.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define NUM_NODE 2

NS_LOG_COMPONENT_DEFINE ("SpExample");

using namespace ns3;

int main (int argc, char *argv[])
{
  uint16_t numNodes = 10;

  LogComponentEnable("MacLow", LogLevel(LOG_DEBUG | LOG_PREFIX_TIME | LOG_PREFIX_NODE) );
  LogComponentEnable("SpRouting", LogLevel(LOG_FUNCTION | LOG_DEBUG | LOG_PREFIX_TIME | LOG_PREFIX_NODE) );
  LogComponentEnable("SpRoutingTable", LogLevel(LOG_FUNCTION | LOG_DEBUG | LOG_PREFIX_TIME | LOG_PREFIX_NODE) );

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

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (10.0, 0.0, 0.0));
  positionAlloc->Add (Vector (20.0, 0.0, 0.0));
  positionAlloc->Add (Vector (30.0, 0.0, 0.0));
  positionAlloc->Add (Vector (40.0, 0.0, 0.0));
  positionAlloc->Add (Vector (50.0, 10.0, 0.0));
  positionAlloc->Add (Vector (50.0, 20.0, 0.0));
  positionAlloc->Add (Vector (50.0, 30.0, 0.0));
  positionAlloc->Add (Vector (50.0, 40.0, 0.0));
  positionAlloc->Add (Vector (50.0, 50.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

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

  UdpServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (numNodes-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (5.0));

  UdpClientHelper client (iface.GetAddress (numNodes-1), 9);
  client.SetAttribute ("MaxPackets", UintegerValue (2));
  client.SetAttribute ("Interval", TimeValue (Seconds (1)));
  client.SetAttribute ("PacketSize", UintegerValue (1000));

  ApplicationContainer clientApps;
  clientApps = client.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (5.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}


/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/packet-sink.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/random-variable-stream.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Giardino-ECE6110-p1");

int
main (int argc, char *argv[])
{
  uint32_t segSize = 512;
  uint32_t queueSize = 64000;
  uint32_t CWND = 64000;
  uint32_t maxBytes = 1000000;
  bool genPCAP = false;
  bool toFile = false;
  uint32_t nFlows = 1;

  std::string filename = "p1.results.txt";

  std::string TCPtype = "TCPTahoe";

  CommandLine cmd;
  cmd.AddValue("segSize", "TCP Segment Size", segSize);
  cmd.AddValue("queueSize", "Queue Size of Bottleneck Link", queueSize);
  cmd.AddValue("CWND", "Maximum receiver advertised window size", CWND);
  cmd.AddValue("TCPtype", "TCPtype (TCPTahoe, TCPReno)", TCPtype);
  cmd.AddValue("maxBytes", "Maximum bytes to send", maxBytes);
  cmd.AddValue("genPCAP", "Generate PCAP trace (boolean)", genPCAP);
  cmd.AddValue("toFile", "Output results to file", toFile);
  cmd.AddValue("filename", "Output filename", filename);
  cmd.AddValue("flows", "Number of flows sending", nFlows);
  cmd.Parse(argc, argv);

  if (TCPtype == "TCPTahoe") {
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpTahoe"));
  } else {
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpReno"));
  }


  std::ostream* output = &std::cout;
  if (toFile) {
    output = new std::ofstream(filename.c_str(), std::ios::out | std::ios::app);
//    output->open();
  }

  Config::SetDefault("ns3::TcpSocket::SegmentSize",UintegerValue(segSize));
  Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue(CWND));
  Config::SetDefault("ns3::DropTailQueue::MaxBytes",UintegerValue(queueSize));
  Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(DropTailQueue::QUEUE_MODE_BYTES));


  LogComponentEnable ("BulkSendApplication", LOG_LEVEL_ERROR);
  LogComponentEnable ("PacketSink", LOG_LEVEL_ERROR);

  NS_LOG_INFO("Create nodes.");

  NodeContainer nodes;
  nodes.Create (4);


  NS_LOG_INFO("Create links.");

  PointToPointHelper link1,link2,link3;
  link1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  link1.SetChannelAttribute ("Delay", StringValue ("10ms"));
  
  link2.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  link2.SetChannelAttribute ("Delay", StringValue ("20ms"));

  link3.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  link3.SetChannelAttribute("Delay", StringValue("10ms"));
  
  NetDeviceContainer net1, net2, net3;
  net1 = link1.Install(nodes.Get(0),nodes.Get(1));
  net2 = link2.Install(nodes.Get(1),nodes.Get(2));
  net3 = link3.Install(nodes.Get(2),nodes.Get(3));
 
  NS_LOG_INFO("Install internet stack.");

  InternetStackHelper stack;
  stack.Install (nodes);

  NS_LOG_INFO("Assign IP Addresses.");

  Ipv4AddressHelper address1, address2, address3;
  
  address1.SetBase ("10.1.1.0", "255.255.255.0");
  address2.SetBase ("10.1.2.0", "255.255.255.0");
  address3.SetBase ("10.1.3.0", "255.255.255.0");


  Ipv4InterfaceContainer interfaces1 = address1.Assign (net1);
  Ipv4InterfaceContainer interfaces2 = address2.Assign (net2);
  Ipv4InterfaceContainer interfaces3 = address3.Assign (net3);
 
  NS_LOG_INFO("Create applications.");

  uint16_t port = 31415;
  double minRand = 0.0;
  double maxRand = 0.1;
  Ptr<UniformRandomVariable> rand= CreateObject<UniformRandomVariable>();
  rand->SetAttribute("Min", DoubleValue(minRand));
	rand->SetAttribute("Max", DoubleValue(maxRand));
	ApplicationContainer sourceApps;
  ApplicationContainer sinkApps;
  int ii = 0;
  for (ii = 0; ii < (int)nFlows; ii++) {
	  BulkSendHelper source("ns3::TcpSocketFactory",InetSocketAddress(interfaces3.GetAddress(1),port+ii));
  	source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
  	sourceApps.Add(source.Install(nodes.Get(0)));
  	(sourceApps.Get(ii))->SetStartTime(Seconds(rand->GetValue()));
  	(sourceApps.Get(ii))->SetStopTime(Seconds(600.0));
	
  	PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),port+ii));
	  sinkApps.Add(sinkHelper.Install(nodes.Get(3)));
  	(sinkApps.Get(ii))->SetStartTime(Seconds(0.0));
 	(sinkApps.Get(ii))->SetStopTime(Seconds(600.0));
	}
  

  NS_LOG_INFO("Populate Routing Tables.");

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  if (genPCAP) {
    AsciiTraceHelper ascii;
    link1.EnableAsciiAll(ascii.CreateFileStream("p1-link1-send.tr"));
    link1.EnablePcapAll("p1-link1-send",false);
  }

  NS_LOG_INFO("Create and install flow monitor.");

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  NS_LOG_INFO("Run Simulation.");
 
  Simulator::Stop(Seconds(120));
  Simulator::Run ();
  
  NS_LOG_INFO("Done.");
  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  
	uint32_t bytesRX = 0;
	Ptr<Application> apps;
	Ptr<PacketSink> sinks;
  for(ii = 0; ii < (int)nFlows; ii++) {
		apps = sinkApps.Get(ii);
		sinks = DynamicCast<PacketSink>(apps);
		bytesRX = bytesRX + sinks->GetTotalRx();
	}
  
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  // std::cout << "Size of FlowMonitor map: " << stats.size() << std::endl;

  std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin ();

  double firstTX = 600.0;
  double lastRX = 0.0;

  for(i = stats.begin(); i != stats.end(); ++i) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
    if (firstTX > i->second.timeFirstTxPacket.GetSeconds()) {
      firstTX = i->second.timeFirstTxPacket.GetSeconds();
    }
    if (lastRX < i->second.timeLastRxPacket.GetSeconds()) {
      lastRX = i->second.timeLastRxPacket.GetSeconds();
    }
  }
  //std::cout << "FirstTX: " << firstTX << " LastRX: " << lastRX << "\n";
  Simulator::Destroy ();
 
  double goodput = (bytesRX*8)/(lastRX-firstTX)/1000000;
  output->width(10);
  *output << TCPtype;
  output->width(4);
  *output << nFlows;
  output->width(6);
  *output << CWND;
  output->width(6);
  *output << queueSize;
  output->width(6);
  *output << segSize;
  output->width(12);
  *output << goodput << "\n"; 
  output->flush();
  return 0;
}

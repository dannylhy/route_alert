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
  uint32_t queueSize = 120;
  double minTh = 30;
  double maxTh = 90;
  double wq = 0.002;
  double maxp = 10;
  uint32_t CWND = 64000;
  uint32_t maxBytes = 1000000;
  bool genPCAP = false;
  bool toFile = false;
//  uint32_t nFlows = 1;
 // uint32_t nCsma = 2;
 // uint32_t niWifi0 = 0;
//  uint32_t niWifi1 = 0;
  uint32_t nExtWifi = 16;
  uint32_t UDPshare = 10;
  uint32_t totalLoad = 90;

  std::string rates_str[5] = {"5000kbps", "3800kbps", "2600kbps", "2500kbps","1000kbps"}; 

  StringValue ytLo = StringValue(rates_str[4]);
  StringValue ytMed = StringValue(rates_str[3]);
  StringValue ytHi = StringValue(rates_str[0]);
  StringValue nfLo = StringValue(rates_str[2]);
  StringValue nfHi = StringValue(rates_str[1]);
  std::string voipr = "100kbps";
  uint32_t voipp = 160;  


  std::string filename = "p1.results.txt";

  std::string TCPtype = "TCPTahoe";
  std::string queueType = "DropTailQueue";

  CommandLine cmd;
  cmd.AddValue("segSize", "TCP Segment Size", segSize);
  cmd.AddValue("queueSize", "Queue Size of Bottleneck Link", queueSize);
  cmd.AddValue("minTh", "Minimum Threshold for RED", minTh);
  cmd.AddValue("maxTh", "Maximum Threshold for RED", maxTh);
  cmd.AddValue("wq", "Weighting factor for avg queue", wq);
  cmd.AddValue("maxp", "Maximum probability of dropping packet (percent)", maxp);
  cmd.AddValue("CWND", "Maximum receiver advertised window size", CWND);
  cmd.AddValue("TCPtype", "TCPtype (TCPTahoe, TCPReno)", TCPtype);
  cmd.AddValue("queueType", "DropTailQueue or RedQueue", queueType);
  cmd.AddValue("maxBytes", "Maximum bytes to send", maxBytes);
  cmd.AddValue("UDPshare", "Amount of UDP traffic (percent)", UDPshare);
  cmd.AddValue("totalLoad", "Total load on bottleneck link", totalLoad);
  cmd.AddValue("genPCAP", "Generate PCAP trace (boolean)", genPCAP);
  cmd.AddValue("toFile", "Output results to file", toFile);
  cmd.AddValue("filename", "Output filename", filename);
  cmd.Parse(argc, argv);

  std::string qt = "ns3::" + queueType;

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
  Config::SetDefault("ns3::RedQueue::QueueLimit", UintegerValue(queueSize));
  Config::SetDefault("ns3::RedQueue::Mode", EnumValue(RedQueue::QUEUE_MODE_PACKETS));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets",UintegerValue(queueSize));
  Config::SetDefault("ns3::DropTailQueue::Mode", EnumValue(DropTailQueue::QUEUE_MODE_PACKETS));
  Config::SetDefault("ns3::Application::StopTime", TimeValue(Seconds(10000.0)));
  Config::SetDefault("ns3::Application::StartTime", TimeValue(Seconds(0.0)));
  //LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  //LogComponentEnableAll(LOG_LEVEL_INFO);
  NS_LOG_INFO("Create nodes.");

  NodeContainer allNodes, streams, webservers, netRouters, hoodRouters, servers, hosts;
  webservers.Create(4);
  streams.Create(2);
  netRouters.Create(11);
  hoodRouters.Create(2);
 
  servers.Add(streams);
  servers.Add(webservers);
  allNodes.Add(servers);
  allNodes.Add(netRouters);
  allNodes.Add(hoodRouters);
  hosts.Add(servers);

  NodeContainer modems;

  modems.Create(8);
  allNodes.Add(modems);
/*
  NodeContainer ap, csmaNodes;
  ap.Create(2);
  csmaNodes.Create(2);
  allNodes.Add(ap);
  allNodes.Add(csmaNodes);
  hosts.Add(csmaNodes);
*/
  NodeContainer iWifiNodes, eWifiNodes;
 
  //iWifiNodes.Create(niWifi0+niWifi1);
  eWifiNodes.Create(nExtWifi);
  //allNodes.Add(iWifiNodes);
  allNodes.Add(eWifiNodes);
  //hosts.Add(iWifiNodes);
  hosts.Add(eWifiNodes);

  NS_LOG_INFO("Create wired links.");

  PointToPointHelper backbone, eth100, gbe, hood, modem, wifiN;

  backbone.SetDeviceAttribute("DataRate", StringValue("1244Mbps"));
  backbone.SetChannelAttribute("Delay", StringValue("10ms"));
  backbone.SetQueue(qt);

  eth100.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  eth100.SetChannelAttribute("Delay", StringValue("500us"));
  eth100.SetQueue(qt);

  gbe.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
  gbe.SetChannelAttribute("Delay", StringValue("500us"));
  gbe.SetQueue(qt);
  
  hood.SetDeviceAttribute("DataRate", StringValue("155Mbps"));
  hood.SetChannelAttribute("Delay", StringValue("5ms"));
  hood.SetQueue(qt);

  modem.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  modem.SetChannelAttribute("Delay", StringValue("10ms"));
  modem.SetQueue(qt);

  wifiN.SetDeviceAttribute("DataRate", StringValue("150Mbps"));
  wifiN.SetChannelAttribute("Delay", StringValue("2ms"));
  wifiN.SetQueue(qt);

  // Very simplified backbone topology

  NetDeviceContainer BBnet;

  BBnet = backbone.Install(netRouters.Get(0), netRouters.Get(1));
  BBnet.Add(backbone.Install(netRouters.Get(1), netRouters.Get(2)));
  BBnet.Add(backbone.Install(netRouters.Get(2), netRouters.Get(3)));
  BBnet.Add(backbone.Install(netRouters.Get(3), netRouters.Get(4)));
  BBnet.Add(backbone.Install(netRouters.Get(3), netRouters.Get(5)));
  BBnet.Add(backbone.Install(netRouters.Get(3), netRouters.Get(6)));
  BBnet.Add(backbone.Install(netRouters.Get(6), netRouters.Get(8)));
  BBnet.Add(backbone.Install(netRouters.Get(1), netRouters.Get(8)));
  BBnet.Add(backbone.Install(netRouters.Get(0), netRouters.Get(8)));
  BBnet.Add(backbone.Install(netRouters.Get(8), netRouters.Get(10)));
  BBnet.Add(backbone.Install(netRouters.Get(8), netRouters.Get(9)));
  BBnet.Add(backbone.Install(netRouters.Get(6), netRouters.Get(7)));  

  // Backbone to server connections

  NetDeviceContainer gbEth;

  gbEth = gbe.Install(netRouters.Get(4), streams.Get(0));
  gbEth.Add(gbe.Install(netRouters.Get(5), webservers.Get(0)));
  gbEth.Add(gbe.Install(netRouters.Get(7), streams.Get(1)));
  gbEth.Add(gbe.Install(netRouters.Get(9), webservers.Get(1)));
  gbEth.Add(gbe.Install(netRouters.Get(9), webservers.Get(2)));
  gbEth.Add(gbe.Install(netRouters.Get(10), webservers.Get(3)));

  // Neighborhood  to backbone network links

  NetDeviceContainer Hnet;
  Hnet = hood.Install(netRouters.Get(0), hoodRouters.Get(0));
  Hnet.Add(hood.Install(netRouters.Get(0), hoodRouters.Get(1)));

  // Modem to neighborhood cable links. Change to arbitrary number of
  // links.

  NetDeviceContainer Mnet;
  int i = 0;
  int k = 0;
  for(i = 0; i < 8; i++) {
    if (k != 1 && i >= 4) {
      k = 1;
    }
    Mnet.Add(modem.Install(modems.Get(i), hoodRouters.Get(k)));
  }

  // "Wifi" links, but really just point-to-point. 

  NetDeviceContainer eWifiNet;

  // Change to be able to make this autogenerate tree-like topology with
  // arbitrary number of nodes. (change i=1 to return to old topology)
  for(i = 0; i < 8; i++) {
     eWifiNet.Add(wifiN.Install(modems.Get(i), eWifiNodes.Get(2*i)));
     eWifiNet.Add(wifiN.Install(modems.Get(i), eWifiNodes.Get(2*i+1)));
  }

  // AP0 to modem0 and AP1 to AP0 on 100mbe
/*
  NetDeviceContainer e100, iWifiNet;
  e100 = eth100.Install(modems.Get(0), aps.Get(0));
  e100.Add(eth100.Install(ap.Get(1), ap.Get(0)));
  e100.Add(eth100.Install(csmaNodes.get(0), ap.Get(0)));
  e100.Add(eth100.Install(csmaNodes.get(1), ap.Get(0)));
  for (i = 0; i < niWifi0+niWifi1; i++) { 
    // iWifiNet = wifiN.Install(iWifiNodes.Get(i), ap.Get(i>niWifi0));
    if (i < niWifi0) {
      iWifiNet = wifiN.Install(iWifiNodes.Get(i), ap.Get(0));
    } else {
      iWifiNet = wifiN.install(iWifiNodes.Get(i), ap.Get(1));
    }
  }
*/
  // Netdevice pointers for PCAP
  
  Ptr<NetDevice> modemDev, hoodDev, streamDev, webDev;
  modemDev = Mnet.Get(0);
  hoodDev = Hnet.Get(0);
  streamDev = gbEth.Get(0);
  webDev = gbEth.Get(4);


  NS_LOG_INFO("Install internet stack.");

  InternetStackHelper stack;
  stack.Install (allNodes);

  NS_LOG_INFO("Assign IP Addresses.");

  Ipv4AddressHelper aWifi, aModem, aHood, aBB, aGbe;
  

  // 192.168.X.X for home networks, 172.16.X.X for cable routers and server,
  // and 10.X.X.X for network backbone interfaces
  //aHome.SetBase("192.168.1.0", "255.255.255.0");
  aWifi.SetBase("192.168.3.0", "255.255.255.0");
  aModem.SetBase("172.16.128.0", "255.255.255.0");
  aHood.SetBase("10.1.1.0", "255.255.255.0");
  aBB.SetBase("10.1.2.0", "255.255.255.0");
  aGbe.SetBase("172.16.1.0", "255.255.255.0");

  Ipv4InterfaceContainer iWifi, iModem,iHood,iBB,iGbe,iHosts;
  //iHome = aHome.Assign(e100);
  //iHome.Add(aHome.Assign(iWifiNet));
  iWifi = aWifi.Assign(eWifiNet);
  for (i = 0; i < (int)iWifi.GetN(); i++) {
//    std::cout << i << ": " << iWifi.GetAddress(i).Get()<<"\n";
  }
//  std::cout << "iWifi: " <<iWifi.GetN() << "\n";
  iModem = aModem.Assign(Mnet);
  iHood = aHood.Assign(Hnet);
  iBB = aBB.Assign(BBnet);
  iGbe = aGbe.Assign(gbEth);
//  std::cout << "iGbe: " <<iGbe.GetN() << "\n";
  iHosts.Add(iWifi);
  iHosts.Add(iGbe);

//  std::cout << "iHosts: "<<iHosts.GetN()<< " hosts: "<< hosts.GetN() << "\n";

  NS_LOG_INFO("Create RNGs.");


  double minRand = 0.0;
  double maxRand = 0.5;
  Ptr<UniformRandomVariable> randStartTime= CreateObject<UniformRandomVariable>();
  randStartTime->SetAttribute("Min", DoubleValue(minRand));
  randStartTime->SetAttribute("Max", DoubleValue(maxRand));
  
  uint32_t firstHost = 0;
  uint32_t lastHost = eWifiNodes.GetN() - 1;
  Ptr<UniformRandomVariable> rand_voip = CreateObject<UniformRandomVariable>();
  rand_voip->SetAttribute("Min", DoubleValue(firstHost));
  rand_voip->SetAttribute("Max", DoubleValue(lastHost));
  
  double webMean = 320000;
  double webVar = 50000;
  Ptr<NormalRandomVariable> rand_web = CreateObject<NormalRandomVariable>();
  rand_web->SetAttribute("Mean", DoubleValue(webMean));
  rand_web->SetAttribute("Variance", DoubleValue(webVar));
  rand_web->SetAttribute("Bound", DoubleValue(10000000));

  double webMeanOn = 1;
  double webMeanOff = 5;
  double webBound = 100;
  
  Ptr<ExponentialRandomVariable> rand_web_on = CreateObject<ExponentialRandomVariable>();
  rand_web_on->SetAttribute("Mean", DoubleValue(webMeanOn));
  rand_web_on->SetAttribute("Bound", DoubleValue(webBound));

  Ptr<ExponentialRandomVariable> rand_web_off= CreateObject<ExponentialRandomVariable>();
  rand_web_off->SetAttribute("Mean",DoubleValue(webMeanOff));
  rand_web_off->SetAttribute("Bound", DoubleValue(webBound));

  double ytMeanOn = 5;
  double ytBound = 300;
  double ytMeanOff = 1.25;


  Ptr<ExponentialRandomVariable> rand_yt_on = CreateObject<ExponentialRandomVariable>();
  rand_yt_on->SetAttribute("Mean", DoubleValue(ytMeanOn));
  rand_yt_on->SetAttribute("Bound", DoubleValue(ytBound));

  Ptr<ExponentialRandomVariable> rand_yt_off = CreateObject<ExponentialRandomVariable>();
  rand_yt_off->SetAttribute("Mean", DoubleValue(ytMeanOff));
  rand_yt_off->SetAttribute("Bound", DoubleValue(ytBound));

  double nfMeanOn = 5;
  double nfMeanOff = 2.5;
  double nfBound = 600;

  Ptr<ExponentialRandomVariable> rand_nf_on = CreateObject<ExponentialRandomVariable>();
  rand_nf_on->SetAttribute("Mean", DoubleValue(nfMeanOn));
  rand_nf_on->SetAttribute("Bound", DoubleValue(nfBound));

  Ptr<ExponentialRandomVariable> rand_nf_off = CreateObject<ExponentialRandomVariable>();
  rand_nf_off->SetAttribute("Mean", DoubleValue(nfMeanOff));
  rand_nf_off->SetAttribute("Bound", DoubleValue(nfBound));

//  ApplicationContainer sourceApps;
//  ApplicationContainer sinkApps;

// Start with UDP traffic. Just VoIP between wireless nodes.
// Takes a percentage of bottleneck link (10Mbps between Modem and Hood)

  NS_LOG_INFO("Creating VoIP (UDP) sources and sinks.");

  ApplicationContainer eWifiApps, iServApps, iSinkApps;
  ApplicationContainer webApps, nfApps, ytApps, voipSinkApps, voipServApps;

  uint16_t webPort = 80;
  uint16_t streamPort = 31415;
  uint16_t voipPort = 1024;

  uint32_t source;
  uint32_t sink;
  AddressValue addr;
  OnOffHelper voipServ("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny()));
  PacketSinkHelper voipSink("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny()));
  double UDPstreams = (double)UDPshare/100.0;
  UDPstreams = UDPstreams*(double)totalLoad; 

  for(i = 0; i < (int)eWifiNodes.GetN(); i++) { 
  } 

  for (i = 0; i < (int)UDPstreams; i++) {
    source = rand_voip->GetInteger();
    sink = rand_voip->GetInteger();
    while (source == sink) {
      sink = rand_voip->GetInteger();
    } 
    addr = AddressValue(InetSocketAddress(iWifi.GetAddress(2*sink+1), voipPort));
//  addrSink = AddressValue(InetSocketAddress(iHosts.GetAddress(sink, voipPort)));
    voipServ.SetAttribute("Remote", addr);
    voipServ.SetAttribute("PacketSize", UintegerValue(voipp));
    voipServ.SetConstantRate(DataRate(voipr), voipp);
    voipServ.SetAttribute("StartTime", TimeValue(Seconds(randStartTime->GetValue())));
    voipServApps.Add(voipServ.Install(servers.Get(source%6)));
    voipSink.SetAttribute("Local", addr);
    voipSink.SetAttribute("StartTime", TimeValue(Seconds(0.0)));
    voipSinkApps.Add(voipSink.Install(eWifiNodes.Get(sink)));
//    std::cout << "voip app added from "<<source <<" to " << sink <<"\n";
 //   std::cout << "source: " << 
  }
  
  rand_voip->SetAttribute("Min", DoubleValue(0.0));
  rand_voip->SetAttribute("Max", DoubleValue(4.0));
  

  double TCPshare = (double)totalLoad - UDPshare;
  double rates[5] = {5.0, 3.8, 2.6, 2.5, 1.0};
  double ds[5] = {0.8, 2/3.0, 0.8, 2/3.0, 0.8};
  //std::string vidnames[5]= {"ythi", "nfhi","nflo","ytmd","ytlo"};
  double webds = 2/12.0;
  double vidShare = 0.5*TCPshare;
  double webShare = 0.5*TCPshare;
  
  double vs = vidShare;
  double ws = webShare;
  // set up streams first
  //double websstats[8];
  //double vidsstats[8];
  //
  
  NS_LOG_INFO("Creating TCP Applications.");
  
  int j = 0;
  k = 0;
  OnOffHelper vid("ns3::TcpSocketFactory", InetSocketAddress(iWifi.GetAddress(0),streamPort));
  OnOffHelper web("ns3::TcpSocketFactory", InetSocketAddress(iWifi.GetAddress(0),webPort));
  PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",InetSocketAddress(Ipv4Address::GetAny()));
  int stream_set = 5;
  int web_set = 5;
  // duty cycle * lowest bitrate
  j=0;
  for(k = 0; k < 8; k++) {
    ws = webShare;
    vs = vidShare;
    stream_set = true;
    web_set = true;
    while(stream_set > 0) {
      stream_set--;
      i = rand_voip->GetInteger();
      if (vs > ds[i]*rates[i]) {
// std::cout << "vs = " << vs <<" ds["<< i << "] = " << ds[i] << " rates["<<i<<"] = "<< rates[i] << "\n";
        stream_set = 5;
        vs = vs - ds[i]*rates[i];
        addr = AddressValue(InetSocketAddress(iWifi.GetAddress(4*k+2*(j%2)+1),streamPort+j));
        vid.SetAttribute("Remote", addr);
        vid.SetConstantRate(rates_str[i]);
        vid.SetAttribute("StartTime", TimeValue(Seconds(randStartTime->GetValue())));
	if (i == 0 || i == 3 || i == 4) {
          vid.SetAttribute("OnTime",PointerValue(rand_yt_on));
          vid.SetAttribute("OffTime",PointerValue(rand_yt_off));
          ytApps.Add(vid.Install(streams.Get(1)));
        } else {
          vid.SetAttribute("OnTime",PointerValue(rand_nf_on));
          vid.SetAttribute("OffTime",PointerValue(rand_nf_off));
          nfApps.Add(vid.Install(streams.Get(0)));
        }
	sinkHelper.SetAttribute("StartTime", TimeValue(Seconds(0.0)));
	sinkHelper.SetAttribute("Local", addr);
        iSinkApps.Add(sinkHelper.Install(eWifiNodes.Get(2*k+(j%2))));
        j++;
	
	  //std::cout << "app: "<< vidnames[i]<<" wifin: "<< 2*k+(j%2)<< "\n"; 
      }
    }
    
//  std::cout << "j = " << j << " (onoff vid apps created) for modem " << k << "\n";
  }
  j = 0;
  for(k = 0; k < 8; k++) {
    ws = webShare;
    while(web_set) {
      web_set--;
      if (ws > webds*10) {
//	std::cout << "ws = " << ws<< " webds = " <<webds << "\n";
        j++;
        web_set = 5;
	ws = ws - webds*10;
	web.SetAttribute("Remote", AddressValue(InetSocketAddress(iWifi.GetAddress(4*k+2*(j%2)+1),webPort+j)));
        sinkHelper.SetAttribute("Local", AddressValue(InetSocketAddress(iWifi.GetAddress(2*k+(j%2)),webPort+j)));
	iSinkApps.Add(sinkHelper.Install(eWifiNodes.Get(2*k+(j%2))));
	web.SetAttribute("StartTime", TimeValue(Seconds(randStartTime->GetValue())));
	web.SetAttribute("OnTime", PointerValue(rand_web_on));
	web.SetAttribute("OffTime",PointerValue(rand_web_off));
	webApps.Add(web.Install(webservers.Get(j%4)));
      }
    }
//    std::cout << "j = " << j << " (onoff web apps created) for modem "<<k<<"\n";
  }
  
  NS_LOG_INFO("Populate Routing Tables.");

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  if (genPCAP) {
    //PcapHelperForDevice pcap;
    AsciiTraceHelper ascii;
    //modem.EnableAsciiAll(ascii.CreateFileStream("p1-modem.tr"));
    //modem.EnablePcapAll("p2-modem",true);
    modem.EnablePcap("p2-modem",modemDev);
    gbe.EnablePcap("p2-web", webDev);
    gbe.EnablePcap("p2-stream", streamDev);
    hood.EnablePcap("p2-hood", hoodDev);
  }

  NS_LOG_INFO("Create and install flow monitor.");

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  NS_LOG_INFO("Run Simulation.");
 
  Simulator::Stop(Seconds(30));
  Simulator::Run ();
  
  NS_LOG_INFO("Done.");
  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());

  std::ostringstream statsfile;
  if (queueType=="DropTailQueue") {
    statsfile << queueType << "-" << queueSize << "-"<<CWND<< "-" << totalLoad << ".txt";
  } else {
    statsfile << queueType << "-" << minTh << "-"<<maxTh<<"-"<<maxp<<"-"<<wq<<"-" << totalLoad<<".txt";
  }
  monitor->SerializeToXmlFile(statsfile.str(), true,true);
  
  int ii;
  uint32_t TCPbytesRX = 0;
  uint32_t UDPbytesRX = 0;
  Ptr<Application> apps;
  Ptr<PacketSink> sinks;
  for(ii = 0; ii < (int)iSinkApps.GetN(); ii++) {
    apps = iSinkApps.Get(ii);
    sinks = DynamicCast<PacketSink>(apps);
    TCPbytesRX = TCPbytesRX + sinks->GetTotalRx();
  }
  for(ii = 0; ii < (int)voipSinkApps.GetN(); ii++) {
    apps = voipSinkApps.Get(ii);
    sinks = DynamicCast<PacketSink>(apps);
    UDPbytesRX = UDPbytesRX + sinks->GetTotalRx();
  }
//  uint32_t UDPmLinkTX, UDPmLinkRX, TCPmLinkTX, TCPmLinkRX;

//std::cout << "UDPbytesRX: "<<UDPbytesRX<< " TCPbytesRX: "<<TCPbytesRX<<"\n";

  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
//std::cout << "Size of FlowMonitor map: " << stats.size() << std::endl;

  std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it = stats.begin ();

  double firstTX = 3600.0;
  double lastRX = 0.0;

  for(it = stats.begin(); it != stats.end(); ++it) {
    //Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (it->first);
    if (firstTX > it->second.timeFirstTxPacket.GetSeconds()) {
      firstTX = it->second.timeFirstTxPacket.GetSeconds();
    }
    if (lastRX < it->second.timeLastRxPacket.GetSeconds()) {
      lastRX = it->second.timeLastRxPacket.GetSeconds();
    }
  }
//std::cout << "FirstTX: " << firstTX << " LastRX: " << lastRX << "\n";
  Simulator::Destroy ();
 
  double goodput = (TCPbytesRX*8)/(lastRX-firstTX)/1000000;
  output->width(12);
  *output << queueType;
  output->width(4);
  *output << minTh;
  output->width(4);
  *output << maxTh;
  output->width(4);
  *output << maxp;
  output->width(4);
  *output << wq;
  output->width(6);
  *output << CWND;
  output->width(4);
  *output <<totalLoad;
  output->width(4);
  *output << UDPshare;
  output->width(12);
  *output << goodput << "\n"; 
  output->flush();

  return 0;
}

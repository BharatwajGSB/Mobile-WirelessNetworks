/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 
 *
 * This is an example script for AODV manet routing protocol. 
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */

#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/v4ping-helper.h"
#include <iostream>
#include <cmath>
#include "ns3/applications-module.h"
//#include "ns3/flow-monitor-module.h"

using namespace ns3;

/**
 * \brief Test script.
 * 
 * This script creates 1-dimensional grid topology and then ping last node from the first one:
 * 
 * [10.0.0.1] <-- step --> [10.0.0.2] <-- step --> [10.0.0.3] <-- step --> [10.0.0.4]
 * 
 * ping 10.0.0.4
 */
class AodvExample 
{
public:
  AodvExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /// Report results
  void Report (std::ostream & os);


private:

  // parameters
  /// Number of nodes
  uint32_t size;
  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;

  // network
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
    // flowmonitor doesnt seem to work with AODV
    //FlowMonitorHelper flowmon;
    //Ptr<FlowMonitor> monitor ;


private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
};

void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
    Vector position = model->GetPosition ();
    NS_LOG_UNCOND (context <<
                   " x = " << position.x << ", y = " << position.y);
}

int main (int argc, char **argv)
{
  AodvExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  size (25),
  step (100),
  totalTime (10),
  pcap (true),
  printRoutes (true)
{
}

bool
AodvExample::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  //LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);
    LogComponentEnable("MobilityHelper", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);

  cmd.Parse (argc, argv);
  return true;
}

void
AodvExample::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
    
    std::ostringstream oss;
  
    for (int i =0; i<size; i++)
    {
        oss.str("");
        oss << "/NodeList/" << nodes.Get (i)->GetId () << "/$ns3::MobilityModel/CourseChange";
        Config::Connect (oss.str (), MakeCallback (&CourseChange));
    
  //    oss << "/NodeList/" << nodes.Get (1)->GetId () << "/$ns3::MobilityModel/CourseChange";
  //  Config::Connect (oss.str (), MakeCallback (&CourseChange));
    }

  Simulator::Run ();

    // Print flow charactristics doesnt seem to work with AODV
    
    /*
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
        // first 2 FlowIds are for ECHO apps, we don't want to display them
        //
        // Duration for throughput measurement is 9.0 seconds, since
        //   StartTime of the OnOffApplication is at about "second 1"
        // and
        //   Simulator::Stops at "second 10".
        if (i->first > 0)
        {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
            std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
            std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
            std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
            std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
            std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
        }
    }
     */
  
    
    
  Simulator::Destroy ();
}

void
AodvExample::Report (std::ostream &)
{ 
}

void
AodvExample::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
  nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), nodes.Get (i));
    }
  // Create static grid
  MobilityHelper mobility;

  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
 // mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));

 /*   mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (step),
                                   "DeltaY", DoubleValue (step),
                                   "GridWidth", UintegerValue (size/10),
                                   "LayoutType", StringValue ("RowFirst"));
  */
    int64_t streamIndex = 0; // used to get consistent mobility across scenarios
    
    ObjectFactory pos;
    pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
    pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    
    Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
    streamIndex += taPositionAlloc->AssignStreams (streamIndex);
     /*
    mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator", "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"), "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=100]") );
    mobility.Install (nodes);
      */
      
    mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", "Speed",StringValue ("ns3::UniformRandomVariable[Min=1|Max=12]"),"PositionAllocator", PointerValue (taPositionAlloc));
    
    mobility.SetPositionAllocator (taPositionAlloc);
    mobility.Install (nodes);


    
}

void
AodvExample::CreateDevices ()
{
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes); 

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}

void
AodvExample::InstallInternetStack ()
{
  AodvHelper aodv;
  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  if (printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
      aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);
    }
}

void
AodvExample::InstallApplications ()
{
   // Ping application
  /*V4PingHelper ping (interfaces.GetAddress (size - 1));
  ping.SetAttribute ("Verbose", BooleanValue (true));

  ApplicationContainer p = ping.Install (nodes.Get (0));
  p.Start (Seconds (0));
  p.Stop (Seconds (totalTime) - Seconds (0.001));
   */
   
    uint16_t sinkPort = 8080;
    Address sinkAddress (InetSocketAddress (interfaces.GetAddress (0),
                                            sinkPort));
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                                       sinkAddress);
    ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get (0));
    sinkApps.Start (Seconds (0));
    sinkApps.Stop (Seconds (totalTime)-Seconds (0.001));
    
   // Address sourceAddress (InetSocketAddress (interfaces.GetAddress (1), sinkPort));
    OnOffHelper onOffHelper ("ns3::TcpSocketFactory", sinkAddress);
    onOffHelper.SetAttribute ("OnTime", StringValue
                              ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue
                              ("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute ("DataRate",StringValue ("1Mbps"));
    onOffHelper.SetAttribute ("PacketSize", UintegerValue (1024));
    
    ApplicationContainer sourceApps;
    sourceApps.Add (onOffHelper.Install (nodes.Get(1)));
    
    // 8. Install FlowMonitor on all nodes
    // Flowmonitor doesn't seem to work with AODV type protocols
    //Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
    
    sourceApps.Start (Seconds (0));
    sourceApps.Stop (Seconds (totalTime)-Seconds (0.001));


  // move node away
  Ptr<Node> node = nodes.Get (size/2);
  Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (5e3, 5e3, 5e3));
}


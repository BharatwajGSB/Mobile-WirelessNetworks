/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*

 

 *

 * This is an example script for AODV manet routing protocol. 

 *

 * Authors: Pavel Boyko <boyko@iitp.ru>

 */
#include "ns3/applications-module.h"
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
using namespace ns3;

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

    NS_LOG_UNCOND (context <<" x = " << position.x << ", y = " << position.y);

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

  
    // error int ccompariosn -GSB
    for (uint32_t i =0; i<size; i++)
    {

        oss.str("");

        oss << "/NodeList/" << nodes.Get (i)->GetId () << "/$ns3::MobilityModel/CourseChange";

        Config::Connect (oss.str (), MakeCallback (&CourseChange));

  //    oss << "/NodeList/" << nodes.Get (1)->GetId () << "/$ns3::MobilityModel/CourseChange";

  //  Config::Connect (oss.str (), MakeCallback (&CourseChange));
    }



  Simulator::Run ();

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


MobilityHelper mobilityAdhoc;
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

 /* std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
*/ 
//mobilityAdhoc.SetPositionAllocator (taPositionAlloc); 
mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue ("ns3::UniformRandomVariable[Min=20|Max=70]"),
                                  "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
  mobilityAdhoc.Install (nodes);
 
}

void

AodvExample::CreateDevices ()

{
  /* gave me error - GSB
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  */
  
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC


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

void AodvExample::InstallApplications ()
{
  V4PingHelper ping (interfaces.GetAddress (size - 1));
  ping.SetAttribute ("Verbose", BooleanValue (true));

        uint16_t sinkPort = 8080;
    Address sinkAddress (InetSocketAddress (interfaces.GetAddress (0),
sinkPort));
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install
(nodes.Get (0));
    sinkApps.Start (Seconds (0.));
    sinkApps.Stop (Seconds (60.));

    OnOffHelper onOffHelper ("ns3::TcpSocketFactory", sinkAddress);
    onOffHelper.SetAttribute ("OnTime", StringValue
("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue
("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute ("DataRate",StringValue ("5Mbps"));
    onOffHelper.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer source;

    source.Add (onOffHelper.Install (nodes.Get(0)));
    source.Start (Seconds (1.1));
    source.Stop (Seconds (60.0));

  // move node away
  Ptr<Node> node = nodes.Get (size/2);
  Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (1e5, 1e5, 1e5));
}



/* EECE 7364 - Team Project
 Performance evaluation of WiFi adhoc routing protocols (AODV, OLSR, DSDV)
 Team Members: Bharatwaj Gowri Shankar, Shenghe Zhao & Vikrant Shah
 
 
 Objective:
The goal of this project was to evaluate the performance of On-demand Distance Vector (AODV), Destination Sequenced Distance-Vector (DSDV) and Optimized Link State Routing (OLSR) Ad hoc routing protocols protocols in a varying sized WiFi environments comprising multi-hop paths.
 */

#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;
using namespace dsr;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Adhoc-routing-compare");

int nRuns;

class RoutingExperiment
{
public:
  RoutingExperiment ();
  void Run (int nSinks, int nSources, double txp, std::string CSVfileName, int64_t streamIndex);
  static void SetMACParam (ns3::NetDeviceContainer & devices, int slotDistance);
  std::string CommandSetup (int argc, char **argv);


private:
  Ptr<Socket> SetupPacketReceive (Ipv4Address addr, Ptr<Node> node);
  void ReceivePacket (Ptr<Socket> socket);
  void CheckThroughput ();

  uint32_t port;
  uint32_t bytesTotal;
  uint32_t TotalDataRcd;

  uint32_t packetsReceived;
  uint32_t TotalPacketsRcd;

  std::string m_CSVfileName;
  int m_nSinks;
  int m_nSources;
  std::string m_protocolName;
  double m_txp;
  bool m_traceMobility;
  uint32_t m_protocol;
    
    uint32_t RunTxPackets;
    uint32_t RunTxBytes;
    uint32_t RunRxPackets;
    uint32_t RunRxBytes;
    uint32_t RunDelay;
    
    uint32_t TotalTxPackets;
    uint32_t TotalTxBytes;
    uint32_t TotalRxPackets;
    uint32_t TotalRxBytes;
    uint32_t TotalDelay;

};

RoutingExperiment::RoutingExperiment ()
  : port (9),
    bytesTotal (0),
    packetsReceived (0),
    m_CSVfileName ("Adhoc-routing.output.csv"),
    m_traceMobility (false),
    m_protocol (2), // 1=OLSR;2=AODV;3=DSDV;4=DSR
    RunTxPackets(0),
    RunTxBytes(0),
    RunRxPackets(0),
    RunRxBytes(0),
    RunDelay(0),
    TotalTxPackets(0),
    TotalTxBytes(0),
    TotalRxPackets(0),
    TotalRxBytes(0),
    TotalDelay(0)
{
}

// Uncomment the following section to turn on packet notification

/* static inline std::string
PrintReceivedPacket (Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress)
{
  std::ostringstream oss;

  oss << Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId ();

  if (InetSocketAddress::IsMatchingType (senderAddress))
    {
      InetSocketAddress addr = InetSocketAddress::ConvertFrom (senderAddress);
      oss << " received one packet from " << addr.GetIpv4 ();
    }
  else
    {
      oss << " received one packet!";
    }
    
    return oss.str ();
    
}
*/
 
void
RoutingExperiment::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address senderAddress;
  while ((packet = socket->RecvFrom (senderAddress)))
    {
      bytesTotal += packet->GetSize ();
      TotalDataRcd += packet->GetSize ();

      packetsReceived += 1;
      TotalPacketsRcd += 1;
        
// Uncomment the following section to turn on packet notification
        
//  NS_LOG_UNCOND (PrintReceivedPacket (socket, packet, senderAddress));
    }
}

void
RoutingExperiment::CheckThroughput ()
{
  double kbs = (bytesTotal * 8.0) / 1000;
  bytesTotal = 0;
    std::stringstream ss4;
    ss4 << nRuns;
    std::string sRate = ss4.str ();  std::ofstream out (sRate+m_CSVfileName.c_str (), std::ios::app);
  out << (Simulator::Now ()).GetSeconds () << ","
      << kbs << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_nSources << ","
      << m_protocolName << ","
      << m_txp << ""
      << std::endl;

  out.close ();
  packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &RoutingExperiment::CheckThroughput, this);
}

Ptr<Socket>
RoutingExperiment::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&RoutingExperiment::ReceivePacket, this));

  return sink;
}

std::string
RoutingExperiment::CommandSetup (int argc, char **argv)
{
  CommandLine cmd;
  cmd.AddValue ("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
  cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR", m_protocol);
  cmd.Parse (argc, argv);
  return m_CSVfileName;
}

int
main (int argc, char *argv[])
{
  RoutingExperiment experiment;
  std::string CSVfileName = experiment.CommandSetup (argc,argv);

  //blank out the last output file and write the column headers
  std::ofstream out (CSVfileName.c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "NumberOfSinks," <<
  "RoutingProtocol," <<
  "TransmissionPower" <<
  std::endl;
  out.close ();

  int nSinks = 1;
  
  int nSources = 5; // Configure number of source here
  double txp = -5 ; //2.5 * 2.5 of the -5db = 2.6

  int64_t streamIndex = 0; // used to get consistent mobility across scenarios
  nRuns = 1;

    for (int iruns = 0; iruns<nRuns; iruns++)
    {
        experiment.Run (nSinks, nSources, txp, CSVfileName, streamIndex);
        streamIndex = streamIndex+50;
    }
}

void
RoutingExperiment::Run (int nSinks, int nSources, double txp, std::string CSVfileName,  int64_t streamIndex)
{
  Packet::EnablePrinting ();
  m_nSinks = nSinks;
  m_nSources = nSources;
  m_txp = txp;
  m_CSVfileName = CSVfileName;

  int nWifis = 75;

  double TotalTime = 150.0;
  std::string rate ("160kbps");
  std::string phyMode ("DsssRate11Mbps");
  std::string tr_name ("adhoc-rt-cmpr");
  int nodeSpeed = 12; //in m/s
  int nodePause = 0; //in s
  double posMax = 100.0;
  m_protocolName = "protocol";

  //Set Non-unicastMode rate to unicast mode
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));

  NodeContainer sinkNodes;
  NodeContainer adhocNodes;
  sinkNodes.Create (nSinks);
  adhocNodes.Create (nWifis);

  // setting up wifi phy and channel using helpers
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  // Configure Constant speed prop delay and log distance prop loss
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel ;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");

  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

    std::string ssidString ("wifi-Apwifi");
    int nodeId = 1;
    std::stringstream sss;
    sss << nodeId;
    ssidString += sss.str ();
    Ssid ssid = Ssid (ssidString);
    
    wifiMac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer sinkDevices = wifi.Install (wifiPhy, wifiMac, sinkNodes);

    wifiMac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer adhocDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);
    
    MobilityHelper mobilityAdhoc;
    MobilityHelper sinkmobilityAdhoc;

  // make posMax command line
  ObjectFactory pos;
  std::stringstream ssPos;
  ssPos << "ns3::UniformRandomVariable[Min=0.0|Max=" << posMax << "]";
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue (ssPos.str ()));
  pos.Set ("Y", StringValue (ssPos.str ()));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=1|Max=" << nodeSpeed << "]";
   // Configure mobility pause behaviour here
  //std::stringstream ssPause;
  //ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);

  mobilityAdhoc.Install (adhocNodes);
    
    Ptr<ListPositionAllocator> positionAllocS = CreateObject<ListPositionAllocator> ();
    positionAllocS->Add(Vector(posMax/2, posMax/2, 0.0));// node 0
    sinkmobilityAdhoc.SetPositionAllocator(positionAllocS);
    sinkmobilityAdhoc.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    
    sinkmobilityAdhoc.Install(sinkNodes);
    
    streamIndex += mobilityAdhoc.AssignStreams (adhocNodes, streamIndex);
  
  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  DsrHelper dsr;
  DsrMainHelper dsrMain;
  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;

  switch (m_protocol)
    {
    case 1:
      list.Add (olsr, 100);
      m_protocolName = "OLSR";
      break;
    case 2:
      list.Add (aodv, 100);
      m_protocolName = "AODV";
      break;
    case 3:
      list.Add (dsdv, 100);
      m_protocolName = "DSDV";
      break;
    case 4:
      m_protocolName = "DSR";
      break;
    default:
      NS_FATAL_ERROR ("No such protocol:" << m_protocol);
    }

  if (m_protocol < 4)
    {
      internet.SetRoutingHelper (list);
      internet.Install (adhocNodes);
      internet.Install (sinkNodes);
    }
  else if (m_protocol == 4)
    {
      internet.Install (adhocNodes);
      internet.Install (sinkNodes);
      dsrMain.Install (dsr, adhocNodes);
    }

  NS_LOG_INFO ("assigning ip address");

  Ipv4AddressHelper addressAdhoc;
  addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer sinkApInterfaces;
  sinkApInterfaces = addressAdhoc.Assign (sinkDevices); 
  Ipv4InterfaceContainer adhocInterfaces;
  adhocInterfaces = addressAdhoc.Assign (adhocDevices);

    AddressValue remoteAddress (InetSocketAddress (adhocInterfaces.GetAddress (0), port));
Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("72")); //100-28 (UDP overhead) = 72
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue(rate));
  unsigned int maxBytes = 20000*72; // 20,000 x packetsize
  std::stringstream ssmaxBytes;
  ssmaxBytes << maxBytes;
  Config::SetDefault ("ns3::OnOffApplication::MaxBytes", StringValue (ssmaxBytes.str()));

    
  OnOffHelper onoff1 ("ns3::UdpSocketFactory",Address(InetSocketAddress (sinkApInterfaces.GetAddress (0), port)));
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    
    Ptr<Socket> sink = SetupPacketReceive (sinkApInterfaces.GetAddress (0), sinkNodes.Get (0));
    
    
    
  for (int i = 0; i < nSources; i++)
    {
      
      Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable> ();
      ApplicationContainer temp = onoff1.Install (adhocNodes.Get (i));
      temp.Start (Seconds (var->GetValue (0,1)));
      temp.Stop (Seconds (TotalTime-0.01));
    }

 PacketSinkHelper sinkk ("ns3::UdpSocketFactory",
                         InetSocketAddress (sinkApInterfaces.GetAddress (0), port));
  ApplicationContainer temp = sinkk.Install (sinkNodes.Get(0));
  temp.Start (Seconds (0));

    
  std::stringstream ss;
  ss << nWifis;
  std::string nodes = ss.str ();

  std::stringstream ss2;
  ss2 << nodeSpeed;
  std::string sNodeSpeed = ss2.str ();

  std::stringstream ss3;
  ss3 << nodePause;
  std::string sNodePause = ss3.str ();

  std::stringstream ss4;
  ss4 << rate;
  std::string sRate = ss4.str ();
    
   static int iteration=0;
    
    std::stringstream ss5;
    ss5 << iteration;
    std::string siteration = ss5.str ();
    
    std::stringstream ss6;
    ss6 << posMax;
    std::string ssposMax = ss6.str ();
    
    NS_LOG_INFO ("Configure Tracing.");
    
  tr_name = tr_name + "_" + m_protocolName +"_" + nodes + "nodes_" + siteration + "iteration_" + sNodePause + "pause_" + sRate + "rate_"+ssposMax+"grid";

    
  AsciiTraceHelper ascii;
  MobilityHelper::EnableAsciiAll (ascii.CreateFileStream (tr_name + ".mob"));

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();


  NS_LOG_INFO ("Run Simulation.");

  CheckThroughput ();

  Simulator::Stop (Seconds (TotalTime));
    
  AnimationInterface anim ("adhoc_routing.xml");
  anim.SetMaxPktsPerTraceFile(500000);  //Get rid of the error

 
    std::string it = std::to_string(iteration);
    wifiPhy.EnablePcap (it, sinkDevices);
    iteration++;
    
  Simulator::Run ();

  flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), false, false);

   flowmon->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = flowmon->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      // Duration for throughput measurement is 100.0 seconds, so..
        
      //  Uncomment the following section to display flow statistics for each node pair
        
      //  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
              if ((int)i->first <= nSources)
       // if (t.destinationAddress == "10.1.1.1")
      {
          /*
          std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 100 / 1000   << " kbps\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          if(i->second.rxPackets)
          std::cout << "  Avg Delay:  " << (i->second.delaySum)/(i->second.rxPackets) << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 100 / 1000   << " kbps\n";
          */
           
          RunTxPackets += i->second.txPackets ;
          RunTxBytes += i->second.txBytes;
          RunRxPackets += i->second.rxPackets;
          RunRxBytes += i->second.rxBytes;
          if(i->second.rxPackets)
          {
              RunDelay += (i->second.delaySum.GetMilliSeconds())/(i->second.rxPackets) ;
          }
        }
 
    }
    std::cout << "\n  Avg Tx Packets this run: " << RunTxPackets/nSources << "\n";
    std::cout << "  Avg Tx Bytes this run:   " << RunTxBytes/nSources << "\n";
    std::cout << "  Avg Rx Packets this run: " << RunRxPackets/nSources << "\n";
    std::cout << "  Avg Rx Bytes this run:   " << RunRxBytes/nSources << "\n";
    std::cout << "  Avg Delay this run:  " << RunDelay/nSources << "\n";
    std::cout << "  Avg Throughput this run: " << RunRxBytes/nSources * 8.0 / 100 / 1000   << " kbps\n";
    
    TotalTxPackets += RunTxPackets/nSources; RunTxPackets = 0;
    TotalTxBytes += RunTxBytes/nSources; RunTxBytes = 0;
    TotalRxPackets += RunRxPackets/nSources; RunRxPackets = 0;
    TotalRxBytes += RunRxBytes/nSources; RunRxBytes = 0;
    TotalDelay+= RunDelay/nSources; RunDelay = 0;
    static int checker =1;
    if(checker == nRuns)
    {
    std::cout << "\n\n  Avg Tx Packets overall: " << TotalTxPackets/nRuns << "\n";
    std::cout << "  Avg Tx Bytes overall:   " << TotalTxBytes/nRuns << "\n";
    std::cout << "  Avg Rx Packets overall: " << TotalRxPackets/nRuns << "\n";
    std::cout << "  Avg Rx Bytes this overall:   " << TotalRxBytes/nRuns << "\n";
    std::cout << "  Avg Delay overall:  " << TotalDelay/nRuns << "\n";
    std::cout << "  Avg Throughput overall: " << TotalRxBytes/nRuns * 8.0 / 100 / 1000   << " kbps\n\n";
    }
    checker++;


  Simulator::Destroy ();
}

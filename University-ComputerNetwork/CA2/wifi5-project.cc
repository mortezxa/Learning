#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/wifi-module.h"
#include <cmath>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Wifi5ProjectPhase1");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  cout << "***Creating Nodes..." << endl;

  NodeContainer apNode;
  apNode.Create (1);
  NodeContainer staNodes;
  staNodes.Create (5);

  cout << "***Setting up Uniform Circular Positions (Radius = 5m)..." << endl;

  MobilityHelper apMobility;
  Ptr<ListPositionAllocator> apAlloc = CreateObject<ListPositionAllocator> ();
  apAlloc->Add (Vector (0.0, 0.0, 0.0));
  apMobility.SetPositionAllocator (apAlloc);
  apMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  apMobility.Install (apNode);

  MobilityHelper staMobility;
  Ptr<ListPositionAllocator> staAlloc = CreateObject<ListPositionAllocator> ();
  
  double radius = 5.0;
  
  for (int i = 0; i < 5; ++i) {
      double angle = i * (2.0 * M_PI / 5.0); 
      double x = radius * cos(angle);
      double y = radius * sin(angle);
      staAlloc->Add (Vector (x, y, 0.0));
  }
  
  staMobility.SetPositionAllocator (staAlloc);
  staMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  staMobility.Install (staNodes);

  cout << "***Configuring Wi-Fi 5 (802.11ac)..." << endl;

  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211ac);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.Set ("RxNoiseFigure", DoubleValue (20.0)); //noise
  phy.SetChannel (channel.Create ());

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns3-wifi5-network");
  
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer apDevice = wifi.Install (phy, mac, apNode);

  mac.SetType ("ns3::StaWifiMac", 
               "Ssid", SsidValue (ssid), 
               "ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices = wifi.Install (phy, mac, staNodes);

  cout << "Wi-Fi setup is complete. " << apDevice.GetN() << " AP and " << staDevices.GetN() << " STAs are configured." << endl;
  cout << "-----------------------------------" << endl;

  cout << "***IP Addressing, Traffic Generation & FlowMonitor..." << endl;

  InternetStackHelper stack;
  stack.Install (apNode);
  stack.Install (staNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");

  Ipv4InterfaceContainer apInterface = address.Assign (apDevice);
  Ipv4InterfaceContainer staInterfaces = address.Assign (staDevices);

  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (apNode.Get (0));
  serverApps.Start (Seconds (0.0)); 
  serverApps.Stop (Seconds (10.0));

  for (uint32_t i = 0; i < 5; ++i) {
      UdpEchoClientHelper echoClient (apInterface.GetAddress (0), 9);
      echoClient.SetAttribute ("MaxPackets", UintegerValue (20));
      echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));

      if (i == 0 || i == 2 || i == 4) {
          echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
      } else {
          echoClient.SetAttribute ("PacketSize", UintegerValue (512));
      }

      ApplicationContainer clientApp = echoClient.Install (staNodes.Get (i));
      clientApp.Start (Seconds (0.0));
      clientApp.Stop (Seconds (10.0));
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Ptr<MobilityModel> apMob = apNode.Get(0)->GetObject<MobilityModel>();
  cout << "AP Position: (" << apMob->GetPosition().x << ", " << apMob->GetPosition().y << ")\n";
  for (uint32_t i = 0; i < staNodes.GetN(); ++i) {
      Ptr<MobilityModel> staMob = staNodes.Get(i)->GetObject<MobilityModel>();
      cout << "STA " << i+1 << " Position: (" << staMob->GetPosition().x << ", " << staMob->GetPosition().y << ")\n";
  }
  cout << "-----------------------------------\n";

  Simulator::Stop (Seconds (10.0)); 
  Simulator::Run ();

  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

  double sumThroughput = 0.0;
  double sumThroughputSq = 0.0;
  double totalDelay = 0.0;
  int validFlows = 0;

  cout << "\n=== Flow Monitor Results (STA to AP) ===\n";
  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
      
      if (t.destinationPort == 9) {
          double rxDuration = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
          double throughput = 0.0;
          if (rxDuration > 0) {
              throughput = (i->second.rxBytes * 8.0) / rxDuration / 1024;
          }
          double delay = 0.0;
          if (i->second.rxPackets > 0) {
              delay = i->second.delaySum.GetSeconds() / i->second.rxPackets;
          }

          cout << "Flow ID " << i->first << " | " << t.sourceAddress << " -> " << t.destinationAddress << "\n";
          cout << "  Tx Packets: " << i->second.txPackets << "\n";
          cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          cout << "  Throughput: " << throughput << " Kbps\n";
          cout << "  Delay     : " << delay << " s\n\n";

          sumThroughput += throughput;
          sumThroughputSq += (throughput * throughput);
          totalDelay += delay;
          validFlows++;
      }
  }

  if (validFlows > 0) {
      cout << "=== Overall Network Metrics ===\n";
      cout << "Average Throughput: " << (sumThroughput / validFlows) << " Kbps\n";
      cout << "Average Delay     : " << (totalDelay / validFlows) << " s\n";
      
      double jainsIndex = (sumThroughput * sumThroughput) / (validFlows * sumThroughputSq);
      cout << "Jain's Fairness Index: " << jainsIndex << "\n";
      cout << "===============================\n";
  }

  monitor->SerializeToXmlFile("wifi5-flowmon.xml", true, true);
  Simulator::Destroy ();
  return 0;
}
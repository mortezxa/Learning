#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include <cmath>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Wifi6Project");

int main (int argc, char *argv[])
{
  uint32_t nWifi = 5;
  double radius = 5.0;
  double simulationTime = 10.0;

  CommandLine cmd;
  cmd.Parse (argc, argv);

  cout << "***Creating Nodes..." << endl;
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  cout << "***Setting up Uniform Circular Positions (Radius = " << radius << "m)..." << endl;
  MobilityHelper mobility;
  
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  
  for (uint32_t i = 0; i < nWifi; ++i) {
      double theta = i * 2.0 * M_PI / nWifi;
      double x = radius * cos(theta);
      double y = radius * sin(theta);
      positionAlloc->Add (Vector (x, y, 0.0));
  }

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

  cout << "***Configuring Wi-Fi 6 (802.11ax) with OFDMA..." << endl;
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211ax);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                             "DataMode", StringValue("HeMcs7"),
                             "ControlMode", StringValue("HeMcs0"));

  SpectrumWifiPhyHelper phy;
  Config::SetDefault ("ns3::HeConfiguration::GuardInterval", TimeValue (NanoSeconds (800)));

  phy.Set ("ChannelSettings", StringValue ("{0, 40, BAND_5GHZ, 0}"));
  phy.Set("Antennas", UintegerValue(2));
  phy.Set("MaxSupportedTxSpatialStreams", UintegerValue(2));
  phy.Set("MaxSupportedRxSpatialStreams", UintegerValue(2));


  
  
  Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
  spectrumChannel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);
  phy.SetChannel (spectrumChannel);

  phy.Set ("RxNoiseFigure", DoubleValue (20.0));
  phy.Set("TxPowerStart", DoubleValue(20.0));
  phy.Set("TxPowerEnd", DoubleValue(20.0));
  
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-wifi6");
  
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));
  NetDeviceContainer staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));

  mac.SetMultiUserScheduler ("ns3::RrMultiUserScheduler",
                             "EnableUlOfdma", BooleanValue (true),
                             "EnableBsrp", BooleanValue (true));
  NetDeviceContainer apDevice = wifi.Install (phy, mac, wifiApNode);

  cout << "Wi-Fi setup complete with 1 AP and " << nWifi << " STAs." << endl;
  cout << "***IP Addressing, Traffic Generation & FlowMonitor..." << endl;
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface = address.Assign (apDevice);
  Ipv4InterfaceContainer staInterfaces = address.Assign (staDevices);

  uint16_t port = 9;
  UdpEchoServerHelper echoServer (port);
  ApplicationContainer serverApps = echoServer.Install (wifiApNode.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (simulationTime));

  for (uint32_t i = 0; i < nWifi; ++i) {
      UdpEchoClientHelper echoClient (apInterface.GetAddress (0), port);
      echoClient.SetAttribute ("MaxPackets", UintegerValue (20));
      echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
      if (i == 0 || i == 2 || i == 4) {
          echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
      } else {
          echoClient.SetAttribute ("PacketSize", UintegerValue (512));
      }
      ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (i));
      clientApps.Start (Seconds (2.0 + (i * 0.01)));
      clientApps.Stop (Seconds (simulationTime));
  }
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop (Seconds (simulationTime + 1.0));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  double totalThroughput = 0.0;
  double totalDelay = 0.0;
  uint32_t numFlows = 0;
  double sumThroughput = 0.0;
  double sumThroughputSq = 0.0;

  cout << "\n--- Final Flow Monitor Results ---" << endl;
  for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.destinationAddress == "192.168.1.1") {
          double throughput = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1000;
          double delay = i->second.delaySum.GetSeconds() / i->second.rxPackets;
          
          cout << "Flow ID " << i->first << " | " << t.sourceAddress << " -> " << t.destinationAddress << endl;
          cout << "   Tx Packets: " << i->second.txPackets << endl;
          cout << "   Rx Packets: " << i->second.rxPackets << endl;
          cout << "   Throughput: " << throughput << " Kbps" << endl;
          cout << "   Delay: " << delay << " s\n" << endl;

          totalThroughput += throughput;
          totalDelay += delay;
          numFlows++;
          sumThroughput += throughput;
          sumThroughputSq += (throughput * throughput);
      }
  }

  if (numFlows > 0) {
      double avgThroughput = totalThroughput / numFlows;
      double avgDelay = totalDelay / numFlows;
      double jainFairnessIndex = (sumThroughput * sumThroughput) / (numFlows * sumThroughputSq);

      cout << "=== Overall Network Metrics ===" << endl;
      cout << "Average Throughput: " << avgThroughput << " Kbps" << endl;
      cout << "Average Delay: " << avgDelay << " s" << endl;
      cout << "Jain's Fairness Index: " << jainFairnessIndex << endl;
  }

  Simulator::Destroy ();
  return 0;
}

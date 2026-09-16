#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-tx-vector.h"
#include <cmath> 

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Phase3Uora");

double totalSinrLinear = 0.0;
uint32_t sinrSampleCount = 0;

void SinrTrace(string context, Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise, uint16_t staId) {
    double signalW = pow(10.0, (signalNoise.signal - 30.0) / 10.0);
    double noiseW = pow(10.0, (signalNoise.noise - 30.0) / 10.0);
    
    if (noiseW > 0) {
        double sinr = signalW / noiseW;
        totalSinrLinear += sinr;
        sinrSampleCount++;
    }
}

int main (int argc, char *argv[])
{
  uint32_t nStas = 40;
  double simulationTime = 11.0; 

  CommandLine cmd;
  cmd.AddValue ("nStas", "Number of STA nodes", nStas);
  cmd.Parse (argc, argv);

  NodeContainer wifiApNode;
  wifiApNode.Create (1);
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nStas);

  SpectrumWifiPhyHelper phy;
  phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  SpectrumChannelHelper channelHelper;
  channelHelper.SetChannel ("ns3::MultiModelSpectrumChannel");
  channelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channelHelper.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");
  Ptr<SpectrumChannel> channel = channelHelper.Create();

  phy.SetChannel (channel);
  phy.Set ("ChannelSettings", StringValue ("{38, 40, BAND_5GHZ, 0}"));
  Config::SetDefault ("ns3::HeConfiguration::GuardInterval", TimeValue (NanoSeconds (800)));

  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211ax);
  
  Config::SetDefault ("ns3::WifiMacQueue::MaxDelay", TimeValue (MilliSeconds (500)));
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", 
                                "DataMode", StringValue ("HeMcs4"), 
                                "ControlMode", StringValue ("HeMcs4"));

  WifiMacHelper mac;
  mac.SetMultiUserScheduler ("ns3::RrMultiUserScheduler",
                            "EnableUlOfdma", BooleanValue (true),
                            "EnableBsrp", BooleanValue (true),
                            "UseCentral26TonesRus", BooleanValue (true));

  Ssid ssid = Ssid ("ns-3-80211ax-uora");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (true));
  NetDeviceContainer staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  NetDeviceContainer apDevice = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                 "X", StringValue ("0.0"),
                                 "Y", StringValue ("0.0"),
                                 "Rho", StringValue ("ns3::UniformRandomVariable[Min=5.0|Max=50.0]"));
  mobility.Install (wifiStaNodes);

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterfaces = address.Assign (staDevices);
  Ipv4InterfaceContainer apNodeInterface = address.Assign (apDevice);

  UdpServerHelper server (9);
  ApplicationContainer serverApp = server.Install (wifiApNode.Get (0));
  serverApp.Start (Seconds (1.0));
  serverApp.Stop (Seconds (simulationTime - 1.0));

  uint32_t payloadSize = 93; 
  Time interPacketInterval = Seconds(0.005); 
  
  Ptr<UniformRandomVariable> randomTime = CreateObject<UniformRandomVariable>();
  randomTime->SetAttribute("Min", DoubleValue(0.0));
  randomTime->SetAttribute("Max", DoubleValue(0.5));

  for (uint32_t i = 0; i < nStas; ++i)
    {
      UdpClientHelper client (apNodeInterface.GetAddress (0), 9);
      client.SetAttribute ("MaxPackets", UintegerValue (1000000));
      client.SetAttribute ("Interval", TimeValue (interPacketInterval));
      client.SetAttribute ("PacketSize", UintegerValue (payloadSize));
      
      ApplicationContainer clientApp = client.Install (wifiStaNodes.Get (i));
      
      double startDelay = 2.0 + randomTime->GetValue(); 
      clientApp.Start (Seconds (startDelay));
      clientApp.Stop (Seconds (simulationTime - 1.0));
    }

  phy.EnablePcapAll ("phase3-uora");

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  Config::Connect("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback(&SinrTrace));

  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  double totalThroughput = 0.0;
  double totalDelay = 0.0;
  uint32_t totalRxPackets = 0;
  uint32_t totalLostPackets = 0;
  uint32_t rxFlows = 0;

  cout << "\n=======================================" << endl;
  for (auto const& [flowId, flowStats] : stats)
    {
      totalLostPackets += flowStats.lostPackets;
      if (flowStats.rxPackets > 0)
        {
          double throughput = (flowStats.rxBytes * 8.0) / (flowStats.timeLastRxPacket.GetSeconds () - flowStats.timeFirstTxPacket.GetSeconds ()) / 1e6;
          totalThroughput += throughput;
          totalDelay += flowStats.delaySum.GetSeconds () / flowStats.rxPackets;
          totalRxPackets += flowStats.rxPackets;
          rxFlows++;
        }
    }

  cout << "--- Phase 3 (UORA) Results ---" << endl;
  cout << "Total Packets Received : " << totalRxPackets << endl;
  cout << "Total Packets Lost     : " << totalLostPackets << endl;
  cout << "Overall Throughput     : " << totalThroughput << " Mbps" << endl;
  if (rxFlows > 0) {
    cout << "Average Delay          : " << (totalDelay / rxFlows) * 1000 << " ms" << endl;
  }
  if (sinrSampleCount > 0) {
    double avgSinrDb = 10 * log10(totalSinrLinear / sinrSampleCount);
    cout << "Average SINR           : " << avgSinrDb << " dB" << endl;
  }
  cout << "=======================================\n" << endl;

  Simulator::Destroy ();
  return 0;
}


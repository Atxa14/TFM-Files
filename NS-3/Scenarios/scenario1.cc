/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
// Copyright (c) 2023 Universidad del País Vasco / Euskal Herriko Unibertsitatea (UPV/EHU)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/applications-module.h"
#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/delay-jitter-estimation.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/stats-module.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-acknowledgment.h"
#include "ns3/wifi-tx-vector.h"
#include <ns3/spectrum-helper.h>
#include <ns3/three-gpp-ftp-m2-helper.h>
#include <ns3/traffic-generator-ngmn-gaming.h>
#include <ns3/traffic-generator-ngmn-video.h>
#include <ns3/traffic-generator-ngmn-voip.h>

#include <iostream>
#include <regex>

using namespace ns3;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LOG COMPONENT DEFINITION
NS_LOG_COMPONENT_DEFINE("Wifi-Scenario1"); // Logging Keyword definition

// Global variables for use in callbacks
// Signal-noise-samples variables for STAs
double g_signalDbmAvg[4];
double g_noiseDbmAvg[4];
uint32_t g_samples[4];

double g_signalDbmAvgSta[4];
double g_noiseDbmAvgSta[4];
uint32_t g_samplesSta[4];

/////////////////////////////////////////////////////////////////////////////////////////////
// Monitor for APs
void
MonitorSniffRxCommon(Ptr<const Packet> packet,
                     uint16_t channelFreqMhz,
                     WifiTxVector txVector,
                     MpduInfo aMpdu,
                     SignalNoiseDbm signalNoise,
                     uint16_t staId,
                     uint8_t groupIndex,
                     bool isStaVersion)
{
    if (isStaVersion)
    {
        g_samplesSta[groupIndex]++;
        g_signalDbmAvgSta[groupIndex] +=
            ((signalNoise.signal - g_signalDbmAvgSta[groupIndex]) / g_samplesSta[groupIndex]);
        g_noiseDbmAvgSta[groupIndex] +=
            ((signalNoise.noise - g_noiseDbmAvgSta[groupIndex]) / g_samplesSta[groupIndex]);
    }
    else
    {
        g_samples[groupIndex]++;
        g_signalDbmAvg[groupIndex] +=
            ((signalNoise.signal - g_signalDbmAvg[groupIndex]) / g_samples[groupIndex]);
        g_noiseDbmAvg[groupIndex] +=
            ((signalNoise.noise - g_noiseDbmAvg[groupIndex]) / g_samples[groupIndex]);
    }
}

void
MonitorSniffRxA(Ptr<const Packet> packet,
                uint16_t channelFreqMhz,
                WifiTxVector txVector,
                MpduInfo aMpdu,
                SignalNoiseDbm signalNoise,
                uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 0, false);
}

void
MonitorSniffRxASta(Ptr<const Packet> packet,
                   uint16_t channelFreqMhz,
                   WifiTxVector txVector,
                   MpduInfo aMpdu,
                   SignalNoiseDbm signalNoise,
                   uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 0, true);
}

void
MonitorSniffRxB(Ptr<const Packet> packet,
                uint16_t channelFreqMhz,
                WifiTxVector txVector,
                MpduInfo aMpdu,
                SignalNoiseDbm signalNoise,
                uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 1, false);
}

void
MonitorSniffRxBSta(Ptr<const Packet> packet,
                   uint16_t channelFreqMhz,
                   WifiTxVector txVector,
                   MpduInfo aMpdu,
                   SignalNoiseDbm signalNoise,
                   uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 1, true);
}

void
MonitorSniffRxC(Ptr<const Packet> packet,
                uint16_t channelFreqMhz,
                WifiTxVector txVector,
                MpduInfo aMpdu,
                SignalNoiseDbm signalNoise,
                uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 2, false);
}

void
MonitorSniffRxCSta(Ptr<const Packet> packet,
                   uint16_t channelFreqMhz,
                   WifiTxVector txVector,
                   MpduInfo aMpdu,
                   SignalNoiseDbm signalNoise,
                   uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 2, true);
}

void
MonitorSniffRxD(Ptr<const Packet> packet,
                uint16_t channelFreqMhz,
                WifiTxVector txVector,
                MpduInfo aMpdu,
                SignalNoiseDbm signalNoise,
                uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 3, false);
}

void
MonitorSniffRxDSta(Ptr<const Packet> packet,
                   uint16_t channelFreqMhz,
                   WifiTxVector txVector,
                   MpduInfo aMpdu,
                   SignalNoiseDbm signalNoise,
                   uint16_t staId)
{
    MonitorSniffRxCommon(packet, channelFreqMhz, txVector, aMpdu, signalNoise, staId, 3, true);
}

void
CreateNodesUniform(NodeContainer& wifiStaNodes,
                   NodeContainer& wifiApNodes,
                   uint32_t& nSta,
                   const uint32_t nAp,
                   const std::string& label)
{
    Ptr<UniformRandomVariable> n = CreateObject<UniformRandomVariable>();
    n->SetAttribute("Min", DoubleValue(1));
    n->SetAttribute("Max", DoubleValue(nSta + 0.01));
    nSta = n->GetValue();

    wifiStaNodes.Create(nSta);
    wifiApNodes.Create(nAp);
}

void
CreateNodesNormal(NodeContainer& wifiStaNodes,
                  NodeContainer& wifiApNodes,
                  uint32_t& nSta,
                  const uint32_t nAp,
                  const std::string& label,
                  const double mean,
                  const double variance)
{
    Ptr<NormalRandomVariable> n = CreateObject<NormalRandomVariable>();
    n->SetAttribute("Mean", DoubleValue(mean));
    n->SetAttribute("Variance", DoubleValue(variance));

    double value = n->GetValue();
    nSta = static_cast<uint32_t>(std::max(1.0, value)); // Ensure at least 1 STA

    wifiStaNodes.Create(nSta);
    wifiApNodes.Create(nAp);
}

int32_t
getRandomChannelNumber(int channelWidth, double frequency)
{
    ns3::Ptr<ns3::UniformRandomVariable> randomSelector =
        ns3::CreateObject<ns3::UniformRandomVariable>();
    randomSelector->SetAttribute("Min", ns3::DoubleValue(0));
    randomSelector->SetAttribute("Max", ns3::DoubleValue(2));

    if (frequency == 6)
    {
        if (channelWidth == 20)
        {
            return 1 + 4 * randomSelector->GetInteger();
        }
        else if (channelWidth == 40)
        {
            return 3 + 8 * randomSelector->GetInteger();
        }
        else if (channelWidth == 80)
        {
            return 7 + 16 * randomSelector->GetInteger();
        }
        else if (channelWidth == 160)
        {
            return 15 + 32 * randomSelector->GetInteger();
        }
    }
    else if (frequency == 5)
    {
        if (channelWidth == 20)
        {
            return 36 + 4 * randomSelector->GetInteger();
        }
        else if (channelWidth == 40)
        {
            return 38 + 8 * randomSelector->GetInteger();
        }
        else if (channelWidth == 80)
        {
            return 106 + 16 * randomSelector->GetInteger();
        }
        else if (channelWidth == 160)
        {
            return 50 + 64 * randomSelector->GetInteger();
        }
    }
    else if (frequency == 2.4 && channelWidth == 20)
    {
        return 1 + 5 * randomSelector->GetInteger();
    }

    return -1; // Invalid channelWidth or frequency
}

void
setChannelInfo(int nNetwork, double frequency, int channelWidth, std::string& channelStr)
{
    int32_t channelNumber = 0;

    if (frequency == 6 || frequency == 5 || frequency == 2.4)
    {
        channelNumber = getRandomChannelNumber(channelWidth, frequency);
        if (channelNumber == -1)
        {
            std::cout << "Wrong frequency value or channel width!" << std::endl;
            return;
        }

        std::ostringstream oss;
        oss << "{" << channelNumber << ", " << channelWidth;

        if (frequency == 6)
        {
            oss << ", BAND_6GHZ, 0";
        }
        else if (frequency == 5)
        {
            oss << ", BAND_5GHZ, 0";
        }
        else if (frequency == 2.4)
        {
            oss << ", BAND_2_4GHZ, 0";
        }

        oss << "}";
        channelStr += oss.str();
    }
    else
    {
        std::cout << "Wrong frequency value!" << std::endl;
        return;
    }
}

void
ConfigureWifiNetwork(uint32_t seedNumber,
                     uint32_t runNumber,
                     bool tracing,
                     WifiHelper wifi,
                     WifiMacHelper mac,
                     double frequency,
                     int mcs,
                     uint32_t channelWidth,
                     uint32_t gi,
                     uint32_t nFloors,
                     uint32_t nWalls,
                     uint32_t nAntennasAp,
                     uint32_t nAntennasSta,
                     uint32_t ntxSpatialStreamsAp,
                     uint32_t ntxSpatialStreamsSta,
                     uint32_t nrxSpatialStreamsAp,
                     uint32_t nrxSpatialStreamsSta,
                     double txPowerMinSta,
                     double txPowerSta,
                     uint32_t nTxPowerLevelsSta,
                     double ccaEdTrSta,
                     double txPowerMinAp,
                     double txPowerAp,
                     uint32_t nTxPowerLevelsAp,
                     double ccaEdTrAp,
                     std::string dlAckSeqType,
                     bool enableUlOfdma,
                     bool useExtendedBlockAck,
                     bool enableBsrp,
                     Time accessReqInterval,
                     Ssid ssid,
                     uint32_t staGroupIndex,
                     NodeContainer wifiStaNodes,
                     NodeContainer wifiApNodes,
                     NetDeviceContainer& staDevices,
                     NetDeviceContainer& apDevices)
{
    // Network configuration: ssid, frequency (band, channel size and number)...
    SpectrumChannelHelper channelHelper;
    std::string channelStr("");
    setChannelInfo(staGroupIndex, frequency, channelWidth, channelStr);

    std::cout << "SSID: " << ssid << std::endl;
    std::cout << "Channel Info: " << channelStr << std::endl;

    // Propagation
    channelHelper.SetChannel("ns3::MultiModelSpectrumChannel");
    channelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    double lossScenario = 40.05 + +20 * log10(frequency / 2.4) + 5 * nWalls +
                          pow(18.3 * nFloors, (nFloors + 2) / (nFloors + 1) - 0.46);
    channelHelper.AddPropagationLoss("ns3::ThreeLogDistancePropagationLossModel",
                                     "ReferenceLoss",
                                     DoubleValue(lossScenario),
                                     "Distance0",
                                     DoubleValue(1),
                                     "Exponent0",
                                     DoubleValue(2),
                                     "Distance1",
                                     DoubleValue(5),
                                     "Exponent1",
                                     DoubleValue(3.5));
    // Station state and data/control traffic generation management
    std::ostringstream oss;
    oss << "HeMcs" << mcs;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue(oss.str()),
                                 "ControlMode",
                                 StringValue(oss.str()));

    // Set guard interval and MPDU buffer size
    wifi.ConfigHeOptions("GuardInterval",
                         TimeValue(NanoSeconds(gi)),
                         "MpduBufferSize",
                         UintegerValue(useExtendedBlockAck ? 256 : 64),
                         "BssColor",
                         UintegerValue(1));

    // GENERAL - PHY
    SpectrumWifiPhyHelper phy;
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(channelHelper.Create());
    phy.Set("ChannelSettings", StringValue(channelStr));

    // STA CONFIGURATION - PHY
    phy.Set("TxPowerStart", DoubleValue(txPowerMinSta));
    phy.Set("TxPowerEnd", DoubleValue(txPowerSta));
    phy.Set("TxPowerLevels", UintegerValue(nTxPowerLevelsSta));
    phy.Set("CcaEdThreshold", DoubleValue(ccaEdTrSta));
    phy.Set("RxSensitivity", DoubleValue(-92.0));

    phy.Set("Antennas", UintegerValue(nAntennasSta));
    phy.Set("MaxSupportedTxSpatialStreams", UintegerValue(ntxSpatialStreamsSta));
    phy.Set("MaxSupportedRxSpatialStreams", UintegerValue(nrxSpatialStreamsSta));

    mac.SetType("ns3::StaWifiMac",
                "Ssid",
                SsidValue(ssid),
                "VO_MaxAmpduSize",
                UintegerValue(8388607),
                "BE_MaxAmpduSize",
                UintegerValue(8388607),
                "BK_MaxAmpduSize",
                UintegerValue(8388607),
                "VI_MaxAmpduSize",
                UintegerValue(8388607)
                //"ActiveProbing", BooleanValue (false)
    );

    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    // AP CONFIGURATION - PHY
    phy.Set("TxPowerStart", DoubleValue(txPowerMinAp));
    phy.Set("TxPowerEnd", DoubleValue(txPowerAp));
    phy.Set("TxPowerLevels", UintegerValue(nTxPowerLevelsAp));
    phy.Set("CcaEdThreshold", DoubleValue(ccaEdTrAp));
    phy.Set("RxSensitivity", DoubleValue(-92.0));
    phy.Set("Antennas", UintegerValue(nAntennasAp));
    phy.Set("MaxSupportedTxSpatialStreams", UintegerValue(ntxSpatialStreamsAp));
    phy.Set("MaxSupportedRxSpatialStreams", UintegerValue(nrxSpatialStreamsAp));

    // AP CONFIGURATION - MAC
    if (dlAckSeqType != "NO-OFDMA")
    {
        mac.SetMultiUserScheduler("ns3::RrMultiUserScheduler",
                                  "EnableUlOfdma",
                                  BooleanValue(enableUlOfdma),
                                  "EnableBsrp",
                                  BooleanValue(enableBsrp),
                                  "AccessReqInterval",
                                  TimeValue(accessReqInterval));
    }

    mac.SetType("ns3::ApWifiMac",
                "Ssid",
                SsidValue(ssid),
                "EnableBeaconJitter",
                BooleanValue(false),
                "VO_MaxAmpduSize",
                UintegerValue(8388607),
                "BE_MaxAmpduSize",
                UintegerValue(8388607),
                "BK_MaxAmpduSize",
                UintegerValue(8388607),
                "VI_MaxAmpduSize",
                UintegerValue(8388607));

    apDevices = wifi.Install(phy, mac, wifiApNodes);

    // Basic tracing
    if (tracing)
    {
        phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy.EnablePcap("Wifi-4-A", apDevices.Get(0)); // traces AP 0
    }

    // RNG: For independent replications, the most rigorous set up is to define both seed
    // (fixed) and run number (incremental)
    RngSeedManager::SetSeed(seedNumber); // set randomness seed
    RngSeedManager::SetRun(runNumber);   // set run number
    int64_t streamNumber = 100;
    streamNumber += wifi.AssignStreams(apDevices, streamNumber);
    streamNumber += wifi.AssignStreams(staDevices, streamNumber);
}

std::string
getUniformRandomValueString(double minVal, double maxVal)
{
    Ptr<UniformRandomVariable> randomVar = CreateObject<UniformRandomVariable>();
    randomVar->SetAttribute("Min", DoubleValue(minVal));
    randomVar->SetAttribute("Max", DoubleValue(maxVal));
    return std::to_string(randomVar->GetValue());
}

double
getNormalRandomValue(double mean, double variance)
{
    Ptr<NormalRandomVariable> randomVar = CreateObject<NormalRandomVariable>();
    randomVar->SetAttribute("Mean", DoubleValue(mean));
    randomVar->SetAttribute("Variance", DoubleValue(variance));
    return randomVar->GetValue();
}

uint32_t
ComputeNumSelectedSta(uint32_t nSta, double mean, double variance)
{
    double percentage = getNormalRandomValue(mean, variance);

    uint32_t numSelectedSta = ceil(nSta * percentage);
    if (numSelectedSta == 0)
    {
        numSelectedSta = 1;
    }
    else if (numSelectedSta > nSta)
    {
        numSelectedSta = nSta;
    }
    return numSelectedSta;
}

void
FillSelectedSta(uint32_t* array, uint32_t size, uint32_t nStaMax)
{
    for (uint32_t i = 0; i < size; i++)
    {
        array[i] = nStaMax * 2;
    }
}

void
selectSta(uint32_t* selectedSta, uint32_t numSelectedSta, uint32_t nStaMin, uint32_t nStaMax)
{
    if (numSelectedSta == 1 && nStaMin == 0 && nStaMax == 0)
    {
        selectedSta[0] = 0;
        return;
    }
    uint32_t staSelectionIndex = 0;
    while (staSelectionIndex < numSelectedSta)
    {
        uint32_t currentSta = 1;
        Ptr<UniformRandomVariable> newSta = CreateObject<UniformRandomVariable>();
        newSta->SetAttribute("Min", DoubleValue(nStaMin));
        newSta->SetAttribute("Max", DoubleValue(nStaMax + 0.01));
        currentSta = newSta->GetValue();
        uint32_t* lastSelectedSta = selectedSta + numSelectedSta;

        if (lastSelectedSta == std::find(selectedSta, lastSelectedSta, currentSta))
        {
            selectedSta[staSelectionIndex] = currentSta;
            staSelectionIndex++;
        }
    }
}

void
printArray(std::string title, uint32_t* array, size_t size)
{
    std::cout << title << ": ";
    for (size_t i = 0; i < size; i++)
    {
        std::cout << array[i] << ' ';
    }
    std::cout << std::endl;
}

void
VoDServer(ApplicationContainer& vodServerApplications,
          ApplicationContainer& vodClientApplications,
          NodeContainer& wifiStaNodes,
          NodeContainer& wifiApNodes,
          uint32_t* selectedStas,
          size_t numStas,
          const uint32_t& dataRate,
          const uint32_t payloadSize,
          uint16_t& portUdp,
          const uint16_t tosValue)
{
    for (size_t i = 0; i < numStas; ++i)
    {
        uint32_t staIndex = selectedStas[i];
        auto ipv4 = wifiStaNodes.Get(staIndex)->GetObject<Ipv4>();
        const auto address = ipv4->GetAddress(1, 0).GetLocal();

        InetSocketAddress sinkSocket(address, portUdp + staIndex);
        sinkSocket.SetTos(tosValue);

        OnOffHelper onOffHelper("ns3::UdpSocketFactory", sinkSocket);
        onOffHelper.SetAttribute(
            "OnTime",
            StringValue("ns3::WeibullRandomVariable[Shape=0.8099|Scale=20850]"));
        onOffHelper.SetAttribute("OffTime",
                                 StringValue("ns3::GammaRandomVariable[Alpha=0.2463|Beta=60.227]"));
        onOffHelper.SetAttribute("DataRate", DataRateValue(dataRate));
        onOffHelper.SetAttribute("PacketSize", UintegerValue(payloadSize));
        onOffHelper.SetAttribute("MaxBytes", UintegerValue(10989173));
        vodServerApplications.Add(onOffHelper.Install(wifiApNodes.Get(0)));

        PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", sinkSocket);
        vodClientApplications.Add(packetSinkHelper.Install(wifiStaNodes.Get(staIndex)));
    }
}

void
StartStopApplication(ApplicationContainer& clientApplications,
                     ApplicationContainer& serverApplications,
                     const double simulationTime)
{
    clientApplications.Start(Seconds(0.0));
    clientApplications.Stop(Seconds(simulationTime));
    serverApplications.Start(Seconds(0.0));
    serverApplications.Stop(Seconds(simulationTime));
}

void
VoDClient(ApplicationContainer& vodServerApplications,
          ApplicationContainer& vodClientApplications,
          NodeContainer& wifiStaNodes,
          NodeContainer& wifiApNodes,
          uint32_t* selectedStas,
          size_t numStas,
          const uint32_t& dataRate,
          const uint32_t payloadSize,
          uint16_t& portUdp,
          const uint16_t tosValue)
{
    for (size_t i = 0; i < numStas;
         ++i) // Not using range-based loop since raw pointers don't support it
    {
        uint32_t staIndex = selectedStas[i];

        auto ipv4 = wifiApNodes.Get(0)->GetObject<Ipv4>();
        const auto address = ipv4->GetAddress(1, 0).GetLocal();

        InetSocketAddress sinkSocket(address, portUdp + staIndex);
        sinkSocket.SetTos(tosValue);

        OnOffHelper onOffHelper("ns3::UdpSocketFactory", sinkSocket);
        onOffHelper.SetAttribute(
            "OnTime",
            StringValue("ns3::WeibullRandomVariable[Shape=0.8099|Scale=20850]"));
        onOffHelper.SetAttribute("OffTime",
                                 StringValue("ns3::GammaRandomVariable[Alpha=0.2463|Beta=60.227]"));
        onOffHelper.SetAttribute("DataRate", DataRateValue(dataRate));
        onOffHelper.SetAttribute("PacketSize", UintegerValue(payloadSize));
        onOffHelper.SetAttribute("MaxBytes", UintegerValue(10989173));
        vodServerApplications.Add(onOffHelper.Install(wifiStaNodes.Get(staIndex)));

        PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", sinkSocket);
        vodClientApplications.Add(packetSinkHelper.Install(wifiApNodes.Get(0)));
    }
}

void
HttpServer(uint32_t& httpAppIndex,
           ApplicationContainer& httpServerApps,
           ApplicationContainer& httpClientApps,
           NodeContainer& wifiStaNodes,
           NodeContainer& wifiApNodes,
           uint32_t* selectedStas,
           size_t numStas)
{
    for (size_t i = 0; i < numStas; ++i)
    {
        uint32_t staIndex = selectedStas[i];

        auto ipv4 = wifiApNodes.Get(0)->GetObject<Ipv4>();
        const auto serverAddress = ipv4->GetAddress(1, 0).GetLocal();

        ThreeGppHttpServerHelper serverHelper(serverAddress);
        httpServerApps.Add(serverHelper.Install(wifiApNodes.Get(0)));

        Ptr<ThreeGppHttpServer> httpServer =
            httpServerApps.Get(httpAppIndex)->GetObject<ThreeGppHttpServer>();
        PointerValue varPtr;
        httpServer->GetAttribute("Variables", varPtr);
        Ptr<ThreeGppHttpVariables> httpVariables = varPtr.Get<ThreeGppHttpVariables>();
        httpVariables->SetMainObjectSizeMean(10710);
        httpVariables->SetMainObjectSizeStdDev(25032);
        httpVariables->SetEmbeddedObjectSizeMean(7758);
        httpVariables->SetEmbeddedObjectSizeStdDev(126168);
        httpVariables->SetNumOfEmbeddedObjectsScale(2);
        httpVariables->SetNumOfEmbeddedObjectsShape(1.1);
        httpVariables->SetNumOfEmbeddedObjectsMax(53);

        ThreeGppHttpClientHelper clientHelper(serverAddress);
        httpClientApps.Add(clientHelper.Install(wifiStaNodes.Get(staIndex)));

        httpAppIndex++;
    }
}

void
GamingUl(uint16_t portGaming,
         uint32_t aPacketSizeDl,
         double aPacketSizeUl,
         double bPacketSizeDl,
         double bPacketSizeUl,
         double aPacketArrivalDl,
         double aPacketArrivalUl,
         double bPacketArrivalDl,
         double bPacketArrivalUl,
         uint32_t initArrivalMin,
         uint32_t initArrivalMax,
         ApplicationContainer& gamingServersAp,
         ApplicationContainer& gamingClientsSta,
         ApplicationContainer& gamingServersSta,
         ApplicationContainer& gamingClientsAp,
         const NodeContainer& wifiApNodes,
         const NodeContainer& wifiStaNodes,
         uint32_t* selectedSta,
         size_t staSize)
{
    for (size_t i = 0; i < staSize; ++i)
    {
        uint32_t staIndex = selectedSta[i];

        // Source
        auto ipv4Server = wifiApNodes.Get(0)->GetObject<Ipv4>();
        Ipv4Address ipAddressServer = ipv4Server->GetAddress(1, 0).GetLocal();
        AddressValue serverAddress(InetSocketAddress(ipAddressServer, portGaming));

        TrafficGeneratorHelper gamingHelper("ns3::UdpSocketFactory",
                                            Address(),
                                            TrafficGeneratorNgmnGaming::GetTypeId());

        gamingHelper.SetAttribute("IsDownlink", BooleanValue(true));
        gamingHelper.SetAttribute("aParamPacketSizeDl", UintegerValue(aPacketSizeDl));
        gamingHelper.SetAttribute("bParamPacketSizeDl", DoubleValue(bPacketSizeDl));
        gamingHelper.SetAttribute("aParamPacketArrivalDl", DoubleValue(aPacketArrivalDl));
        gamingHelper.SetAttribute("bParamPacketArrivalDl", DoubleValue(bPacketArrivalDl));
        gamingHelper.SetAttribute("InitialPacketArrivalMin", UintegerValue(initArrivalMin));
        gamingHelper.SetAttribute("InitialPacketArrivalMax", UintegerValue(initArrivalMax));

        gamingHelper.SetAttribute("Remote", serverAddress);
        gamingClientsSta.Add(gamingHelper.Install(wifiStaNodes.Get(staIndex)));
    }

    // Sink
    InetSocketAddress clientAddress(Ipv4Address::GetAny(), portGaming);
    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", clientAddress);
    gamingServersAp.Add(packetSinkHelper.Install(wifiApNodes.Get(0)));
}

void
GamingDl(uint32_t portGaming,
         uint32_t aPacketSizeDl,
         uint32_t aPacketSizeUl,
         double bPacketSizeDl,
         double bPacketSizeUl,
         double aPacketArrivalDl,
         double aPacketArrivalUl,
         double bPacketArrivalDl,
         double bPacketArrivalUl,
         uint32_t initArrivalMin,
         uint32_t initArrivalMax,
         ApplicationContainer& gamingServersAp,
         ApplicationContainer& gamingClientsSta,
         ApplicationContainer& gamingServersSta,
         ApplicationContainer& gamingClientsAp,
         NodeContainer& wifiApNodes,
         NodeContainer& wifiStaNodes,
         uint32_t* selectedSta,
         size_t staSize)
{
    for (size_t i = 0; i < staSize; ++i)
    {
        uint32_t staIndex = selectedSta[i];

        // Source
        auto ipv4Server = wifiStaNodes.Get(staIndex)->GetObject<Ipv4>();
        Ipv4Address ipAddressServer = ipv4Server->GetAddress(1, 0).GetLocal();
        AddressValue serverAddress(InetSocketAddress(ipAddressServer, portGaming + 50));

        TrafficGeneratorHelper gamingHelper("ns3::UdpSocketFactory",
                                            InetSocketAddress(ipAddressServer, portGaming + 50),
                                            TrafficGeneratorNgmnGaming::GetTypeId());
        gamingHelper.SetAttribute("IsDownlink", BooleanValue(true));
        gamingHelper.SetAttribute("aParamPacketSizeDl", UintegerValue(aPacketSizeUl));
        gamingHelper.SetAttribute("bParamPacketSizeDl", DoubleValue(bPacketSizeUl));
        gamingHelper.SetAttribute("aParamPacketArrivalDl", DoubleValue(aPacketArrivalUl));
        gamingHelper.SetAttribute("bParamPacketArrivalDl", DoubleValue(bPacketArrivalUl));
        gamingHelper.SetAttribute("InitialPacketArrivalMin", UintegerValue(initArrivalMin));
        gamingHelper.SetAttribute("InitialPacketArrivalMax", UintegerValue(initArrivalMax));

        gamingHelper.SetAttribute("Remote", serverAddress);
        gamingClientsAp.Add(gamingHelper.Install(wifiApNodes.Get(0)));

        // Sink
        InetSocketAddress clientAddress(Ipv4Address::GetAny(), portGaming + 50);
        PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", clientAddress);
        gamingServersSta.Add(packetSinkHelper.Install(wifiStaNodes.Get(staIndex)));
    }
}

void
VoIPUl(uint32_t portVoIP,
       uint32_t encoderFrameLength,
       uint32_t meanTalkSpurtDuration,
       double voiceActivityFactor,
       uint32_t voicePayload,
       uint32_t sidPeriodicity,
       uint32_t sidPayload,
       ApplicationContainer& voIPServersAp,
       ApplicationContainer& voIPClientsSta,
       ApplicationContainer& voIPServersSta,
       ApplicationContainer& voIPClientsAp,
       const NodeContainer& wifiApNodes,
       const NodeContainer& wifiStaNodes,
       uint32_t* selectedSta,
       size_t staSize)
{
    for (size_t i = 0; i < staSize; ++i)
    {
        uint32_t staIndex = selectedSta[i];

        // Source
        auto ipv4Server = wifiApNodes.Get(0)->GetObject<Ipv4>();
        Ipv4Address ipAddressServer = ipv4Server->GetAddress(1, 0).GetLocal();
        AddressValue serverAddress(InetSocketAddress(ipAddressServer, portVoIP));

        TrafficGeneratorHelper voIPHelper("ns3::UdpSocketFactory",
                                          Address(),
                                          TrafficGeneratorNgmnVoip::GetTypeId());

        voIPHelper.SetAttribute("EncoderFrameLength", UintegerValue(encoderFrameLength));
        voIPHelper.SetAttribute("MeanTalkSpurtDuration", UintegerValue(meanTalkSpurtDuration));
        voIPHelper.SetAttribute("VoiceActivityFactor", DoubleValue(voiceActivityFactor));
        voIPHelper.SetAttribute("VoicePayload", UintegerValue(voicePayload));
        voIPHelper.SetAttribute("SIDPeriodicity", UintegerValue(sidPeriodicity));
        voIPHelper.SetAttribute("SIDPayload", UintegerValue(sidPayload));

        voIPHelper.SetAttribute("Remote", serverAddress);
        voIPClientsSta.Add(voIPHelper.Install(wifiStaNodes.Get(staIndex)));
    }

    // Sink
    InetSocketAddress clientAddress(Ipv4Address::GetAny(), portVoIP);
    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", clientAddress);
    voIPServersAp.Add(packetSinkHelper.Install(wifiApNodes.Get(0)));
}

void
VoIPDl(uint32_t portVoIP,
       uint32_t encoderFrameLength,
       uint32_t meanTalkSpurtDuration,
       double voiceActivityFactor,
       uint32_t voicePayload,
       uint32_t sidPeriodicity,
       uint32_t sidPayload,
       ApplicationContainer& voIPServersAp,
       ApplicationContainer& voIPClientsSta,
       ApplicationContainer& voIPServersSta,
       ApplicationContainer& voIPClientsAp,
       const NodeContainer& wifiApNodes,
       const NodeContainer& wifiStaNodes,
       uint32_t* selectedSta,
       size_t staSize)
{
    for (size_t i = 0; i < staSize; ++i)
    {
        uint32_t staIndex = selectedSta[i];

        // Source
        auto ipv4Server = wifiStaNodes.Get(staIndex)->GetObject<Ipv4>();
        Ipv4Address ipAddressServer = ipv4Server->GetAddress(1, 0).GetLocal();
        AddressValue serverAddress(InetSocketAddress(ipAddressServer, portVoIP + 50));

        TrafficGeneratorHelper voIPHelper("ns3::UdpSocketFactory",
                                          InetSocketAddress(ipAddressServer, portVoIP + 50),
                                          TrafficGeneratorNgmnVoip::GetTypeId());
        voIPHelper.SetAttribute("EncoderFrameLength", UintegerValue(encoderFrameLength));
        voIPHelper.SetAttribute("MeanTalkSpurtDuration", UintegerValue(meanTalkSpurtDuration));
        voIPHelper.SetAttribute("VoiceActivityFactor", DoubleValue(voiceActivityFactor));
        voIPHelper.SetAttribute("VoicePayload", UintegerValue(voicePayload));
        voIPHelper.SetAttribute("SIDPeriodicity", UintegerValue(sidPeriodicity));
        voIPHelper.SetAttribute("SIDPayload", UintegerValue(sidPayload));

        voIPHelper.SetAttribute("Remote", serverAddress);
        voIPClientsAp.Add(voIPHelper.Install(wifiApNodes.Get(0)));

        // Sink
        InetSocketAddress clientAddress(Ipv4Address::GetAny(), portVoIP + 50);
        PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", clientAddress);
        voIPServersSta.Add(packetSinkHelper.Install(wifiStaNodes.Get(staIndex)));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Monitor for movement
// static void
// CourseChangeCallback(std::string path, Ptr<const MobilityModel> model)
//{
//     Vector position = model->GetPosition();
//     std::string node {path.substr(10,path.length()-43)};
//     std::cout << "Node:" << node << " x=" << position.x << ", y=" << position.y
//               << ", z=" << position.z << ", time = " << Simulator::Now().GetSeconds() <<
//               std::endl;
// }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN
int
main(int argc, char* argv[])
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ENVIRONMENT SETTINGS - DEFAULT PARAMETERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Simulation basic parameters
    uint32_t simulationTime = 10; // Simulation duration in seconds
    uint32_t seedNumber = 1;      // RNG seed
    uint32_t runNumber = 1;       // Run number

    // Logging and tracing
    bool verbose = false; // Turn on all Wifi logging
    bool tracing = false; // Turn on PCAP tracing on AP 0

    // Network Settings
    uint32_t nNetwork = 4; // Number of Networks
    uint32_t nStaA = 1;    // Number of STAs of network A
    uint32_t nStaB = 1;    // Number of STAs of network B
    uint32_t nStaC = 1;    // Number of STAs of network C
    uint32_t nStaD = 1;    // Number of STAs of network D
    uint32_t nAp = 1;      // Number of APs

    // Physical aspects of the environment
    double distance = 1;               // Meters
    double nFloors = 0;                // Propagation loss object
    double nWalls = 2;                 // Propagation loss object
    double lossScenario = 0;           // Loss scenario selection
    double lossFrequencyReference = 0; // Frequency reference for loss calculation
    double minXsize = 8.0;
    double maxXSize = 10.0;
    double minYsize = 8.0;
    double maxYSize = 10.0;
    double minZSize = 1.0;
    double maxZSize = 1.5;

    // Transmission Configuration
    double txPowerAp = 21;     // Max TX Power for APs
    double txPowerSta = 15;    // Max TX Power for STAs
    double txPowerMinAp = 10;  // Min TX Power reference for APs and STAs
    double txPowerMinSta = 10; // Min TX Power reference for APs and STAs

    uint32_t nTxPowerLevelsAp =
        (uint32_t)(txPowerAp - txPowerMinAp); // Iteration levels for TX power
    uint32_t nTxPowerLevelsSta =
        (uint32_t)(txPowerSta - txPowerMinSta); // Iteration levels for TX power

    uint32_t nAntennasAp = 2;  // Number of antennas for APs
    uint32_t nAntennasSta = 1; // Number of antennas for STAs

    uint32_t ntxSpatialStreamsAp = 2;  // Number of TX Spatial Streams for APs
    uint32_t ntxSpatialStreamsSta = 1; // Number of TX Spatial Streams for STAs
    uint32_t nrxSpatialStreamsAp = 2;  // Number of RX Spatial Streams for APs
    uint32_t nrxSpatialStreamsSta = 1; // Number of RX Spatial Streams for STAs

    // Phy parameters
    double frequency = 5;             // 2.4 GHz / 5 GHz / 6 GHz
    int mcs = -1;                     //-1 indicates unset value
    uint32_t gi = 800;                // Guard interval (ns)
    uint32_t channelWidth = 80;       // Channel Width (MHz)
    std::string phyModel{"Spectrum"}; // Phy level model (Yans/Spectrum)
    bool enableObssPd = true;         // Enable OBSS/PD
    double obssPdThreshold = -72.0;   // OBSS/PD threshold in dBm
    double ccaEdTrSta = -62;          // CCA-ED Threshold of STAs in dBm
    double ccaEdTrAp = -62;           // CCA-ED Threshold of APs in dBm

    // Mac parameters
    std::string dlAckSeqType{"NO-OFDMA"}; // Type of acknowledgment sequence for DL MU PPDUs of DL
                                          // OFDMA (NO-OFDMA, ACK-SU-FORMAT, MU-BAR, AGGR-MU-BAR)
    bool enableUlOfdma = false;           // In case of OFDMA: enable UL
    bool enableBsrp = false;              // In case of OFDMA: enable BSRP
    Time accessReqInterval = Seconds(0.1);

    // Traffic configuration
    bool useRts = false;              // RTS / CTS
    bool useExtendedBlockAck = false; // Mpdu Buffer Size (256 for extended / 64 for normal size)
    uint32_t payloadSize =
        1448; // Must fit in the max TX duration when transmitting at MCS 0 over an RU of 26 tones
    uint32_t dataRate = 100000000; // Data Rate (bps)

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PARAMETERS FROM COMMAND LINE
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CommandLine cmd(__FILE__);

    // Simulation basic parameters
    cmd.AddValue("simulationTime", "Simulation time (seconds)", simulationTime);
    cmd.AddValue("seedNumber", "RNG seed number", seedNumber);
    cmd.AddValue("runNumber", "Simulation run number", runNumber);

    // Logging and tracing
    cmd.AddValue("verbose", "Enable log components", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    // Network Settings
    cmd.AddValue("nNetwork", "Number of wifi Networks", nNetwork);
    cmd.AddValue("nStaA", "Max number of wifi STA devices for AP A", nStaA);
    cmd.AddValue("nStaB", "Max number of wifi STA devices for AP B", nStaB);
    cmd.AddValue("nStaC", "Max number of wifi STA devices for AP C", nStaC);
    cmd.AddValue("nStaD", "Max number of wifi STA devices for AP D", nStaD);

    // Physical aspects of the environment
    cmd.AddValue("lossScenario", "Loss scenario selection", lossScenario);
    cmd.AddValue("distance",
                 "Distance (meters) between STA and AP for propagation loss calculation",
                 distance);
    cmd.AddValue("nFloors",
                 "Number of floors to be considered in propagation loss calculation",
                 nFloors);
    cmd.AddValue("nWalls",
                 "Number of walls to be considered in propagation loss calculation",
                 nWalls);
    cmd.AddValue("lossFrequencyReference",
                 "Loss frequency reference for propagation loss calculation",
                 lossFrequencyReference);
    cmd.AddValue("minXSize", "Lower bound for X size of place definition", minXSize);
    cmd.AddValue("maxXSize", "Upper bound for X size of place definition", maxXSize);
    cmd.AddValue("minYSize", "Lower bound for Y size of place definition", minYSize);
    cmd.AddValue("maxYSize", "Upper bound for Y size of place definition", maxYSize);
    cmd.AddValue("minZSize", "Lower bound for Z size of place definition", minZSize);
    cmd.AddValue("maxZSize", "Upper bound for Z size of place definition", maxZSize);

    // Transmission Configuration
    cmd.AddValue("txPowerAp", "AP transmission power in dBm", txPowerAp);
    cmd.AddValue("txPowerSta", "STA transmission power in dBm", txPowerSta);
    cmd.AddValue("txPowerMinAp", "AP minimum transmission power in dBm", txPowerMinAp);
    cmd.AddValue("txPowerMinSta", "STA minimum transmission power in dBm", txPowerMinSta);
    cmd.AddValue("nAntennasAp", "Number of antennas per AP", nAntennasAp);
    cmd.AddValue("nAntennasSta", "Number of antennas per STA", nAntennasSta);
    cmd.AddValue("ntxSpatialStreamsAp", "Number of TX Spatial Streams per AP", ntxSpatialStreamsAp);
    cmd.AddValue("ntxSpatialStreamsSta",
                 "Number of TX Spatial Streams per STA",
                 ntxSpatialStreamsSta);
    cmd.AddValue("nrxSpatialStreamsAp", "Number of RX Spatial Streams per AP", nrxSpatialStreamsAp);
    cmd.AddValue("nrxSpatialStreamsSta",
                 "Number of RX Spatial Streams per STA",
                 nrxSpatialStreamsSta);

    // Phy parameters
    cmd.AddValue("frequency",
                 "Whether working in the 2.4, 5 or 6 GHz band (other values gets rejected)",
                 frequency);
    cmd.AddValue("mcs", "If set, limit testing to a specific MCS (0-11)", mcs);
    cmd.AddValue("gi", "Guard Interval (3200/1600/800)", gi);
    cmd.AddValue("channelWidth", "Channel Width (20/40/80/160MHz)", channelWidth);
    cmd.AddValue("phyModel", "PHY Model (Yans / Spectrum)", phyModel);
    cmd.AddValue("enableObssPd", "Enable/disable OBSS_PD", enableObssPd);
    cmd.AddValue("obssPdThreshold", "OBSS PD Threshold (dBm)", obssPdThreshold);
    cmd.AddValue("ccaEdTrSta", "CCA ED Threshold for STAs (dBm)", ccaEdTrSta);
    cmd.AddValue("ccaEdTrAp", "OBSS PD Threshold for APs (dBm)", ccaEdTrAp);

    // Mac parameters
    cmd.AddValue("dlAckType",
                 "Ack sequence type for DL OFDMA (NO-OFDMA, ACK-SU-FORMAT, MU-BAR, AGGR-MU-BAR)",
                 dlAckSeqType);
    cmd.AddValue("enableUlOfdma",
                 "Enable UL OFDMA (useful if DL OFDMA is enabled and TCP is used)",
                 enableUlOfdma);
    cmd.AddValue("enableBsrp",
                 "Enable BSRP (useful if DL and UL OFDMA are enabled and TCP is used)",
                 enableBsrp);
    cmd.AddValue("accessReqInterval", "ARI value (miliseconds) for OFDMA", accessReqInterval);

    // Traffic configuration
    cmd.AddValue("useRts", "Enable RTS/CTS", useRts);
    cmd.AddValue("useExtendedBlockAck",
                 "MPDU Long Buffer Size (256 for extended / 64 for normal size)",
                 useExtendedBlockAck);
    cmd.AddValue("payloadSize", "The application payload size in bytes", payloadSize);
    cmd.AddValue("dataRate", "Data rate (bps)", dataRate);

    cmd.Parse(argc, argv);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ENVIRONMET SETTINGS - FIXING AND ARRANGEMENTS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Enable RTS/CTS
    if (useRts)
    {
        Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("0"));
    }

    // Set + check type of acknowledgment sequence
    if (dlAckSeqType == "ACK-SU-FORMAT")
    {
        Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                           EnumValue(WifiAcknowledgment::DL_MU_BAR_BA_SEQUENCE));
    }
    else if (dlAckSeqType == "MU-BAR")
    {
        Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                           EnumValue(WifiAcknowledgment::DL_MU_TF_MU_BAR));
    }
    else if (dlAckSeqType == "AGGR-MU-BAR")
    {
        Config::SetDefault("ns3::WifiDefaultAckManager::DlMuAckSequenceType",
                           EnumValue(WifiAcknowledgment::DL_MU_AGGREGATE_TF));
    }
    else if (dlAckSeqType != "NO-OFDMA")
    {
        NS_ABORT_MSG("Invalid DL ack sequence type (must be NO-OFDMA, ACK-SU-FORMAT, MU-BAR or "
                     "AGGR-MU-BAR)");
    }

    // Check Phy level model
    if (phyModel != "Spectrum")
    {
        NS_ABORT_MSG("Invalid PHY model (must be Spectrum)");
    }

    // Set compatible Phy in case of OFDMA
    if (dlAckSeqType != "NO-OFDMA")
    {
        // SpectrumWifiPhy is required for OFDMA
        phyModel = "Spectrum";
    }

    // Set TCP Segment Size
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    // Enable RNG Custom seed and run
    RngSeedManager::SetSeed(seedNumber);
    RngSeedManager::SetRun(runNumber);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Node Creation
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    NodeContainer wifiStaNodesA;
    NodeContainer wifiApNodesA;
    NodeContainer wifiStaNodesB;
    NodeContainer wifiApNodesB;
    NodeContainer wifiStaNodesC;
    NodeContainer wifiApNodesC;
    NodeContainer wifiStaNodesD;
    NodeContainer wifiApNodesD;
    std::cout << "nStaA: " << nStaA << std::endl;
    std::cout << "nStaB: " << nStaB << std::endl;
    std::cout << "nStaC: " << nStaC << std::endl;
    std::cout << "nStaD: " << nStaD << std::endl;
    CreateNodesNormal(wifiStaNodesA, wifiApNodesA, nStaA, nAp, "A", nStaA, 5);
    if (nNetwork >= 2)
        CreateNodesNormal(wifiStaNodesB, wifiApNodesB, nStaB, nAp, "B", nStaB, 5);
    if (nNetwork >= 3)
        CreateNodesNormal(wifiStaNodesC, wifiApNodesC, nStaC, nAp, "C", nStaC, 5);
    if (nNetwork >= 4)
        CreateNodesNormal(wifiStaNodesD, wifiApNodesD, nStaD, nAp, "D", nStaD, 5);

    std::cout << "nStaA: " << nStaA << std::endl;
    std::cout << "nStaB: " << nStaB << std::endl;
    std::cout << "nStaC: " << nStaC << std::endl;
    std::cout << "nStaD: " << nStaD << std::endl;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NetDevice Creation
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    NetDeviceContainer apDevicesA, staDevicesA;
    NetDeviceContainer apDevicesB, staDevicesB;
    NetDeviceContainer apDevicesC, staDevicesC;
    NetDeviceContainer apDevicesD, staDevicesD;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // WIFI (and WIFI MAC) HELPERS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Wifi Helper
    WifiHelper wifi;
    Ssid ssid;

    // Wifi Logging
    if (verbose)
    {
        wifi.EnableLogComponents();
    }

    // Wifi Standard
    wifi.SetStandard(WIFI_STANDARD_80211ax);

    if (enableObssPd)
    {
        wifi.SetObssPdAlgorithm("ns3::ConstantObssPdAlgorithm",
                                "ObssPdLevel",
                                DoubleValue(obssPdThreshold));
    }

    // Mac Helper
    WifiMacHelper mac;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NETWORK CONFIGURATION: PHY + MAC
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Network configuration: ssid, frequency (band, channel size and number)...
    if (nNetwork >= 1)
    {
        ssid = Ssid("ns3-802.11ax-A");
        ConfigureWifiNetwork(seedNumber,
                             runNumber,
                             tracing,
                             wifi,
                             mac,
                             frequency,
                             mcs,
                             channelWidth,
                             gi,
                             nFloors,
                             nWalls,
                             nAntennasAp,
                             nAntennasSta,
                             ntxSpatialStreamsAp,
                             ntxSpatialStreamsSta,
                             nrxSpatialStreamsAp,
                             nrxSpatialStreamsSta,
                             txPowerMinSta,
                             txPowerSta,
                             nTxPowerLevelsSta,
                             ccaEdTrSta,
                             txPowerMinAp,
                             txPowerAp,
                             nTxPowerLevelsAp,
                             ccaEdTrAp,
                             dlAckSeqType,
                             enableUlOfdma,
                             useExtendedBlockAck,
                             enableBsrp,
                             accessReqInterval,
                             ssid,
                             0,
                             wifiStaNodesA,
                             wifiApNodesA,
                             staDevicesA,
                             apDevicesA);
    }

    if (nNetwork >= 2)
    {
        ssid = Ssid("ns3-802.11ax-B");
        ConfigureWifiNetwork(seedNumber,
                             runNumber,
                             tracing,
                             wifi,
                             mac,
                             frequency,
                             mcs,
                             channelWidth,
                             gi,
                             nFloors,
                             nWalls,
                             nAntennasAp,
                             nAntennasSta,
                             ntxSpatialStreamsAp,
                             ntxSpatialStreamsSta,
                             nrxSpatialStreamsAp,
                             nrxSpatialStreamsSta,
                             txPowerMinSta,
                             txPowerSta,
                             nTxPowerLevelsSta,
                             ccaEdTrSta,
                             txPowerMinAp,
                             txPowerAp,
                             nTxPowerLevelsAp,
                             ccaEdTrAp,
                             dlAckSeqType,
                             enableUlOfdma,
                             useExtendedBlockAck,
                             enableBsrp,
                             accessReqInterval,
                             ssid,
                             1,
                             wifiStaNodesB,
                             wifiApNodesB,
                             staDevicesB,
                             apDevicesB);
    }

    if (nNetwork >= 3)
    {
        ssid = Ssid("ns3-802.11ax-C");
        ConfigureWifiNetwork(seedNumber,
                             runNumber,
                             tracing,
                             wifi,
                             mac,
                             frequency,
                             mcs,
                             channelWidth,
                             gi,
                             nFloors,
                             nWalls,
                             nAntennasAp,
                             nAntennasSta,
                             ntxSpatialStreamsAp,
                             ntxSpatialStreamsSta,
                             nrxSpatialStreamsAp,
                             nrxSpatialStreamsSta,
                             txPowerMinSta,
                             txPowerSta,
                             nTxPowerLevelsSta,
                             ccaEdTrSta,
                             txPowerMinAp,
                             txPowerAp,
                             nTxPowerLevelsAp,
                             ccaEdTrAp,
                             dlAckSeqType,
                             enableUlOfdma,
                             useExtendedBlockAck,
                             enableBsrp,
                             accessReqInterval,
                             ssid,
                             2,
                             wifiStaNodesC,
                             wifiApNodesC,
                             staDevicesC,
                             apDevicesC);
    }

    if (nNetwork >= 4)
    {
        ssid = Ssid("ns3-802.11ax-D");
        ConfigureWifiNetwork(seedNumber,
                             runNumber,
                             tracing,
                             wifi,
                             mac,
                             frequency,
                             mcs,
                             channelWidth,
                             gi,
                             nFloors,
                             nWalls,
                             nAntennasAp,
                             nAntennasSta,
                             ntxSpatialStreamsAp,
                             ntxSpatialStreamsSta,
                             nrxSpatialStreamsAp,
                             nrxSpatialStreamsSta,
                             txPowerMinSta,
                             txPowerSta,
                             nTxPowerLevelsSta,
                             ccaEdTrSta,
                             txPowerMinAp,
                             txPowerAp,
                             nTxPowerLevelsAp,
                             ccaEdTrAp,
                             dlAckSeqType,
                             enableUlOfdma,
                             useExtendedBlockAck,
                             enableBsrp,
                             accessReqInterval,
                             ssid,
                             3,
                             wifiStaNodesD,
                             wifiApNodesD,
                             staDevicesD,
                             apDevicesD);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MOBILITY
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Mobility Helper
    MobilityHelper mobility;

    std::string xSize = getUniformRandomValueString(minXsize, maxXSize);
    std::string xSize2 = std::to_string(2 * std::stod(xSize));

    std::string ySize = getUniformRandomValueString(minYsize, maxYSize);
    std::string ySize2 = std::to_string(2 * std::stod(ySize));

    std::string zSize = getUniformRandomValueString(minZSize, maxZSize);

    // STA - Dynamic configuration: Random Walk
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + xSize + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + ySize + "]"),
        "Z",
        StringValue(zSize));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode",
                              StringValue("Time"),
                              "Time",
                              StringValue("3s"),
                              "Speed",
                              StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds",
                              StringValue("0|" + xSize + "|0|" + ySize));
    mobility.Install(wifiStaNodesA);

    // STA - Dynamic configuration: Random Walk
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=" + xSize + "|Max=" + xSize2 + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + ySize + "]"),
        "Z",
        StringValue(zSize));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode",
                              StringValue("Time"),
                              "Time",
                              StringValue("3s"),
                              "Speed",
                              StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds",
                              StringValue(xSize + "|" + xSize2 + "|0|" + ySize));
    mobility.Install(wifiStaNodesB);

    // STA - Dynamic configuration: Random Walk
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + xSize + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=" + ySize + "|Max=" + ySize2 + "]"),
        "Z",
        StringValue(zSize));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode",
                              StringValue("Time"),
                              "Time",
                              StringValue("3s"),
                              "Speed",
                              StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds",
                              StringValue("0|" + xSize + "|" + ySize + "|" + ySize2));
    mobility.Install(wifiStaNodesC);

    // STA - Dynamic configuration: Random Walk
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=" + xSize + "|Max=" + xSize2 + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=" + ySize + "|Max=" + ySize2 + "]"),
        "Z",
        StringValue(zSize));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Mode",
                              StringValue("Time"),
                              "Time",
                              StringValue("3s"),
                              "Speed",
                              StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                              "Bounds",
                              StringValue(xSize + "|" + xSize2 + "|" + ySize + "|" + ySize2));
    mobility.Install(wifiStaNodesD);

    // AP - Constant position
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + xSize + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + ySize + "]"),
        "Z",
        StringValue("1.5"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodesA);

    // AP - Constant position
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=" + xSize + "|Max=" + xSize2 + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + ySize + "]"),
        "Z",
        StringValue("1.5"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodesB);

    // AP - Constant position
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=0|Max=" + xSize + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=" + ySize + "|Max=" + ySize2 + "]"),
        "Z",
        StringValue("1.5"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodesC);

    // AP - Constant position
    mobility.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        StringValue("ns3::UniformRandomVariable[Min=" + xSize + "|Max=" + xSize2 + "]"),
        "Y",
        StringValue("ns3::UniformRandomVariable[Min=" + ySize + "|Max=" + ySize2 + "]"),
        "Z",
        StringValue("1.5"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodesD);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // INTERNET PROTOCOL STACK
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    InternetStackHelper stack;

    stack.Install(wifiApNodesA);
    stack.Install(wifiStaNodesA);

    stack.Install(wifiApNodesB);
    stack.Install(wifiStaNodesB);

    stack.Install(wifiApNodesC);
    stack.Install(wifiStaNodesC);

    stack.Install(wifiApNodesD);
    stack.Install(wifiStaNodesD);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // ADDRESSING
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Ipv4AddressHelper address;

    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer staNodeInterfacesA;
    Ipv4InterfaceContainer apNodeInterfacesA;
    staNodeInterfacesA = address.Assign(staDevicesA);
    apNodeInterfacesA = address.Assign(apDevicesA);

    address.SetBase("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer staNodeInterfacesB;
    Ipv4InterfaceContainer apNodeInterfacesB;
    staNodeInterfacesB = address.Assign(staDevicesB);
    apNodeInterfacesB = address.Assign(apDevicesB);

    address.SetBase("192.168.3.0", "255.255.255.0");
    Ipv4InterfaceContainer staNodeInterfacesC;
    Ipv4InterfaceContainer apNodeInterfacesC;
    staNodeInterfacesC = address.Assign(staDevicesC);
    apNodeInterfacesC = address.Assign(apDevicesC);

    address.SetBase("192.168.4.0", "255.255.255.0");
    Ipv4InterfaceContainer staNodeInterfacesD;
    Ipv4InterfaceContainer apNodeInterfacesD;
    staNodeInterfacesD = address.Assign(staDevicesD);
    apNodeInterfacesD = address.Assign(apDevicesD);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // APPLICATIONS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NETWORK A
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1. Traffic type: VoD
    // 1.1. Random Node Selection
    // Minimum Value Selection: STA definition depends on random values
    uint32_t nStaAmin = 0;                    // index from TOTAL NODES
    uint32_t nStaAmax = nStaAmin + nStaA - 1; // index from TOTAL NODES

    uint32_t numSelectedStaA = ComputeNumSelectedSta(nStaA, 0.6, 0.01);
    uint32_t selectedStaA[numSelectedStaA];
    FillSelectedSta(selectedStaA, sizeof(selectedStaA) / sizeof(selectedStaA[0]), nStaAmax);
    selectSta(selectedStaA, numSelectedStaA, nStaAmin, nStaAmax);
    printArray("ARRAY A1", selectedStaA, sizeof(selectedStaA) / sizeof(selectedStaA[0]));

    uint32_t numSelectedStaA1 = ComputeNumSelectedSta(nStaA, 0.1, 0.01);
    uint32_t selectedStaA1[numSelectedStaA1];
    FillSelectedSta(selectedStaA1, sizeof(selectedStaA1) / sizeof(selectedStaA1[0]), nStaAmax);
    selectSta(selectedStaA1, numSelectedStaA1, nStaAmin, nStaAmax);
    printArray("ARRAY A11", selectedStaA1, sizeof(selectedStaA1) / sizeof(selectedStaA1[0]));

    // 1.2. VoD Flow
    ApplicationContainer vodServerApplicationsA, vodClientApplicationsA;
    ApplicationContainer vodServerApplicationsA1, vodClientApplicationsA1;
    uint16_t portUdp = 5050;
    uint16_t tosValue = 0xb8;
    size_t numStasA = sizeof(selectedStaA) / sizeof(selectedStaA[0]);

    VoDServer(vodServerApplicationsA,
              vodClientApplicationsA,
              wifiStaNodesA,
              wifiApNodesA,
              selectedStaA,
              numStasA,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsA, vodServerApplicationsA, simulationTime);

    VoDClient(vodServerApplicationsA1,
              vodClientApplicationsA1,
              wifiStaNodesA,
              wifiApNodesA,
              selectedStaA,
              numStasA,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsA1, vodServerApplicationsA1, simulationTime);

    // 2. Traffic type: HTTP
    // 2.1. Random Node Selection
    uint32_t numSelectedStaA2 = ComputeNumSelectedSta(nStaA, 0.4, 0.01);
    uint32_t selectedStaA2[numSelectedStaA2];
    FillSelectedSta(selectedStaA2, sizeof(selectedStaA2) / sizeof(selectedStaA2[0]), nStaAmax);
    selectSta(selectedStaA2, numSelectedStaA2, nStaAmin, nStaAmax);
    printArray("ARRAY A2", selectedStaA2, sizeof(selectedStaA2) / sizeof(selectedStaA2[0]));

    // 2.2. HTTP Flow
    uint32_t httpAppIndexA = 0;
    ApplicationContainer httpServerAppsA, httpClientAppsA;
    HttpServer(httpAppIndexA,
               httpServerAppsA,
               httpClientAppsA,
               wifiStaNodesA,
               wifiApNodesA,
               selectedStaA2,
               sizeof(selectedStaA2) / sizeof(uint32_t));
    StartStopApplication(httpClientAppsA, httpServerAppsA, simulationTime);

    // 3. Traffic type: FTP
    // 3.1. Random Node Selection
    uint32_t numSelectedStaA3 = ComputeNumSelectedSta(nStaA, 0.2, 0.01);
    uint32_t selectedStaA3[numSelectedStaA3];
    FillSelectedSta(selectedStaA3, sizeof(selectedStaA3) / sizeof(selectedStaA3[0]), nStaAmax);
    selectSta(selectedStaA3, numSelectedStaA3, nStaAmin, nStaAmax);
    printArray("ARRAY A3", selectedStaA3, sizeof(selectedStaA3) / sizeof(selectedStaA3[0]));

    // 3.2. FTP Flow
    uint16_t portFtp = 5150;
    double lambdaFtp = 0.2;
    double muFtp = 14.45;
    double sigmaFtp = 0.35;
    ApplicationContainer ftpServersA, ftpClientsA;

    NodeContainer ftpStaNodesA;
    Ipv4InterfaceContainer ftpStaInterfacesA;

    for (uint32_t staIndex : selectedStaA3)
    {
        ftpStaNodesA.Add(wifiStaNodesA.Get(staIndex));
        ftpStaInterfacesA.Add(staNodeInterfacesA.Get(staIndex));
    }

    Ptr<ThreeGppFtpM2Helper> ftpHelperA;
    ftpHelperA = CreateObject<ThreeGppFtpM2Helper>(&ftpServersA,
                                                   &ftpClientsA,
                                                   &ftpStaNodesA,
                                                   &wifiApNodesA,
                                                   &ftpStaInterfacesA);
    ftpHelperA->Configure(portFtp,
                          Seconds(0.0),
                          Seconds(1.0),
                          Seconds(simulationTime),
                          lambdaFtp,
                          muFtp,
                          sigmaFtp,
                          dataRate);

    ftpHelperA->Start();

    // 4. Traffic type: Gaming
    // 4.1. Random Node Selection
    uint32_t numSelectedStaA4 = ComputeNumSelectedSta(nStaA, 0.3, 0.01);
    uint32_t selectedStaA4[numSelectedStaA4];
    FillSelectedSta(selectedStaA4, sizeof(selectedStaA4) / sizeof(selectedStaA4[0]), nStaAmax);
    selectSta(selectedStaA4, numSelectedStaA4, nStaAmin, nStaAmax);
    printArray("ARRAY A4", selectedStaA4, sizeof(selectedStaA4) / sizeof(selectedStaA4[0]));

    // 4.2. Gaming Flow:
    uint32_t portGaming = 5250;
    uint32_t aPacketSizeDl = 330;
    uint32_t aPacketSizeUl = 45;
    double bPacketSizeDl = 82;
    double bPacketSizeUl = 5.7;
    double aPacketArrivalDl = 50;
    double aPacketArrivalUl = 40;
    double bPacketArrivalDl = 4.5;
    double bPacketArrivalUl = 6;
    uint32_t initArrivalMin = 0;
    uint32_t initArrivalMax = 40;
    ApplicationContainer gamingServersApA, gamingClientsStaA, gamingServersStaA, gamingClientsApA;
    size_t staSizeA = sizeof(selectedStaA4) / sizeof(selectedStaA4[0]);

    GamingDl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApA,
             gamingClientsStaA,
             gamingServersStaA,
             gamingClientsApA,
             wifiApNodesA,
             wifiStaNodesA,
             selectedStaA4,
             staSizeA);
    GamingUl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApA,
             gamingClientsStaA,
             gamingServersStaA,
             gamingClientsApA,
             wifiApNodesA,
             wifiStaNodesA,
             selectedStaA4,
             staSizeA);

    StartStopApplication(gamingClientsStaA, gamingServersApA, simulationTime);
    StartStopApplication(gamingClientsApA, gamingServersStaA, simulationTime);

    // 5. Traffic type: VoIP
    // 5.1. Random Node Selection
    uint32_t numSelectedStaA5 = ComputeNumSelectedSta(nStaA, 0.2, 0.01);
    uint32_t selectedStaA5[numSelectedStaA5];
    FillSelectedSta(selectedStaA5, sizeof(selectedStaA5) / sizeof(selectedStaA5[0]), nStaAmax);
    selectSta(selectedStaA5, numSelectedStaA5, nStaAmin, nStaAmax);
    printArray("ARRAY A5", selectedStaA5, sizeof(selectedStaA5) / sizeof(selectedStaA5[0]));

    // 5.2. VoIP Flow:
    uint32_t portVoIP = 5350;
    uint32_t encoderFrameLength = 20;
    uint32_t meanTalkSpurtDuration = 1250;
    double voiceActivityFactor = 0.5;
    uint32_t voicePayload = 40;
    uint32_t sidPeriodicity = 160;
    uint32_t sidPayload = 15;
    ApplicationContainer voIPServersApA, voIPClientsStaA, voIPServersStaA, voIPClientsApA;

    VoIPDl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApA,
           voIPClientsStaA,
           voIPServersStaA,
           voIPClientsApA,
           wifiApNodesA,
           wifiStaNodesA,
           selectedStaA5,
           sizeof(selectedStaA5) / sizeof(selectedStaA5[0]));
    VoIPUl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApA,
           voIPClientsStaA,
           voIPServersStaA,
           voIPClientsApA,
           wifiApNodesA,
           wifiStaNodesA,
           selectedStaA5,
           sizeof(selectedStaA5) / sizeof(selectedStaA5[0]));

    StartStopApplication(voIPClientsStaA, voIPServersApA, simulationTime);
    StartStopApplication(voIPClientsApA, voIPServersStaA, simulationTime);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NETWORK B
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1. Traffic type: VoD
    // 1.1. Random Node Selection
    // Minimum Value Selection: STA definition depends on random values
    uint32_t nStaBmin = 0;                    // index from TOTAL NODES
    uint32_t nStaBmax = nStaBmin + nStaB - 1; // index from TOTAL NODES

    uint32_t numSelectedStaB = ComputeNumSelectedSta(nStaB, 0.6, 0.01);
    uint32_t selectedStaB[numSelectedStaB];
    FillSelectedSta(selectedStaB, sizeof(selectedStaB) / sizeof(selectedStaB[0]), nStaBmax);
    selectSta(selectedStaB, numSelectedStaB, nStaBmin, nStaBmax);
    printArray("ARRAY B1", selectedStaB, sizeof(selectedStaB) / sizeof(selectedStaB[0]));

    uint32_t numSelectedStaB1 = ComputeNumSelectedSta(nStaB, 0.1, 0.01);
    uint32_t selectedStaB1[numSelectedStaB1];
    FillSelectedSta(selectedStaB1, sizeof(selectedStaB1) / sizeof(selectedStaB1[0]), nStaBmax);
    selectSta(selectedStaB1, numSelectedStaB1, nStaBmin, nStaBmax);
    printArray("ARRAY B11", selectedStaB1, sizeof(selectedStaB1) / sizeof(selectedStaB1[0]));

    // 1.2. VoD Flow
    ApplicationContainer vodServerApplicationsB, vodClientApplicationsB;
    ApplicationContainer vodServerApplicationsB1, vodClientApplicationsB1;
    portUdp = 5050;
    tosValue = 0xb8;
    size_t numStasB = sizeof(selectedStaB) / sizeof(selectedStaB[0]);

    VoDServer(vodServerApplicationsB,
              vodClientApplicationsB,
              wifiStaNodesB,
              wifiApNodesB,
              selectedStaB,
              numStasB,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsB, vodServerApplicationsB, simulationTime);

    VoDClient(vodServerApplicationsB1,
              vodClientApplicationsB1,
              wifiStaNodesB,
              wifiApNodesB,
              selectedStaB,
              numStasB,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsB1, vodServerApplicationsB1, simulationTime);

    // 2. Traffic type: HTTP
    // 2.1. Random Node Selection
    uint32_t numSelectedStaB2 = ComputeNumSelectedSta(nStaB, 0.4, 0.01);
    uint32_t selectedStaB2[numSelectedStaB2];
    FillSelectedSta(selectedStaB2, sizeof(selectedStaB2) / sizeof(selectedStaB2[0]), nStaBmax);
    selectSta(selectedStaB2, numSelectedStaB2, nStaBmin, nStaBmax);
    printArray("ARRAY B2", selectedStaB2, sizeof(selectedStaB2) / sizeof(selectedStaB2[0]));

    // 2.2. HTTP Flow
    uint32_t httpAppIndexB = 0;
    ApplicationContainer httpServerAppsB, httpClientAppsB;
    HttpServer(httpAppIndexB,
               httpServerAppsB,
               httpClientAppsB,
               wifiStaNodesB,
               wifiApNodesB,
               selectedStaB2,
               sizeof(selectedStaB2) / sizeof(uint32_t));
    StartStopApplication(httpClientAppsB, httpServerAppsB, simulationTime);

    // 3. Traffic type: FTP
    // 3.1. Random Node Selection
    uint32_t numSelectedStaB3 = ComputeNumSelectedSta(nStaB, 0.2, 0.01);
    uint32_t selectedStaB3[numSelectedStaB3];
    FillSelectedSta(selectedStaB3, sizeof(selectedStaB3) / sizeof(selectedStaB3[0]), nStaBmax);
    selectSta(selectedStaB3, numSelectedStaB3, nStaBmin, nStaBmax);
    printArray("ARRAY B3", selectedStaB3, sizeof(selectedStaB3) / sizeof(selectedStaB3[0]));

    // 3.2. FTP Flow
    portFtp = 5150;
    lambdaFtp = 0.2;
    muFtp = 14.45;
    sigmaFtp = 0.35;
    ApplicationContainer ftpServersB, ftpClientsB;

    NodeContainer ftpStaNodesB;
    Ipv4InterfaceContainer ftpStaInterfacesB;
    for (uint32_t staIndex : selectedStaB3)
    {
        ftpStaNodesB.Add(wifiStaNodesB.Get(staIndex));
        ftpStaInterfacesB.Add(staNodeInterfacesB.Get(staIndex));
    }

    Ptr<ThreeGppFtpM2Helper> ftpHelperB;
    ftpHelperB = CreateObject<ThreeGppFtpM2Helper>(&ftpServersB,
                                                   &ftpClientsB,
                                                   &ftpStaNodesB,
                                                   &wifiApNodesB,
                                                   &ftpStaInterfacesB);
    ftpHelperB->Configure(portFtp,
                          Seconds(0.0),
                          Seconds(1.0),
                          Seconds(simulationTime),
                          lambdaFtp,
                          muFtp,
                          sigmaFtp,
                          dataRate);
    ftpHelperB->Start();

    // 4. Traffic type: Gaming
    // 4.1. Random Node Selection
    uint32_t numSelectedStaB4 = ComputeNumSelectedSta(nStaB, 0.3, 0.01);
    uint32_t selectedStaB4[numSelectedStaB4];
    FillSelectedSta(selectedStaB4, sizeof(selectedStaB4) / sizeof(selectedStaB4[0]), nStaBmax);
    selectSta(selectedStaB4, numSelectedStaB4, nStaBmin, nStaBmax);
    printArray("ARRAY B4", selectedStaB4, sizeof(selectedStaB4) / sizeof(selectedStaB4[0]));

    // 4.2. Gaming Flow:
    portGaming = 5250;
    ApplicationContainer gamingServersApB, gamingClientsStaB, gamingServersStaB, gamingClientsApB;
    size_t staSizeB = sizeof(selectedStaB4) / sizeof(selectedStaB4[0]);

    GamingDl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApB,
             gamingClientsStaB,
             gamingServersStaB,
             gamingClientsApB,
             wifiApNodesB,
             wifiStaNodesB,
             selectedStaB4,
             staSizeB);
    GamingUl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApB,
             gamingClientsStaB,
             gamingServersStaB,
             gamingClientsApB,
             wifiApNodesB,
             wifiStaNodesB,
             selectedStaB4,
             staSizeB);

    StartStopApplication(gamingClientsStaB, gamingServersApB, simulationTime);
    StartStopApplication(gamingClientsApB, gamingServersStaB, simulationTime);

    // 5. Traffic type: VoIP
    // 5.1. Random Node Selection
    uint32_t numSelectedStaB5 = ComputeNumSelectedSta(nStaB, 0.2, 0.01);
    uint32_t selectedStaB5[numSelectedStaB5];
    FillSelectedSta(selectedStaB5, sizeof(selectedStaB5) / sizeof(selectedStaB5[0]), nStaBmax);
    selectSta(selectedStaB5, numSelectedStaB5, nStaBmin, nStaBmax);
    printArray("ARRAY B5", selectedStaB5, sizeof(selectedStaB5) / sizeof(selectedStaB5[0]));

    // 5.2. VoIP Flow:
    portVoIP = 5350;
    ApplicationContainer voIPServersApB, voIPClientsStaB, voIPServersStaB, voIPClientsApB;

    VoIPDl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApB,
           voIPClientsStaB,
           voIPServersStaB,
           voIPClientsApB,
           wifiApNodesB,
           wifiStaNodesB,
           selectedStaB5,
           sizeof(selectedStaB5) / sizeof(selectedStaB5[0]));
    VoIPUl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApB,
           voIPClientsStaB,
           voIPServersStaB,
           voIPClientsApB,
           wifiApNodesB,
           wifiStaNodesB,
           selectedStaB5,
           sizeof(selectedStaB5) / sizeof(selectedStaB5[0]));

    StartStopApplication(voIPClientsStaB, voIPServersApB, simulationTime);
    StartStopApplication(voIPClientsApB, voIPServersStaB, simulationTime);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NETWORK C
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1. Traffic type: VoD
    // 1.1. Random Node Selection
    // Minimum Value Selection: STA definition depends on random values
    uint32_t nStaCmin = 0;                    // index from TOTAL NODES
    uint32_t nStaCmax = nStaCmin + nStaC - 1; // index from TOTAL NODES

    uint32_t numSelectedStaC = ComputeNumSelectedSta(nStaC, 0.6, 0.01);
    uint32_t selectedStaC[numSelectedStaC];
    FillSelectedSta(selectedStaC, sizeof(selectedStaC) / sizeof(selectedStaC[0]), nStaCmax);
    selectSta(selectedStaC, numSelectedStaC, nStaCmin, nStaCmax);
    printArray("ARRAY C1", selectedStaC, sizeof(selectedStaC) / sizeof(selectedStaC[0]));

    uint32_t numSelectedStaC1 = ComputeNumSelectedSta(nStaC, 0.1, 0.01);
    uint32_t selectedStaC1[numSelectedStaC1];
    FillSelectedSta(selectedStaC1, sizeof(selectedStaC1) / sizeof(selectedStaC1[0]), nStaCmax);
    selectSta(selectedStaC1, numSelectedStaC1, nStaCmin, nStaCmax);
    printArray("ARRAY C11", selectedStaC1, sizeof(selectedStaC1) / sizeof(selectedStaC1[0]));

    // 1.2. VoD Flow
    ApplicationContainer vodServerApplicationsC, vodClientApplicationsC;
    ApplicationContainer vodServerApplicationsC1, vodClientApplicationsC1;
    portUdp = 5050;
    tosValue = 0xb8;
    size_t numStasC = sizeof(selectedStaC) / sizeof(selectedStaC[0]);

    VoDServer(vodServerApplicationsC,
              vodClientApplicationsC,
              wifiStaNodesC,
              wifiApNodesC,
              selectedStaC,
              numStasC,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsC, vodServerApplicationsC, simulationTime);

    VoDClient(vodServerApplicationsC1,
              vodClientApplicationsC1,
              wifiStaNodesC,
              wifiApNodesC,
              selectedStaC,
              numStasC,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsC1, vodServerApplicationsC1, simulationTime);

    // 2. Traffic type: HTTP
    // 2.1. Random Node Selection
    uint32_t numSelectedStaC2 = ComputeNumSelectedSta(nStaC, 0.4, 0.01);
    uint32_t selectedStaC2[numSelectedStaC2];
    FillSelectedSta(selectedStaC2, sizeof(selectedStaC2) / sizeof(selectedStaC2[0]), nStaCmax);
    selectSta(selectedStaC2, numSelectedStaC2, nStaCmin, nStaCmax);
    printArray("ARRAY C2", selectedStaC2, sizeof(selectedStaC2) / sizeof(selectedStaC2[0]));

    // 2.2. HTTP Flow
    uint32_t httpAppIndexC = 0;
    ApplicationContainer httpServerAppsC, httpClientAppsC;
    HttpServer(httpAppIndexC,
               httpServerAppsC,
               httpClientAppsC,
               wifiStaNodesC,
               wifiApNodesC,
               selectedStaC2,
               sizeof(selectedStaC2) / sizeof(uint32_t));
    StartStopApplication(httpClientAppsC, httpServerAppsC, simulationTime);

    // 3. Traffic type: FTP
    // 3.1. Random Node Selection
    uint32_t numSelectedStaC3 = ComputeNumSelectedSta(nStaC, 0.2, 0.01);
    uint32_t selectedStaC3[numSelectedStaC3];
    FillSelectedSta(selectedStaC3, sizeof(selectedStaC3) / sizeof(selectedStaC3[0]), nStaCmax);
    selectSta(selectedStaC3, numSelectedStaC3, nStaCmin, nStaCmax);
    printArray("ARRAY C3", selectedStaC3, sizeof(selectedStaC3) / sizeof(selectedStaC3[0]));

    // 3.2. FTP Flow
    portFtp = 5150;
    lambdaFtp = 0.2;
    muFtp = 14.45;
    sigmaFtp = 0.35;
    ApplicationContainer ftpServersC, ftpClientsC;

    NodeContainer ftpStaNodesC;
    Ipv4InterfaceContainer ftpStaInterfacesC;
    for (uint32_t staIndex : selectedStaC3)
    {
        ftpStaNodesC.Add(wifiStaNodesC.Get(staIndex));
        ftpStaInterfacesC.Add(staNodeInterfacesC.Get(staIndex));
    }

    Ptr<ThreeGppFtpM2Helper> ftpHelperC;
    ftpHelperC = CreateObject<ThreeGppFtpM2Helper>(&ftpServersC,
                                                   &ftpClientsC,
                                                   &ftpStaNodesC,
                                                   &wifiApNodesC,
                                                   &ftpStaInterfacesC);
    ftpHelperC->Configure(portFtp,
                          Seconds(0.0),
                          Seconds(1.0),
                          Seconds(simulationTime),
                          lambdaFtp,
                          muFtp,
                          sigmaFtp,
                          dataRate);
    ftpHelperC->Start();

    // 4. Traffic type: Gaming
    // 4.1. Random Node Selection
    uint32_t numSelectedStaC4 = ComputeNumSelectedSta(nStaC, 0.3, 0.01);
    uint32_t selectedStaC4[numSelectedStaC4];
    FillSelectedSta(selectedStaC4, sizeof(selectedStaC4) / sizeof(selectedStaC4[0]), nStaCmax);
    selectSta(selectedStaC4, numSelectedStaC4, nStaCmin, nStaCmax);
    printArray("ARRAY C4", selectedStaC4, sizeof(selectedStaC4) / sizeof(selectedStaC4[0]));

    // 4.2. Gaming Flow:
    portGaming = 5250;
    ApplicationContainer gamingServersApC, gamingClientsStaC, gamingServersStaC, gamingClientsApC;
    size_t staSizeC = sizeof(selectedStaC4) / sizeof(selectedStaC4[0]);

    GamingDl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApC,
             gamingClientsStaC,
             gamingServersStaC,
             gamingClientsApC,
             wifiApNodesC,
             wifiStaNodesC,
             selectedStaC4,
             staSizeC);
    GamingUl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApC,
             gamingClientsStaC,
             gamingServersStaC,
             gamingClientsApC,
             wifiApNodesC,
             wifiStaNodesC,
             selectedStaC4,
             staSizeC);

    StartStopApplication(gamingClientsStaC, gamingServersApC, simulationTime);
    StartStopApplication(gamingClientsApC, gamingServersStaC, simulationTime);

    // 5. Traffic type: VoIP
    // 5.1. Random Node Selection
    uint32_t numSelectedStaC5 = ComputeNumSelectedSta(nStaC, 0.2, 0.01);
    uint32_t selectedStaC5[numSelectedStaC5];
    FillSelectedSta(selectedStaC5, sizeof(selectedStaC5) / sizeof(selectedStaC5[0]), nStaCmax);
    selectSta(selectedStaC5, numSelectedStaC5, nStaCmin, nStaCmax);
    printArray("ARRAY C5", selectedStaC5, sizeof(selectedStaC5) / sizeof(selectedStaC5[0]));

    // 5.2. VoIP Flow:
    portVoIP = 5350;
    ApplicationContainer voIPServersApC, voIPClientsStaC, voIPServersStaC, voIPClientsApC;

    VoIPDl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApC,
           voIPClientsStaC,
           voIPServersStaC,
           voIPClientsApC,
           wifiApNodesC,
           wifiStaNodesC,
           selectedStaC5,
           sizeof(selectedStaC5) / sizeof(selectedStaC5[0]));
    VoIPUl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApC,
           voIPClientsStaC,
           voIPServersStaC,
           voIPClientsApC,
           wifiApNodesC,
           wifiStaNodesC,
           selectedStaC5,
           sizeof(selectedStaC5) / sizeof(selectedStaC5[0]));

    StartStopApplication(voIPClientsStaC, voIPServersApC, simulationTime);
    StartStopApplication(voIPClientsApC, voIPServersStaC, simulationTime);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NETWORK D
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1. Traffic type: VoD
    // 1.1. Random Node Selection
    // Minimum Value Selection: STA definition depends on random values
    uint32_t nStaDmin = 0;                    // index from TOTAL NODES
    uint32_t nStaDmax = nStaDmin + nStaD - 1; // index from TOTAL NODES

    uint32_t numSelectedStaD = ComputeNumSelectedSta(nStaD, 0.6, 0.01);
    uint32_t selectedStaD[numSelectedStaD];
    FillSelectedSta(selectedStaD, sizeof(selectedStaD) / sizeof(selectedStaD[0]), nStaDmax);
    selectSta(selectedStaD, numSelectedStaD, nStaDmin, nStaDmax);
    printArray("ARRAY D1", selectedStaD, sizeof(selectedStaD) / sizeof(selectedStaD[0]));

    uint32_t numSelectedStaD1 = ComputeNumSelectedSta(nStaD, 0.1, 0.01);
    uint32_t selectedStaD1[numSelectedStaD1];
    FillSelectedSta(selectedStaD1, sizeof(selectedStaD1) / sizeof(selectedStaD1[0]), nStaDmax);
    selectSta(selectedStaD1, numSelectedStaD1, nStaDmin, nStaDmax);
    printArray("ARRAY D11", selectedStaD1, sizeof(selectedStaD1) / sizeof(selectedStaD1[0]));

    // 1.2. VoD Flow
    ApplicationContainer vodServerApplicationsD, vodClientApplicationsD;
    ApplicationContainer vodServerApplicationsD1, vodClientApplicationsD1;
    portUdp = 5050;
    tosValue = 0xb8;
    size_t numStasD = sizeof(selectedStaD) / sizeof(selectedStaD[0]);

    VoDServer(vodServerApplicationsD,
              vodClientApplicationsD,
              wifiStaNodesD,
              wifiApNodesD,
              selectedStaD,
              numStasD,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsD, vodServerApplicationsD, simulationTime);

    VoDClient(vodServerApplicationsD1,
              vodClientApplicationsD1,
              wifiStaNodesD,
              wifiApNodesD,
              selectedStaD,
              numStasD,
              dataRate,
              payloadSize,
              portUdp,
              tosValue);
    StartStopApplication(vodClientApplicationsD1, vodServerApplicationsD1, simulationTime);

    // 2. Traffic type: HTTP
    // 2.1. Random Node Selection
    uint32_t numSelectedStaD2 = ComputeNumSelectedSta(nStaD, 0.4, 0.01);
    uint32_t selectedStaD2[numSelectedStaD2];
    FillSelectedSta(selectedStaD2, sizeof(selectedStaD2) / sizeof(selectedStaD2[0]), nStaDmax);
    selectSta(selectedStaD2, numSelectedStaD2, nStaDmin, nStaDmax);
    printArray("ARRAY D2", selectedStaD2, sizeof(selectedStaD2) / sizeof(selectedStaD2[0]));

    // 2.2. HTTP Flow
    uint32_t httpAppIndexD = 0;
    ApplicationContainer httpServerAppsD, httpClientAppsD;
    HttpServer(httpAppIndexD,
               httpServerAppsD,
               httpClientAppsD,
               wifiStaNodesD,
               wifiApNodesD,
               selectedStaD2,
               sizeof(selectedStaD2) / sizeof(uint32_t));
    StartStopApplication(httpClientAppsD, httpServerAppsD, simulationTime);

    // 3. Traffic type: FTP
    // 3.1. Random Node Selection
    uint32_t numSelectedStaD3 = ComputeNumSelectedSta(nStaD, 0.2, 0.01);
    uint32_t selectedStaD3[numSelectedStaD3];
    FillSelectedSta(selectedStaD3, sizeof(selectedStaD3) / sizeof(selectedStaD3[0]), nStaDmax);
    selectSta(selectedStaD3, numSelectedStaD3, nStaDmin, nStaDmax);
    printArray("ARRAY D3", selectedStaD3, sizeof(selectedStaD3) / sizeof(selectedStaD3[0]));

    // 3.2. FTP Flow
    portFtp = 5150;
    lambdaFtp = 0.2;
    muFtp = 14.45;
    sigmaFtp = 0.35;
    ApplicationContainer ftpServersD, ftpClientsD;

    NodeContainer ftpStaNodesD;
    Ipv4InterfaceContainer ftpStaInterfacesD;
    for (uint32_t staIndex : selectedStaD3)
    {
        ftpStaNodesD.Add(wifiStaNodesD.Get(staIndex));
        ftpStaInterfacesD.Add(staNodeInterfacesD.Get(staIndex));
    }

    Ptr<ThreeGppFtpM2Helper> ftpHelperD;
    ftpHelperD = CreateObject<ThreeGppFtpM2Helper>(&ftpServersD,
                                                   &ftpClientsD,
                                                   &ftpStaNodesD,
                                                   &wifiApNodesD,
                                                   &ftpStaInterfacesD);
    ftpHelperD->Configure(portFtp,
                          Seconds(0.0),
                          Seconds(1.0),
                          Seconds(simulationTime),
                          lambdaFtp,
                          muFtp,
                          sigmaFtp,
                          dataRate);
    ftpHelperD->Start();

    // 4. Traffic type: Gaming
    // 4.1. Random Node Selection
    uint32_t numSelectedStaD4 = ComputeNumSelectedSta(nStaD, 0.3, 0.01);
    uint32_t selectedStaD4[numSelectedStaD4];
    FillSelectedSta(selectedStaD4, sizeof(selectedStaD4) / sizeof(selectedStaD4[0]), nStaDmax);
    selectSta(selectedStaD4, numSelectedStaD4, nStaDmin, nStaDmax);
    printArray("ARRAY D4", selectedStaD4, sizeof(selectedStaD4) / sizeof(selectedStaD4[0]));

    // 4.2. Gaming Flow:
    portGaming = 5250;
    ApplicationContainer gamingServersApD, gamingClientsStaD, gamingServersStaD, gamingClientsApD;
    size_t staSizeD = sizeof(selectedStaD4) / sizeof(selectedStaD4[0]);

    GamingDl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApD,
             gamingClientsStaD,
             gamingServersStaD,
             gamingClientsApD,
             wifiApNodesD,
             wifiStaNodesD,
             selectedStaD4,
             staSizeD);
    GamingUl(portGaming,
             aPacketSizeDl,
             aPacketSizeUl,
             bPacketSizeDl,
             bPacketSizeUl,
             aPacketArrivalDl,
             aPacketArrivalUl,
             bPacketArrivalDl,
             bPacketArrivalUl,
             initArrivalMin,
             initArrivalMax,
             gamingServersApD,
             gamingClientsStaD,
             gamingServersStaD,
             gamingClientsApD,
             wifiApNodesD,
             wifiStaNodesD,
             selectedStaD4,
             staSizeD);

    StartStopApplication(gamingClientsStaD, gamingServersApD, simulationTime);
    StartStopApplication(gamingClientsApD, gamingServersStaD, simulationTime);

    // 5. Traffic type: VoIP
    // 5.1. Random Node Selection
    uint32_t numSelectedStaD5 = ComputeNumSelectedSta(nStaD, 0.2, 0.01);
    uint32_t selectedStaD5[numSelectedStaD5];
    FillSelectedSta(selectedStaD5, sizeof(selectedStaD5) / sizeof(selectedStaD5[0]), nStaDmax);
    selectSta(selectedStaD5, numSelectedStaD5, nStaDmin, nStaDmax);
    printArray("ARRAY D5", selectedStaD5, sizeof(selectedStaD5) / sizeof(selectedStaD5[0]));

    // 5.2. VoIP Flow:
    portVoIP = 5350;
    ApplicationContainer voIPServersApD, voIPClientsStaD, voIPServersStaD, voIPClientsApD;

    VoIPDl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApD,
           voIPClientsStaD,
           voIPServersStaD,
           voIPClientsApD,
           wifiApNodesD,
           wifiStaNodesD,
           selectedStaD5,
           sizeof(selectedStaD5) / sizeof(selectedStaD5[0]));
    VoIPUl(portVoIP,
           encoderFrameLength,
           meanTalkSpurtDuration,
           voiceActivityFactor,
           voicePayload,
           sidPeriodicity,
           sidPayload,
           voIPServersApD,
           voIPClientsStaD,
           voIPServersStaD,
           voIPClientsApD,
           wifiApNodesD,
           wifiStaNodesD,
           selectedStaD5,
           sizeof(selectedStaD5) / sizeof(selectedStaD5[0]));

    StartStopApplication(voIPClientsStaD, voIPServersApD, simulationTime);
    StartStopApplication(voIPClientsApD, voIPServersStaD, simulationTime);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MONITORING
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Signal and noise analysis from AP point of view
    Config::ConnectWithoutContext("/NodeList/" + std::to_string(nStaA) +
                                      "/DeviceList/*/Phy/MonitorSnifferRx",
                                  MakeCallback(&MonitorSniffRxA));
    g_signalDbmAvg[0] = 0;
    g_noiseDbmAvg[0] = 0;
    g_samples[0] = 0;
    Config::ConnectWithoutContext("/NodeList/" + std::to_string(0) +
                                      "/DeviceList/*/Phy/MonitorSnifferRx",
                                  MakeCallback(&MonitorSniffRxASta));
    g_signalDbmAvgSta[0] = 0;
    g_noiseDbmAvgSta[0] = 0;
    g_samplesSta[0] = 0;

    if (nNetwork >= 2)
    {
        Config::ConnectWithoutContext("/NodeList/" + std::to_string(nStaA + nStaB + nAp) +
                                          "/DeviceList/*/Phy/MonitorSnifferRx",
                                      MakeCallback(&MonitorSniffRxB));
        g_signalDbmAvg[1] = 0;
        g_noiseDbmAvg[1] = 0;
        g_samples[1] = 0;
        Config::ConnectWithoutContext("/NodeList/" + std::to_string(nStaA + nAp) +
                                          "/DeviceList/*/Phy/MonitorSnifferRx",
                                      MakeCallback(&MonitorSniffRxBSta));
        g_signalDbmAvgSta[1] = 0;
        g_noiseDbmAvgSta[1] = 0;
        g_samplesSta[1] = 0;
    }

    if (nNetwork >= 3)
    {
        Config::ConnectWithoutContext("/NodeList/" +
                                          std::to_string(nStaA + nStaB + nAp + nStaC + nAp) +
                                          "/DeviceList/*/Phy/MonitorSnifferRx",
                                      MakeCallback(&MonitorSniffRxC));
        g_signalDbmAvg[2] = 0;
        g_noiseDbmAvg[2] = 0;
        g_samples[2] = 0;
        Config::ConnectWithoutContext("/NodeList/" + std::to_string(nStaA + nStaB + nAp + nAp) +
                                          "/DeviceList/*/Phy/MonitorSnifferRx",
                                      MakeCallback(&MonitorSniffRxCSta));
        g_signalDbmAvgSta[2] = 0;
        g_noiseDbmAvgSta[2] = 0;
        g_samplesSta[2] = 0;
    }

    if (nNetwork >= 4)
    {
        Config::ConnectWithoutContext(
            "/NodeList/" + std::to_string(nStaA + nStaB + nAp + nStaC + nAp + nStaD + nAp) +
                "/DeviceList/*/Phy/MonitorSnifferRx",
            MakeCallback(&MonitorSniffRxD));
        g_signalDbmAvg[3] = 0;
        g_noiseDbmAvg[3] = 0;
        g_samples[3] = 0;
        Config::ConnectWithoutContext("/NodeList/" +
                                          std::to_string(nStaA + nAp + nStaB + nAp + nStaC + nAp) +
                                          "/DeviceList/*/Phy/MonitorSnifferRx",
                                      MakeCallback(&MonitorSniffRxDSta));
        g_signalDbmAvgSta[3] = 0;
        g_noiseDbmAvgSta[3] = 0;
        g_samplesSta[3] = 0;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FLOW MONITOR
    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MOVEMENT MONITOR
    // Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
    // MakeCallback(&CourseChangeCallback));
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // RUNNING
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    NS_LOG_INFO("Running simulation...");
    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // RESULT CALCULATION
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string oss("");
    oss += "HeMcs" + std::to_string(mcs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Flow statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    double delayValues[stats.size()];
    uint64_t j = 0;

    uint16_t jNode[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {0};
    Time avgJitter[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {Seconds(0.0)};
    Time avgDelay[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {Seconds(0.0)};
    double avgThroughput[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {0.0};
    uint32_t avgtxPackets[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {0};
    uint32_t avgtxBytes[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {0};
    uint32_t avgrxPackets[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {0};
    uint32_t avgrxBytes[nStaA + nStaB + nStaC + nStaD + nAp * nNetwork] = {0};

    uint32_t flowID[stats.size() + 1];
    std::string flowSourceAddress[stats.size() + 1];
    uint32_t flowSourcePort[stats.size() + 1];
    std::string flowDestinationAddress[stats.size() + 1];
    uint32_t flowDestinationPort[stats.size() + 1];
    std::string flowProtocol[stats.size() + 1];
    uint32_t flowTxPackets[stats.size() + 1];
    uint32_t flowTxBytes[stats.size() + 1];
    double flowTxOffered[stats.size() + 1];
    uint32_t flowRxPackets[stats.size() + 1];
    uint32_t flowRxBytes[stats.size() + 1];
    double flowThroughput[stats.size() + 1];
    double flowMeanDelay[stats.size() + 1];
    double flowLastDelay[stats.size() + 1];
    double flowMeanJitter[stats.size() + 1];

    uint32_t flowIndex = 0;
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();
         i != stats.end();
         ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
        flowID[flowIndex] = i->first;

        std::stringstream protoStream;
        protoStream << (uint16_t)t.protocol;
        if (t.protocol == 6)
        {
            protoStream.str("TCP");
            flowProtocol[flowIndex] = "TCP";
        }
        if (t.protocol == 17)
        {
            protoStream.str("UDP");
            flowProtocol[flowIndex] = "UDP";
        }
        std::ostringstream oss;
        t.sourceAddress.Print(oss);
        flowSourceAddress[flowIndex] = oss.str();
        flowSourcePort[flowIndex] = t.sourcePort;
        std::ostringstream oss2;
        t.destinationAddress.Print(oss2);
        flowDestinationAddress[flowIndex] = oss2.str();
        flowDestinationPort[flowIndex] = t.destinationPort;
        std::string divider = std::string(265, '*') + "\n";

        flowRxPackets[flowIndex] = i->second.rxPackets;
        flowRxBytes[flowIndex] = i->second.rxBytes;

        flowTxPackets[flowIndex] = i->second.txPackets;
        flowTxBytes[flowIndex] = i->second.txBytes;
        flowTxOffered[flowIndex] = i->second.txBytes * 8.0 / simulationTime / 1000.0 / 1000.0;

        if (i->second.rxPackets > 0)
        {
            double rxDuration =
                i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();

            delayValues[j] = 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;
            j++;

            flowThroughput[flowIndex] = i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
            flowMeanDelay[flowIndex] =
                double(1000 * i->second.delaySum.GetSeconds()) / (i->second.rxPackets);
            flowLastDelay[flowIndex] = i->second.lastDelay.GetDouble() / 1000 / 1000;
            flowMeanJitter[flowIndex] =
                1000 * i->second.jitterSum.GetSeconds() / i->second.rxPackets;
        }
        else
        {
            flowThroughput[flowIndex] = 0;
            flowMeanDelay[flowIndex] = 0;
            flowLastDelay[flowIndex] = 0;
            flowMeanJitter[flowIndex] = 0;
        }

        std::ostringstream oss3;
        t.sourceAddress.Print(oss3);
        std::string sourceAddress = oss3.str();

        std::regex rgxNetwork("[0-9]{1,3}\\.[0-9]{1,3}\\.(.*)\\.[0-9]{1,3}");
        std::smatch matchNetwork;
        std::regex_search(sourceAddress, matchNetwork, rgxNetwork);

        std::regex rgxNode("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.(.*)");
        std::smatch matchNode;
        std::regex_search(sourceAddress, matchNode, rgxNode);

        uint32_t pos = 0;

        if (matchNetwork[1].str() == "1")
        {
            pos = std::stoi(matchNode[1].str()) - 1;
        }
        else if (matchNetwork[1].str() == "2")
        {
            pos = std::stoi(matchNode[1].str()) + nStaA;
        }
        else if (matchNetwork[1].str() == "3")
        {
            pos = std::stoi(matchNode[1].str()) + nStaA + nStaB + nAp;
        }
        else
        {
            pos = std::stoi(matchNode[1].str()) + nStaA + nStaB + nStaC + 2 * nAp;
        }

        avgThroughput[pos] += i->second.txBytes * 8.0 / (simulationTime - 1) / 1000.0 / 1000.0;
        avgDelay[pos] += (i->second.delaySum) / (i->second.rxPackets);
        avgJitter[pos] += (i->second.jitterSum) / (i->second.rxPackets);
        avgtxPackets[pos] += (i->second.txPackets);
        avgtxBytes[pos] += (i->second.txBytes);
        avgrxPackets[pos] += (i->second.rxPackets);
        avgrxBytes[pos] += (i->second.rxBytes);
        jNode[pos]++;
        flowIndex++;
    }
    std::sort(delayValues, delayValues + stats.size());
    for (uint32_t i = 0; i < nStaA + nStaB + nStaC + nStaD + nNetwork; i++)
    {
        if (jNode[i] == 0)
        {
            jNode[i] = 1;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Device File
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string filenameDev("");
    filenameDev += "Scenario1-DeviceStats.csv";
    const char* c = filenameDev.c_str();
    (void)!freopen(c, "a", stdout);
    // std::cout << "Seed" << "," << "Run" << "," << "Network" << "," << "Device" << "," << "IP
    // Address" << "," << "MCS value" << "," << "Channel width (MHz)" << "," << "GI (ns)" << "," <<
    // "Avg Throughput (Mbit/s)" << "," << "Avg Tx Bytes" << "," << "Avg Tx Packets" << "," << "Avg
    // Rx Bytes" << "," << "Avg Rx Packets" << "," << "Avg Total Delay"<< "," << "Avg Total Jitter"
    // << "," << "Signal (dBm)" << "," << "Noise (dBm)" << "," << "SNR (dB)" << '\n';

    NodeContainer nodes;
    nodes.Add(wifiStaNodesA);
    nodes.Add(wifiApNodesA);
    nodes.Add(wifiStaNodesB);
    nodes.Add(wifiApNodesB);
    nodes.Add(wifiStaNodesC);
    nodes.Add(wifiApNodesC);
    nodes.Add(wifiStaNodesD);
    nodes.Add(wifiApNodesD);
    for (uint32_t i = 0; i < nStaA + nStaB + nStaC + nStaD + nNetwork; i++)
    {
        std::cout << seedNumber << "," << runNumber << ",";

        if (i <= nStaA)
        {
            std::cout << "A"
                      << ",";
        }
        else if (i > nStaA && i <= nStaA + nStaB + 1)
        {
            std::cout << "B"
                      << ",";
        }
        else if (i > nStaA + nStaB + 1 && i <= nStaA + nStaB + nStaC + 2)
        {
            std::cout << "C"
                      << ",";
        }
        else
        {
            std::cout << "D"
                      << ",";
        }

        if (i == nStaA || i == nStaA + nStaB + 1 || i == nStaA + nStaB + nStaC + 2 ||
            i == nStaA + nStaB + nStaC + nStaD + 3)
        {
            std::cout << "AP"
                      << ",";
        }
        else
        {
            std::cout << "STA"
                      << ",";
        }

        std::ostringstream oss;
        const auto addressIP = nodes.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
        addressIP.Print(oss);
        std::cout << oss.str() << ",";

        std::cout << mcs << "," << channelWidth << "," << gi << "," << avgThroughput[i] / jNode[i]
                  << "," << avgtxBytes[i] / jNode[i] << "," << avgtxPackets[i] / jNode[i] << ","
                  << avgrxBytes[i] / jNode[i] << "," << avgrxPackets[i] / jNode[i] << ","
                  << avgDelay[i] / jNode[i] << "," << avgJitter[i] / jNode[i] << ",";

        if (i < nStaA)
        {
            std::cout << g_signalDbmAvgSta[0] << "," << g_noiseDbmAvgSta[0] << ","
                      << (g_signalDbmAvgSta[0] - g_noiseDbmAvgSta[0]) << std::endl;
        }
        else if (i == nStaA)
        {
            std::cout << g_signalDbmAvg[0] << "," << g_noiseDbmAvg[0] << ","
                      << (g_signalDbmAvg[0] - g_noiseDbmAvg[0]) << std::endl;
        }
        else if (i > nStaA && i < nStaA + nStaB + 1)
        {
            std::cout << g_signalDbmAvgSta[1] << "," << g_noiseDbmAvgSta[1] << ","
                      << (g_signalDbmAvgSta[1] - g_noiseDbmAvgSta[1]) << std::endl;
        }
        else if (i == nStaA + nStaB + 1)
        {
            std::cout << g_signalDbmAvg[1] << "," << g_noiseDbmAvg[1] << ","
                      << (g_signalDbmAvg[1] - g_noiseDbmAvg[1]) << std::endl;
        }
        else if (i > nStaA + nStaB + 1 && i < nStaA + nStaB + nStaC + 2)
        {
            std::cout << g_signalDbmAvgSta[2] << "," << g_noiseDbmAvgSta[2] << ","
                      << (g_signalDbmAvgSta[2] - g_noiseDbmAvgSta[2]) << std::endl;
        }
        else if (i == nStaA + nStaB + nStaC + 2)
        {
            std::cout << g_signalDbmAvg[2] << "," << g_noiseDbmAvg[2] << ","
                      << (g_signalDbmAvg[2] - g_noiseDbmAvg[2]) << std::endl;
        }
        else if (i > nStaA + nStaB + nStaC + 2 && i < nStaA + nStaB + nStaC + nStaD + 3)
        {
            std::cout << g_signalDbmAvgSta[3] << "," << g_noiseDbmAvgSta[3] << ","
                      << (g_signalDbmAvgSta[3] - g_noiseDbmAvgSta[3]) << std::endl;
        }
        else
        {
            std::cout << g_signalDbmAvg[3] << "," << g_noiseDbmAvg[3] << ","
                      << (g_signalDbmAvg[3] - g_noiseDbmAvg[3]) << std::endl;
        }
    }
    fclose(stdout);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Flow File
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string filenameFlow("");
    filenameFlow += "Scenario1-FlowStats.csv";
    const char* c2 = filenameFlow.c_str();
    (void)!freopen(c2, "a", stdout);
    // std::cout << "Seed" << "," << "Run" << "," << "Network" << "," << "DL/UL" << "," <<"Flow ID"
    // << "," << "Source IP" << "," << "Source Port" << "," << "Destination IP" << "," <<
    // "Destination Port" << "," << "Protocol" << "," << "Tx Packets" << "," << "Tx Bytes" << "," <<
    // "Tx Offered" << "," << "Rx Packets" << "," << "Rx Bytes" << "," << "Avg Throughput" << "," <<
    // "Mean Delay" << "," << "Last Packet Delay"<< "," << "Mean Jitter" << '\n';

    for (flowIndex = 0; flowIndex < stats.size(); flowIndex++)
    {
        std::cout << seedNumber << "," << runNumber << ",";

        std::regex rgxNetwork("[0-9]{1,3}\\.[0-9]{1,3}\\.(.*)\\.[0-9]{1,3}");
        std::smatch matchNetwork;
        std::regex_search(flowSourceAddress[flowIndex], matchNetwork, rgxNetwork);

        std::regex rgxNode("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.(.*)");
        std::smatch matchNode;
        std::regex_search(flowSourceAddress[flowIndex], matchNode, rgxNode);

        if (matchNetwork[1].str() == "1")
        {
            std::cout << "A"
                      << ",";
            if ((uint32_t)std::stoi(matchNode[1].str()) == nStaA + nAp)
            {
                std::cout << "DL"
                          << ",";
            }
            else
            {
                std::cout << "UL"
                          << ",";
            }
        }
        else if (matchNetwork[1].str() == "2")
        {
            std::cout << "B"
                      << ",";
            if ((uint32_t)std::stoi(matchNode[1].str()) == nStaB + nAp)
            {
                std::cout << "DL"
                          << ",";
            }
            else
            {
                std::cout << "UL"
                          << ",";
            }
        }
        else if (matchNetwork[1].str() == "3")
        {
            std::cout << "C"
                      << ",";
            if ((uint32_t)std::stoi(matchNode[1].str()) == nStaC + nAp)
            {
                std::cout << "DL"
                          << ",";
            }
            else
            {
                std::cout << "UL"
                          << ",";
            }
        }
        else
        {
            std::cout << "D"
                      << ",";
            if ((uint32_t)std::stoi(matchNode[1].str()) == nStaD + nAp)
            {
                std::cout << "DL"
                          << ",";
            }
            else
            {
                std::cout << "UL"
                          << ",";
            }
        }

        std::cout << flowID[flowIndex] << "," << flowSourceAddress[flowIndex] << ","
                  << flowSourcePort[flowIndex] << "," << flowDestinationAddress[flowIndex] << ","
                  << flowDestinationPort[flowIndex] << "," << flowProtocol[flowIndex] << ","
                  << flowTxPackets[flowIndex] << "," << flowTxBytes[flowIndex] << ","
                  << flowTxOffered[flowIndex] << "," << flowRxPackets[flowIndex] << ","
                  << flowRxBytes[flowIndex] << "," << flowThroughput[flowIndex] << ","
                  << flowMeanDelay[flowIndex] << "," << flowLastDelay[flowIndex] << ","
                  << flowMeanJitter[flowIndex] << "\n";
    }
    return 0;
}

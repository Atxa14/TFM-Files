/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
// Copyright (c) 2023 Universidad del País Vasco / Euskal Herriko Unibertsitatea (UPV/EHU)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "three-gpp-ftp-m2-helper.h"
#include <ns3/packet-sink-helper.h>
#include <ns3/traffic-generator-ftp-single.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThreeGppFtpM2Helper");

ThreeGppFtpM2Helper::ThreeGppFtpM2Helper(ApplicationContainer* serverApps,
                                         ApplicationContainer* clientApps,
                                         NodeContainer* serverNodes,
                                         NodeContainer* clientNodes,
                                         Ipv4InterfaceContainer* serversIps)
{
    NS_LOG_FUNCTION(this);
    m_serverApps = serverApps;
    m_clientApps = clientApps;
    m_serverNodes = serverNodes;
    m_clientNodes = clientNodes;
    m_serversIps = serversIps;
}

ThreeGppFtpM2Helper::ThreeGppFtpM2Helper()
{
    NS_LOG_FUNCTION(this);
}

ThreeGppFtpM2Helper::~ThreeGppFtpM2Helper()
{
    NS_LOG_FUNCTION(this);
}

TypeId
ThreeGppFtpM2Helper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ThreeGppFtpM2Helper")
                            .SetParent<Object>()
                            .AddConstructor<ThreeGppFtpM2Helper>();
    return tid;
}

void
ThreeGppFtpM2Helper::DoConfigureFtpServers()
{
    NS_LOG_FUNCTION(this);
    Address apLocalAddress(InetSocketAddress(Ipv4Address::GetAny(), m_port));
    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", apLocalAddress);
    *m_serverApps = packetSinkHelper.Install(*m_serverNodes);
    m_serverApps->Start(m_serverStartTime);
}

void
ThreeGppFtpM2Helper::DoConfigureFtpClients()
{
    NS_LOG_FUNCTION(this);
    uint32_t ftpSegSize = 1448; // bytes
    TrafficGeneratorHelper ftpHelper("ns3::UdpSocketFactory",
                                     Address(),
                                     TrafficGeneratorFtpSingle::GetTypeId());
    ftpHelper.SetAttribute("PacketSize", UintegerValue(ftpSegSize));
    ftpHelper.SetAttribute("FileSize", UintegerValue(m_ftpFileSize->GetValue()));

    for (uint32_t i = 0; i < m_serversIps->GetN(); i++)
    {
        Ipv4Address ipAddress = m_serversIps->GetAddress(i, 0);
        AddressValue remoteAddress(InetSocketAddress(ipAddress, m_port));
        ftpHelper.SetAttribute("Remote", remoteAddress);
        m_clientApps->Add(ftpHelper.Install(*m_clientNodes));
    }

    m_clientApps->Start(m_clientStartTime + Seconds(m_startJitter->GetValue()));
}

void
ThreeGppFtpM2Helper::DoStartFileTransfer()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_lastClient >= 0 && m_lastClient < m_clientApps->GetN());
    Ptr<Application> app = m_clientApps->Get(m_lastClient);
    NS_ASSERT(app);
    Ptr<TrafficGenerator> fileTransfer = DynamicCast<TrafficGenerator>(app);
    NS_ASSERT(fileTransfer);
    fileTransfer->SendPacketBurst();

    m_lastClient += 1;
    if (m_lastClient == m_clientApps->GetN())
    {
        m_lastClient = 0;
    }
    Simulator::Schedule(DoGetNextTime(), &ThreeGppFtpM2Helper::DoStartFileTransfer, this);
}

void
ThreeGppFtpM2Helper::Configure(uint16_t port,
                               Time serverStartTime,
                               Time clientStartTime,
                               Time clientStopTime,
                               double ftpLambda,
                               double ftpMu,
                               double ftpSigma,
                               double dataRate)
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_IF(m_boolConfigured, "Already configured FTP M2 helper.");
    NS_ABORT_MSG_IF(m_serverNodes->GetN() == 0 || m_clientNodes->GetN() == 0 ||
                        m_serversIps->GetN() == 0,
                    "Server and/or client nodes or IP server interfaces not set.");
    m_port = port;
    m_clientStartTime = clientStartTime;
    m_clientStopTime = clientStopTime;
    m_ftpLambda = ftpLambda;
    m_ftpMu = ftpMu;
    m_ftpSigma = ftpSigma;
    m_dataRate = dataRate;
    m_serverStartTime = serverStartTime;
    m_boolConfigured = true;

    m_ftpFileSize = CreateObject<LogNormalRandomVariable>();
    m_ftpFileSize->SetAttribute("Mu", DoubleValue(m_ftpMu));
    m_ftpFileSize->SetAttribute("Sigma", DoubleValue(m_ftpSigma));
    m_transferTime = pow(m_dataRate,-1)*m_ftpFileSize->GetValue();

    m_ftpArrivals = CreateObject<ExponentialRandomVariable>();
    m_ftpArrivals->SetAttribute("Mean", DoubleValue(1 / m_ftpLambda));
    // Randomly distribute the start times across 100ms interval
    m_startJitter = CreateObject<UniformRandomVariable>();
    m_startJitter->SetAttribute("Max", DoubleValue(0.100));
}

void
ThreeGppFtpM2Helper::Start()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_boolConfigured);

    DoConfigureFtpServers();
    DoConfigureFtpClients();

    Simulator::Schedule(m_clientStartTime + DoGetNextTime(),
                        &ThreeGppFtpM2Helper::DoStartFileTransfer,
                        this);
}

Time
ThreeGppFtpM2Helper::DoGetNextTime() const
{
    return Seconds(m_ftpArrivals->GetValue())+ Seconds(m_transferTime);
}

}


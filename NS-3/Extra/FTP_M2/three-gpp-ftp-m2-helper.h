/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
// Copyright (c) 2023 Universidad del País Vasco / Euskal Herriko Unibertsitatea (UPV/EHU)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef THREE_GPP_FTP_M2_HELPER_H
#define THREE_GPP_FTP_M2_HELPER_H

#include <ns3/internet-apps-module.h>
#include <ns3/internet-module.h>
#include <ns3/network-module.h>
#include <ns3/node.h>
#include <ns3/ping-helper.h>
#include <ns3/traffic-generator-helper.h>

namespace ns3
{
/**
 * \ingroup helper
 * \brief Helper for a correct setup of every FTP Model 2 applications
 *
 * This class can help you in setting up a simulation that is using
 * an FTP Model 2 application.
 *
 * FTP Model 2 is described in section A.2.1.3.1 of TR36.814.
 *
 */

class ThreeGppFtpM2Helper : public Object
{
  public:
    ThreeGppFtpM2Helper();
    ThreeGppFtpM2Helper(ApplicationContainer* serverApps,
                        ApplicationContainer* clientApps,
                        NodeContainer* serverNodes,
                        NodeContainer* clientNodes,
                        Ipv4InterfaceContainer* serversIps);
    ~ThreeGppFtpM2Helper() override;
    static TypeId GetTypeId();
    void Configure(uint16_t port,
                   Time serverStartTime,
                   Time clientStartTime,
                   Time clientStopTime,
                   double ftpLambda,
                   double ftpMu,
                   double ftpSigma,
                   double dataRate);
    void Start();

  private:
    void DoConfigureFtpServers();
    void DoConfigureFtpClients();
    void DoStartFileTransfer();
    Time DoGetNextTime() const;
    uint32_t m_lastClient{0};
    Ptr<ExponentialRandomVariable> m_ftpArrivals;
    Ptr<UniformRandomVariable> m_startJitter;
    uint16_t m_port{0};
    Time m_clientStartTime{Seconds(0)};
    Time m_clientStopTime{Seconds(0)};
    double m_ftpLambda{0.0};
    double m_ftpMu{0.0};
    double m_ftpSigma{0.0};
    double m_dataRate{0.0};
    double m_transferTime{0.0};
    Ptr<LogNormalRandomVariable> m_ftpFileSize;
    Time m_serverStartTime{Seconds(0)};
    bool m_boolConfigured{false};
    ApplicationContainer* m_serverApps; 
    ApplicationContainer* m_clientApps;
    NodeContainer* m_serverNodes;
    NodeContainer* m_clientNodes;
    Ipv4InterfaceContainer* m_serversIps;
    ApplicationContainer m_pingApps;
};

};
#endif


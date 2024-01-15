/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Pennsylvania
 *
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

#ifndef PENN_SEARCH_H
#define PENN_SEARCH_H

#include "ns3/penn-application.h"
#include "penn-chord.h"
#include "penn-search-message.h"
#include "ns3/ping-request.h"
#include "ns3/ipv4-address.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include "ns3/socket.h"
#include "ns3/nstime.h"
#include "ns3/timer.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"

using namespace ns3;

class PennSearch : public PennApplication
{
  public:
    static TypeId GetTypeId (void);
    PennSearch ();
    virtual ~PennSearch ();

    void SendPing (std::string nodeId, std::string pingMessage);
    void SendPennSearchPing (Ipv4Address destAddress, std::string pingMessage);
    void RecvMessage (Ptr<Socket> socket);
    void ProcessPingReq (PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessPingRsp (PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void SendProcessSearchREQ(std::string req_node, std::string exec_node, std::vector<std::string> keys);
    void ProcessSearchREQLookup (PennSearchMessage message);
   // void HandleChordLookupSuccess(Ipv4Address KeyStoringNode, std::string key, uint32_t txnid, std::string LookupOriginator, PennChord::LookupType type );
    void ProcessSearchLookupRSP(PennSearchMessage message);
    void ProcessSearchREQ(PennSearchMessage message);
    void ProcessSearchRSP (PennSearchMessage message);
    void ProcessPublishReq (PennSearchMessage message);
    void AuditPings ();
    uint32_t GetNextTransactionId ();
    void Tokenize (const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters);
    std::map<std::string, std::vector<std::string>> CreateInvertedList(std::string metadataFile);

    void SendInvertedList(Ipv4Address destAddress);
    void ProcessInvertedList (PennSearchMessage message, Ipv4Address sourceAddress);
    bool IsKeyOwnedByMySuccessor(std::string key, Ipv4Address newNodeIp, Ipv4Address successorIp);
    void ProcessKeyTransferRejoin(PennSearchMessage msg);

 //*****************MS-2*************************************
  struct SearchEntry
  {
   Ipv4Address RequesterNode;
   std::vector<std::string> terms; 
   uint32_t termsCount;
   std::vector<std::string> movies;
   std::map<std::string, std::vector<std::string>> term_movies;
  };

  struct PublishEntry
  {
   std::string actor;
   std::vector<std::string> movies;
  };

    // Chord Callbacks
    void HandleChordPingSuccess (Ipv4Address destAddress, std::string message);
    void HandleChordPingFailure (Ipv4Address destAddress, std::string message);
    void HandleChordPingRecv (Ipv4Address destAddress, std::string message);
    void HandleChordLookupSuccess(Ipv4Address KeyStoringNode, std::string key, uint32_t txnid, std::string LookupOriginator, PennChord::LookupType type);
    void HandleLeavingApplication(Ipv4Address successorIp);
    void HandleRejoiningApplication(Ipv4Address newNodeIp, Ipv4Address successorIp);
    // From PennApplication
    virtual void ProcessCommand (std::vector<std::string> tokens);
    // From PennLog
    virtual void SetTrafficVerbose (bool on);
    virtual void SetErrorVerbose (bool on);
    virtual void SetDebugVerbose (bool on);
    virtual void SetStatusVerbose (bool on);
    virtual void SetChordVerbose (bool on);
    virtual void SetSearchVerbose (bool on);

  protected:
    virtual void DoDispose ();
    
  private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    Ptr<PennChord> m_chord;
    uint32_t m_currentTransactionId;
    Ptr<Socket> m_socket;
    Time m_pingTimeout;
    uint16_t m_appPort, m_chordPort;
    // Timers
    Timer m_auditPingsTimer;
    // Ping tracker
    std::map<uint32_t, Ptr<PingRequest> > m_pingTracker;
    //*****************MS-2*************************************
    // node could contain something like this contains {T1: [DOC1, DOC2, DOC3], T3: [DOC2,DOC4]} where DOC is movie and T is actor
    std::map<std::string, std::vector<std::string>> m_invertedList;
    std::map<uint32_t, PublishEntry> m_publishTracker;
    std::map<uint32_t, SearchEntry > m_searchEntryTracker;
    uint32_t m_lookupCount = 0;
};

#endif



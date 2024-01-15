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

#ifndef PENN_CHORD_H
#define PENN_CHORD_H

#include "ns3/penn-application.h"
#include "ns3/penn-chord-message.h"
#include "ns3/ping-request.h"
#include <openssl/sha.h>

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
#include <vector>

using namespace ns3;

class PennChord : public PennApplication
{
  public:
    static TypeId GetTypeId (void);
    enum LookupType
    {
      PUBLISH,
      SEARCH
    };
    PennChord ();
    virtual ~PennChord ();

    void SendPing (Ipv4Address destAddress, std::string pingMessage);
    void RecvMessage (Ptr<Socket> socket);
    void ProcessPingReq (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessPingRsp (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void AuditPings ();
    uint32_t GetNextTransactionId ();
    void StopChord ();

    // Callback with Application Layer (add more when required)
    void SetPingSuccessCallback (Callback <void, Ipv4Address, std::string> pingSuccessFn);
    void SetPingFailureCallback (Callback <void, Ipv4Address, std::string> pingFailureFn);
    void SetPingRecvCallback (Callback <void, Ipv4Address, std::string> pingRecvFn);

    // From PennApplication
    virtual void ProcessCommand (std::vector<std::string> tokens);
    
    
    // ---------------------------------------


  protected:
    virtual void DoDispose ();
    
  public:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    void SendRingStateMessage(std::string originator);
    void PrintRingState();
    void SendRequestToJoin(Ipv4Address nodeActingOnIp);
    void ProcessFindSuccessorMessage (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void SendFindSuccessor(std::string nodeToJoin);
    void ExecuteJoin(std::string nodeToJoin);
    void ExecuteLeave();
    void AddNodeAsSuccessor(std::string nodeToJoin);
    void SendStabilizeNotice();
    void SendCalculateFingerTableRequest(uint32_t key, uint32_t index, std::string originator, uint32_t transactionId);
    void SendSuccessorNotice(std::string recipient, std::string newSuccessor);
    void SendPredecessorNotice(std::string recipient, std::string newPredecessor, int leaveBoolean);
    void ProcessSuccessorNotice(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessPredecessorNotice(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessRequestToJoin(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessRingStateMessage(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessStabilizationRequest(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessStabilizationAnswer(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessCalculateFingerTableRequest(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessCalculateFingerTableAnswer(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void StabilizationTimerFunction();
    void SendCalculateFingerTableAnswer(std::string successorForKey, uint32_t key, uint32_t index, std::string originator, uint32_t transactionId);
    void FixFingerTable();
    void SetLookupSuccessCallback(Callback <void, Ipv4Address, std::string, uint32_t, std::string, PennChord::LookupType> lookupSuccess);
    void SetLeaveApplicationCallback(Callback <void, Ipv4Address> leaveApplication);

    void SetRejoinApplicationCallback(Callback <void, Ipv4Address, Ipv4Address> rejoinApplication);

    void SendGetSuccessorRequest(Ipv4Address needSuccessor, uint32_t transId);
    void ProcessGetSuccessorRequest(PennChordMessage message, Ipv4Address sourceAddress);

    void ProcessGetSuccessorRsp(PennChordMessage message, Ipv4Address sourceAddress);

bool IsKeyOwnedByMySuccessor(std::string key);
    struct LookupMessage {
      std::string key;
      uint32_t transactionId;
      uint32_t nodeHops;
      std::string lookupOriginator;
      PennChord::LookupType lookupType;
    };
    

    

    void FixFingerTableTimeFunction();

    // Lookups
    void ProcessLookupMessage(PennChordMessage message);
    void Lookup (std:: string key, uint32_t transactionId, uint16_t nodeHops, std::string LookupOriginator, PennChord::LookupType type);
    void SendLookUpMessage(std::string key, uint32_t transactionId, uint16_t nodeHops, std::string LookupOriginator, PennChord::LookupType type, Ipv4Address lastEntry);
void LookupSuccess();
void LookupFromSearch (std::string key, uint32_t transactionId, std::string LookupOriginator, PennChord::LookupType type);

    struct FingerTableEntry{
      //uint32_t index;
      Ipv4Address nodeIP;
      uint32_t key; //store IP and hashed key of a node where finger index points to
    };

    std::string m_predecessor;
    std::string m_successor;    
    std::string m_thisNode; 
    Ipv4Address m_thisNodeIp;
    std::map<uint32_t, LookupMessage> m_LookupMessages;
    std::vector<FingerTableEntry> m_FingerTable;
    std::map<uint32_t, Ipv4Address> m_FingerMap;
    uint32_t m_currentTransactionId;
    Ptr<Socket> m_socket;
    Time m_pingTimeout;
    Time m_stabilizeTimeout;
    Time m_fixFingerTimeout;
    uint16_t m_appPort;
    // Timers
    Timer m_auditPingsTimer;
    Timer m_stabilizeTimer;
    Timer m_fixFingerTimer;
    // Ping tracker
    std::map<uint32_t, Ptr<PingRequest> > m_pingTracker;
    


    void SetLookupType(PennChord::LookupType type);
    uint16_t FromLookupType(PennChord::LookupType lookUpType);
    PennChord::LookupType GetLookupTypeEnum(uint16_t type );
    PennChord::LookupType GetLookupType();  
    PennChord::LookupType m_lookupType;

    // Callbacks
    Callback <void, Ipv4Address, std::string> m_pingSuccessFn;
    Callback <void, Ipv4Address, std::string> m_pingFailureFn;
    Callback <void, Ipv4Address, std::string> m_pingRecvFn;
    Callback <void, Ipv4Address, std::string, uint32_t, std::string, PennChord::LookupType> m_lookupSuccess; //keystoring_node, key, transaction_id, lookup_originator Node
    Callback <void, Ipv4Address> m_leaveApplication;
    Callback <void, Ipv4Address, Ipv4Address> m_rejoinApplication;
    uint32_t m_totalHopCount;
    uint32_t m_totalLookUpCount;
};

#endif


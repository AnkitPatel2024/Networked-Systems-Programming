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


#include "penn-chord-message.h"
#include "penn-chord.h"
#include "ns3/inet-socket-address.h"
#include "ns3/random-variable-stream.h"
#include "penn-key-helper.h"
#include "grader-logs.h"
#include <openssl/sha.h>
#include <chrono>  // seconds, milliseconds
#include <thread>  // sleep_for
using namespace ns3;

TypeId
PennChord::GetTypeId ()
{
  static TypeId tid
      = TypeId ("PennChord")
            .SetParent<PennApplication> ()
            .AddConstructor<PennChord> ()
            .AddAttribute ("AppPort", "Listening port for Application", UintegerValue (10001),
                           MakeUintegerAccessor (&PennChord::m_appPort), MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("PingTimeout", "Timeout value for PING_REQ in milliseconds", TimeValue (MilliSeconds (2000)),
                           MakeTimeAccessor (&PennChord::m_pingTimeout), MakeTimeChecker ())
            .AddAttribute ("StabilizationTimeout", "Timeout value for stabilize in milliseconds", TimeValue (MilliSeconds (2000)),
                           MakeTimeAccessor (&PennChord::m_stabilizeTimeout), MakeTimeChecker ())
            .AddAttribute ("fixFingerTimeout", "Timeout value for fixing the finger table in milliseconds", TimeValue (MilliSeconds (5000)),
                           MakeTimeAccessor (&PennChord::m_fixFingerTimeout), MakeTimeChecker ())
  ;
  return tid;
}

PennChord::PennChord ()
    : m_auditPingsTimer (Timer::CANCEL_ON_DESTROY)
{
  Ptr<UniformRandomVariable> m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
  m_currentTransactionId = m_uniformRandomVariable->GetValue (0x00000000, 0xFFFFFFFF);
}

PennChord::~PennChord ()
{

}

void
PennChord::DoDispose ()
{
  StopApplication ();
  PennApplication::DoDispose ();
}


void
PennChord::StartApplication (void)
{
  //std::cout << "PennChord::StartApplication()!!!!!" << std::endl;
  if (m_socket == 0)
    { 
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), m_appPort);
      m_socket->Bind (local);
      m_socket->SetRecvCallback (MakeCallback (&PennChord::RecvMessage, this));
    }  
  
  // Configure timers
  m_auditPingsTimer.SetFunction (&PennChord::AuditPings, this);
  m_stabilizeTimer.SetFunction (&PennChord::StabilizationTimerFunction, this);
  m_fixFingerTimer.SetFunction (&PennChord::FixFingerTableTimeFunction, this);

  
  // Start timers
  m_auditPingsTimer.Schedule (m_pingTimeout);
  m_stabilizeTimer.Schedule (m_stabilizeTimeout);
  m_fixFingerTimer.Schedule(m_fixFingerTimeout);

  m_thisNodeIp = GetLocalAddress();
  m_thisNode = ReverseLookup(m_thisNodeIp); 
  m_predecessor = "-1";
  m_successor = "-1";
  m_FingerTable.resize(32);
  m_totalLookUpCount = 0;
 m_totalHopCount = 0;
}

void
PennChord::StopApplication (void)
{
  // Close socket
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  // Cancel timers
  m_auditPingsTimer.Cancel ();
  m_stabilizeTimer.Cancel();
  m_fixFingerTimer.Cancel();
  m_pingTracker.clear ();
  GraderLogs::AverageHopCount(m_thisNode, m_totalLookUpCount, m_totalHopCount);


}

void
PennChord::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;
  Ipv4Address nodeIp = GetLocalAddress();
  std::string node = ReverseLookup(nodeIp); 

  if (tokens.size() >= 2) {
  iterator++;
  std::string nodeActingOn = *iterator;
  tokens.erase (iterator);
  Ipv4Address nodeActingOnIp = PennApplication::ResolveNodeIpAddress(nodeActingOn);

  if (command == "JOIN") {
      
      // Bootstrap the Chord
     if (node == nodeActingOn) {
        m_successor = node; 
        m_predecessor = node;
      }
      // Start adding to the Chord
      //nodeActingOn is the node that will help the new node join the chord
      else {
      SendRequestToJoin(nodeActingOnIp);    
      }
  }
  }
    if (command == "RINGSTATE") {
      // Prints the ring state
     PrintRingState();
     // Sends RingState message to successor
     SendRingStateMessage(node); 
  }

      if (command == "FIX") {
        FixFingerTable();

  }

      if (command == "PRINT") {

          std::cout << "finger table for " << m_thisNode << "\n";

        for (int i = 0; i < int(m_FingerTable.size()); i++) {


          std::cout << "\n" << "index " << i << " " << ReverseLookup(m_FingerTable[i].nodeIP) <<  " IP KEY: " << PennKeyHelper::CreateShaKey(m_FingerTable[i].nodeIP)<< "\n";
        }
                  std::cout << "\n" << "end" << "\n";
  }







if (command == "FS") {

  Ipv4Address node4 = ResolveNodeIpAddress("4");
   Ipv4Address node6 = ResolveNodeIpAddress("6");
  Ipv4Address node8 = ResolveNodeIpAddress("8");


  SendGetSuccessorRequest(node4, GetNextTransactionId());
  SendGetSuccessorRequest(node6, GetNextTransactionId());
  SendGetSuccessorRequest(node8, GetNextTransactionId());

}




  // if (command == "LOOKUP") {
  //     // Prints the ring state
  //   Lookup ("George", PennChord::GetNextTransactionId(), 1, "1", GetLookupTypeEnum(1));
  //    // Sends RingState message to successor
  //    //SendRingStateMessage(node); 
  // }

  if (command == "LEAVE") {
    ExecuteLeave();
  }
}

void PennChord::PrintRingState() {
  Ipv4Address nodeIp = GetLocalAddress();
  std::string node = ReverseLookup(nodeIp); 
  std::uint32_t nodeKey = PennKeyHelper::CreateShaKey(nodeIp);

  Ipv4Address predIp = PennApplication::ResolveNodeIpAddress(m_predecessor); 
  std::uint32_t predKey = PennKeyHelper::CreateShaKey(predIp);

  Ipv4Address succIp = PennApplication::ResolveNodeIpAddress(m_successor); 
  std::uint32_t succKey = PennKeyHelper::CreateShaKey(succIp);

  // Prints the Ringstate
  GraderLogs::RingState(nodeIp, node, nodeKey, predIp, m_predecessor, predKey, succIp, m_successor, succKey); 
  // std::cout << "\n" << " NODE : " << node << " CURRENT HASH KEY " <<  nodeKey << "\n";

  }

void PennChord::SendRingStateMessage(std::string originator) {
    Ipv4Address destAddress = PennApplication::ResolveNodeIpAddress(m_successor);

    if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = PennChord::GetNextTransactionId ();
     //CHORD_LOG ("Sending Ring State Message to Node: " << m_successor << " IP: " << destAddress << " Sending RingStateMessage to:  " << m_successor << " transactionId: " << transactionId);

      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::RING_STATE, transactionId);
      message.SetRingState (originator);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
      else
    {
      // Report failure   
      m_pingFailureFn (destAddress, m_successor);
    }
}

void PennChord::ProcessRingStateMessage(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
    std::string fromNode = ReverseLookup (sourceAddress);

    std::string originator = message.GetRingState().originator;
    // This indicator that this node send the RingState message in the first place and should not re-print or re-send
    if (m_thisNode == originator) {
      GraderLogs::EndOfRingState();
    }
    else {
      PrintRingState();
      SendRingStateMessage(originator); 
       // std::cout << "\n" << "Fix finger table for node: " << m_thisNode << " This nodes key: " << PennKeyHelper::CreateShaKey(m_thisNodeIp) << "\n";
         // std::cout << "\n" << "Key for JK simmons: " << PennKeyHelper::CreateShaKey(std::string("JK-Simmons")) << "\n";

        // for (int i = 0; i < int(m_FingerTable.size()); i++) {
        //       std::cout << "Key: " << m_FingerTable[i].key << " Key: " <<  PennKeyHelper::CreateShaKey(m_FingerTable[i].nodeIP) << "Node: " << ReverseLookup(m_FingerTable[i].nodeIP) << "\n";
        // }


    }
}

void PennChord::SendRequestToJoin(Ipv4Address nodeActingOnIp) {
    // Node that will help the new node join
    std::string nodeActingOn = ReverseLookup (nodeActingOnIp);
    Ipv4Address destAddress = nodeActingOnIp;
    // The current node that is executing this code, which is the node that wants to join
  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = PennChord::GetNextTransactionId ();
    //  CHORD_LOG ("Request to join chord, From Node: " << thisNode << ", Message: " << "I want to join chord via node: " + nodeActingOn);
      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::REQ_JOIN, transactionId);
      message.SetReqJoin (m_thisNode);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, nodeActingOn);
    }
}

void PennChord:: ProcessRequestToJoin(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  // the fromNode and nodeToJoin will actually be the same value
    std::string fromNode = ReverseLookup (sourceAddress);
    std::string nodeToJoin = message.GetReqJoin().newReqJoinMessage;
//    CHORD_LOG ("Received Request to join the chord, From Node: " << fromNode << ", Message: " << nodeToJoin );
    ExecuteJoin(nodeToJoin);
}


void PennChord::ExecuteJoin(std::string nodeToJoin) {


 uint32_t successorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_successor));
    uint32_t nodeToJoinHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(nodeToJoin));
    uint32_t thisNodeHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_thisNode));


    // If successor is greater than the node that wants to join, add the node between the current node and its successor
    // If the successor is not greater than the node that wants to join because it is zero, we will add the node between the current
    // node and zero ONLY if the node is greater than the current node. If not, we will send the findSuccessor message.
    //if (std::stoi(m_successor) > std::stoi(nodeToJoin) || ((m_successor) == "0" && std::stoi(nodeToJoin) > std::stoi(m_thisNode))) {
    //if (m_thisNode == m_predecessor) {  
     // m_successor = nodeToJoin;
    // m_predecessor = nodeToJoin;
   // SendPredecessorNotice(nodeToJoin, m_thisNode, 0);
   //  SendSuccessorNotice(nodeToJoin, m_thisNode);
    //}
    //else{
    if (successorHash > thisNodeHash) {

        if (nodeToJoinHash > thisNodeHash && nodeToJoinHash <= successorHash) {
              AddNodeAsSuccessor(nodeToJoin);
               Ipv4Address succAddress = PennApplication::ResolveNodeIpAddress(m_successor);
              Ipv4Address nodeToJoinAddress = PennApplication::ResolveNodeIpAddress(nodeToJoin);
              m_rejoinApplication(nodeToJoinAddress, succAddress);
              m_successor = nodeToJoin;
              //I am the predecessor for this node who wants to join
              //send predecessor notice to this nodetoJoin
          //  SendPredecessorNotice(nodeToJoin, m_thisNode, 0);
          //    SendPredecessorNotice(m_successor,nodeToJoin, 0);
             
        }
        else {
            SendFindSuccessor(nodeToJoin);

        }
    }

    else {
      if (nodeToJoinHash > thisNodeHash || nodeToJoinHash <= successorHash) {
        if(m_successor == m_thisNode)
        {

          m_successor = nodeToJoin;
          m_predecessor = nodeToJoin;
         //  SendPredecessorNotice(nodeToJoin, m_thisNode, 0);
          //SendSuccessorNotice( m_thisNode, nodeToJoin);         
         SendSuccessorNotice(nodeToJoin, m_thisNode);
        }
       else{
            // Tell node that its successor is my successor
            AddNodeAsSuccessor(nodeToJoin);
            // then make it my successor'

             Ipv4Address succAddress = PennApplication::ResolveNodeIpAddress(m_successor);
            Ipv4Address nodeToJoinAddress = PennApplication::ResolveNodeIpAddress(nodeToJoin);
            m_rejoinApplication(nodeToJoinAddress, succAddress);
            m_successor = nodeToJoin;

            //I am the predecessor for this node who wants to join
              //send predecessor notice to this nodetoJoin
          //  SendPredecessorNotice(nodeToJoin, m_thisNode, 0);
          //    SendPredecessorNotice(m_successor,nodeToJoin, 0);
      }
      }
      else {
              SendFindSuccessor(nodeToJoin);

      }
       Ipv4Address succAddress = PennApplication::ResolveNodeIpAddress(m_successor);
            Ipv4Address nodeToJoinAddress = PennApplication::ResolveNodeIpAddress(nodeToJoin);
            m_rejoinApplication(nodeToJoinAddress, succAddress);
    
    }
      
    //}
}

void PennChord::ExecuteLeave() {

  // std::cout <<  "I am leaving: " << m_thisNode << "\n";
    SendPredecessorNotice(m_successor, m_predecessor, 1);

    SendSuccessorNotice(m_predecessor, m_successor);
    Ipv4Address succAddress = PennApplication::ResolveNodeIpAddress(m_successor);
    m_successor = "-1";
    m_predecessor = "-1";
    m_leaveApplication(succAddress);
    
}


void PennChord::SendFindSuccessor(std::string nodeToJoin) {

  Ipv4Address destAddress = PennApplication::ResolveNodeIpAddress(m_successor);

  if (destAddress != Ipv4Address::GetAny ())
    {

      uint32_t transactionId = PennChord::GetNextTransactionId ();
    // CHORD_LOG ("Sending Find Successor Message to Node: " << m_successor << " IP: " << destAddress << " Finding successor for node: " << nodeToJoin << " transactionId: " << transactionId);

      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::FIND_SUCC, transactionId);
      message.SetFindSucc (nodeToJoin);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, "Could not send Find Successor notice for " + nodeToJoin);
    }
}

void PennChord::ProcessFindSuccessorMessage(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
    std::string fromNode = ReverseLookup (sourceAddress);
    std::string nodeToJoin = message.GetFindSucc().nodeToJoin;
   // CHORD_LOG ("Received Find Succesor, From Node: " << fromNode << ", Message: " << nodeToJoin );
    ExecuteJoin(nodeToJoin);
}

void PennChord::AddNodeAsSuccessor(std::string nodeToJoin) {

    SendSuccessorNotice(nodeToJoin, m_successor);
}

void PennChord::SendSuccessorNotice(std::string recipient, std::string newSuccessor ) {
      Ipv4Address destAddress = PennApplication::ResolveNodeIpAddress(recipient);

  if (destAddress != Ipv4Address::GetAny ())
    {
      //CHORD_LOG("execute SendSuccessorNotice");
      uint32_t transactionId = PennChord::GetNextTransactionId ();
      CHORD_LOG ("Sending Successor Notice to Node: " << recipient << " IP: " << destAddress << " New Successor: " << newSuccessor << " transactionId: " << transactionId);

      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::NEW_SUCC, transactionId);
      message.SetNewSucc (newSuccessor);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, newSuccessor);
    }
}

void PennChord::SendPredecessorNotice(std::string recipient, std::string newPredecessor, int leaveBoolean ) {
      Ipv4Address destAddress = PennApplication::ResolveNodeIpAddress(recipient);

  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = PennChord::GetNextTransactionId ();
      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::NEW_PRED, transactionId);
      message.SetNewPred (newPredecessor, leaveBoolean);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, newPredecessor);
    }
}

void
PennChord::ProcessSuccessorNotice(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
//CHORD_LOG("execute ProcessSuccessorNotice");
    // Use reverse lookup for ease of debug
    std::string sender = ReverseLookup (sourceAddress);
    std::string newSuccessor = message.GetNewSucc().newSuccMessage;

    CHORD_LOG ("Received Successor Notice, From Node: " << sender << ", New Successor: " << newSuccessor);
    m_successor = newSuccessor;
    
    //Stabilize
    SendStabilizeNotice();
}

void PennChord::SendStabilizeNotice() {

  if (m_successor != "-1") {
  Ipv4Address destAddress = PennApplication::ResolveNodeIpAddress(m_successor);

  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = PennChord::GetNextTransactionId ();
    //  CHORD_LOG ("Sending Successor Notice to Node: " << recipient << " IP: " << destAddress << " New Successor: " << newSuccessor << " transactionId: " << transactionId);

      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::STABILIZE_REQUEST, transactionId);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, "Could not send stabilization notice");
    }
  }
}

void PennChord::FixFingerTableTimeFunction() {
      FixFingerTable();
      m_fixFingerTimer.Schedule (m_fixFingerTimeout);

}

void PennChord::StabilizationTimerFunction() {
  //New function to reset timer. Seperate from SendStabilizeNotice to avoid scheduling it twice since SendStabilizationNotice could be called via timer or via new node joining chord
  SendStabilizeNotice();
    m_stabilizeTimer.Schedule (m_stabilizeTimeout);
}

void PennChord::ProcessStabilizationRequest(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
    // uint32_t successorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_successor));
    // uint32_t predHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_predecessor));
    // uint32_t thisNodeHash = PennKeyHelper::CreateShaKey(m_thisNodeIp);
    PennChordMessage resp = PennChordMessage (PennChordMessage::STABILIZE_ANSWER, message.GetTransactionId());
    //PRINT_LOG("line 489 in chord: I am node: "<< m_thisNode<<"My Hash: "<< thisNodeHash <<" And My predecessor: "<< m_predecessor
    //<<"Pred Hash: " << predHash << " SUCESSOR: "<< m_successor <<"SUC HASH :"<<successorHash );
    resp.SetStabilizeAnswer(m_predecessor);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
}

void PennChord::ProcessStabilizationAnswer(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
    std::string successorPredecessor = message.GetStabilizeAnswer().predecessor; 

    if (successorPredecessor != m_thisNode) {
    //If the successor's predecessor is less than the current node, then the current node is a better predecessor. In this case,
    // this node should tell the successor that its new predecessor is it
    uint32_t successorPredecessorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(successorPredecessor));
    // this is 2
    uint32_t successorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_successor));
   // uint32_t predHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_predecessor));
    uint32_t thisNodeHash = PennKeyHelper::CreateShaKey(m_thisNodeIp);
  // this node is 1
  // if (m_successor == "-1")
  // {  PRINT_LOG("line 510 I am node: " <<m_thisNode );}

  if (successorPredecessor == "-1") {
      SendPredecessorNotice(m_successor, m_thisNode, 0);

  }


   else if (successorHash > thisNodeHash){

    if (successorPredecessorHash > thisNodeHash) {
      m_successor = successorPredecessor;
      SendPredecessorNotice(m_successor, m_thisNode, 0);
    }

     else if (thisNodeHash > successorPredecessorHash){
        SendPredecessorNotice(m_successor, m_thisNode, 0);
    }
   }
    else {
        
        
        if (successorPredecessorHash < thisNodeHash ) {
              SendPredecessorNotice(m_successor, m_thisNode, 0);
        } 
        else {
          m_successor = successorPredecessorHash;
                        SendPredecessorNotice(m_successor, m_thisNode, 0);

        }
        
    

      // //crossing over 0
      // if (successorPredecessorHash > thisNodeHash) {
      //   m_successor = successorPredecessor;
      //     SendPredecessorNotice(m_successor, m_thisNode, 0);
      // }
      // else if (successorPredecessorHash < thisNodeHash && successorPredecessorHash > successorHash) {
      //    m_successor = successorPredecessor;
      //     SendPredecessorNotice(m_successor, m_thisNode, 0);
      // }
      // else if ((successorPredecessorHash < thisNodeHash)  &&(successorPredecessorHash > predHash || m_predecessor == "-1"))
      //   //Tell successor new predecessor is thisNode
      //   SendPredecessorNotice(m_successor, m_thisNode, 0);
    }
  }
    
}

void
PennChord::ProcessPredecessorNotice (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  
    // Use reverse lookup for ease of debug
    std::string sender = ReverseLookup (sourceAddress);
    std::string newPredecessor = message.GetNewPred().newPredMessage;
    //uint16_t leaveBoolean = message.GetNewPred().leaveBoolean;
    CHORD_LOG ("Received Predecessor Notice, From Node: " << sender << ", New Predecessor: " << newPredecessor);
    // If the new predecessor is greater than the current, it is closer to the node and thus should be the new predecessor
    /*if (leaveBoolean == 1) {
        m_predecessor = newPredecessor;
    }
    else {
    if (m_predecessor == "-1" || std::stoi(newPredecessor) > std::stoi(m_predecessor)) {
        m_predecessor = newPredecessor;
    }
    }
    */
  m_predecessor = newPredecessor;

}

void PennChord::SendCalculateFingerTableRequest(uint32_t key, uint32_t index, std::string originator, uint32_t transactionId) {

if (m_successor != "-1") {
  Ipv4Address destAddress = PennApplication::ResolveNodeIpAddress(m_successor);

  if (destAddress != Ipv4Address::GetAny ())
    {
     // std::cout << "\n" << "SENDING FIX FINGER TABLE" << "\n";
      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::CALCULATE_FINGER_TABLE_REQ, transactionId);
      message.SetCalculateFingerTableRequest(key, originator, index);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    //  std::cout << "Sent calculate finger table message ";
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, "Could not send calculate finger table entry notice");
    }
}
  
}

void PennChord::ProcessCalculateFingerTableRequest(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
       //     std::cout << "\n" << "got fix finger request" << "\n";

     PennChordMessage::CalculateFingerTableRequest fingerRequest = message.GetCalculateFingerTableRequest();
     uint32_t key = fingerRequest.key;
     std::string originator = fingerRequest.originator;
    uint32_t index = fingerRequest.index;
    uint32_t currentNodeHash = PennKeyHelper::CreateShaKey(m_thisNodeIp);
  Ipv4Address successorIp = PennApplication::ResolveNodeIpAddress(m_successor);
    uint32_t sucNodeHash = PennKeyHelper::CreateShaKey(successorIp );

  



     if (m_successor != "-1") {

    if( sucNodeHash > currentNodeHash){
      if (key <= sucNodeHash  && key > currentNodeHash) {
          PennChord::SendCalculateFingerTableAnswer(m_successor, key, index, originator, message.GetTransactionId());
      }
      else {
              PennChord::SendCalculateFingerTableRequest(key, index, originator, message.GetTransactionId());
      }
    
      }
    else{
       //sucNodeHash <= currentNodeHash)
        if (key <= sucNodeHash  || key > currentNodeHash) {
          PennChord::SendCalculateFingerTableAnswer(m_successor, key, index, originator, message.GetTransactionId());
       }
       else {
              PennChord::SendCalculateFingerTableRequest(key, index, originator, message.GetTransactionId());
       }
      }
     






}
}

void PennChord::SendCalculateFingerTableAnswer(std::string successorForKey, uint32_t key, uint32_t index, std::string originator, uint32_t transactionId) {
    
    Ipv4Address originatorIpv4 = PennApplication::ResolveNodeIpAddress(originator);
    PennChordMessage resp = PennChordMessage (PennChordMessage::CALCULATE_FINGER_TABLE_ANSWER, transactionId);
    resp.SetCalculateFingerTableAnswer(successorForKey, key, index);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (originatorIpv4, m_appPort));
}

void PennChord::ProcessCalculateFingerTableAnswer(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {


    std::string successorForKey = message.GetCalculateFingerTableAnswer().successorForKey;
    uint32_t key = message.GetCalculateFingerTableAnswer().key;
    uint32_t index = message.GetCalculateFingerTableAnswer().index;
    FingerTableEntry entry;
    entry.key = key;
    entry.nodeIP = PennApplication::ResolveNodeIpAddress(successorForKey);
    m_FingerTable[index] = entry;
}




void PennChord::FixFingerTable() {
  if (m_successor != "-1") {
    // Move this variable
    uint32_t mBits = 32;
    Ipv4Address successorIp = PennApplication::ResolveNodeIpAddress(m_successor);
     uint32_t currentHash = PennKeyHelper::CreateShaKey(m_thisNodeIp);
      uint32_t sucHash = PennKeyHelper::CreateShaKey(successorIp);

    for (uint32_t i = 0; i < mBits; i++) {
      //uint32_t key = (PennKeyHelper::CreateShaKey(m_thisNodeIp) + int(std::pow(2, i))) % int(std::pow(2, mBits));
  

      uint32_t key = (PennKeyHelper::CreateShaKey(m_thisNodeIp) + int(std::pow(2, i)));

      if (int(key) > int(std::pow(2, 32))) {
        key = key % int(std::pow(2, mBits));
      }

      // if ( m_thisNode == "4") {

      
      // if (i == 29 || i == 30 || i == 31) {
      //   std::cout << "\n" << "index: " << i << " key " << key << "\n";
      // }
      // }

    FingerTableEntry entry;
      entry.key = key;



    // if ( i ==31 && m_thisNode == "2") {
    //   std::cout << "\n" << key << "\n";

    // }

        if (i == 0) {
        entry.key = key;
        entry.nodeIP = ResolveNodeIpAddress(m_successor);
      m_FingerTable[0] = entry;
      }
      else if (key == currentHash) {
           entry.key = key;
        entry.nodeIP = m_thisNodeIp;
      m_FingerTable[i] = entry;
      }

      // If the finger[i] is greater than the current node (always) and less than the current node's succesor,
      // then the finger entry is the current successor
      else if (sucHash > currentHash){
        if(key > currentHash && key <= sucHash) {
      entry.nodeIP = PennApplication::ResolveNodeIpAddress(m_successor);
      m_FingerTable[i] = entry;
      }
      else {
      SendCalculateFingerTableRequest( key, i, m_thisNode, GetNextTransactionId());
      }
      }

      else if (sucHash <= currentHash) {
        //sucHash <= currentHash and wrap around
        if (key > currentHash || key <= sucHash){
          entry.nodeIP = PennApplication::ResolveNodeIpAddress(m_successor);
      m_FingerTable[i] = entry;
        }
        else {
              SendCalculateFingerTableRequest( key, i, m_thisNode, GetNextTransactionId());

        }
      }
      // Otherwise if node + 2^i mod (2^mBits) is greater than the successor, then we need the next node's successor
      // for the finger table
      else {

  
       SendCalculateFingerTableRequest( key, i, m_thisNode, GetNextTransactionId());
  
      }
    }
    }
  }
















// void PennChord::FixFingerTable() {

//   if (m_successor != "-1") {
//     // Move this variable
//     u_int32_t mBits = 32;
//     Ipv4Address successorIp = PennApplication::ResolveNodeIpAddress(m_successor);

//     for (uint32_t i = 0; i < mBits; i++) {
//       uint32_t key = (PennKeyHelper::CreateShaKey(m_thisNodeIp) + int(std::pow(2, i)));

//       if (int(key) > int(std::pow(2, 32))) {
//         key = key % int(std::pow(2, mBits));
//       }


//       FingerTableEntry entry;
//       entry.key = key;

//       if (i == 0) {
//         entry.key = key;
//         entry.nodeIP = ResolveNodeIpAddress(m_successor);
//       m_FingerTable[0] = entry;
//       }

//       // If the finger[i] is greater than the current node (always) and less than the current node's succesor,
//       // then the finger entry is the current successor
//       else if (key > PennKeyHelper::CreateShaKey(m_thisNodeIp) && 
//       key <= PennKeyHelper::CreateShaKey(successorIp)) {
        
//       entry.nodeIP = PennApplication::ResolveNodeIpAddress(m_successor);

//       m_FingerTable[i] = entry;
//       }
//       // Otherwise if node + 2^i mod (2^mBits) is greater than the successor, then we need the next node's successor
//       // for the finger table
//       else {
//        SendCalculateFingerTableRequest( key, i, m_thisNode, GetNextTransactionId());
//       }
//     }
//   }
// }

void
PennChord::SetLookupType(LookupType type){
  m_lookupType = type;
}
PennChord::LookupType
PennChord::GetLookupType(){
  return m_lookupType;
}


void
PennChord::SendPing (Ipv4Address destAddress, std::string pingMessage)
{
  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = PennChord::GetNextTransactionId ();
   //   CHORD_LOG ("Sending PING_REQ to Node: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << pingMessage << " transactionId: " << transactionId);
      Ptr<PingRequest> pingRequest = Create<PingRequest> (transactionId, Simulator::Now(), destAddress, pingMessage);
      // Add to ping-tracker
      m_pingTracker.insert (std::make_pair (transactionId, pingRequest));
      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::PING_REQ, transactionId);
      message.SetPingReq (pingMessage);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
      
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, pingMessage);
    }
}

void
PennChord::RecvMessage (Ptr<Socket> socket)
{
  Address sourceAddr;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
  Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
  uint16_t sourcePort = inetSocketAddr.GetPort ();
  PennChordMessage message;
  packet->RemoveHeader (message);

  switch (message.GetMessageType ())
    {
      case PennChordMessage::PING_REQ:
        ProcessPingReq (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::PING_RSP:
        ProcessPingRsp (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::NEW_SUCC:
        ProcessSuccessorNotice (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::NEW_PRED:
        ProcessPredecessorNotice (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::FIND_SUCC:
        ProcessFindSuccessorMessage (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::REQ_JOIN:
        ProcessRequestToJoin (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::RING_STATE:
        ProcessRingStateMessage (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::STABILIZE_REQUEST:
         ProcessStabilizationRequest(message, sourceAddress, sourcePort);
         break;
      case PennChordMessage::STABILIZE_ANSWER:
         ProcessStabilizationAnswer(message, sourceAddress, sourcePort);
         break;
      case PennChordMessage::CALCULATE_FINGER_TABLE_REQ:
         ProcessCalculateFingerTableRequest(message, sourceAddress, sourcePort);
         break;
      case PennChordMessage::CALCULATE_FINGER_TABLE_ANSWER:
         ProcessCalculateFingerTableAnswer(message, sourceAddress, sourcePort);
         break;
      case PennChordMessage::LOOKUP_REQ:
         ProcessLookupMessage(message);
         break;
     case PennChordMessage::GET_SUCCESSOR:
         ProcessGetSuccessorRequest(message, sourceAddress);
         break;
    case PennChordMessage::GET_SUCCESSOR_RSP:
        ProcessGetSuccessorRsp(message, sourceAddress);
         break;
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}

void
PennChord::ProcessPingReq (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    // Use reverse lookup for ease of debug
    std::string fromNode = ReverseLookup (sourceAddress);

 //   CHORD_LOG ("Received PING_REQ, From Node: " << fromNode << ", Message: " << message.GetPingReq().pingMessage);
    // Send Ping Response
    PennChordMessage resp = PennChordMessage (PennChordMessage::PING_RSP, message.GetTransactionId());
    resp.SetPingRsp (message.GetPingReq().pingMessage);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
    // Send indication to application layer
    m_pingRecvFn (sourceAddress, message.GetPingReq().pingMessage);
}

void
PennChord::ProcessPingRsp (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // Remove from pingTracker
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  iter = m_pingTracker.find (message.GetTransactionId ());
  if (iter != m_pingTracker.end ())
    {
      std::string fromNode = ReverseLookup (sourceAddress);
   //   CHORD_LOG ("Received PING_RSP, From Node: " << fromNode << ", Message: " << message.GetPingRsp().pingMessage);
      m_pingTracker.erase (iter);
      // Send indication to application layer
      m_pingSuccessFn (sourceAddress, message.GetPingRsp().pingMessage);
    }
  else
    {
      DEBUG_LOG ("Received invalid PING_RSP!");
    }
}

void
PennChord::AuditPings ()
{
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  for (iter = m_pingTracker.begin () ; iter != m_pingTracker.end();)
    {
      Ptr<PingRequest> pingRequest = iter->second;
      if (pingRequest->GetTimestamp().GetMilliSeconds() + m_pingTimeout.GetMilliSeconds() <= Simulator::Now().GetMilliSeconds())
        {
          DEBUG_LOG ("Ping expired. Message: " << pingRequest->GetPingMessage () << " Timestamp: " << pingRequest->GetTimestamp().GetMilliSeconds () << " CurrentTime: " << Simulator::Now().GetMilliSeconds ());
          // Remove stale entries
          m_pingTracker.erase (iter++);
          // Send indication to application layer
          m_pingFailureFn (pingRequest->GetDestinationAddress(), pingRequest->GetPingMessage ());
        }
      else
        {
          ++iter;
        }
    }
  // Rechedule timer
  m_auditPingsTimer.Schedule (m_pingTimeout); 
}

uint32_t
PennChord::GetNextTransactionId ()
{
  return m_currentTransactionId++;
}

void
PennChord::StopChord ()
{
  StopApplication ();
}

void
PennChord::SetPingSuccessCallback (Callback <void, Ipv4Address, std::string> pingSuccessFn)
{
  m_pingSuccessFn = pingSuccessFn;
}


void
PennChord::SetPingFailureCallback (Callback <void, Ipv4Address, std::string> pingFailureFn)
{
  m_pingFailureFn = pingFailureFn;
}

void
PennChord::SetPingRecvCallback (Callback <void, Ipv4Address, std::string> pingRecvFn)
{
  m_pingRecvFn = pingRecvFn;
}

void PennChord::LookupFromSearch (std::string key, uint32_t transactionId, std::string LookupOriginator, PennChord::LookupType type) {
      
      std::uint32_t nodeKey = PennKeyHelper::CreateShaKey(m_thisNodeIp);
      uint32_t hashed_key = PennKeyHelper::CreateShaKey(key);
      
      GraderLogs::GetLookupIssueLogStr(nodeKey, hashed_key);
      if (type == LookupType::SEARCH) {
              m_totalLookUpCount++;
      }
      uint16_t nodeHops = 0;
      Lookup(key, transactionId, nodeHops, LookupOriginator, type);
}

bool
PennChord::IsKeyOwnedByMySuccessor(std::string key) {

 uint32_t hashed_key = PennKeyHelper::CreateShaKey(key);
  uint32_t thisNodeHash = PennKeyHelper::CreateShaKey(m_thisNodeIp);
   uint32_t successorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_successor));



  if (successorHash > thisNodeHash) {

    if (hashed_key > thisNodeHash && hashed_key <= successorHash ) {

       // std::cout << "\n" << "Hashed key: " << hashed_key << " This Node: " << m_thisNode << " This node hash: " << thisNodeHash << " Successor: " << m_successor << " successor hash " << successorHash;

      return true;
    }
    else {
            
      return false;
    }
  }

  else {
    if (hashed_key <= successorHash || hashed_key > thisNodeHash)  {
                
      return true;

    }
    else  {
      return false;
    }
    // else if ( hashed_key >= 0 && hashed_key <= successorHash) {
    //               if (key == std::string("Ian-McKellen")) {
    //     std::cout << "\n" << "HERE HERE HERE1" << "\n";
    //     std::cout << "\n" << "HERE HERE HERE1" << "\n";
      
    //   }
    //   return true;
    // }
    // else {
    //   return false;
    // }
  }
}



void
PennChord::Lookup (std::string key, uint32_t transactionId, uint16_t nodeHops, std::string LookupOriginator, PennChord::LookupType type)
{
 Ipv4Address lastEntry;
 Ipv4Address successorIp = PennApplication::ResolveNodeIpAddress(m_successor);

 uint32_t hashed_key = PennKeyHelper::CreateShaKey(key);
 uint32_t thisNodeHash = PennKeyHelper::CreateShaKey(m_thisNodeIp);
 //uint32_t successorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_successor));
 uint32_t predecessorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_predecessor));
     Ipv4Address originatorIp = PennApplication::ResolveNodeIpAddress(LookupOriginator);
        std::uint32_t originatorKey = PennKeyHelper::CreateShaKey(originatorIp);
        
 bool continueON = true;

//check to see if the current node is the key storing node
if (thisNodeHash > predecessorHash){
  if (hashed_key > predecessorHash && hashed_key <= thisNodeHash){
     if (nodeHops > 0) {
          nodeHops = nodeHops -1;
        }
    m_totalHopCount = m_totalHopCount + nodeHops;
    m_lookupSuccess(m_thisNodeIp, key, transactionId,  LookupOriginator, type);
CHORD_LOG(GraderLogs::GetLookupResultLogStr(thisNodeHash, hashed_key, LookupOriginator, originatorKey));
    continueON = false;
  }
}

else{
  //(thisNodeHash <= predecessorHash)
  if ((hashed_key > predecessorHash) || (hashed_key <= thisNodeHash))
      {
        if (nodeHops > 0) {
          nodeHops = nodeHops -1;
        }
            m_totalHopCount = m_totalHopCount + nodeHops;

   m_lookupSuccess(m_thisNodeIp, key, transactionId,  LookupOriginator, type);
CHORD_LOG(GraderLogs::GetLookupResultLogStr(thisNodeHash, hashed_key, LookupOriginator, originatorKey));
    continueON = false; 
//}
  }
}

 if (continueON){   

 if (IsKeyOwnedByMySuccessor(key)) {
    //   std::cout << "\n" << "The key of: " << key << " Is OWned by mu successor which is: " << ReverseLookup(successorIp) << "\n";
     if (nodeHops > 0) {
          nodeHops = nodeHops -1;
        }
        m_totalHopCount = m_totalHopCount + nodeHops;
      m_lookupSuccess(successorIp, key, transactionId,  LookupOriginator, type);       
      CHORD_LOG(GraderLogs::GetLookupResultLogStr(thisNodeHash, hashed_key, LookupOriginator, originatorKey));
 }
 
 else
  {

  //iterate through the FingerTable entry from last to second last and find the appropriate node storing the key
   Ipv4Address closestPreceding;
   uint32_t precedingBy = 0;
  bool predecessor = false;
    for (int i = 0; i <  int(m_FingerTable.size() - 1); i++) {
    
          FingerTableEntry entry = m_FingerTable[i];
          FingerTableEntry nextEntry = m_FingerTable[i+1];
        
                
          //if (hashed_key <= PennKeyHelper::CreateShaKey(entry.nodeIP) && hashed_key > PennKeyHelper::CreateShaKey(prevEntry.nodeIP)) {
          //if (hashed_key <= entry.key && hashed_key > prevEntry.key) {


          if  (PennKeyHelper::CreateShaKey(entry.nodeIP) > PennKeyHelper::CreateShaKey(nextEntry.nodeIP)){
            if (hashed_key > PennKeyHelper::CreateShaKey(entry.nodeIP)) {
                
                if (predecessor == false ) {
                    predecessor = true;
                    precedingBy = hashed_key - PennKeyHelper::CreateShaKey(entry.nodeIP);
                    closestPreceding = entry.nodeIP;
          

                }
                else {
                  uint32_t difference = hashed_key - PennKeyHelper::CreateShaKey(entry.nodeIP);
                    if (difference <= precedingBy && difference != 0) {
                        closestPreceding = entry.nodeIP;
                            
                    }
                }
             }             
            }
            else if (PennKeyHelper::CreateShaKey(entry.nodeIP) == PennKeyHelper::CreateShaKey(nextEntry.nodeIP)){

               if (hashed_key > PennKeyHelper::CreateShaKey(entry.nodeIP)) {
                  if (predecessor == false ) {
                    predecessor = true;
                    precedingBy = hashed_key - PennKeyHelper::CreateShaKey(entry.nodeIP);
                    closestPreceding = entry.nodeIP;
                    

                }
               
                else {
                  uint32_t difference = hashed_key - PennKeyHelper::CreateShaKey(entry.nodeIP);
                    if (difference <= precedingBy && difference != 0) {
                        closestPreceding = entry.nodeIP;
                          
                    }
                }
               }
            }

          else{

            if (hashed_key > PennKeyHelper::CreateShaKey(nextEntry.nodeIP)) {

            if (predecessor == false) {
                    predecessor = true;
                    precedingBy = hashed_key - PennKeyHelper::CreateShaKey(nextEntry.nodeIP);
                    closestPreceding = nextEntry.nodeIP;
                  

                }
                else {
                  uint32_t difference = hashed_key - PennKeyHelper::CreateShaKey(nextEntry.nodeIP);


                    if (difference < precedingBy) {
                        closestPreceding = nextEntry.nodeIP;
                           
                    }
                }
            }
          }


          }
    
  if (predecessor) {
                uint32_t transId = GetNextTransactionId();
                LookupMessage lookUpMessage; 
                lookUpMessage.key = key;
                lookUpMessage.transactionId = transId;
                lookUpMessage.nodeHops = nodeHops + 1;
                lookUpMessage.lookupOriginator = LookupOriginator;
                lookUpMessage.lookupType = type;
                SendLookUpMessage(key, transactionId, nodeHops + 1, LookupOriginator, type, closestPreceding);
          GraderLogs::GetLookupForwardingLogStr(thisNodeHash , ReverseLookup(closestPreceding), PennKeyHelper::CreateShaKey(closestPreceding), hashed_key );

              //  std::cout << "\n" << "Sent lookup to: " <<  ReverseLookup(closestPreceding) << " for key: " << key <<  "\n";

        }
        else  {
        FingerTableEntry nextNode = m_FingerTable[int(m_FingerTable.size() -1)];

          uint32_t i = (m_FingerTable.size() -1);
        while (nextNode.nodeIP == m_thisNodeIp) {
            i--; 
            nextNode = m_FingerTable[i];
        }

        std::string nextNodeId = ReverseLookup(nextNode.nodeIP);

       // Ipv4Address successorIp = ResolveNodeIpAddress(m_successor);

        uint32_t transId = GetNextTransactionId();
        
        LookupMessage lookUpMessage; 
        lookUpMessage.key = key;
        lookUpMessage.transactionId = transId;
        lookUpMessage.nodeHops = nodeHops + 1;
        lookUpMessage.lookupOriginator = LookupOriginator;
        lookUpMessage.lookupType = type;
        m_LookupMessages.insert(std::make_pair(transId, lookUpMessage));

        SendLookUpMessage(key, transactionId, nodeHops + 1, LookupOriginator, type, nextNode.nodeIP);
          GraderLogs::GetLookupForwardingLogStr(thisNodeHash , nextNodeId, PennKeyHelper::CreateShaKey(nextNode.nodeIP), hashed_key );

      // std::cout << "\n" << "Not in table.. Sent lookup to: " <<  ReverseLookup(nextNode.nodeIP) << " for key: " << key <<  "\n";
    }
   //GraderLogs::GetLookupForwardingLogStr(thisNodeNum , nextNodeID, nextNodeKey, hashed_key );
  }
 }
}


// void
// PennChord::Lookup (std::string key, uint32_t transactionId, uint16_t nodeHops, std::string LookupOriginator, PennChord::LookupType type)
// {

//   if (type == PennChord::LookupType::SEARCH) {
//   std::cout << "\n" << " Lookup from search " << " for " << key <<  "\n";

//   }
//  Ipv4Address lastEntry;
//  Ipv4Address successorIp = PennApplication::ResolveNodeIpAddress(m_successor);

//  uint32_t hashed_key = PennKeyHelper::CreateShaKey(key);
//  uint32_t thisNodeHash = PennKeyHelper::CreateShaKey(m_thisNodeIp);
//  //uint32_t successorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_successor));
//  uint32_t predecessorHash = PennKeyHelper::CreateShaKey(ResolveNodeIpAddress(m_predecessor));
//      Ipv4Address originatorIp = PennApplication::ResolveNodeIpAddress(LookupOriginator);
//         std::uint32_t originatorKey = PennKeyHelper::CreateShaKey(originatorIp);
        
//  bool continueON = true;
  
// //check to see if the current node is the key storing node
// if (thisNodeHash > predecessorHash){
//   if (hashed_key > predecessorHash && hashed_key <= thisNodeHash){
//     m_totalHopCount = m_totalHopCount + nodeHops;
//     m_lookupSuccess(m_thisNodeIp, key, transactionId,  LookupOriginator, type);
// CHORD_LOG(GraderLogs::GetLookupResultLogStr(thisNodeHash, hashed_key, LookupOriginator, originatorKey));
//     continueON = false;
//       std::cout << "\n" << " Lookup from search1 " << " for " << key <<  "\n";

//   }
// }

// else{
//   //(thisNodeHash <= predecessorHash)
//   if ((hashed_key > predecessorHash) || (hashed_key <= thisNodeHash))
//       {
//             m_totalHopCount = m_totalHopCount + nodeHops;

//    m_lookupSuccess(m_thisNodeIp, key, transactionId,  LookupOriginator, type);
// CHORD_LOG(GraderLogs::GetLookupResultLogStr(thisNodeHash, hashed_key, LookupOriginator, originatorKey));
//     continueON = false; 
// //}
//   std::cout << "\n" << " Lookup from search2 " << " for " << key <<  "\n";

//   }
// }
// if (continueON) {
//  if (IsKeyOwnedByMySuccessor(key)) {
//     //   std::cout << "\n" << "The key of: " << key << " Is OWned by mu successor which is: " << ReverseLookup(successorIp) << "\n";
//           m_totalHopCount = m_totalHopCount + nodeHops;

//       m_lookupSuccess(successorIp, key, transactionId,  LookupOriginator, type);       
//       CHORD_LOG(GraderLogs::GetLookupResultLogStr(thisNodeHash, hashed_key, LookupOriginator, originatorKey));
//         std::cout << "\n" << " Lookup from search3 " << " for " << key <<  "\n";

//  }
 
//  else
//   {

//   std::cout << "\n" << " Lookup from search4 " << " for " << key <<  "\n";

//   //iterate through the FingerTable entry from last to second last and find the appropriate node storing the key
//    Ipv4Address closestPreceding;
//    int precedingBy = 0;
//   bool predecessor = false;
//     for (int i = int(m_FingerTable.size()) -1; i > 0; i-- ) {
    
//           FingerTableEntry entry = m_FingerTable[i];
//           //FingerTableEntry prevEntry = m_FingerTable[i-1];

//           uint32_t entryHash = PennKeyHelper::CreateShaKey(entry.nodeIP);
        
//           if (hashed_key > entryHash && entry.nodeIP != m_thisNodeIp) {
//               if (!predecessor) {
//                 predecessor = true;
//                 precedingBy = hashed_key - entryHash;
//                 closestPreceding = entry.nodeIP;
//               }
//               else {
//                 int difference = hashed_key - entryHash;
//                 if (difference < precedingBy) {
//                   closestPreceding = entry.nodeIP;
//                 }
//               }
//           }
//     }
//   if (predecessor) {
//                 uint32_t transId = GetNextTransactionId();
//                 LookupMessage lookUpMessage; 
//                 lookUpMessage.key = key;
//                 lookUpMessage.transactionId = transId;
//                 lookUpMessage.nodeHops = nodeHops + 1;
//                 lookUpMessage.lookupOriginator = LookupOriginator;
//                 lookUpMessage.lookupType = type;
//                   std::cout << "\n" << " Lookup from search6 " << " for " << key << " this node is: " << m_thisNode << "\n";
//                       std::cout << "\n" << "Sent lookup to: " <<  ReverseLookup(closestPreceding) << " for key: " << key <<  "\n";


//                 SendLookUpMessage(key, transactionId, nodeHops + 1, LookupOriginator, type, closestPreceding);
//           GraderLogs::GetLookupForwardingLogStr(thisNodeHash , ReverseLookup(closestPreceding), PennKeyHelper::CreateShaKey(closestPreceding), hashed_key );

//   std::cout << "\n" << " Lookup from search6 " << " for " << key <<  "\n";

//               std::cout << "\n" << "Sent lookup to: " <<  ReverseLookup(closestPreceding) << " for key: " << key <<  "\n";
//               std::cout << "\n" << "closest preceding ip: " <<  closestPreceding << " node " << ReverseLookup(closestPreceding) <<  "\n";


//         }
//         else  {



//         FingerTableEntry nextNode = m_FingerTable[int(m_FingerTable.size() -1)];

//         uint32_t i = (m_FingerTable.size() -1);
//         while (nextNode.nodeIP == m_thisNodeIp) {
//             i--; 
//             nextNode = m_FingerTable[i];

//         }




//         std::string nextNodeId = ReverseLookup(nextNode.nodeIP);

//        // Ipv4Address successorIp = ResolveNodeIpAddress(m_successor);

//         uint32_t transId = GetNextTransactionId();
        
//         LookupMessage lookUpMessage; 
//         lookUpMessage.key = key;
//         lookUpMessage.transactionId = transId;
//         lookUpMessage.nodeHops = nodeHops + 1;
//         lookUpMessage.lookupOriginator = LookupOriginator;
//         lookUpMessage.lookupType = type;
//         m_LookupMessages.insert(std::make_pair(transId, lookUpMessage));

//         SendLookUpMessage(key, transactionId, nodeHops + 1, LookupOriginator, type, nextNode.nodeIP);
//           GraderLogs::GetLookupForwardingLogStr(thisNodeHash , nextNodeId, PennKeyHelper::CreateShaKey(nextNode.nodeIP), hashed_key );

//   std::cout << "\n" << " Lookup from search7 " << " for " << key <<  "\n";

//       // std::cout << "\n" << "Not in table.. Sent lookup to: " <<  ReverseLookup(nextNode.nodeIP) << " for key: " << key <<  "\n";
//     }
//    //GraderLogs::GetLookupForwardingLogStr(thisNodeNum , nextNodeID, nextNodeKey, hashed_key );
//   }
//   }//conitueON
 
// }

void PennChord::SendGetSuccessorRequest(Ipv4Address needSuccessor, uint32_t transId) {

    Ipv4Address destAddress = needSuccessor;

    if (destAddress != Ipv4Address::GetAny ())
    {
     //CHORD_LOG ("Sending Ring State Message to Node: " << m_successor << " IP: " << destAddress << " Sending RingStateMessage to:  " << m_successor << " transactionId: " << transactionId);

      PennChordMessage message = PennChordMessage (PennChordMessage::GET_SUCCESSOR, transId);
      message.SetGetSuccessor(m_thisNode);
      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
      else
    {
      // Report failure  
     // std::cout << "\n" << "CANT GET SUCCESSOR FOR THIS NODE- BAD IPADDR" << "\n"; 
     }
}

void PennChord::ProcessGetSuccessorRequest(PennChordMessage message, Ipv4Address sourceAddress) {


      uint32_t transactionId = message.GetTransactionId();
      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage newMessage = PennChordMessage (PennChordMessage::GET_SUCCESSOR_RSP, transactionId);
      newMessage.SetGetSuccessorRsp (m_successor);
      packet->AddHeader (newMessage);
      m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, m_appPort));
}

void PennChord::ProcessGetSuccessorRsp(PennChordMessage message, Ipv4Address sourceAddress) {

    std::string successor = message.GetSuccessorResponse().successor;
    Ipv4Address successorIp = ResolveNodeIpAddress(successor);
    std::uint32_t nodeKey = PennKeyHelper::CreateShaKey(m_thisNodeIp);
    std::uint32_t nextNodeKey = PennKeyHelper::CreateShaKey(successorIp);
    uint32_t transactionId = message.GetTransactionId();

       if (m_LookupMessages.find(transactionId) != m_LookupMessages.end()) {
        
         LookupMessage lookUpMessage = m_LookupMessages[transactionId];

        std::string key = lookUpMessage.key;
        uint32_t transactionId = lookUpMessage.transactionId;
        uint32_t nodeHops = lookUpMessage.nodeHops;
        std::string lookupOriginator = lookUpMessage.lookupOriginator;
        PennChord::LookupType lookupType = lookUpMessage.lookupType;

         SendLookUpMessage(key, transactionId, nodeHops, lookupOriginator, lookupType, successorIp);

      GraderLogs::GetLookupForwardingLogStr(nodeKey, successor, nextNodeKey, PennKeyHelper::CreateShaKey(key));

    } 

    m_LookupMessages.erase(transactionId);
}



void PennChord::SendLookUpMessage(std:: string key, uint32_t transactionId, uint16_t nodeHops, std::string LookupOriginator, PennChord::LookupType type, Ipv4Address dest) {
        // std::cout << "LOOK UP FOR KEY: " << key << "\n";

        // std::cout << "FWDING LOOKUP REQUEST TO: " << ReverseLookup(dest) << "\n";

      PennChordMessage message = PennChordMessage (PennChordMessage::LOOKUP_REQ, transactionId);
      message.SetLookUpRequest (key, LookupOriginator, nodeHops, FromLookupType(type));
      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (dest, m_appPort));
}

void 
PennChord::ProcessLookupMessage(PennChordMessage message){

  PennChordMessage::LookUpRequest lookUpRequest = message.GetLookUpRequest();
  std::string key = lookUpRequest.key;
  std::string lookUpOriginator = lookUpRequest.originator;
  uint16_t type = lookUpRequest.lookUpType;
  uint16_t nodeHops = lookUpRequest.nodeHops;
    //  std::cout << "PROCESSING LOOKUP REQUEST " << "for key " << " i am node " << m_thisNode << "\n";


  PennChord::Lookup (key, message.GetTransactionId(), nodeHops, lookUpOriginator, GetLookupTypeEnum(type));
}


void PennChord::LookupSuccess() {
 // std::cout << "\n" << "LOOK UP SUCCESS" << "\n";
}

PennChord::LookupType PennChord::GetLookupTypeEnum(uint16_t type)
  {
    if (type == 0) {
        return PennChord::LookupType::PUBLISH;
      }
      else {
        return PennChord::LookupType::SEARCH;
      }
  }

  uint16_t PennChord::FromLookupType(PennChord::LookupType lookUpType)
  {
    if (lookUpType == PennChord::LookupType::PUBLISH) {
        return 0;
      }
      else {
        return 1;
      }
  }

//callback setter in chord
void 
PennChord::SetLookupSuccessCallback(Callback <void, Ipv4Address, std::string, uint32_t, std::string, PennChord::LookupType> lookupSuccess)
{

  m_lookupSuccess = lookupSuccess;
}

//callback setter in chord for leaving application
void 
PennChord::SetLeaveApplicationCallback(Callback <void, Ipv4Address> leaveApplication)
{
  m_leaveApplication = leaveApplication;
}

//callback setter in chord for rejoining application
void 
PennChord::SetRejoinApplicationCallback(Callback <void, Ipv4Address, Ipv4Address> rejoinApplication)
{
  m_rejoinApplication = rejoinApplication;
}


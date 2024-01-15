/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PennChordMessage");
NS_OBJECT_ENSURE_REGISTERED (PennChordMessage);

PennChordMessage::PennChordMessage ()
{
}

PennChordMessage::~PennChordMessage ()
{
}

PennChordMessage::PennChordMessage (PennChordMessage::MessageType messageType, uint32_t transactionId)
{
  m_messageType = messageType;
  m_transactionId = transactionId;
}

TypeId 
PennChordMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("PennChordMessage")
    .SetParent<Header> ()
    .AddConstructor<PennChordMessage> ()
  ;
  return tid;
}

TypeId
PennChordMessage::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


uint32_t
PennChordMessage::GetSerializedSize (void) const
{
  // size of messageType, transaction id
  uint32_t size = sizeof (uint8_t) + sizeof (uint32_t);
  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.GetSerializedSize ();
        break;
      case PING_RSP:
        size += m_message.pingRsp.GetSerializedSize ();
        break;
      case FIND_SUCC:
        size += m_message.findSucc.GetSerializedSize ();
        break;
      case NEW_PRED:
        size += m_message.newPred.GetSerializedSize ();
        break;
      case NEW_SUCC:
        size += m_message.newSucc.GetSerializedSize ();
        break;
      case REQ_JOIN:
        size += m_message.reqJoin.GetSerializedSize ();
        break;
      case RING_STATE:
        size += m_message.ringState.GetSerializedSize ();
        break;
      case STABILIZE_REQUEST:
        size += m_message.stabilizeRequest.GetSerializedSize ();
        break;
      case STABILIZE_ANSWER:
        size += m_message.stabilizeAnswer.GetSerializedSize ();
        break;
      case CALCULATE_FINGER_TABLE_REQ:
        size += m_message.calculateFingerTableRequest.GetSerializedSize ();
        break;
      case CALCULATE_FINGER_TABLE_ANSWER:
        size += m_message.calculateFingerTableAnswer.GetSerializedSize ();
        break;
      case LOOKUP_REQ:
        size += m_message.lookUpRequest.GetSerializedSize ();
        break;
      case GET_SUCCESSOR:
        size += m_message.getSuccessor.GetSerializedSize ();
        break;
      case GET_SUCCESSOR_RSP:
        size += m_message.getSuccessorRsp.GetSerializedSize ();
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
PennChordMessage::Print (std::ostream &os) const
{
  os << "\n****PennChordMessage Dump****\n" ;
  os << "messageType: " << m_messageType << "\n";
  os << "transactionId: " << m_transactionId << "\n";
  os << "PAYLOAD:: \n";
  
  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Print (os);
        break;
      case PING_RSP:
        m_message.pingRsp.Print (os);
        break;
      case FIND_SUCC:
        m_message.findSucc.Print (os);
        break;
      case NEW_PRED:
        m_message.newPred.Print (os);
        break;
      case NEW_SUCC:
        m_message.newSucc.Print (os);
        break;
      case REQ_JOIN:
        m_message.reqJoin.Print (os);
        break;
      case RING_STATE:
        m_message.ringState.Print (os);
        break;
      case STABILIZE_REQUEST:
        m_message.stabilizeRequest.Print (os);
        break;
      case STABILIZE_ANSWER:
        m_message.stabilizeAnswer.Print (os);
        break;
      case CALCULATE_FINGER_TABLE_REQ:
        m_message.calculateFingerTableRequest.Print(os);
        break;
      case CALCULATE_FINGER_TABLE_ANSWER:
        m_message.calculateFingerTableAnswer.Print(os);
        break;
      case LOOKUP_REQ:
        m_message.lookUpRequest.Print(os);
        break;
      case GET_SUCCESSOR:
        m_message.getSuccessor.Print(os);
        break;
      case GET_SUCCESSOR_RSP:
        m_message.getSuccessorRsp.Print(os);
        break;
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
PennChordMessage::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteHtonU32 (m_transactionId);

  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Serialize (i);
        break;
      case PING_RSP:
        m_message.pingRsp.Serialize (i);
        break;
      case FIND_SUCC:
        m_message.findSucc.Serialize (i);
        break;
      case NEW_PRED:
        m_message.newPred.Serialize (i);
        break;
      case NEW_SUCC:
        m_message.newSucc.Serialize (i);
        break;
      case REQ_JOIN:
        m_message.reqJoin.Serialize (i);
        break;
      case RING_STATE:
        m_message.ringState.Serialize (i);
        break;
      case STABILIZE_REQUEST:
        m_message.stabilizeRequest.Serialize (i);
        break;
      case STABILIZE_ANSWER:
        m_message.stabilizeAnswer.Serialize (i);
        break;
      case CALCULATE_FINGER_TABLE_REQ:
        m_message.calculateFingerTableRequest.Serialize(i);
        break;
      case CALCULATE_FINGER_TABLE_ANSWER:
        m_message.calculateFingerTableAnswer.Serialize(i);
        break;
      case LOOKUP_REQ:
        m_message.lookUpRequest.Serialize(i);
        break;
      case GET_SUCCESSOR:
        m_message.getSuccessor.Serialize(i);
        break;
      case GET_SUCCESSOR_RSP:
        m_message.getSuccessorRsp.Serialize(i);
        break;
      default:
      std::cout << "\n" << "MESSAGE ERROR TYPE: " << m_messageType << "\n";
        NS_ASSERT (false);   
    }
}

uint32_t 
PennChordMessage::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType = (MessageType) i.ReadU8 ();
  m_transactionId = i.ReadNtohU32 ();

  size = sizeof (uint8_t) + sizeof (uint32_t);

  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.Deserialize (i);
        break;
      case PING_RSP:
        size += m_message.pingRsp.Deserialize (i);
        break;
      case FIND_SUCC:
        m_message.findSucc.Deserialize (i);
        break;
      case NEW_PRED:
        m_message.newPred.Deserialize (i);
        break;
      case NEW_SUCC:
        m_message.newSucc.Deserialize (i);
        break;
      case REQ_JOIN:
        m_message.reqJoin.Deserialize (i);
        break;
      case RING_STATE:
        m_message.ringState.Deserialize (i);
        break;
      case STABILIZE_REQUEST:
        m_message.stabilizeRequest.Deserialize (i);
        break;
      case STABILIZE_ANSWER:
        m_message.stabilizeAnswer.Deserialize (i);
        break;
      case CALCULATE_FINGER_TABLE_REQ:
        m_message.calculateFingerTableRequest.Deserialize(i);
        break;
      case CALCULATE_FINGER_TABLE_ANSWER:
        m_message.calculateFingerTableAnswer.Deserialize(i);
        break;
      case LOOKUP_REQ:
        m_message.lookUpRequest.Deserialize(i);
        break;
      case GET_SUCCESSOR:
        m_message.getSuccessor.Deserialize(i);
        break;
      case GET_SUCCESSOR_RSP:
        m_message.getSuccessorRsp.Deserialize(i);
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

/* PING_REQ */

uint32_t 
PennChordMessage::PingReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
PennChordMessage::PingReq::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
PennChordMessage::PingReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
PennChordMessage::PingReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingReq::GetSerializedSize ();
}

void
PennChordMessage::SetPingReq (std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_REQ);
    }
  m_message.pingReq.pingMessage = pingMessage;
}

PennChordMessage::PingReq
PennChordMessage::GetPingReq ()
{
  return m_message.pingReq;
}

/* PING_RSP */

uint32_t 
PennChordMessage::PingRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
PennChordMessage::PingRsp::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
PennChordMessage::PingRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
PennChordMessage::PingRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingRsp::GetSerializedSize ();
}

void
PennChordMessage::SetPingRsp (std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_RSP);
    }
  m_message.pingRsp.pingMessage = pingMessage;
}

PennChordMessage::PingRsp
PennChordMessage::GetPingRsp ()
{
  return m_message.pingRsp;
}

/* END PING_RSP */


/* Find_Succ */

uint32_t 
PennChordMessage::FindSucc::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + nodeToJoin.length();
  return size;
}

void
PennChordMessage::FindSucc::Print (std::ostream &os) const
{
  os << "NewNodeMessage:: Message: " << nodeToJoin << "\n";
}

void
PennChordMessage::FindSucc::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (nodeToJoin.length ());
  start.Write ((uint8_t *) (const_cast<char*> (nodeToJoin.c_str())), nodeToJoin.length());
}

uint32_t
PennChordMessage::FindSucc::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  nodeToJoin = std::string (str, length);
  free (str);
  return FindSucc::GetSerializedSize ();
}

void
PennChordMessage::SetFindSucc (std::string nodeToJoin)
{
  if (m_messageType == 3)
    {
      m_messageType = FIND_SUCC;
    }
  else
    {
      NS_ASSERT (m_messageType == FIND_SUCC);
    }
  m_message.findSucc.nodeToJoin = nodeToJoin;
}

PennChordMessage::FindSucc
PennChordMessage::GetFindSucc ()
{
  return m_message.findSucc;
}

/* END Find_Succ */

/* New_Pred */

uint32_t 
PennChordMessage::NewPred::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + newPredMessage.length() + sizeof(uint16_t);
  return size;
}

void
PennChordMessage::NewPred::Print (std::ostream &os) const
{
  os << "NewPredMessage:: Message: " << newPredMessage << "\n";
}

void
PennChordMessage::NewPred::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (newPredMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (newPredMessage.c_str())), newPredMessage.length());
  start.WriteU16(leaveBoolean);
}

uint32_t
PennChordMessage::NewPred::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  newPredMessage = std::string (str, length);
  free (str);
  leaveBoolean = start.ReadU16();
  return NewPred::GetSerializedSize ();
}

void
PennChordMessage::SetNewPred (std::string newPredMessage, uint16_t leaveBoolean)
{
  if (m_messageType == 4)
    {
      m_messageType = NEW_PRED;
    }
  else
    {
      NS_ASSERT (m_messageType == NEW_PRED);
    }
  m_message.newPred.newPredMessage = newPredMessage;
  m_message.newPred.leaveBoolean = leaveBoolean;
}

PennChordMessage::NewPred
PennChordMessage::GetNewPred ()
{
  return m_message.newPred;
}

/* END New_Pred */


/* New_Succ */

uint32_t 
PennChordMessage::NewSucc::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + newSuccMessage.length();
  return size;
}

void
PennChordMessage::NewSucc::Print (std::ostream &os) const
{
  os << "NewSuccMessage:: Message: " << newSuccMessage << "\n";
}

void
PennChordMessage::NewSucc::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (newSuccMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (newSuccMessage.c_str())), newSuccMessage.length());
}

uint32_t
PennChordMessage::NewSucc::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  newSuccMessage = std::string (str, length);
  free (str);
  return NewSucc::GetSerializedSize ();
}

void
PennChordMessage::SetNewSucc (std::string newSuccMessage)
{
  if (m_messageType == 5)
    {
      m_messageType = NEW_SUCC;
    }
  else
    {
      NS_ASSERT (m_messageType == NEW_SUCC);
    }
  m_message.newSucc.newSuccMessage = newSuccMessage;
}

PennChordMessage::NewSucc
PennChordMessage::GetNewSucc ()
{
  return m_message.newSucc;
}

/* END New_Succ */

/* Req_Join */

uint32_t 
PennChordMessage::ReqJoin::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + newReqJoinMessage.length();
  return size;
}

void
PennChordMessage::ReqJoin::Print (std::ostream &os) const
{
  os << "NewRequstToJoin:: Message: " << newReqJoinMessage << "\n";
}

void
PennChordMessage::ReqJoin::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (newReqJoinMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (newReqJoinMessage.c_str())), newReqJoinMessage.length());
}

uint32_t
PennChordMessage::ReqJoin::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  newReqJoinMessage = std::string (str, length);
  free (str);
  return ReqJoin::GetSerializedSize ();
}

void
PennChordMessage::SetReqJoin (std::string newReqJoinMessage)
{
  if (m_messageType == 6)
    {
      m_messageType = REQ_JOIN;
    }
  else
    {
      NS_ASSERT (m_messageType == REQ_JOIN);
    }
  m_message.reqJoin.newReqJoinMessage = newReqJoinMessage;
}

PennChordMessage::ReqJoin
PennChordMessage::GetReqJoin ()
{
  return m_message.reqJoin;
}

/* END req_join */

/* Ring_state */

uint32_t 
PennChordMessage::RingState::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + originator.length();
  return size;
}

void
PennChordMessage::RingState::Print (std::ostream &os) const
{
  os << "NewRequstToJoin:: Message: " << originator << "\n";
}

void
PennChordMessage::RingState::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (originator.length ());
  start.Write ((uint8_t *) (const_cast<char*> (originator.c_str())), originator.length());
}

uint32_t
PennChordMessage::RingState::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  originator = std::string (str, length);
  free (str);
  return RingState::GetSerializedSize ();
}

void
PennChordMessage::SetRingState (std::string originator)
{
  if (m_messageType == 7)
    {
      m_messageType = RING_STATE;
    }
  else
    {
      NS_ASSERT (m_messageType == RING_STATE);
    }
  m_message.ringState.originator = originator;
}

PennChordMessage::RingState
PennChordMessage::GetRingState ()
{
  return m_message.ringState;
}

/* END New_Succ */

/* Stabilize_Request */

uint32_t 
PennChordMessage::StabilizeRequest::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + message.length();
  return size;
}

void
PennChordMessage::StabilizeRequest::Print (std::ostream &os) const
{
  os << "NewRequstToJoin:: Message: " << message << "\n";
}

void
PennChordMessage::StabilizeRequest::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (message.length ());
  start.Write ((uint8_t *) (const_cast<char*> (message.c_str())), message.length());
}

uint32_t
PennChordMessage::StabilizeRequest::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  message = std::string (str, length);
  free (str);
  return StabilizeRequest::GetSerializedSize ();
}

void
PennChordMessage::SetStabilizeRequest (std::string message)
{
  if (m_messageType == 8)
    {
      m_messageType = STABILIZE_REQUEST;
    }
  else
    {
      NS_ASSERT (m_messageType == STABILIZE_REQUEST);
    }
  m_message.stabilizeRequest.message = message;
}

PennChordMessage::StabilizeRequest
PennChordMessage::GetStabilizeRequest ()
{
  return m_message.stabilizeRequest;
}

/* END Stabilize_Request */


/* Stabilize_Answer */

uint32_t 
PennChordMessage::StabilizeAnswer::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + predecessor.length();
  return size;
}

void
PennChordMessage::StabilizeAnswer::Print (std::ostream &os) const
{
  os << "NewRequstToJoin:: Message: " << predecessor << "\n";
}

void
PennChordMessage::StabilizeAnswer::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (predecessor.length ());
  start.Write ((uint8_t *) (const_cast<char*> (predecessor.c_str())), predecessor.length());
}

uint32_t
PennChordMessage::StabilizeAnswer::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  predecessor = std::string (str, length);
  free (str);
  return StabilizeAnswer::GetSerializedSize ();
}

void
PennChordMessage::SetStabilizeAnswer (std::string predecessor)
{
  if (m_messageType == 9)
    {
      m_messageType = STABILIZE_ANSWER;
    }
  else
    {
      NS_ASSERT (m_messageType == STABILIZE_ANSWER);
    }
  m_message.stabilizeAnswer.predecessor = predecessor;
}

PennChordMessage::StabilizeAnswer
PennChordMessage::GetStabilizeAnswer ()
{
  return m_message.stabilizeAnswer;
}

/* END Stabilize_Request */

/* CalculateFingerTableRequest */

uint32_t 
PennChordMessage::CalculateFingerTableRequest::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint32_t);
  size += sizeof(uint16_t) + originator.length();
  size += sizeof(uint32_t);

  return size;
}

void
PennChordMessage::CalculateFingerTableRequest::Print (std::ostream &os) const
{
  os << "CalculateFingerTableRequest:: Key: " << key << "\n";
}

void
PennChordMessage::CalculateFingerTableRequest::Serialize (Buffer::Iterator &start) const
{
  start.WriteU32 (key);
  start.WriteU16 (originator.length ());
  start.Write ((uint8_t *) (const_cast<char*> (originator.c_str())), originator.length());
  start.WriteU32 (index);

}

uint32_t
PennChordMessage::CalculateFingerTableRequest::Deserialize (Buffer::Iterator &start)
{  
  uint32_t length = start.ReadU32 ();
  key = length;

  uint16_t lengthTwo = start.ReadU16 ();
  char* strTwo = (char*) malloc (length);
  start.Read ((uint8_t*)strTwo, lengthTwo);
  originator = std::string (strTwo, lengthTwo);
  free (strTwo);
  
  uint32_t lengthThree = start.ReadU32 ();

  index = lengthThree;


  return CalculateFingerTableRequest::GetSerializedSize ();
}

void
PennChordMessage::SetCalculateFingerTableRequest (uint32_t key, std::string originator, uint32_t index )
{
  if (m_messageType == 12)
    {
      m_messageType = CALCULATE_FINGER_TABLE_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == CALCULATE_FINGER_TABLE_REQ);
    }
  m_message.calculateFingerTableRequest.key = key;
  m_message.calculateFingerTableRequest.originator = originator;
    m_message.calculateFingerTableRequest.index = index;


}

PennChordMessage::CalculateFingerTableRequest
PennChordMessage::GetCalculateFingerTableRequest ()
{
  return m_message.calculateFingerTableRequest;
}

/* END CalculateFingerTableRequest */

/* CalculateFingerTableAnswer */

uint32_t 
PennChordMessage::CalculateFingerTableAnswer::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + successorForKey.length();
  size += sizeof(uint32_t);
    size += sizeof(uint32_t);
  return size;
}

void
PennChordMessage::CalculateFingerTableAnswer::Print (std::ostream &os) const
{
  os << "CalculateFingerTableAnswer:: Successor for the key: " << successorForKey << "\n";
}

void
PennChordMessage::CalculateFingerTableAnswer::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (successorForKey.length ());
  start.Write ((uint8_t *) (const_cast<char*> (successorForKey.c_str())), successorForKey.length());
  start.WriteU32 (key);
    start.WriteU32 (index);
}

uint32_t
PennChordMessage::CalculateFingerTableAnswer::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  successorForKey = std::string (str, length);
  free (str);
  uint32_t lengthTwo = start.ReadU32 ();
  key = lengthTwo;
    uint32_t lengthThree = start.ReadU32 ();
  index = lengthThree;
  return CalculateFingerTableAnswer::GetSerializedSize ();
}

void
PennChordMessage::SetCalculateFingerTableAnswer (std::string successorForKey, uint32_t key, uint32_t index)
{
  if (m_messageType == 13)
    {
      m_messageType = CALCULATE_FINGER_TABLE_ANSWER;
    }
  else
    {
      NS_ASSERT (m_messageType == CALCULATE_FINGER_TABLE_ANSWER);
    }
  m_message.calculateFingerTableAnswer.successorForKey = successorForKey;
  m_message.calculateFingerTableAnswer.key = key;
    m_message.calculateFingerTableAnswer.index = index;

}

PennChordMessage::CalculateFingerTableAnswer
PennChordMessage::GetCalculateFingerTableAnswer ()
{
  return m_message.calculateFingerTableAnswer;
}

/* END CalculateFingerTableAnswer */

/* LookUpRequest */

uint32_t 
PennChordMessage::LookUpRequest::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + key.length();
  size += sizeof(uint16_t) + originator.length();
  size += sizeof(uint16_t);
  size += sizeof(uint16_t);
  return size;
}

void
PennChordMessage::LookUpRequest::Print (std::ostream &os) const
{
  os << "LookUpRequest:: Originaitor: " << originator << "\n";
  os << "LookUpRequest:: Key: " << key << "\n";

}

void
PennChordMessage::LookUpRequest::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (key.length ());
  start.Write ((uint8_t *) (const_cast<char*> (key.c_str())), key.length());
  start.WriteU16 (originator.length ());
  start.Write ((uint8_t *) (const_cast<char*> (originator.c_str())), originator.length());
  start.WriteU16 (lookUpType);
  start.WriteU16(nodeHops);

}

uint32_t
PennChordMessage::LookUpRequest::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  key = std::string (str, length);
  free (str);

  uint16_t lengthTwo = start.ReadU16 ();
  char* strTwo = (char*) malloc (lengthTwo);
  start.Read ((uint8_t*)strTwo, lengthTwo);
  originator = std::string (strTwo, lengthTwo);
  free (strTwo);

  lookUpType = start.ReadU16 ();
  nodeHops = start.ReadU16();

  return LookUpRequest::GetSerializedSize ();
}

void
PennChordMessage::SetLookUpRequest (std::string key, std::string originator, uint16_t nodeHops, uint16_t lookUpType)
{
  if (m_messageType == 10)
    {
      m_messageType = LOOKUP_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == LOOKUP_REQ);
    }
  m_message.lookUpRequest.key = key;
  m_message.lookUpRequest.originator = originator;
  m_message.lookUpRequest.lookUpType = lookUpType;
  m_message.lookUpRequest.nodeHops = nodeHops;
}

PennChordMessage::LookUpRequest
PennChordMessage::GetLookUpRequest ()
{
  return m_message.lookUpRequest;
}

/* END LookUpRequest */

/* GetSuccessor */

uint32_t 
PennChordMessage::GetSuccessor::GetSerializedSize (void) const
{

  uint32_t size;
  size = sizeof(uint16_t) + originator.length();

  return size;
}

void
PennChordMessage::GetSuccessor::Print (std::ostream &os) const
{
  os << "GetSuccessor:: Originaitor: " << originator << "\n";

}

void
PennChordMessage::GetSuccessor::Serialize (Buffer::Iterator &start) const
{

    start.WriteU16 (originator.length ());
  start.Write ((uint8_t *) (const_cast<char*> (originator.c_str())), originator.length());


}

uint32_t
PennChordMessage::GetSuccessor::Deserialize (Buffer::Iterator &start)
{  
    uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  originator = std::string (str, length);
  free (str);

  return GetSuccessor::GetSerializedSize ();

}

PennChordMessage::GetSuccessor
PennChordMessage::GetSuccessorRequest ()
{
  return m_message.getSuccessor;
}

void
PennChordMessage::SetGetSuccessor (std::string originator)
{
  if (m_messageType == 14)
    {
      m_messageType = GET_SUCCESSOR;
    }
  else
    {
      NS_ASSERT (m_messageType == GET_SUCCESSOR);
    }
  m_message.getSuccessor.originator = originator;
}

/* END CalculateFingerTableAnswer */



/* Get_Succ_Rsp */

uint32_t 
PennChordMessage::GetSuccessorRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + successor.length();
  return size;
}

void
PennChordMessage::GetSuccessorRsp::Print (std::ostream &os) const
{
  os << "GetSuccessorRSp:: Message: " << successor << "\n";
}

void
PennChordMessage::GetSuccessorRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (successor.length ());
  start.Write ((uint8_t *) (const_cast<char*> (successor.c_str())), successor.length());

}

uint32_t
PennChordMessage::GetSuccessorRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  successor = std::string (str, length);
  free (str);
  return GetSuccessorRsp::GetSerializedSize ();
}

void
PennChordMessage::SetGetSuccessorRsp (std::string successor)
{
  if (m_messageType == 15)
    {
      m_messageType = GET_SUCCESSOR_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == GET_SUCCESSOR_RSP);
    }
  m_message.getSuccessorRsp.successor = successor;
}

PennChordMessage::GetSuccessorRsp
PennChordMessage::GetSuccessorResponse()
{
  return m_message.getSuccessorRsp;
}

/* END Find_Succ */



void
PennChordMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

PennChordMessage::MessageType
PennChordMessage::GetMessageType () const
{
  return m_messageType;
}

void
PennChordMessage::SetTransactionId (uint32_t transactionId)
{
  m_transactionId = transactionId;
}

uint32_t 
PennChordMessage::GetTransactionId (void) const
{
  return m_transactionId;
}


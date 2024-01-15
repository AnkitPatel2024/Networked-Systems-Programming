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

#ifndef PENN_CHORD_MESSAGE_H
#define PENN_CHORD_MESSAGE_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include <vector>

using namespace ns3;

#define IPV4_ADDRESS_SIZE 4

class PennChordMessage : public Header
{
  public:
    PennChordMessage ();
    virtual ~PennChordMessage ();

    enum MessageType
    {
      PING_REQ = 1,
      PING_RSP = 2,
      FIND_SUCC = 3,
      NEW_PRED = 4,
      NEW_SUCC= 5,
      REQ_JOIN = 6,
      RING_STATE = 7,
      STABILIZE_REQUEST = 8,
      STABILIZE_ANSWER = 9,
      LOOKUP_REQ = 10,
      LOOKUP_RSP = 11,
      CALCULATE_FINGER_TABLE_REQ = 12,
      CALCULATE_FINGER_TABLE_ANSWER = 13,
      GET_SUCCESSOR = 14,
      GET_SUCCESSOR_RSP = 15
      // Define extra message types when needed
    };

    PennChordMessage (PennChordMessage::MessageType messageType, uint32_t transactionId);

    /**
    *  \brief Sets message type
    *  \param messageType message type
    */
    void SetMessageType (MessageType messageType);

    /**
     *  \returns message type
     */
    MessageType GetMessageType () const;

    /**
     *  \brief Sets Transaction Id
     *  \param transactionId Transaction Id of the request
     */
    void SetTransactionId (uint32_t transactionId);

    /**
     *  \returns Transaction Id
     */
    uint32_t GetTransactionId () const;

  private:
    /**
     *  \cond
     */
    MessageType m_messageType;
    uint32_t m_transactionId;
    /**
     *  \endcond
     */
  public:
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);
    
    struct PingReq
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string pingMessage;
      };

    struct PingRsp
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string pingMessage;
      };

    struct FindSucc
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string nodeToJoin;
      };

    struct NewPred
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string newPredMessage;
        uint16_t leaveBoolean;
      };

    struct NewSucc
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string newSuccMessage;
      };

    struct ReqJoin
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string newReqJoinMessage;
      };
    struct RingState
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string originator;
      };

    struct StabilizeRequest
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string message;
      };

    struct StabilizeAnswer
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string predecessor;
      };

    struct CalculateFingerTableRequest
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        uint32_t key;
        std::string originator;
        uint32_t index;
      };

    struct CalculateFingerTableAnswer
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string successorForKey;
        uint32_t key;
        uint32_t index;

      };

      struct LookUpRequest
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string key;
        std::string originator;
        uint16_t lookUpType;
        uint16_t nodeHops;
      };
      struct GetSuccessor
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        std::string originator;

      };
      struct GetSuccessorRsp 
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string successor;
      };

  private:
    struct
      {
        PingReq pingReq;
        PingRsp pingRsp;
        FindSucc findSucc;
        NewPred newPred;
        NewSucc newSucc;
        ReqJoin reqJoin;
        RingState ringState;
        StabilizeRequest stabilizeRequest;
        StabilizeAnswer stabilizeAnswer;
        CalculateFingerTableRequest calculateFingerTableRequest;
        CalculateFingerTableAnswer calculateFingerTableAnswer;
        LookUpRequest lookUpRequest;
        GetSuccessor getSuccessor;
        GetSuccessorRsp getSuccessorRsp;
      } m_message;
  public:
    /**
     *  \returns PingReq Struct
     */
    PingReq GetPingReq ();

    /**
     *  \brief Sets PingReq message params
     *  \param message Payload String
     */

    void SetPingReq (std::string message);

    /**
     * \returns PingRsp Struct
     */
    PingRsp GetPingRsp ();
    /**
     *  \brief Sets PingRsp message params
     *  \param message Payload String
     */
    void SetPingRsp (std::string message);

    /**
     * \returns FindSucc Struc
     */
    FindSucc GetFindSucc ();
    /**
     *  \brief Sets FindSucc message params
     *  \param message Payload String
     */
    void SetFindSucc (std::string message);

        /**
     * \returns GetNewPred Struc
     */
    NewPred GetNewPred ();
    /**
     *  \brief Sets NewPred message params
     *  \param message Payload String
     */
    void SetNewPred (std::string message, uint16_t leaveBoolean);

        /**
     * \returns NewSucc Struc
     */
    NewSucc GetNewSucc ();
    /**
     *  \brief Sets NewSucc message params
     *  \param message Payload String
     */
    void SetNewSucc (std::string message);

    /**
     * \returns ReqJoin Struc
     */
    ReqJoin GetReqJoin ();
    /**
     *  \brief Sets ReqJoin message params
     *  \param message Payload String
     */
    void SetReqJoin (std::string message);

      /**
     * \returns RingState Struc
     */
    RingState GetRingState ();
    /**
     *  \brief Sets RingState message params
     *  \param message Payload String
     */
    void SetRingState (std::string message);

          /**
     * \returns RingState Struc
     */
    StabilizeRequest GetStabilizeRequest ();
    /**
     *  \brief Sets RingState message params
     *  \param message Payload String
     */
    void SetStabilizeRequest (std::string message);

          /**
     * \returns RingState Struc
     */
    StabilizeAnswer GetStabilizeAnswer ();
    /**
     *  \brief Sets RingState message params
     *  \param message Payload String
     */
    void SetStabilizeAnswer (std::string predecessor);

    /**
     * \returns CalculateFingerTableRequest Struc
     */
    CalculateFingerTableRequest GetCalculateFingerTableRequest ();
    /**
     *  \brief Sets CalculateFingerTableRequest message params
     *  \param message Payload String
     */
    void SetCalculateFingerTableRequest (uint32_t key, std::string originator, uint32_t index);


    /**
     * \returns CalculateFingerTableAnswer Struc
     */
    CalculateFingerTableAnswer GetCalculateFingerTableAnswer ();
    /**
     *  \brief Sets CalculateFingerTableAnswer message params
     *  \param message Payload String
     */
    void SetCalculateFingerTableAnswer (std::string successorForKey, uint32_t key, uint32_t index);

        /**
     * \returns LookUpRequest Struc
     */
    LookUpRequest GetLookUpRequest ();
    /**
     *  \brief Sets CalculateFingerTableAnswer message params
     *  \param message Payload String
     */
    void SetLookUpRequest (std::string key, std::string originator, uint16_t nodeHops, uint16_t lookUpType);


      /**
     * \returns GetSuccessor
     */
    GetSuccessor GetSuccessorRequest ();
    /**
     *  \brief Sets CalculateFingerTableAnswer message params
     *  \param message Payload String
     */
    void SetGetSuccessor (std::string originator);

          /**
     * \returns GetSuccessorRsp
     */
    GetSuccessorRsp GetSuccessorResponse ();
    /**
     *  \brief Sets GetSuccessorRsp
     *  \param message Payload String
     */
    void SetGetSuccessorRsp (std::string successor);


}; // class PennChordMessage

static inline std::ostream& operator<< (std::ostream& os, const PennChordMessage& message)
{
  message.Print (os);
  return os;
};

#endif

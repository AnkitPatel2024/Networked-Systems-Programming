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

#ifndef PENN_SEARCH_MESSAGE_H
#define PENN_SEARCH_MESSAGE_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/object.h"

using namespace ns3;

#define IPV4_ADDRESS_SIZE 4

class PennSearchMessage : public Header
{
  public:
    PennSearchMessage ();
    virtual ~PennSearchMessage ();


    enum MessageType
      {
        PING_REQ = 1,
        PING_RSP = 2,
        SEARCH_REQ_LOOKUP = 3,
        SEARCH_LOOKUP_RSP = 4,
        SEARCH_REQ = 5,
        SEARCH_RSP = 6,
        PUBLISH_REQ = 7,
        INVERTED_LIST= 8,
        KEY_TRANSFER_REJOIN = 9,
        // Define extra message types when needed       
      };

    PennSearchMessage (PennSearchMessage::MessageType messageType, uint32_t transactionId);

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

//***************MS-2 structs**********************
     struct SearchReqLookup
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        Ipv4Address RequesterNode;
        std::vector<std::string> terms;
       // std::vector<std::string> movies;
       uint32_t termsCt;
      };

    struct SearchLookupRsp
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        Ipv4Address KeyStoringNode;
        std::string key;
      };
    
     struct SearchReq
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        Ipv4Address RequesterNode;
        std::vector<std::string> terms;
        std::vector<std::string> movies;
        uint32_t termsCt;
      };

    struct SearchRsp
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        Ipv4Address RequesterNode;
        std::vector<std::string> movies;
         uint32_t termsCt;
      };

    struct PublishReq
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string actor;
        std::string movie;
      };
    struct InvertedList
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::map<std::string, std::vector<std::string>> invertedList;
      };
    struct KeyTransferRejoin
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        Ipv4Address newNode;
      };

  private:
    struct
      {
        PingReq pingReq;
        PingRsp pingRsp;
        SearchReqLookup searchReqLookup;
        SearchLookupRsp searchLookupRsp;
        SearchReq searchReq;
        SearchRsp searchRsp;
        PublishReq publishReq;
        InvertedList invertedList;
        KeyTransferRejoin keyTransferRejoin;
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

//******************MS-2 methods*********************
//***************Publish methods*********************
    /**
     *  \returns PublsihReq Struct
     */
    PublishReq GetPublishReq ();

    /**
     *  \brief Sets PingReq message params
     *  \param message Payload String
     */

    void SetPublishReq (std::string actor, std::string movie);

    /**
     * \returns PingRsp Struct
     */

//****************Search methods*********************
    SearchReqLookup GetSearchReqLookup();
    Ipv4Address GetRequesterNode ();
    std::vector<std::string> GetTerms ();
    uint32_t GetTermsCount();
    void SetSearchReqLookup (Ipv4Address ReqNode, std::vector<std::string> actors , uint32_t termsCount);

    SearchLookupRsp GetSearchLookupRsp();
    Ipv4Address GetKeyStoringNode();
    void SetSearchLookupRsp (Ipv4Address keyNode, std::string k);
    std::string GetKey();

    SearchReq GetSearchReq();
    void SetSearchReq (Ipv4Address ReqNode, std::vector<std::string> actors, std::vector<std::string> docs, uint32_t termsCount );
    std::vector<std::string> GetMovies ();
    //uint32_t GetLookupHopCount ();
   
    SearchRsp GetSearchRsp();
    void SetSearchRsp (Ipv4Address ReqNode,  std::vector<std::string> docs, uint32_t termsCount );

    InvertedList GetInvertedList();
    void SetInvertedList (std::map<std::string, std::vector<std::string>> invertedList);

    KeyTransferRejoin GetKeyTransferRejoin();
    Ipv4Address GetNewNode();
    void SetKeyTransferRejoin (Ipv4Address newNode);

}; // class PennSearchMessage

static inline std::ostream& operator<< (std::ostream& os, const PennSearchMessage& message)
{
  message.Print (os);
  return os;
}

#endif
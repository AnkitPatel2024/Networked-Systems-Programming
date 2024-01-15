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

#include "penn-search-message.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PennSearchMessage");
NS_OBJECT_ENSURE_REGISTERED (PennSearchMessage);

PennSearchMessage::PennSearchMessage ()
{
}

PennSearchMessage::~PennSearchMessage ()
{
}

PennSearchMessage::PennSearchMessage (PennSearchMessage::MessageType messageType, uint32_t transactionId)
{
  m_messageType = messageType;
  m_transactionId = transactionId;
}

TypeId 
PennSearchMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("PennSearchMessage")
    .SetParent<Header> ()
    .AddConstructor<PennSearchMessage> ()
  ;
  return tid;
}

TypeId
PennSearchMessage::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


uint32_t
PennSearchMessage::GetSerializedSize (void) const
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
      case SEARCH_REQ_LOOKUP:
        size += m_message.searchReqLookup.GetSerializedSize();
        break;
      case SEARCH_LOOKUP_RSP:
        size += m_message.searchLookupRsp.GetSerializedSize();
        break;  
      case SEARCH_REQ:
        size += m_message.searchReq.GetSerializedSize();
        break;
      case SEARCH_RSP:
        size += m_message.searchRsp.GetSerializedSize();
        break;
      case PUBLISH_REQ:
        size += m_message.publishReq.GetSerializedSize();
        break;
      case INVERTED_LIST:
      size+= m_message.invertedList.GetSerializedSize();
      break;
      case KEY_TRANSFER_REJOIN:
      size+= m_message.keyTransferRejoin.GetSerializedSize();
      break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
PennSearchMessage::Print (std::ostream &os) const
{
  os << "\n****PennSearchMessage Dump****\n" ;
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
      case SEARCH_REQ_LOOKUP:
       m_message.searchReqLookup.Print (os);
       break;
      case SEARCH_LOOKUP_RSP:
       m_message.searchLookupRsp.Print (os);
       break;
      case SEARCH_REQ:
       m_message.searchReq.Print (os);
       break; 
      case SEARCH_RSP:
       m_message.searchRsp.Print (os);
       break; 
      case PUBLISH_REQ:
      m_message.publishReq.Print (os);
      break;
      case INVERTED_LIST:
      m_message.invertedList.Print (os);
      break;
      case KEY_TRANSFER_REJOIN:
      m_message.keyTransferRejoin.Print (os);
      break;
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
PennSearchMessage::Serialize (Buffer::Iterator start) const
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
      case SEARCH_REQ_LOOKUP:
       m_message.searchReqLookup.Serialize (i);
       break;
      case SEARCH_LOOKUP_RSP:
       m_message.searchLookupRsp.Serialize (i);
       break;
      case SEARCH_REQ:
       m_message.searchReq.Serialize (i);
       break; 
      case SEARCH_RSP:
       m_message.searchRsp.Serialize (i);
       break; 
      case PUBLISH_REQ:
       m_message.publishReq.Serialize (i) ;
       break;
      case INVERTED_LIST:
       m_message.invertedList.Serialize (i) ;
       break;
      case KEY_TRANSFER_REJOIN:
       m_message.keyTransferRejoin.Serialize (i) ;
       break;
      default:
        NS_ASSERT (false);   
    }
}

uint32_t 
PennSearchMessage::Deserialize (Buffer::Iterator start)
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
      case SEARCH_REQ_LOOKUP:
        size += m_message.searchReqLookup.Deserialize (i);
        break;
      case SEARCH_LOOKUP_RSP:
        size += m_message.searchLookupRsp.Deserialize (i);
       break;
      case SEARCH_REQ:
       size += m_message.searchReq.Deserialize (i);
       break; 
      case SEARCH_RSP:
       size += m_message.searchRsp.Deserialize (i);
       break; 
      case PUBLISH_REQ:
       size += m_message.publishReq.Deserialize (i);
       break;
      case INVERTED_LIST:
       size += m_message.invertedList.Deserialize (i);
       break;
      case KEY_TRANSFER_REJOIN:
       size += m_message.keyTransferRejoin.Deserialize (i);
       break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

/* PING_REQ */

uint32_t 
PennSearchMessage::PingReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
PennSearchMessage::PingReq::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
PennSearchMessage::PingReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
PennSearchMessage::PingReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingReq::GetSerializedSize ();
}

void
PennSearchMessage::SetPingReq (std::string pingMessage)
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

PennSearchMessage::PingReq
PennSearchMessage::GetPingReq ()
{
  return m_message.pingReq;
}

/* PING_RSP */

uint32_t 
PennSearchMessage::PingRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
PennSearchMessage::PingRsp::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
PennSearchMessage::PingRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())),
   pingMessage.length());
}

uint32_t
PennSearchMessage::PingRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingRsp::GetSerializedSize ();
}

void
PennSearchMessage::SetPingRsp (std::string pingMessage)
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

PennSearchMessage::PingRsp
PennSearchMessage::GetPingRsp ()
{
  return m_message.pingRsp;
}

//*****************MS-2**************************
//* SEARCH_REQ_LOOKUP */

uint32_t 
PennSearchMessage::SearchReqLookup::GetSerializedSize (void) const
{
  uint32_t size;
  // size of terms
  uint32_t vec_size = sizeof(uint16_t);
 
  for (unsigned i =0; i< terms.size(); i++){
    //vec_size = vec_size + sizeof(uint8_t)*(a_vector[i].length());
    vec_size += sizeof(uint16_t);
    vec_size += terms[i].length();
  }
  size =  IPV4_ADDRESS_SIZE + vec_size  + sizeof(uint32_t);
  return size;
}

void
PennSearchMessage::SearchReqLookup::Print (std::ostream &os) const
{
  std::vector<std::string> a_vector = terms;
  os << "searchReqLookup:: Message: RequesterNode " << RequesterNode << "\n";
  os << "searchReqLookup:: Message: Terms: " <<"\n";
  for (unsigned i =0; i< a_vector.size(); i++){
    os <<  a_vector[i] << "\n";
  } 
    os << "searchReqLookup:: Message: TermsCt: " << termsCt<<"\n";
}

void
PennSearchMessage::SearchReqLookup::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (RequesterNode.Get ());
  start.WriteU16(terms.size());
   for (unsigned i =0; i< terms.size(); i++){
    start.WriteU16(terms[i].length());
     start.Write ((uint8_t *) (const_cast<char*> (terms[i].c_str())),
   terms[i].length());
   }
   start.WriteHtonU32(termsCt);
}

uint32_t
PennSearchMessage::SearchReqLookup::Deserialize (Buffer::Iterator &start)
{  
  RequesterNode = Ipv4Address (start.ReadNtohU32 ());
  uint16_t lengthofVector = start.ReadU16();
  //std::vector<std::string> actors;

  for (unsigned i =0; i < lengthofVector; i++){
    std::string term;
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    term = std::string (str, length);   
    terms.push_back(term);
    free(str);
  }
  termsCt = uint32_t(start.ReadNtohU32 ());
  return SearchReqLookup::GetSerializedSize ();
}


void
PennSearchMessage::SetSearchReqLookup (Ipv4Address ReqNode, std::vector<std::string> actors, uint32_t tc )
{
  if (m_messageType == 3)
    {
      m_messageType = SEARCH_REQ_LOOKUP;
    }
  else
    {
      NS_ASSERT (m_messageType == SEARCH_REQ_LOOKUP);
    }
  m_message.searchReqLookup.RequesterNode = ReqNode;
  m_message.searchReqLookup.terms = actors;
  m_message.searchReqLookup.termsCt = tc;
}


Ipv4Address
PennSearchMessage::GetRequesterNode ()
{
  if (m_messageType == 3)
  {
    return m_message.searchReqLookup.RequesterNode;
  }
  else {
    //if (m_messageType == 5) 
    return m_message.searchReq.RequesterNode;  
  }
}

std::vector<std::string>
PennSearchMessage::GetTerms ()
{
  if (m_messageType == 3)
  {    
    return m_message.searchReqLookup.terms;
  }
  else 
  //if (m_messageType == 5)
  { 
    return m_message.searchReq.terms;
  }
}

PennSearchMessage::SearchReqLookup
PennSearchMessage::GetSearchReqLookup()
{
  return m_message.searchReqLookup;
}

//* SEARCH_LOOKUP_RSP */

uint32_t 
PennSearchMessage::SearchLookupRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + IPV4_ADDRESS_SIZE + key.length();
  return size;
}

void
PennSearchMessage::SearchLookupRsp::Print (std::ostream &os) const
{ 
   os << "searchLookupRsp:: Message: RequesterNode " << KeyStoringNode << "\n";
   os << "searchLookupRsp:: Message: Key " << key<< "\n";
}

void
PennSearchMessage::SearchLookupRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (KeyStoringNode.Get ());
  start.WriteU16 (key.length ());
  start.Write ((uint8_t *) (const_cast<char*> (key.c_str())), key.length());
}

uint32_t
PennSearchMessage::SearchLookupRsp::Deserialize (Buffer::Iterator &start)
{  
  KeyStoringNode = Ipv4Address (start.ReadNtohU32 ());
  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  key = std::string (str, length);
  free (str);
  return SearchLookupRsp::GetSerializedSize ();
}


void
PennSearchMessage::SetSearchLookupRsp (Ipv4Address keyNode, std::string k)
{
  if (m_messageType == 4)
    {
      m_messageType = SEARCH_LOOKUP_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == SEARCH_LOOKUP_RSP);
    }
  m_message.searchLookupRsp.KeyStoringNode = keyNode;
  m_message.searchLookupRsp.key = k;
}

Ipv4Address
PennSearchMessage::GetKeyStoringNode()
{
  return m_message.searchLookupRsp.KeyStoringNode;
}

std::string
PennSearchMessage::GetKey ()
{
 // if (m_messageType == 4){
  return m_message.searchLookupRsp.key;
 // }
}


PennSearchMessage::SearchLookupRsp
PennSearchMessage::GetSearchLookupRsp()
{
  return m_message.searchLookupRsp;
}

//* SEARCH_REQ */

uint32_t 
PennSearchMessage::SearchReq::GetSerializedSize (void) const
{
  uint32_t size;
  // size of terms
  uint32_t vec_size = sizeof(uint16_t);

  for (unsigned i =0; i< terms.size(); i++){
    //vec_size = vec_size + sizeof(uint8_t)*(a_vector[i].length());
    vec_size += sizeof(uint16_t);
    vec_size += terms[i].length();
  }
// size of movies
  vec_size += sizeof(uint16_t);

  for (unsigned i =0; i< movies.size(); i++){
    //vec_size = vec_size + sizeof(uint8_t)*(a_vector[i].length());
    vec_size += sizeof(uint16_t);
    vec_size += movies[i].length();
  }
  size = IPV4_ADDRESS_SIZE + vec_size + sizeof(uint32_t);
  return size;
}

void
PennSearchMessage::SearchReq::Print (std::ostream &os) const
{
  std::vector<std::string> a_vector = terms;
  os << "searchReq:: Message: RequesterNode " << RequesterNode << "\n";
  os << "searchReq:: Message: Terms: " <<"\n";
  for (unsigned i =0; i< a_vector.size(); i++){
    os <<  a_vector[i] << "\n";
  } 
    os << "searchReq:: Message: Movies: " <<"\n";
  for (unsigned i =0; i< movies.size(); i++){
    os <<  movies[i] << "\n";
  } 
  os << "searchReq:: Message: termcount" << termsCt<< "\n";
}

void
PennSearchMessage::SearchReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (RequesterNode.Get ());
    start.WriteU16(terms.size());
   for (unsigned i =0; i< terms.size(); i++){
    start.WriteU16(terms[i].length());
     start.Write ((uint8_t *) (const_cast<char*> (terms[i].c_str())), terms[i].length());
   }
   start.WriteU16(movies.size());
   for (unsigned i =0; i< movies.size(); i++){
    start.WriteU16(movies[i].length());
     start.Write ((uint8_t *) (const_cast<char*> (movies[i].c_str())),
   movies[i].length());
   }

   start.WriteHtonU32(termsCt);
}

uint32_t
PennSearchMessage::SearchReq::Deserialize (Buffer::Iterator &start)
{  
  RequesterNode = Ipv4Address (start.ReadNtohU32 ());
  uint16_t lengthofterms = start.ReadU16();
   for (unsigned i =0; i < lengthofterms; i++){
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    std::string term = std::string (str, length); 
    terms.push_back(term);
    free(str);
  }
     uint16_t lengthofmovies = start.ReadU16();
    for (unsigned i =0; i < lengthofmovies; i++){
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    std::string movie = std::string (str, length);
    movies.push_back(movie);
    free(str);
  }
  termsCt = uint32_t(start.ReadNtohU32 ());
  return SearchReq::GetSerializedSize ();
}


void
PennSearchMessage::SetSearchReq(Ipv4Address ReqNode, std::vector<std::string> actors, std::vector<std::string> docs, uint32_t tc  )
{
  if (m_messageType == 5)
    {
      m_messageType = SEARCH_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == SEARCH_REQ);
    }
  m_message.searchReq.RequesterNode = ReqNode;
  m_message.searchReq.terms = actors;
  m_message.searchReq.movies = docs;
  m_message.searchReq.termsCt = tc;
}

std::vector<std::string>
PennSearchMessage::GetMovies ()
{

   if (m_messageType ==  5)
  {
    return m_message.searchReq.movies;
  }
     else 
     //if (m_messageType ==  6)
  { 
    return m_message.searchRsp.movies;
  }
}

uint32_t
PennSearchMessage::GetTermsCount ()
{
  if (m_messageType ==  3){
    return m_message.searchReqLookup.termsCt;
  }
  else if (m_messageType ==  5){
    return m_message.searchReq.termsCt;
  }
  else 
  //if (m_messageType ==  6)
  {
    return m_message.searchRsp.termsCt;
  }
}

PennSearchMessage::SearchReq
PennSearchMessage::GetSearchReq()
{
  return m_message.searchReq;
}


//* SEARCH_RSP */

uint32_t 
PennSearchMessage::SearchRsp::GetSerializedSize (void) const
{        
  uint32_t size;
  uint32_t vec_size = 0;
  vec_size += sizeof(uint16_t);
  for (unsigned i =0; i< movies.size(); i++){
    vec_size += sizeof(uint16_t);
    vec_size += movies[i].length();
  }
  size = IPV4_ADDRESS_SIZE + vec_size + sizeof(uint32_t);
  return size;
}

void
PennSearchMessage::SearchRsp::Print (std::ostream &os) const
{
  //std::vector<std::string> a_vector = terms;
  os << "searchRsp:: Message: RequesterNode " << RequesterNode << "\n";
  /* os << "searchReqLookup:: Message: Terms: " <<"\n";
  for (unsigned i =0; i< a_vector.size(); i++){
    os <<  a_vector[i] << "\n";
  } */
    os << "searchRsp:: Message: Movies: " <<"\n";
  for (unsigned i =0; i< movies.size(); i++){
    os <<  movies[i] << "\n";
  } 
  os << "searchRsp:: Message: termsCount " <<  termsCt<< "\n";
}

void
PennSearchMessage::SearchRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (RequesterNode.Get ());
   start.WriteU16(movies.size());
   for (unsigned i =0; i< movies.size(); i++){
    start.WriteU16(movies[i].length());
     start.Write ((uint8_t *) (const_cast<char*> (movies[i].c_str())),
   movies[i].length());
   }
   start.WriteHtonU32(termsCt);
}

uint32_t
PennSearchMessage::SearchRsp::Deserialize (Buffer::Iterator &start)
{  
  RequesterNode = Ipv4Address (start.ReadNtohU32 ());
    uint16_t lengthofmovies = start.ReadU16();
    for (unsigned i =0; i < lengthofmovies; i++){
    uint16_t length = start.ReadU16 ();
    char* str = (char*) malloc (length);
    start.Read ((uint8_t*)str, length);
    std::string movie = std::string (str, length);
    movies.push_back(movie);
    free(str);
  }
   termsCt = uint32_t(start.ReadNtohU32 ());
  return SearchRsp::GetSerializedSize ();
}



void
PennSearchMessage::SetSearchRsp(Ipv4Address ReqNode,  std::vector<std::string> docs, uint32_t termsCount  )
{
  if (m_messageType == 6)
    {
      m_messageType = SEARCH_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == SEARCH_RSP);
    }
  m_message.searchRsp.RequesterNode = ReqNode;
  m_message.searchRsp.movies = docs;
  m_message.searchRsp.termsCt = termsCount;
}


PennSearchMessage::SearchRsp
PennSearchMessage::GetSearchRsp()
{
  return m_message.searchRsp;
}

//**************************************//

void
PennSearchMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

PennSearchMessage::MessageType
PennSearchMessage::GetMessageType () const
{
  return m_messageType;
}

void
PennSearchMessage::SetTransactionId (uint32_t transactionId)
{
  m_transactionId = transactionId;
}

uint32_t 
PennSearchMessage::GetTransactionId (void) const
{
  return m_transactionId;
}

/* PUBLISH_REQ */

uint32_t 
PennSearchMessage::PublishReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + actor.length();
  size += sizeof(uint16_t) + movie.length();
  return size;
}

void
PennSearchMessage::PublishReq::Print (std::ostream &os) const
{
  os << "PublishReq:: Actor: " << actor << "Movie" << movie << "\n";
}

void
PennSearchMessage::PublishReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (actor.length ());
  start.Write ((uint8_t *) (const_cast<char*> (actor.c_str())), actor.length());
  start.WriteU16 (movie.length ());
  start.Write ((uint8_t *) (const_cast<char*> (movie.c_str())), movie.length());
}

uint32_t
PennSearchMessage::PublishReq::Deserialize (Buffer::Iterator &start)
{  
  // Get length of first string (which is actor, because you serialized actor first)
  uint16_t length = start.ReadU16 ();
  // create a char array on the heap by malloc memory for the length of the string
  char* str = (char*) malloc (length);
  //Read the string into the char array
  start.Read ((uint8_t*)str, length);
  // Assign from the start of the char array to the length of the string to actor
  actor = std::string (str, length);
  //Free the memory stored for str
  free (str);

 // Repeat the process for movie
  uint16_t lengthTwo = start.ReadU16 ();
  char* strTwo = (char*) malloc (lengthTwo);
  start.Read ((uint8_t*)strTwo, lengthTwo);
  movie = std::string (strTwo, lengthTwo);
  free(strTwo);

  return PublishReq::GetSerializedSize ();

}

void
PennSearchMessage::SetPublishReq(std::string actor, std::string movie)
{
  if (m_messageType == 7)
    {
      m_messageType = PUBLISH_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == PUBLISH_REQ);
    }
  m_message.publishReq.actor = actor;
  m_message.publishReq.movie = movie;
}

PennSearchMessage::PublishReq
PennSearchMessage::GetPublishReq ()
{
  return m_message.publishReq;
}


/* INVERTED_LIST */

uint32_t 
PennSearchMessage::InvertedList::GetSerializedSize (void) const
{
  uint32_t size;

  size = sizeof(uint16_t);

  for (auto iter = invertedList.begin(); iter !=invertedList.end(); iter++) {
      // length of key
      size += sizeof(uint16_t);
      //key itself
      size += (iter->first).length(); 
      // length of vector (value)
      size += sizeof(uint16_t);

      for (int i = 0 ; i < int(iter->second.size()); i++) {
          // length of string
          size += sizeof(uint16_t);
          // string itself
          size += iter->second[i].length();
      }

  }
  return size;
}

void
PennSearchMessage::InvertedList::Print (std::ostream &os) const
{
  os << "Inverted List: " << "print not implemented" << "\n";
}

void
PennSearchMessage::InvertedList::Serialize (Buffer::Iterator &start) const
{
    // size of map 
    start.WriteU16 (invertedList.size());

  for (auto iter = invertedList.begin(); iter !=invertedList.end(); iter++) {
      // length of key
        start.WriteU16 (iter->first.length());
        // key itself
        start.Write ((uint8_t *) (const_cast<char*> (iter->first.c_str())), iter->first.length());

        // length of vector
        start.WriteU16 (iter->second.size());
      
      for (int i = 0 ; i < int(iter->second.size()); i++) {
          // length of string
          start.WriteU16 (iter->second[i].size());
          start.Write ((uint8_t *) (const_cast<char*> (iter->second[i].c_str())), iter->second[i].length());
      }
      }
}

uint32_t
PennSearchMessage::InvertedList::Deserialize (Buffer::Iterator &start)
{  
    // size of map 
    uint16_t lengthOfMap = start.ReadU16 ();

    for (int i = 0; i < lengthOfMap; i++) {
      std::string key;
      uint16_t lengthOfKey = start.ReadU16 ();
      char* str = (char*) malloc (lengthOfKey);
      start.Read ((uint8_t*)str, lengthOfKey);
      key = std::string (str, lengthOfKey);

      uint16_t lengthOfVector = start.ReadU16 ();
      std::vector<std::string> values; 


      for (int i = 0; i < lengthOfVector; i++) {
          std::string value;
          uint16_t lengthOfValue = start.ReadU16 ();
          char* strTwo = (char*) malloc (lengthOfValue);
          start.Read ((uint8_t*)strTwo, lengthOfValue);
          value = std::string (strTwo, lengthOfValue);
          values.push_back(value);
          free(strTwo);
      }
      invertedList.insert(std::make_pair (key, values));
      free(str);
    }
    

  return InvertedList::GetSerializedSize ();
}

void
PennSearchMessage::SetInvertedList (std::map<std::string, std::vector<std::string>> invertedList)
{
  if (m_messageType == 8)
    {
      m_messageType = INVERTED_LIST;
    }
  else
    {
      NS_ASSERT (m_messageType == PUBLISH_REQ);
    }
  m_message.invertedList.invertedList = invertedList;
}

PennSearchMessage::InvertedList
PennSearchMessage::GetInvertedList ()
{
  return m_message.invertedList;
}

//* KEY_TRANSFER_REJOIN */

uint32_t 
PennSearchMessage::KeyTransferRejoin::GetSerializedSize (void) const
{
  uint32_t size;
  size = IPV4_ADDRESS_SIZE;
  return size;
}

void
PennSearchMessage::KeyTransferRejoin::Print (std::ostream &os) const
{ 
   os << "keyTransferRejoin:: Message: newNode " << newNode << "\n";

}

void
PennSearchMessage::KeyTransferRejoin::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (newNode.Get ());
  
}

uint32_t
PennSearchMessage::KeyTransferRejoin::Deserialize (Buffer::Iterator &start)
{  
  newNode = Ipv4Address (start.ReadNtohU32 ());
  
  return KeyTransferRejoin::GetSerializedSize ();
}


void
PennSearchMessage::SetKeyTransferRejoin (Ipv4Address newNode)
{
  if (m_messageType == 9)
    {
      m_messageType = KEY_TRANSFER_REJOIN;
    }
  else
    {
      NS_ASSERT (m_messageType == KEY_TRANSFER_REJOIN);
    }
  m_message.keyTransferRejoin.newNode = newNode;

}

Ipv4Address
PennSearchMessage::GetNewNode()
{
  return m_message.keyTransferRejoin.newNode;
}

PennSearchMessage::KeyTransferRejoin
PennSearchMessage::GetKeyTransferRejoin()
{
  return m_message.keyTransferRejoin;
}


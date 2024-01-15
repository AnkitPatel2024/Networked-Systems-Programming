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


#include "penn-search-message.h"
#include "penn-search.h"
#include "grader-logs.h"
#include <set>
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include <algorithm>
#include <string>


using namespace ns3;

TypeId
PennSearch::GetTypeId ()
{
  static TypeId tid = TypeId ("PennSearch")
    .SetParent<PennApplication> ()
    .AddConstructor<PennSearch> ()
    .AddAttribute ("AppPort",
                   "Listening port for Application",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&PennSearch::m_appPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ChordPort",
                   "Listening port for Application",
                   UintegerValue (10001),
                   MakeUintegerAccessor (&PennSearch::m_chordPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PingTimeout",
                   "Timeout value for PING_REQ in milliseconds",
                   TimeValue (MilliSeconds (2000)),
                   MakeTimeAccessor (&PennSearch::m_pingTimeout),
                   MakeTimeChecker ())
    ;
  return tid;
}

PennSearch::PennSearch ()
  : m_auditPingsTimer (Timer::CANCEL_ON_DESTROY)
{
  m_chord = NULL;

  Ptr<UniformRandomVariable> m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
  m_currentTransactionId = m_uniformRandomVariable->GetValue (0x00000000, 0xFFFFFFFF);


}

PennSearch::~PennSearch ()
{

}

void
PennSearch::DoDispose ()
{
  StopApplication ();
  PennApplication::DoDispose ();
  
  // FOR TESTING
  GraderLogs::HelloGrader(ReverseLookup(GetLocalAddress()), GetLocalAddress());
}

void
PennSearch::StartApplication (void)
{
  //std::cout << "PennSearch::StartApplication()!!!!!" << std::endl;
  // Create and Configure PennChord
  ObjectFactory factory;

  factory.SetTypeId (PennChord::GetTypeId ());
  factory.Set ("AppPort", UintegerValue (m_chordPort));
  m_chord = factory.Create<PennChord> ();
  m_chord->SetNode (GetNode ());
  m_chord->SetNodeAddressMap (m_nodeAddressMap);
  m_chord->SetAddressNodeMap (m_addressNodeMap);
  m_chord->SetModuleName ("CHORD");
  std::string nodeId = GetNodeId ();
  m_chord->SetNodeId (nodeId);
  m_chord->SetLocalAddress (m_local);

  // Configure Callbacks with Chord
  m_chord->SetPingSuccessCallback (MakeCallback (&PennSearch::HandleChordPingSuccess, this)); 
  m_chord->SetPingFailureCallback (MakeCallback (&PennSearch::HandleChordPingFailure, this));
  m_chord->SetPingRecvCallback (MakeCallback (&PennSearch::HandleChordPingRecv, this)); 
  m_chord->SetLeaveApplicationCallback (MakeCallback (&PennSearch::HandleLeavingApplication, this)); 
  m_chord->SetRejoinApplicationCallback (MakeCallback (&PennSearch::HandleRejoiningApplication, this)); 

  //*****************MS-2*************************************
  //confirgure callback with Chord for HandleLookup
  //Penn-search using the setter to register a callback(i.e. "Hey chord when you
  //have a lookupSuccess please execute my function HandleChordLookupSuccess")

  m_chord->SetLookupSuccessCallback(MakeCallback (&PennSearch::HandleChordLookupSuccess, this));

  
  // Start Chord
  m_chord->SetStartTime (Simulator::Now());
  m_chord->Initialize();

  if (m_socket == 0)
    { 
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), m_appPort);
      m_socket->Bind (local);
      m_socket->SetRecvCallback (MakeCallback (&PennSearch::RecvMessage, this));
    }  
  
  // Configure timers
  m_auditPingsTimer.SetFunction (&PennSearch::AuditPings, this);
  // Start timers
  m_auditPingsTimer.Schedule (m_pingTimeout);
}

void
PennSearch::StopApplication (void)
{
  //Stop chord
  m_chord->StopChord ();
  // Close socket
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  // Cancel timers
  m_auditPingsTimer.Cancel ();
  m_pingTracker.clear ();
}

void
PennSearch::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;
  if (command == "CHORD")
    { 
      // Send to Chord Sub-Layer
      tokens.erase (iterator);
      m_chord->ProcessCommand (tokens);
    } 
  if (command == "PING")
    {
      if (tokens.size() < 3)
        {
          ERROR_LOG ("Insufficient PING params..."); 
          return;
        }
      iterator++;
      if (*iterator != "*")
        {
          std::string nodeId = *iterator;
          iterator++;
          std::string pingMessage = *iterator;
          SendPing (nodeId, pingMessage);
        }
      else
        {
          iterator++;
          std::string pingMessage = *iterator;
          std::map<uint32_t, Ipv4Address>::iterator iter;
          for (iter = m_nodeAddressMap.begin () ; iter != m_nodeAddressMap.end (); iter++)  
            {
              std::ostringstream sin;
              uint32_t nodeNumber = iter->first;
              sin << nodeNumber;
              std::string nodeId = sin.str();    
              SendPing (nodeId, pingMessage);
            }
        }
    }
  //*****************MS-2*************************************
  //Handler for PUBLISH
  if (command == "PUBLISH"){
    iterator++;
    std::string metadataFile = *iterator;
    /*
  - (1) parse files and create inverted lists (Done)
  - (2) for each inverted list start a lookup request for the corresponding term (artist) with a transaction Id. (Done)
  - (3) store transaction id and inverted list in a map for pending responses. (Done)
  - (4) when chord layer has a lookup response it should invoke a callback in search with the ip and transaction id (I think this is done in Ankita's branch just have to call lookup)
  - (5) the callback function goes to the map takes the inverted list using transaction id and sends it to the owner for storage. (once merged I will do this)
    */
  std::map<std::string, std::vector<std::string>> invertedList = PennSearch::CreateInvertedList(metadataFile);
  // Testing 
  // for (auto it = invertedList.begin(); it != invertedList.end(); it++)
  // {
  //     std::vector<std::string> docList = it->second;
  //     for (auto doc = docList.begin(); doc != docList.end(); doc++){
  //       SEARCH_LOG ("Movie " << *doc << " by actor " << it->first);
  //     }
  // }

    for (auto it = invertedList.begin(); it != invertedList.end(); it++)
    {
        // may need some logic to make sure we dont recreate transaction ids but seems unlikely
      
        uint32_t txn_id = GetNextTransactionId ();
        std::string actor = it->first;
        PublishEntry entry;
        std::vector<std::string> movies = it->second;
        entry.actor = actor;
        entry.movies = movies;
        m_publishTracker.insert(std::make_pair (txn_id, entry));
        // Lookup not defined in this branch
        Ipv4Address nodeIp = GetLocalAddress();
        std::string lookupOriginator = ReverseLookup(nodeIp);
        m_chord ->SetLookupType(PennChord::LookupType::PUBLISH);  
        //std::cout << "LOOK UP FROM SEARCH 1" << "\n";
        m_chord ->LookupFromSearch(actor, txn_id, lookupOriginator, PennChord::LookupType::PUBLISH);
        for(unsigned int i = 0; i < entry.movies.size(); i++){
          SEARCH_LOG(GraderLogs::GetPublishLogStr(actor, entry.movies[i]));
        }
      
    //  std::cout << "\n" <<  "RECEIVED CMD TO PUBLISH: " << actor << "\n";
    
    }
            // std::cout << "DONE" << "\n";

  }

  if (command == "TESTPUBLISH") {

      for (auto iter = m_invertedList.begin(); iter !=m_invertedList.end(); iter++)
  {
  std::cout << "Actor: " << iter->first << " ";
   std::cout << "Movies: ";

          for (uint32_t i = 0; i < iter->second.size(); i++) {
                  std::cout << iter->second[i] << " ";

          }
  std::cout << "\n";
   }
  }
    
  
    //Handler for SEARCH
    if (command == "SEARCH")
    {

      std::vector<std::string> terms;
      Ipv4Address nodeIp = GetLocalAddress();
      std::string requesterNode = ReverseLookup(nodeIp);
      tokens.erase (iterator);
      iterator = tokens.begin ();
      std::string executingNode = *iterator;
      tokens.erase (iterator);    
      iterator = tokens.begin() ;
      std::string term = *iterator;
      terms.push_back(term);
      iterator++;
      for (unsigned i = 1; i< tokens.size(); i++)
      {
      terms.push_back(*iterator);
      iterator++;
       }

/////////for Debugging

      ////////////////Delete above after debugging
     SendProcessSearchREQ ( requesterNode, executingNode, terms);


    //  for (int i = 0; i < int(terms.size()); i++) {
    //   std::cout << terms[i];
    //  }


///////for debugging
          // PRINT_LOG("Requester Node is: "<< requesterNode << "Exec Node: "<< executingNode<<"term size:" << terms.size())
    } 
    
    if (command == "SENDIL") {
      Ipv4Address nodeTwo = ResolveNodeIpAddress("2");
      PennSearch::SendInvertedList(nodeTwo);
    }
}

// (1) parse files and create inverted lists 
std::map<std::string, std::vector<std::string>> PennSearch::CreateInvertedList(std::string metadataFile){
  //SEARCH_LOG ("File name: " << metadataFile);
  std::map<std::string, std::vector<std::string>> invertedList;
  std::ifstream file;
  file.open (metadataFile.c_str ());
  // based on https://edstem.org/us/courses/39229/discussion/3256002 we should only expect absolute file paths like in m2i.sce
  // like this ./contrib/upenn-cis553/keys/metadata2.keys not ../keys/metadata0.keys
  if (file.is_open ())
      {
        std::string currentLine;
        std::vector<std::string> m_tokens;
        // SEARCH_LOG ("Reading script file: " << metadataFile);
        // Iterate over file to create inverted list
        do {
          std::getline (file, currentLine, '\n');
          if (currentLine == "")
            continue;
          PennSearch::Tokenize (currentLine, m_tokens, " ");
          // create inverted lists 
          std::vector<std::string>::iterator iterator = m_tokens.begin ();
          std::string doc = *iterator;
          //SEARCH_LOG ("Movie token: " << doc);
          m_tokens.erase(m_tokens.begin());
          // For each inverted list start a lookup request for the corresponding term (artist) with a transaction Id. 
          for (auto iterator = m_tokens.begin(); iterator != m_tokens.end();)
          {
            //SEARCH_LOG ("Actors token: " << *iterator);
            if (invertedList.find(*iterator) != invertedList.end())
            {
              //append to doc/movie to list
              invertedList[*iterator].push_back(doc);
            }
            else{
              // create vector with doc/movie
              std::vector<std::string> docList;
              docList.push_back(doc);
              invertedList.insert(std::pair<std::string,std::vector<std::string>>(*iterator, docList));
            }
            iterator++;
          }
        m_tokens.clear ();

        }
        while (!file.eof ());
      }
    else{
      SEARCH_LOG ("Unable to open file. Make sure to use absolute path.");
    }
     
    return invertedList;

}

/* Method Tokenize, Credits:  http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html */
/* Also used in simulator-main but since main does not have .h file I don't think we can import it*/
void
PennSearch::Tokenize (const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters)
{
  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of (delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos = str.find_first_of (delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      // Found a token, add it to the vector.
      tokens.push_back (str.substr (lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of (delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of (delimiters, lastPos);
    }
}


//the requestor node sends a SEARCH_REQ_LOOKUP message to the executing node. THe SEARCH_REQ_LOOKUP payload will contain SearchEntry that will contain the vector of terms/keys
void 
PennSearch::SendProcessSearchREQ(std::string req_node, std::string exec_node, std::vector<std::string> keys)
{
  //////////////////Debugging PRINT
    // PRINT_LOG("Enters SendProcessSearchREQ" << " AND Req_node is: "<< req_node << "\n");
  Ipv4Address nodeIp = GetLocalAddress();
  std::string thisNode = ReverseLookup(nodeIp);
   SearchEntry searchentry; 
   searchentry.termsCount = keys.size();

  if (req_node == exec_node){
  // PRINT_LOG("Req node and exec node is same");   ////DElete this later
  if (keys.size()>0){
    std::string search_term;
    searchentry.RequesterNode = nodeIp;
    searchentry.terms = keys; 
    search_term = keys[0];
    searchentry.termsCount = keys.size(); 
    uint32_t txn_id = GetNextTransactionId ();
    std::string lookupOriginator = thisNode;
    m_searchEntryTracker.insert(std::make_pair (txn_id, searchentry));
    PennChord::LookupType type = PennChord::LookupType::SEARCH;
    m_lookupCount +=1;  //need to verify if this is what intended
           // std::cout << "LOOK UP FROM SEARCH 2 "  << "\n";

    m_chord ->LookupFromSearch(search_term, txn_id, lookupOriginator, type);
    uint32_t currentNodeKey = PennKeyHelper::CreateShaKey(nodeIp);
    uint32_t targetKey = PennKeyHelper::CreateShaKey(search_term);
    SEARCH_LOG(GraderLogs::GetLookupIssueLogStr(currentNodeKey, targetKey));
    SEARCH_LOG(GraderLogs::GetSearchLogStr(keys));
  }
  }
   
    else{ //req_node != exec_node
    uint32_t comp = thisNode.compare(req_node);
    if (comp == 0)
    {
  
      uint32_t transactionId = GetNextTransactionId ();
      // send SEARCH_REQ_LOOKUP to exec_node
      Ipv4Address destAddress = PennApplication::ResolveNodeIpAddress(exec_node);
          /////////////////Debugging PRINT
      //PRINT_LOG("Current Node" << thisNode<< " Exec node: " << exec_node<< " destAddr: "<< destAddress << "\n");
      //SearchEntry searchentry;
      searchentry.RequesterNode = nodeIp;
      searchentry.terms = keys;
      searchentry.termsCount = keys.size(); 

      Ptr<Packet> packet = Create<Packet> ();
      PennSearchMessage message = PennSearchMessage (PennSearchMessage::SEARCH_REQ_LOOKUP, transactionId); //TODO add SEARCH_REQ_LOOKUP message Type . Payload = SearchEntry entry
      message.SetSearchReqLookup(nodeIp, keys, searchentry.termsCount);
      packet->AddHeader (message);
       /////////////////Debugging PRINT
      // PRINT_LOG("line 418 Size of keys: "<< keys.size() << " SIze of entry terms: "<< searchentry.terms.size() << "dest addr: " << destAddress << 
      // "m_appPort: " << m_appPort);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
      SEARCH_LOG(GraderLogs::GetSearchLogStr(keys));
      /////////////////Debugging PRINT
       
    }
    // PRINT_LOG("Leaves SendProcessSearchREQ");
}
}

//The node that receives the SEARCH_REQ_LOOKUP message, will call the lookup function to find the node storing the key. 
void
PennSearch::ProcessSearchREQLookup (PennSearchMessage message)
{

  ///msg type being processed = SEARCH_REQ_LOOKUP
     //////////////////Debugging PRINT
    // PRINT_LOG("Line 434 Enters ProcessSearchREQLookup" << "\n");

  std::string search_term;
  SearchEntry entry ;  
  std::vector<std::string> actors = message.GetTerms();
  entry.RequesterNode = message.GetRequesterNode();
  entry.terms = actors;
  entry.termsCount = message.GetTermsCount();

  if (actors.size() > 0)
  {
    Ipv4Address nodeIp = GetLocalAddress();
    std::string lookupOriginator = ReverseLookup(nodeIp);
    search_term = actors[0];
    uint32_t txn_id = message.GetTransactionId();
  
    auto ite = m_searchEntryTracker.find(txn_id);
    if (ite != m_searchEntryTracker.end()){
      m_searchEntryTracker.erase(ite);
     // PRINT_LOG("line 458 entry was deleted from tracker")
    }
   
    m_searchEntryTracker.insert(std::make_pair (txn_id, entry));
   
    PennChord::LookupType type = PennChord::LookupType::SEARCH;
    m_lookupCount +=1;  //need to verify if this is what intended
            //std::cout << "LOOK UP FROM SEARCH 3 " << "\n";
    m_chord ->LookupFromSearch(search_term, txn_id, lookupOriginator, type);

    uint32_t currentNodeKey = PennKeyHelper::CreateShaKey(nodeIp);
    uint32_t targetKey = PennKeyHelper::CreateShaKey(search_term);
    SEARCH_LOG(GraderLogs::GetLookupIssueLogStr(currentNodeKey, targetKey));
  }
  else
  {
   // SEARCH_LOG("No search term is found");
  }
}

//Interface for PennChord
//Handle Chord Lookup
//once executing node gets the IPaddress of the node storing the key, it will send a message to that node with a payload that contains the SearchEntry
void 
PennSearch::HandleChordLookupSuccess(Ipv4Address KeyStoringNode, std::string key, uint32_t txnid, std::string LookupOriginator, PennChord::LookupType type )
{
//     if (KeyStoringNode != Ipv4Address::GetAny ())
// {
// std::cout << "\n" << "NOT A VALID ADDRESS" << "\n";
//   KeyStoringNode.Print(std::cout);
//   std::cout << "Node that has IP address: " << ReverseLookup(KeyStoringNode);
// }
// if (type == PennChord::LookupType::PUBLISH){
// std::cout << "\n" <<  "LOOKUP FOR PUBLISH: " << key << " NODE THAT HAS IT: " << ReverseLookup(KeyStoringNode) << "\n";

// }
// if (type == PennChord::LookupType::SEARCH){
// std::cout << "\n" <<  "LOOKUP FOR PUBLISH: " << key << " NODE THAT HAS IT: " << ReverseLookup(KeyStoringNode) << "\n";

// }



 uint32_t currentNodeKey = PennKeyHelper::CreateShaKey(GetLocalAddress());
 uint32_t targetKey = PennKeyHelper::CreateShaKey(key);
 
 Ipv4Address lookupOrigIp = ResolveNodeIpAddress(LookupOriginator);
 uint32_t originatorNodeKey = PennKeyHelper::CreateShaKey(lookupOrigIp);
 std::string cur_node = ReverseLookup(GetLocalAddress());

//VAL: TESTING PDF says this should use CHORD_LOG ???
// Runs with chord log here so adding it
 SEARCH_LOG(GraderLogs::GetLookupResultLogStr(currentNodeKey, targetKey, LookupOriginator, originatorNodeKey));

  // send a message to lookup originator with the keystoring node, key, and transaction id
   Ptr<Packet> packet = Create<Packet> ();
   if (type == PennChord::LookupType::SEARCH){
    //if the node that got back is the same as the node that send the lookup request
    if (cur_node == LookupOriginator )
     {
  //  std::cout << "look up from search - i am node";

   //   PRINT_LOG("line 506 lookupOriginator is the same as current node in HandleLookupSuccess");
       SearchEntry entry ;
      for (auto itr = m_searchEntryTracker.begin(); itr !=m_searchEntryTracker.end(); itr++)
      {
        uint32_t transactionid = itr ->first;
        if (transactionid == txnid)
        {
        entry = itr->second;
        }
      }
      Ptr<Packet> packet = Create<Packet> ();
      PennSearchMessage message = PennSearchMessage (PennSearchMessage::SEARCH_REQ, txnid);
      message.SetSearchReq(entry.RequesterNode, entry.terms, entry.movies, entry.termsCount);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (KeyStoringNode, m_appPort));
      //SEARCH_LOG(GraderLogs::GetInvertedListShipLogStr( key, entry.movies));
     }
    else { // cur_node != LookupOriginator
       //  std::cout << "look up from search - send to node";

    PennSearchMessage message = PennSearchMessage (PennSearchMessage::SEARCH_LOOKUP_RSP, txnid); //TODO add message type. Payload = node storing key and key
    //message.SetKeyStoringNode(KeyStoringNode); // MODIFY
   // message.SetKey(key);  // MODIFY
   message.SetSearchLookupRsp(KeyStoringNode, key);
        packet->AddHeader (message);
   m_socket->SendTo (packet, 0 , InetSocketAddress (lookupOrigIp, m_appPort));

  // std::cout << "\n" << "LOOK UP FOR SEARCH: " << key << "Node that has it: " << ReverseLookup(KeyStoringNode) << "\n";
   }
   }

      if (type == PennChord::LookupType::PUBLISH){
        if (cur_node == LookupOriginator ){

      PennSearch::PublishEntry entry;
      uint32_t transactionid;
        for (auto itr = m_publishTracker.begin(); itr !=m_publishTracker.end(); itr++)

        {
           transactionid = itr ->first;
          if (transactionid == txnid)
            {
            entry = itr->second;
            break;
            }
            else {
      //       std::cout << "\n" << "Not in here" << "\n";
            }
        }
      //m_publishTracker.erase(iter->first); FROM PAT: this is how it originally was. But iter->first doesnt exist
      //outside of the for loop, so I set it as a variable and erased it afterward. Not sure if this is right.

      // since we read through an entire file when parsing we could have looked up an actor with multiple moves but based on 
      // the logging guidelines I think we should send each the actor and each of its movies in different messages
      //  std::cout << "\n" << " PUBLISHING: " << entry.actor << "TO: " << ReverseLookup(KeyStoringNode) << "\n";

   //    std::cout << "\n" << " SIZE OF MOVIES TO PUBLISH: " << entry.movies.size() << "\n";
      
      for(unsigned int i = 0; i < entry.movies.size(); i++){

        PennSearchMessage message = PennSearchMessage (PennSearchMessage::PUBLISH_REQ, txnid);
        message.SetPublishReq(entry.actor, entry.movies[i]); // TO DO: Add setter methods
        packet->AddHeader (message);
        m_socket->SendTo (packet, 0 , InetSocketAddress (KeyStoringNode, m_appPort));
        // Whenever a node publishes a new invested list entry (an entry is a key, value pair) GraderLoggs::GetPublishLogStr() should be called
        //(std::string keyword, std::string docID)
        // I assume its the actor since we are passing a string as the key not the hashed value but we can always update
   //   std::cout << "\n" << " PUBLISHING: " << entry.actor << "TO: " << ReverseLookup(KeyStoringNode) << "\n";
     // std::cout << "\n" << " HERE 1" << "\n";

        //GraderLogs::GetPublishLogStr(entry.actor, entry.movies[i]);
      }
        m_publishTracker.erase(transactionid);
      } // end of if for if same nodes
      else{
     //   std::cout << "look up from publish - send to node";
         PennSearchMessage message = PennSearchMessage (PennSearchMessage::SEARCH_LOOKUP_RSP, txnid); //TODO add message type. Payload = node storing key and key
          //message.SetKeyStoringNode(KeyStoringNode); // MODIFY
        // message.SetKey(key);  // MODIFY
        message.SetSearchLookupRsp(KeyStoringNode, key);
        packet->AddHeader (message);
        m_socket->SendTo (packet, 0 , InetSocketAddress (lookupOrigIp, m_appPort));

       // std::cout << "\n" << "LOOK UP FOR PUBLISH: " << key << "Node that has it: " << ReverseLookup(KeyStoringNode) << "\n";

      }// publish end of else  different nodes
    }// end publish
  
}


//Now the node who originally called look up gets the keystoring node. Now the node sends SEARCH_REQ to key storing node
void 
PennSearch::ProcessSearchLookupRSP(PennSearchMessage msg)
{
  bool searchTxn = false;
  uint32_t txnid = msg.GetTransactionId ();
  Ipv4Address KeyStoringNode = msg.GetKeyStoringNode();
  
  SearchEntry entry ;
  for (auto itr = m_searchEntryTracker.begin(); itr !=m_searchEntryTracker.end(); itr++)
  {
    uint32_t transactionid = itr ->first;
    if (transactionid == txnid)
      {
      searchTxn = true;
      entry = itr->second;
      }
  }
  Ptr<Packet> packet = Create<Packet> ();
  if (searchTxn == true){ //sesarch logic

  //std::cout << "\n" << " IN MAP FOR" << entry.terms[0] << " Node that has it: " << ReverseLookup(KeyStoringNode) << "\n";
  // uint32_t currentNodeKey = PennKeyHelper::CreateShaKey(GetLocalAddress());
  // uint32_t targetKey = PennKeyHelper::CreateShaKey(key);
 
   //entry.hop_count += 1;  //TODO check on hopcount and lookup count and modify as needed after getting clarification
   PennSearchMessage message = PennSearchMessage (PennSearchMessage::SEARCH_REQ, txnid); //TODO add SEARCH_REQ message Type . Payload = SearchEntry entry
 
   message.SetSearchReq(entry.RequesterNode, entry.terms, entry.movies, entry.termsCount);
   
    //SEARCH_LOG(GraderLogs::GetInvertedListShipLogStr( entry.terms[0], entry.movies));
    
   packet->AddHeader (message);
   m_socket->SendTo (packet, 0 , InetSocketAddress (KeyStoringNode, m_appPort));
  } 
  else{ //publish transaction
      PennSearch::PublishEntry entry;
      uint32_t transactionid;
        for (auto itr = m_publishTracker.begin(); itr !=m_publishTracker.end(); itr++)

        {
           transactionid = itr ->first;
          if (transactionid == txnid)
            {
            entry = itr->second;
            break;
            }
            else {
            // std::cout << "\n" << "Not in here" << "\n";
            }
        }
      //m_publishTracker.erase(iter->first); FROM PAT: this is how it originally was. But iter->first doesnt exist
      //outside of the for loop, so I set it as a variable and erased it afterward. Not sure if this is right.

      // since we read through an entire file when parsing we could have looked up an actor with multiple moves but based on 
      // the logging guidelines I think we should send each the actor and each of its movies in different messages
        //std::cout << "\n" << " PUBLISHING: " << entry.actor << "TO: " << ReverseLookup(KeyStoringNode) << "\n";

     //  std::cout << "\n" << " SIZE OF MOVIES TO PUBLISH: " << entry.movies.size() << "\n";
      
      for(unsigned int i = 0; i < entry.movies.size(); i++){

        PennSearchMessage message = PennSearchMessage (PennSearchMessage::PUBLISH_REQ, txnid);
        message.SetPublishReq(entry.actor, entry.movies[i]); // TO DO: Add setter methods
        packet->AddHeader (message);
        m_socket->SendTo (packet, 0 , InetSocketAddress (KeyStoringNode, m_appPort));
        // Whenever a node publishes a new invested list entry (an entry is a key, value pair) GraderLoggs::GetPublishLogStr() should be called
        //(std::string keyword, std::string docID)
        // I assume its the actor since we are passing a string as the key not the hashed value but we can always update
     // std::cout << "\n" << " PUBLISHING: " << entry.actor << "TO: " << ReverseLookup(KeyStoringNode) << "\n";
     // std::cout << "\n" << " HERE 1" << "\n";

        //GraderLogs::GetPublishLogStr(entry.actor, entry.movies[i]);
      }
        m_publishTracker.erase(transactionid);
  }

   
   }

//node receiving ProcessSearchREQ will add list of movies for a key/actor and then either send back to the requester node if there are no more keys or do the lookup for next key
void
PennSearch::ProcessSearchREQ (PennSearchMessage mes)
{
SearchEntry ent ;   
  std::vector<std::string> actor_movies;
  std::vector<std::string> actors = mes.GetTerms();
  std::vector<std::string> movies = mes.GetMovies();
  ent.RequesterNode = mes.GetRequesterNode();
  //entry.lookupHopCount = mes.GetLookupHopCount ();
  uint32_t txn_id = mes.GetTransactionId();
  //ent.terms = actors;
  //ent.movies = movies;
   bool continueOn = true;
  
  if (actors.size()>0) {


  std::string targetKeyword = actors[0];


   //////////////////Debugging PRINT
  //  PRINT_LOG("line618 target keyword: "<< targetKeyword<< "mInvertedlist size: "<<m_invertedList.size());
   std::string ac ;
  for (auto iter = m_invertedList.begin(); iter !=m_invertedList.end(); iter++)
  {
    ac = iter->first;
   if (ac == targetKeyword) 
   {
    std::vector<std::string> act = iter->second;
    for (unsigned i =0; i<act.size(); i++){
      actor_movies.push_back(act[i]);
     //std::cout << "/n" << " Push to actor_movies: " << act[i] << "\n";

    }
    break;
   }
  }


  //if any of the actor has zero movies, we can cut short and return back 
 
  if (actor_movies.size() == 0)
  {
    //send "no result" to originator/requester node
   // std::cout << "/n" << " no result 738" << "\n";

     std::string no_results = "no result";
     SendPing(ReverseLookup(ent.RequesterNode), no_results);    
        Ptr<Packet> packet = Create<Packet> (); 
   PennSearchMessage message = PennSearchMessage (PennSearchMessage::SEARCH_RSP, txn_id);    
   message.SetSearchRsp(ent.RequesterNode, ent.movies, ent.termsCount);
   packet->AddHeader (message);
   m_socket->SendTo (packet, 0 , InetSocketAddress (ent.RequesterNode, m_appPort));  
  }
  
  else{
/////////////////Debugging PRINT

    // PRINT_LOG("line 663 actor and actor_movie size: " << ac<<" : "<<"targetKeyword: " << targetKeyword<< " ; " <<actor_movies.size());
     //for (unsigned i=0; i<actor_movies.size(); i++)
     //{PRINT_LOG(actor_movies[i]);}
     std::sort(actor_movies.begin(), actor_movies.end());

     if (movies.size() ==0){

       for (unsigned i =0; i<actor_movies.size(); i++){
         ent.movies.push_back(actor_movies[i]);     
      //std::cout << "/n" << " Movies == 0 so pushing: " << actor_movies[i] << "\n";
  
      }
     // PRINT_LOG("line 655: ent.movies size: " << ent.movies.size() << "actor_movies size: " << actor_movies.size());
     SEARCH_LOG(GraderLogs::GetInvertedListShipLogStr( targetKeyword, actor_movies));
       actor_movies.clear();
      // PRINT_LOG("line 657: ent.movies size: " << ent.movies.size() << "actor_movies size: " << actor_movies.size());
     }
     else {

      std::vector<std::string> intersectedMovies;
      std::sort(movies.begin(), movies.end());


     std::set_intersection(actor_movies.begin(), actor_movies.end(), movies.begin(), movies.end(), std::back_inserter(intersectedMovies) );


      if(intersectedMovies.size() > 0){
        for (unsigned i =0; i< intersectedMovies.size(); i++){
        ent.movies.push_back(intersectedMovies[i]);
        }
        SEARCH_LOG(GraderLogs::GetInvertedListShipLogStr( targetKeyword, actor_movies));
        actor_movies.clear();
       // PRINT_LOG("line 685 size of intersected movies: " << intersectedMovies.size());
      }
     else {
      //send "no result" to originator/requester node
    std::string no_results = "no result";
    SendPing(ReverseLookup(ent.RequesterNode), no_results);  
     continueOn = false;     
     //delete  &ent;
     ent.terms.clear();
     actors.clear();
     movies.clear();
     ent.movies.clear();
      }
     }
 if (continueOn){

//Now delete the first actor whose movies are already added in the movies list


  actors.erase(actors.begin());


//PRINT_LOG("SIZE OF ACTORS on 687:"<<actors.size());
  for (unsigned i =0; i< actors.size(); i++){
   ent.terms.push_back(actors[i]);
  
  }

  //check if there are more actors in the terms 
  //if there are more terms/actors then just get the first actor and send lookup to chord
  if (ent.terms.size() > 0)
  {
   std::string search_term = ent.terms[0];
   //PRINT_LOG("line 713 search_term: " << search_term);
   Ipv4Address nodeIp = GetLocalAddress();
   std::string lookupOriginator = ReverseLookup(nodeIp);
   //entry.lookupHopCount += 1;
  // PRINT_LOG("line 701 search_term: " << search_term<<"\n");

   auto ite = m_searchEntryTracker.find(txn_id);
   if (ite != m_searchEntryTracker.end()){
      m_searchEntryTracker.erase(ite);
  //    PRINT_LOG("line 706 entry was deleted from tracker")
    }

   m_searchEntryTracker.insert(std::make_pair (txn_id, ent));
 //  PRINT_LOG("line 726 search_term: " << search_term<<"\n");
    ent.terms.clear();
     actors.clear();
     movies.clear();
     ent.movies.clear();
//  PRINT_LOG("line 731 search_term: " << search_term<<"\n");
   PennChord::LookupType type = PennChord::LookupType::SEARCH;
   //m_lookupCount +=1;  //need to verify if this is what intended
    //std::cout << "LOOK UP FROM SEARCH 4 " << "\n";
            //std::cout << "Term to look up: " << search_term << "\n";

  //PRINT_LOG("line 737 search_term: " << search_term<<"\n");
   m_chord ->LookupFromSearch(search_term, txn_id, lookupOriginator, type);

  }
  // if there are no more terms/actors then intersect movies and send the movies list back to the Requester node
  //////////////////Debugging PRINT
 //  PRINT_LOG("line 742");
  if (ent.terms.empty())
  {
    // PRINT_LOG("line 744 ent.movies size:" << ent.movies.size());
//if the list of movies is not empty

  if (ent.movies.size() == 0)
  {
     //send "no result" to originator/requester node
    std::string no_result = "no result";
    SendPing(ReverseLookup(ent.RequesterNode), no_result);
  }
  
   Ptr<Packet> packet = Create<Packet> (); 
   PennSearchMessage message = PennSearchMessage (PennSearchMessage::SEARCH_RSP, txn_id);    
   message.SetSearchRsp(ent.RequesterNode, ent.movies, ent.termsCount);
   packet->AddHeader (message);
   m_socket->SendTo (packet, 0 , InetSocketAddress (ent.RequesterNode, m_appPort));   
  }
  } //continueON
} //else
}
}

//The requester node receives back the movies vector . It will use GraderLogs function to print the movies
void
PennSearch::ProcessSearchRSP (PennSearchMessage message)
{

 // std::cout << "search complete";
  std::vector<std::string> movies = message.GetMovies(); 
// PRINT_LOG("Debugging on line 768:");
// for (unsigned int i =0; i<movies.size(); i++)
// {
//   PRINT_LOG(movies[i]);
// }
  SEARCH_LOG(GraderLogs::GetSearchResultsLogStr(message.GetRequesterNode(), movies));
  }

//*********************************************************************

void
PennSearch::SendPing (std::string nodeId, std::string pingMessage)
{
  // Send Ping Via-Chord layer 
  // SEARCH_LOG ("Sending Ping via Chord Layer to node: " << nodeId << " Message: " << pingMessage);
  Ipv4Address destAddress = ResolveNodeIpAddress(nodeId);
  m_chord->SendPing (destAddress, pingMessage);
}

void
PennSearch::SendPennSearchPing (Ipv4Address destAddress, std::string pingMessage)
{
  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = GetNextTransactionId ();
      // SEARCH_LOG ("Sending PING_REQ to Node: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << pingMessage << " transactionId: " << transactionId);
      Ptr<PingRequest> pingRequest = Create<PingRequest> (transactionId, Simulator::Now(), destAddress, pingMessage);
      // Add to ping-tracker
      m_pingTracker.insert (std::make_pair (transactionId, pingRequest));
      Ptr<Packet> packet = Create<Packet> ();
      PennSearchMessage message = PennSearchMessage (PennSearchMessage::PING_REQ, transactionId);
      message.SetPingReq (pingMessage);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
}

void
PennSearch::SendInvertedList (Ipv4Address destAddress)
{
      uint32_t transactionId = GetNextTransactionId ();
      Ptr<Packet> packet = Create<Packet> ();
      PennSearchMessage message = PennSearchMessage (PennSearchMessage::INVERTED_LIST, transactionId); 
      message.SetInvertedList (m_invertedList);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
}



void 
PennSearch::ProcessInvertedList(PennSearchMessage msg, Ipv4Address sourceAddress)
{
  std::map<std::string, std::vector<std::string>> invertedList = msg.GetInvertedList().invertedList;
  std::string originator = ReverseLookup(sourceAddress);
  // std::cout << "RECEIVED INVERTED LIST FROM NODE: " << originator << "\n";

  // std::cout << "SIZE OF INVERTED LIST BEFORE: " << m_invertedList.size() << "\n";

  for (auto iter = invertedList.begin(); iter !=invertedList.end(); iter++)
  {

    if (m_invertedList.find(iter->first) != m_invertedList.end()) {
        
          std::vector<std::string> values = iter->second;

          for (int i = 0; i < int(values.size()); i++) {

                if (std::find(iter->second.begin(), iter->second.end(), values[i]) == iter->second.end()) {
                        m_invertedList[iter->first].push_back(values[i]);
                }
          }
    }
    else {
        m_invertedList.insert(std::make_pair(iter->first, iter->second));
    }
    std::vector<std::string> movies = iter->second;
    //SEARCH_LOG("From LEAVING Node!");
    // When key leaves another node it should trigger GetStoreLogStr
    for(unsigned int i = 0; i < movies.size(); i++){
      SEARCH_LOG(GraderLogs::GetStoreLogStr(iter->first, movies[i]));
    }

  }
    // std::cout << "SIZE OF INVERTED LIST AFTER: " << m_invertedList.size() << "\n";

}

void
PennSearch::RecvMessage (Ptr<Socket> socket)
{
  Address sourceAddr;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
  Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
  uint16_t sourcePort = inetSocketAddr.GetPort ();
  PennSearchMessage message;
  packet->RemoveHeader (message);

  switch (message.GetMessageType ())
    {
      case PennSearchMessage::PING_REQ:
        ProcessPingReq (message, sourceAddress, sourcePort);
        break;
      case PennSearchMessage::PING_RSP:
        ProcessPingRsp (message, sourceAddress, sourcePort);
        break;
      //*************************MS-2********************************
      case PennSearchMessage::SEARCH_REQ_LOOKUP:
        ProcessSearchREQLookup (message);
        break;
      case PennSearchMessage::SEARCH_LOOKUP_RSP:
        ProcessSearchLookupRSP (message);
        break;
      case PennSearchMessage::SEARCH_REQ:
        ProcessSearchREQ (message);
        break;
      case PennSearchMessage::SEARCH_RSP:
        ProcessSearchRSP (message);
        break;
      case PennSearchMessage::PUBLISH_REQ:
        ProcessPublishReq (message);
        break;
      case PennSearchMessage::INVERTED_LIST:
        ProcessInvertedList(message, sourceAddress);
        break;
      case PennSearchMessage::KEY_TRANSFER_REJOIN:
        ProcessKeyTransferRejoin(message);
        break;
        
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}

void
PennSearch::ProcessPingReq (PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    // Use reverse lookup for ease of debug
    std::string fromNode = ReverseLookup (sourceAddress);
    // SEARCH_LOG ("Received PING_REQ, From Node: " << fromNode << ", Message: " << message.GetPingReq().pingMessage);
    // Send Ping Response
    PennSearchMessage resp = PennSearchMessage (PennSearchMessage::PING_RSP, message.GetTransactionId());
    resp.SetPingRsp (message.GetPingReq().pingMessage);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
}

void
PennSearch::ProcessPingRsp (PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // Remove from pingTracker
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  iter = m_pingTracker.find (message.GetTransactionId ());
  if (iter != m_pingTracker.end ())
    {
      std::string fromNode = ReverseLookup (sourceAddress);
      // SEARCH_LOG ("Received PING_RSP, From Node: " << fromNode << ", Message: " << message.GetPingRsp().pingMessage);
      m_pingTracker.erase (iter);
    }
  else
    {
      // DEBUG_LOG ("Received invalid PING_RSP!");
    }
}

// node receiving ProcessPublishReq will add list of movies for a actor updating m_invertedList
void
PennSearch::ProcessPublishReq (PennSearchMessage message){
  // Whenever a node (that the keyword is hashed to) received a new inverted list entry to be stored the following should be called GetStoreLogStr()
  //GetStoreLogStr(std::string keyword, std::string docID)
  std::string actor = message.GetPublishReq().actor;
  std::string movie = message.GetPublishReq().movie;
  SEARCH_LOG(GraderLogs::GetStoreLogStr(actor, movie));
  std::string currentNode = ReverseLookup (GetLocalAddress());
  // SEARCH_LOG ("Publishing to Node: " << currentNode << ", Actor: " << actor << ", Movie: " << movie);
  // check if actor is already in inverted list
  if (m_invertedList.find(actor) != m_invertedList.end())
  {
    // if it is append to doc/movie to list
    m_invertedList[actor].push_back(movie);
  }
  else{
    // if not create vector with doc/movie
    std::vector<std::string> docList;
    docList.push_back(movie);
    m_invertedList.insert(std::pair<std::string,std::vector<std::string>>(actor, docList));
  }
 // std::cout << "\n" << actor << " PUBLISHED TO THIS NODE: " <<   ReverseLookup(GetLocalAddress()) << "\n";

}

void
PennSearch::AuditPings ()
{
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  for (iter = m_pingTracker.begin () ; iter != m_pingTracker.end();)
    {
      Ptr<PingRequest> pingRequest = iter->second;
      if (pingRequest->GetTimestamp().GetMilliSeconds() + m_pingTimeout.GetMilliSeconds() <= Simulator::Now().GetMilliSeconds())
        {
          // DEBUG_LOG ("Ping expired. Message: " << pingRequest->GetPingMessage () << " Timestamp: " << pingRequest->GetTimestamp().GetMilliSeconds () << " CurrentTime: " << Simulator::Now().GetMilliSeconds ());
          // Remove stale entries
          m_pingTracker.erase (iter++);
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
PennSearch::GetNextTransactionId ()
{
  return m_currentTransactionId++;
}

// Handle Chord Callbacks

void
PennSearch::HandleChordPingFailure (Ipv4Address destAddress, std::string message)
{
  SEARCH_LOG ("Chord Ping Expired! Destination nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);
}

void
PennSearch::HandleChordPingSuccess (Ipv4Address destAddress, std::string message)
{
  SEARCH_LOG ("Chord Ping Success! Destination nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);
  // Send ping via search layer 
  SendPennSearchPing (destAddress, message);
}

void
PennSearch::HandleChordPingRecv (Ipv4Address destAddress, std::string message)
{
  SEARCH_LOG ("Chord Layer Received Ping! Source nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);
}

void 
PennSearch::HandleLeavingApplication(Ipv4Address successorIp) {
  //SEARCH_LOG("execute HandleLeavingApplication");
    SendInvertedList(successorIp);

    // std::cout << "Leaving network. Sending Inverted List to: " << ReverseLookup(successorIp) << "\n";

}

void 
PennSearch::HandleRejoiningApplication(Ipv4Address newNodeIp, Ipv4Address successorIp) {
  //SEARCH_LOG("execute HandleRejoiningApplication");
  Ipv4Address nodeIp = GetLocalAddress();
  std::string thisNode = ReverseLookup(nodeIp);
  std::string succNode = ReverseLookup(successorIp);
  if (thisNode == succNode) {
    //SEARCH_LOG("thisnode == succ node in HandleRejoiningApplication");
    if (m_invertedList.size() > 0) {
    std::map<std::string, std::vector<std::string>> invertedListToNewNode;
    for (auto iter = m_invertedList.begin(); iter !=m_invertedList.end(); iter++) {
      if (!IsKeyOwnedByMySuccessor(iter->first, newNodeIp, successorIp)) {
        invertedListToNewNode.insert(std::make_pair(iter->first, iter->second));
      }
    }
    uint32_t transactionId = GetNextTransactionId ();
    Ptr<Packet> packet = Create<Packet> ();
    PennSearchMessage message = PennSearchMessage (PennSearchMessage::INVERTED_LIST, transactionId); 
    message.SetInvertedList (invertedListToNewNode);
    packet->AddHeader (message);
    m_socket->SendTo (packet, 0 , InetSocketAddress (newNodeIp, m_appPort));
    }
  } else {
    //SEARCH_LOG("need to redirect to successor node");
    uint32_t transactionId = GetNextTransactionId ();
    Ptr<Packet> packet = Create<Packet> ();
    PennSearchMessage message = PennSearchMessage (PennSearchMessage::KEY_TRANSFER_REJOIN, transactionId); 
    message.SetKeyTransferRejoin(newNodeIp);
    packet->AddHeader (message);
    m_socket->SendTo (packet, 0 , InetSocketAddress (successorIp, m_appPort));
  }
}

void 
PennSearch::ProcessKeyTransferRejoin(PennSearchMessage msg) {
  Ipv4Address newNodeIp = msg.GetNewNode();
  Ipv4Address nodeIp = GetLocalAddress(); //newly joined node's successor IP

  if (m_invertedList.size() > 0) {
    std::map<std::string, std::vector<std::string>> invertedListToNewNode;
    for (auto iter = m_invertedList.begin(); iter !=m_invertedList.end(); iter++) {
      if (!IsKeyOwnedByMySuccessor(iter->first, newNodeIp, nodeIp)) {
        invertedListToNewNode.insert(std::make_pair(iter->first, iter->second));
      }
    }
    uint32_t transactionId = GetNextTransactionId ();
    Ptr<Packet> packet = Create<Packet> ();
    PennSearchMessage message = PennSearchMessage (PennSearchMessage::INVERTED_LIST, transactionId); 
    message.SetInvertedList (invertedListToNewNode);
    packet->AddHeader (message);
    m_socket->SendTo (packet, 0 , InetSocketAddress (newNodeIp, m_appPort));
  }
}

bool
PennSearch::IsKeyOwnedByMySuccessor(std::string key, Ipv4Address newNodeIp, Ipv4Address successorIp) {

  uint32_t hashed_key = PennKeyHelper::CreateShaKey(key);
  uint32_t thisNodeHash = PennKeyHelper::CreateShaKey(newNodeIp);
  uint32_t successorHash = PennKeyHelper::CreateShaKey((successorIp));

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
    if ((hashed_key <= (uint32_t(std::pow(2, 32)) - 1) && hashed_key > thisNodeHash) || (hashed_key <= successorHash))  {        
      return true;
    }
    else  {
      return false;
    }
  }
}

// Override PennLog

void
PennSearch::SetTrafficVerbose (bool on)
{ 
  m_chord->SetTrafficVerbose (on);
  g_trafficVerbose = on;
}

void
PennSearch::SetErrorVerbose (bool on)
{ 
  m_chord->SetErrorVerbose (on);
  g_errorVerbose = on;
}

void
PennSearch::SetDebugVerbose (bool on)
{
  m_chord->SetDebugVerbose (on);
  g_debugVerbose = on;
}

void
PennSearch::SetStatusVerbose (bool on)
{
  m_chord->SetStatusVerbose (on);
  g_statusVerbose = on;
}

void
PennSearch::SetChordVerbose (bool on)
{
  m_chord->SetChordVerbose (on);
  g_chordVerbose = on;
}

void
PennSearch::SetSearchVerbose (bool on)
{
  m_chord->SetSearchVerbose (on);
  g_searchVerbose = on;
}





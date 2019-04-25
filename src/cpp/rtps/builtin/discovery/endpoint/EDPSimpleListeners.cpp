// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file EDPSimpleListener.cpp
 *
 */

#include "EDPSimpleListeners.h"

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/history/WriterHistory.h>

#include <fastrtps/rtps/common/InstanceHandle.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

void EDPSimplePUBListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->publications_reader_.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }

    ReaderHistory* reader_history =
#if HAVE_SECURITY
        reader == sedp_->publications_secure_reader_.first ?
        sedp_->publications_secure_reader_.second :
#endif
        sedp_->publications_reader_.second;

    if(change->kind == ALIVE)
    {
        //LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
        WriterProxyData writerProxyData;
        CDRMessage_t tempMsg(change_in->serializedPayload);

        if(writerProxyData.readFromCDRMessage(&tempMsg))
        {
            change->instanceHandle = writerProxyData.key();
            if(writerProxyData.guid().guidPrefix == sedp_->mp_RTPSParticipant->getGuid().guidPrefix)
            {
                logInfo(RTPS_EDP,"Message from own RTPSParticipant, ignoring");
                reader_history->remove_change(change);
                return;
            }


            std::vector<std::string> names= this->sedp_->mp_PDP->getRTPSParticipant()->getParticipantNames();

            std::string this_name = names.front();
            std::string topic_name = writerProxyData.topicName();

            std::cout<<"PUB MESSAGE Activating: " << writerProxyData.guid().entityId << " in topic " << topic_name << std::endl;
            std::cout<<"PARTICIPANT PROXY DATA: "<< this_name<<std::endl;

            std::set<std::pair<std::string, std::string>> whitelist;

            // this is the PUBLISHER listener
            // create a list of pairs of nodes and topics they subscribe to
            // e.g. lyon subscribes to amazon, so has to accept publishers related to amazon
            // note that the ROS topic names are prepended with "rt/" by the DDS
            whitelist.insert(std::make_pair("lyon", "rt/amazon"));
            whitelist.insert(std::make_pair("hamburg", "rt/nile"));
            whitelist.insert(std::make_pair("hamburg", "rt/tigris"));
            whitelist.insert(std::make_pair("hamburg", "rt/ganges"));
            whitelist.insert(std::make_pair("hamburg", "rt/danube"));
            whitelist.insert(std::make_pair("osaka", "rt/parana"));
            whitelist.insert(std::make_pair("mandalay", "rt/salween"));
            whitelist.insert(std::make_pair("mandalay", "rt/danube"));
            whitelist.insert(std::make_pair("ponce", "rt/missouri"));
            whitelist.insert(std::make_pair("ponce", "rt/danube"));
            whitelist.insert(std::make_pair("ponce", "rt/volga"));
            whitelist.insert(std::make_pair("barcelona", "rt/mekong"));
            whitelist.insert(std::make_pair("georgetown", "rt/lena"));
            whitelist.insert(std::make_pair("geneva", "rt/congo"));
            whitelist.insert(std::make_pair("geneva", "rt/danube"));
            whitelist.insert(std::make_pair("geneva", "rt/parana"));
            whitelist.insert(std::make_pair("arequipa", "rt/arkansas"));

            std::set<std::pair<std::string, std::string>> t_list;

            // augment the list with additional topics for intraprocess
            // NOTE: if you only have intraprocess communication,
            // you could directly use only the "/_intra" topics
            for (auto p : whitelist){
                t_list.insert(std::make_pair(p.first, p.second));
                std::string intra_name = std::string(p.second) + std::string("/_intra");
                t_list.insert(std::make_pair(p.first, intra_name));
            }

            whitelist = t_list;

            if (whitelist.find(std::make_pair(this_name, topic_name)) == whitelist.end()){
                std::cout<<"DISCARDED topic"<<std::endl;
                return;
            }

            std::cout<<"APPROVED topic"<<std::endl;

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData pdata;
            if(this->sedp_->mp_PDP->addWriterProxyData(&writerProxyData, pdata)) //ADDED NEW DATA
            {
                // At this point we can release reader lock, cause change is not used
                reader->getMutex()->unlock();

                sedp_->pairing_writer_proxy_with_any_local_reader(&pdata, &writerProxyData);

                // Take again the reader lock.
                reader->getMutex()->lock();
            }
            else //NOT ADDED BECAUSE IT WAS ALREADY THERE
            {
                logWarning(RTPS_EDP,"Received message from UNKNOWN RTPSParticipant, removing");
            }
        }
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP,"Disposed Remote Writer, removing...");

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->sedp_->mp_PDP->removeWriterProxyData(auxGUID);
    }

    //Removing change from history
    reader_history->remove_change(change);

    return;
}

bool EDPSimplePUBListener::computeKey(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_ENDPOINT_GUID);
}

bool EDPSimpleSUBListener::computeKey(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_ENDPOINT_GUID);
}

void EDPSimpleSUBListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    CacheChange_t* change = (CacheChange_t*)change_in;
    //std::lock_guard<std::recursive_mutex> guard(*this->sedp_->subscriptions_reader_.first->getMutex());
    logInfo(RTPS_EDP,"");
    if(!computeKey(change))
    {
        logWarning(RTPS_EDP,"Received change with no Key");
    }

    ReaderHistory* reader_history =
#if HAVE_SECURITY
        reader == sedp_->subscriptions_secure_reader_.first ?
        sedp_->subscriptions_secure_reader_.second :
#endif
        sedp_->subscriptions_reader_.second;

    if(change->kind == ALIVE)
    {
        //LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
        ReaderProxyData readerProxyData;
        CDRMessage_t tempMsg(change_in->serializedPayload);

        if(readerProxyData.readFromCDRMessage(&tempMsg))
        {
            change->instanceHandle = readerProxyData.key();
            if(readerProxyData.guid().guidPrefix == sedp_->mp_RTPSParticipant->getGuid().guidPrefix)
            {
                logInfo(RTPS_EDP,"From own RTPSParticipant, ignoring");
                reader_history->remove_change(change);
                return;
            }

            std::vector<std::string> names= this->sedp_->mp_PDP->getRTPSParticipant()->getParticipantNames();

            std::string this_name = names.front();
            std::string topic_name = readerProxyData.topicName();

            std::cout<<"SUB MESSAGE Activating: " << readerProxyData.guid().entityId << " in topic " << topic_name << std::endl;
            std::cout<<"PARTICIPANT PROXY DATA: "<< this_name<<std::endl;

            std::set<std::pair<std::string, std::string>> whitelist;

            // this is the SUBSCRIBER listener
            // create a list of pairs of nodes and topics they publish to
            // e.g. montreal publishes to amazon, so has to accept subscribers related to amazon
            // note that the ROS topic names are prepended with "rt/" by the DDS
            whitelist.insert(std::make_pair("montreal", "rt/amazon"));
            whitelist.insert(std::make_pair("montreal", "rt/nile"));
            whitelist.insert(std::make_pair("montreal", "rt/ganges"));
            whitelist.insert(std::make_pair("montreal", "rt/danube"));
            whitelist.insert(std::make_pair("lyon", "rt/tigris"));
            whitelist.insert(std::make_pair("hamburg", "rt/parana"));
            whitelist.insert(std::make_pair("osaka", "rt/salween"));
            whitelist.insert(std::make_pair("mandalay", "rt/missouri"));
            whitelist.insert(std::make_pair("ponce", "rt/mekong"));
            whitelist.insert(std::make_pair("ponce", "rt/congo"));
            whitelist.insert(std::make_pair("barcelona", "rt/lena"));
            whitelist.insert(std::make_pair("georgetown", "rt/volga"));
            whitelist.insert(std::make_pair("geneva", "rt/arkansas"));

            std::set<std::pair<std::string, std::string>> t_list;

            // augment the list with additional topics for intraprocess
            // NOTE: if you only have intraprocess communication,
            // you could directly use only the "/_intra" topics
            for (auto p : whitelist){
                t_list.insert(std::make_pair(p.first, p.second));
                std::string intra_name = std::string(p.second) + std::string("/_intra");
                t_list.insert(std::make_pair(p.first, intra_name));
            }

            whitelist = t_list;

            if (whitelist.find(std::make_pair(this_name, topic_name)) == whitelist.end()){
                std::cout<<"DISCARDED topic"<<std::endl;
                return;
            }

            std::cout<<"APPROVED topic"<<std::endl;

            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData pdata;
            if(this->sedp_->mp_PDP->addReaderProxyData(&readerProxyData, pdata)) //ADDED NEW DATA
            {
                // At this point we can release reader lock, cause change is not used
                reader->getMutex()->unlock();

                sedp_->pairing_reader_proxy_with_any_local_writer(&pdata, &readerProxyData);

                // Take again the reader lock.
                reader->getMutex()->lock();
            }
            else
            {
                logWarning(RTPS_EDP,"From UNKNOWN RTPSParticipant, removing");
            }
        }
    }
    else
    {
        //REMOVE WRITER FROM OUR READERS:
        logInfo(RTPS_EDP,"Disposed Remote Reader, removing...");

        GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
        this->sedp_->mp_PDP->removeReaderProxyData(auxGUID);
    }

    // Remove change from history.
    reader_history->remove_change(change);

    return;
}

void EDPSimplePUBListener::onWriterChangeReceivedByAll(RTPSWriter* writer, CacheChange_t* change)
{
    (void)writer;

    if(ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
#if HAVE_SECURITY
            writer == sedp_->publications_secure_writer_.first ?
            sedp_->publications_secure_writer_.second :
#endif
            sedp_->publications_writer_.second;

        writer_history->remove_change(change);
    }
}

void EDPSimpleSUBListener::onWriterChangeReceivedByAll(RTPSWriter* writer, CacheChange_t* change)
{
    (void)writer;

    if(ChangeKind_t::NOT_ALIVE_DISPOSED_UNREGISTERED == change->kind)
    {
        WriterHistory* writer_history =
#if HAVE_SECURITY
            writer == sedp_->subscriptions_secure_writer_.first ?
            sedp_->subscriptions_secure_writer_.second :
#endif
            sedp_->subscriptions_writer_.second;

        writer_history->remove_change(change);
    }

}

} /* namespace rtps */
}
} /* namespace eprosima */

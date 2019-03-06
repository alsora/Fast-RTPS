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

/*************************************************************************
 * @file deadlinepayloadSubscriber.h
 * This header file contains the declaration of the subscriber functions.
 *
 * This file was generated by the tool fastcdrgen.
 */


#ifndef _DEADLINEPAYLOAD_SUBSCRIBER_H_
#define _DEADLINEPAYLOAD_SUBSCRIBER_H_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include "deadlinepayloadPubSubTypes.h"

#include "mapableKey.h"

class deadlinepayloadSubscriber
{
public:

    /**
     * @brief Constructor
     */
    deadlinepayloadSubscriber();

    /**
     * @brief Destructor
     */
    virtual ~deadlinepayloadSubscriber();

    /**
     * @brief Initialises the subscriber
     * @param deadline_ms The deadline period in milliseconds
     * @return True if initialised correctly
     */
    bool init(double deadline_ms);

    /**
     * @brief Runs the subscriber
     */
    void run();

private:
    eprosima::fastrtps::Participant *mp_participant;
    eprosima::fastrtps::Subscriber *mp_subscriber;
    HelloMsgPubSubType myType;
    class SubListener : public eprosima::fastrtps::SubscriberListener
    {
    public:
        SubListener() : n_matched(0),n_msg(0){};
        ~SubListener(){};
        void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info) override;
        void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override;
        void on_requested_deadline_missed(eprosima::fastrtps::rtps::InstanceHandle_t& handle) override;
        eprosima::fastrtps::SampleInfo_t m_info;
        int n_matched;
        int n_msg;
    } m_listener;
};

#endif // _deadlinepayload_SUBSCRIBER_H_

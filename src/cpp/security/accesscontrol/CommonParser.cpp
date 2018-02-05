// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "CommonParser.h"

#include <fastrtps/log/Log.h>

#include <cassert>

static const char* DomainId_str = "id";
static const char* DomainIdRange_str = "id_range";
static const char* Min_str = "min";
static const char* Max_str = "max";

using namespace eprosima::fastrtps;
using namespace ::rtps::security;

bool eprosima::fastrtps::rtps::security::parse_domain_id_set(tinyxml2::XMLElement* root, Domains& domains)
{
    assert(root);

    bool returned_value = false;
    tinyxml2::XMLElement* node = root->FirstChildElement();

    if(node != nullptr)
    {
        returned_value = true;

        do
        {
            if(strcmp(node->Name(), DomainId_str) == 0)
            {
                uint32_t domain_id = 0;

                if(tinyxml2::XMLError::XML_SUCCESS == node->QueryUnsignedText(&domain_id))
                {
                    domains.ranges.push_back(std::make_pair(domain_id, 0));
                }
                else
                {
                    logError(XMLPARSER, "Invalid value of " << DomainId_str <<
                            " tag. Line " << node->GetLineNum());
                    returned_value = false;
                }
            }
            else if(strcmp(node->Name() ,DomainIdRange_str) == 0)
            {
                tinyxml2::XMLElement* subnode = node->FirstChildElement();

                if(subnode != nullptr)
                {
                    uint32_t min_domain_id = 0;

                    if(strcmp(subnode->Name(), Min_str) == 0)
                    {
                        if(tinyxml2::XMLError::XML_SUCCESS != subnode->QueryUnsignedText(&min_domain_id))
                        {
                            logError(XMLPARSER, "Invalid value of " << DomainId_str <<
                                    " tag. Line " << subnode->GetLineNum());
                            returned_value = false;
                        }
                    }
                    else
                    {
                        logError(XMLPARSER, "Expected " << Min_str << " tag. Line " <<
                                subnode->GetLineNum());
                        returned_value = false;
                    }

                    if(returned_value && (subnode = subnode->NextSiblingElement()) != nullptr)
                    {
                        if(strcmp(subnode->Name(), Max_str) == 0)
                        {
                            uint32_t max_domain_id = 0;

                            if(tinyxml2::XMLError::XML_SUCCESS == subnode->QueryUnsignedText(&max_domain_id))
                            {
                                domains.ranges.push_back(std::make_pair(min_domain_id, max_domain_id));
                            }
                            else
                            {
                                logError(XMLPARSER, "Invalid value of " << DomainId_str <<
                                        " tag. Line " << subnode->GetLineNum());
                                returned_value = false;
                            }
                        }
                        else
                        {
                            logError(XMLPARSER, "Expected " << Max_str << " tag. Line " <<
                                    subnode->GetLineNum());
                            returned_value = false;
                        }
                    }
                    else
                    {
                        logError(XMLPARSER, "Expected " << Max_str << " tag. Line " <<
                                node->GetLineNum());
                        returned_value = false;
                    }
                }
                else
                {
                    logError(XMLPARSER, "Expected " << Min_str << " and " << Max_str << " tags. Line " <<
                            node->GetLineNum() + 1);
                    returned_value = false;
                }
            }
            else
            {
                logError(XMLPARSER, "Not valid tag. Expected " << DomainId_str << " or " << DomainIdRange_str <<
                        " tag. Line " << node->GetLineNum());
                returned_value = false;
            }
        }
        while(returned_value && (node = node->NextSiblingElement()) != nullptr);
    }
    else
    {
        logError(XMLPARSER, "Minimum one " << DomainId_str << " or " << DomainIdRange_str << " tag. Line " <<
                root->GetLineNum() + 1);
    }

    return returned_value;
}


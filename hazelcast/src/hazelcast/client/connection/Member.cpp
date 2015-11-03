/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/29/13.



#include "hazelcast/client/Member.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {

        Member::Member(std::auto_ptr<Address> address, std::auto_ptr<std::string> uuid,
                       std::auto_ptr<std::map<std::string, std::string> > attributes, bool lite) :
                address(address), uuid(uuid), attributes(attributes), liteMember(lite) {
        }

        Member::Member() :
                address(new Address), uuid(new std::string), attributes(new std::map<std::string, std::string>){
        }

        Member::Member(const Member &rhs) : address(new Address(rhs.getAddress())),
                                            uuid(new std::string(rhs.getUuid())),
                                            attributes(new std::map<std::string, std::string >(*rhs.attributes)) {
        }

        Member &Member::operator=(const Member &rhs) {
            address = std::auto_ptr<Address>(new Address(rhs.getAddress()));
            uuid = std::auto_ptr<std::string>(new std::string(rhs.getUuid())) ;
            attributes = std::auto_ptr<std::map<std::string, std::string > > (
                    new std::map<std::string, std::string >(*rhs.attributes));

            return *this;
        }

        bool Member::operator ==(const Member &rhs) const {
            return *address == *rhs.address;
        }

        const Address &Member::getAddress() const {
            return *address;
        }

        const std::string &Member::getUuid() const {
            return *uuid;
        }

        int Member::getFactoryId() const {
            return protocol::ProtocolConstants::DATA_FACTORY_ID;
        }

        int Member::getClassId() const {
            return protocol::ProtocolConstants::MEMBER_ID;
        }

        bool Member::isLiteMember() const {
            return liteMember;
        }

        std::ostream &operator <<(std::ostream &stream, const Member &member) {
            return stream << "Member[" << member.getAddress() << "]";
        }

        const std::string &Member::getAttribute(const std::string &key) const {
            return (*attributes)[key];
        }

        bool Member::lookupAttribute(const std::string &key) const {
            return (*attributes).find(key) != (*attributes).end();
        }

        void Member::setAttribute(const std::string &key, const std::string &value) {
            (*attributes)[key] = value;
        }

        bool Member::removeAttribute(const std::string &key) {
            return 0 != (*attributes).erase(key);
        }
    }
}


/*
 * Copyright 2014-2016 CyberVision, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * NOTE: This is a auto-generated file. Do not edit it.
 */

#include "kaa/KaaDefaults.hpp"

#include <algorithm>
#include <cstdint>
#include <sstream>

#include "kaa/channel/GenericTransportInfo.hpp"

namespace kaa {

const char * const BUILD_VERSION = "0.9.0";

const char * const BUILD_COMMIT_HASH = "";

const char * const SDK_TOKEN = "ZJyeFywCv7lv7XITXlwHpkZwFMo";

const std::uint32_t POLLING_PERIOD_SECONDS = 5;

const char * const CLIENT_PUB_KEY_LOCATION = "key.public";

const char * const CLIENT_PRIV_KEY_LOCATION = "key.private";

const char * const CLIENT_STATUS_FILE_LOCATION = "kaa.status";

const char * const DEFAULT_USER_VERIFIER_TOKEN = "";

ITransportConnectionInfoPtr createTransportInfo(const std::int32_t& accessPointId
                                              , const std::int32_t& protocolId
                                              , const std::int32_t& protocolVersion
                                              , const std::string& encodedConnectionData)
{
    auto buffer = Botan::base64_decode(encodedConnectionData);
    ITransportConnectionInfoPtr connectionInfo(
            new GenericTransportInfo(ServerType::BOOTSTRAP
                                   , accessPointId
                                   , TransportProtocolId(protocolId, protocolVersion)
                                   , std::vector<std::uint8_t>(buffer.begin(), buffer.end())));

    return connectionInfo;
}

const BootstrapServers& getBootstrapServers() 
{
    static BootstrapServers listOfServers;
    if (listOfServers.empty()) {
        listOfServers.push_back(createTransportInfo(0x929a2016, 0xfb9a3cf0, 1, "AAABJjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJwUuVfo0mAfeqFGtiPtzESnGUnSHSSMcQ3kxZLnPi4P8ynY83+ugFW6g/I0i/rOp3Urd9x9P9aAYZmqpg6etGlRaAm/46FPIqRsxNcTNCgf5fkbvcSRbjqI2VeFUYF32sTGqfL+VQhlXEu/z46ofbsio39xmzp59Dbz4xhuFzfNYgOQ6XI7YwwLXJBE0SQ1j4h7MXadN0jqTY50ADrxltk/+NYMlDljlEUKj81+IqEAKwxr0gkUcqt6nghpZxLy3MTpdkmH9YMoP0+RON1y8OBnVJqwPpWETmw5Jzapqe4inY4wyo/MEfTfN5xk2LioBbG6YLHgbiApIWON0cp4yHMCAwEAAQAAAAlsb2NhbGhvc3QAACah"));
listOfServers.push_back(createTransportInfo(0x929a2016, 0x56c8ff92, 1, "AAABJjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJwUuVfo0mAfeqFGtiPtzESnGUnSHSSMcQ3kxZLnPi4P8ynY83+ugFW6g/I0i/rOp3Urd9x9P9aAYZmqpg6etGlRaAm/46FPIqRsxNcTNCgf5fkbvcSRbjqI2VeFUYF32sTGqfL+VQhlXEu/z46ofbsio39xmzp59Dbz4xhuFzfNYgOQ6XI7YwwLXJBE0SQ1j4h7MXadN0jqTY50ADrxltk/+NYMlDljlEUKj81+IqEAKwxr0gkUcqt6nghpZxLy3MTpdkmH9YMoP0+RON1y8OBnVJqwPpWETmw5Jzapqe4inY4wyo/MEfTfN5xk2LioBbG6YLHgbiApIWON0cp4yHMCAwEAAQAAAAlsb2NhbGhvc3QAACag"));
;
        std::random_shuffle(listOfServers.begin(), listOfServers.end());
    }
    return listOfServers;
}

const Botan::secure_vector<std::uint8_t>& getDefaultConfigData() 
{
    static const Botan::secure_vector<std::uint8_t> configData = Botan::base64_decode("AAgWS2FhIHdlYnNpdGUyaHR0cDovL3d3dy5rYWFwcm9qZWN0Lm9yZwDgAafvgXlJ47tnP9/y9SwXKkthYSBHaXRIdWIgcmVwb3NpdG9yeUJodHRwczovL2dpdGh1Yi5jb20va2FhcHJvamVjdC9rYWEAfRoanhOjRMW7CGUWV2UI8BBLYWEgZG9jc3hodHRwOi8vZG9jcy5rYWFwcm9qZWN0Lm9yZy9kaXNwbGF5L0tBQS9LYWErSW9UK1BsYXRmb3JtK0hvbWUAolyL5ma7SpKTkRndtEr5IkRLYWEgY29uZmlndXJhdGlvbiBkZXNpZ24gcmVmZXJlbmNlaGh0dHA6Ly9kb2NzLmthYXByb2plY3Qub3JnL2Rpc3BsYXkvS0FBL0NvbmZpZ3VyYXRpb24AIdlwURhpTnmVU2qBp38kggAASuddjzq2QAKGPm5e/xDX2Q==");
    return configData;
}

HashDigest getPropertiesHash()
{
    std::ostringstream ss;

    ss << SDK_TOKEN;
    ss << POLLING_PERIOD_SECONDS;
    ss << CLIENT_PUB_KEY_LOCATION;
    ss << CLIENT_PRIV_KEY_LOCATION;
    ss << CLIENT_STATUS_FILE_LOCATION;

    for (const auto& server : getBootstrapServers()) {
        const auto& connectionInfo = server->getConnectionInfo();
        ss.write(reinterpret_cast<const char*>(connectionInfo.data()), connectionInfo.size());
    }

    ss.write(reinterpret_cast<const char*>(
            getDefaultConfigData().data()), getDefaultConfigData().size());

    return EndpointObjectHash(ss.str()).getHashDigest();
}

}

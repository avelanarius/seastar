/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (C) 2019 ScyllaDB Ltd.
 */

#include "kafka_connection.hh"

namespace seastar {

namespace kafka {

future<lw_shared_ptr<kafka_connection>> kafka_connection::connect(const std::string& host, uint16_t port,
        const std::string& client_id, uint32_t timeout_ms) {
    return tcp_connection::connect(host, port, timeout_ms)
    .then([client_id] (lw_shared_ptr<tcp_connection> connection) {
        return make_lw_shared<kafka_connection>(connection, client_id);
    }).then([] (lw_shared_ptr<kafka_connection> connection) {
        return connection->init().then([connection] {
            return connection;
        });
    });
}

future<> kafka_connection::init() {
    api_versions_request request;
    return send(request, api_versions_request::MAX_SUPPORTED_VERSION)
            .then([this, request](api_versions_response response) {
                if (*response._error_code == 35) {
                    auto retry_version = api_versions_request::MIN_SUPPORTED_VERSION;
                    if (response.contains(api_versions_request::API_KEY)) {
                        retry_version = response.max_version<api_versions_request>();
                    }
                    return send(request, retry_version)
                    .then([this](api_versions_response response) {
                        if (*response._error_code != 0) {
                            throw kafka_connection_exception();
                        } else {
                            _api_versions = response;
                        }
                    });
                } else if (*response._error_code != 0) {
                    throw kafka_connection_exception();
                } else {
                    _api_versions = response;
                    return make_ready_future<>();
                }
            });
}

}

}

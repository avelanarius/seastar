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

#include "connection_manager.hh"

namespace seastar {

namespace kafka {

future<> connection_manager::connect(const std::string& host, uint16_t port) {
    return _connections.find(host) != _connections.end()
        ? make_ready_future()
        : kafka_connection::connect(host, port, _client_id, 500).then([this, host] (lw_shared_ptr<kafka_connection> conn) {
            _connections.insert(std::make_pair(host, conn));
    });
}

std::optional<lw_shared_ptr<kafka_connection>> connection_manager::get_connection(const std::string& host) {
    auto conn = _connections.find(host);
    return conn != _connections.end() ? std::make_optional(conn->second) : std::nullopt;
}

future<> connection_manager::disconnect(const std::string& host) {
    auto conn = _connections.find(host);
    if (conn == _connections.end()) {
        _connections.erase(conn);
    }

    return make_ready_future();
}

future<metadata_response> connection_manager::ask_for_metadata(const seastar::kafka::metadata_request &request) {
    return _connections.begin()->second->send(request);
}

}

}
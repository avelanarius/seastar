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

#include <sstream>
#include <vector>
#include <iostream>

#include "../protocol/kafka_primitives.hh"
#include "../protocol/metadata_request.hh"
#include "../protocol/metadata_response.hh"
#include "../protocol/api_versions_request.hh"
#include "../protocol/api_versions_response.hh"
#include "../connection/tcp_connection.hh"
#include "../protocol/produce_request.hh"
#include "../protocol/produce_response.hh"

#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#include <seastar/core/print.hh>
#include <seastar/core/thread.hh>
#include <seastar/kafka/producer/kafka_producer.hh>
#include <seastar/kafka/producer/producer_properties.hh>

namespace seastar {

namespace kafka {

kafka_producer::kafka_producer(producer_properties&& properties)
    : _properties(std::move(properties)),
      _connection_manager(make_lw_shared<connection_manager>(_properties._client_id)),
      _metadata_manager(make_lw_shared<metadata_manager>(_connection_manager)),
      _batcher(_metadata_manager, _connection_manager, _properties._retries, _properties._retry_backoff_strategy) {}

seastar::future<> kafka_producer::init() {
    std::vector<future<lw_shared_ptr<kafka_connection>>> fs;

    for (auto &server : _properties._servers) {
        fs.push_back(_connection_manager->connect(server.first, server.second, _properties._request_timeout));
    }

    return when_all_succeed(fs.begin(), fs.end()).discard_result().then([this] {
        _metadata_manager->start_refresh();
        return _metadata_manager->refresh_metadata().discard_result();
    });
}

seastar::future<> kafka_producer::produce(std::string topic_name, std::string key, std::string value) {
    return _metadata_manager->get_metadata().then([this, topic_name, key, value](metadata_response metadata){

        auto partition_index = 0;
        for (const auto& topic : *metadata._topics) {
            if (*topic._name == topic_name) {
                partition_index = *_properties._partitioner->get_partition(key, topic._partitions)._partition_index;
                break;
            }
        }

        sender_message message;
        message._topic = std::move(topic_name);
        message._key = std::move(key);
        message._value = std::move(value);
        message._partition_index = partition_index;

        auto send_future = message._promise.get_future();
        _batcher.queue_message(std::move(message));
        return send_future;
    });
}

seastar::future<> kafka_producer::flush() {
    return _batcher.flush(_properties._request_timeout);
}

seastar::future<> kafka_producer::disconnect() {
    return seastar::async({}, [this] {return _metadata_manager->stop_refresh();});
}

}

}

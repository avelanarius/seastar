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

#pragma once

#include <atomic>
#include "../protocol/metadata_response.hh"

namespace seastar {

namespace kafka {

class partitioner {

public:

    virtual metadata_response_partition get_partition(const std::string &key, const kafka_array_t<metadata_response_partition> &partitions) = 0;

};

class basic_partitioner : public partitioner {
public:
    metadata_response_partition get_partition(const std::string &key, const kafka_array_t<metadata_response_partition> &partitions) override;
};

class rr_partitioner : public partitioner {
public:
    metadata_response_partition get_partition(const std::string &key, const kafka_array_t<metadata_response_partition> &partitions) override;
private:
    uint32_t counter = 0;
};

}

}
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

#include <seastar/testing/test_case.hh>
#include <seastar/testing/test_runner.hh>
#include "../../src/kafka/connection/tcp_connection.hh"
#include "../../src/kafka/protocol/produce_request.hh"
#include "../../src/kafka/protocol/produce_response.hh"
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

using namespace seastar;

// All of the tests below assume that there is a Kafka broker running
// on address BROKER_ADDRESS
constexpr char BROKER_ADDRESS[] = "172.13.0.1:9092";

constexpr char message_str[] = "\x00\x00\x00\x0E\x00\x12\x00\x02\x00\x00\x00\x00\x00\x04\x74\x65\x73\x74";
constexpr size_t message_len = 18;

SEASTAR_TEST_CASE(kafka_establish_connection_test) {
    return kafka::tcp_connection::connect(BROKER_ADDRESS).then([] (lw_shared_ptr<kafka::tcp_connection> conn) {
       return conn->close().finally([conn] {});
    }).then_wrapped([] (auto&& f) {
        try {
            f.get();
        } catch (std::exception& ex) {
            BOOST_FAIL(ex.what());
        }
    });
}

SEASTAR_TEST_CASE(kafka_connection_write_without_errors_test) {
    temporary_buffer<char> message {message_str, message_len};

    return kafka::tcp_connection::connect(BROKER_ADDRESS).then([message = std::move(message)] (lw_shared_ptr<kafka::tcp_connection> conn) {
        return conn->write(message.clone()).then([conn] {
            return conn->close().finally([conn] {});
        });
    }).then_wrapped([] (auto&& f) {
        try {
            f.get();
        } catch (std::exception& ex) {
            BOOST_FAIL(ex.what());
        }
    });
}

SEASTAR_TEST_CASE(kafka_connection_read_without_errors_test) {
    return make_ready_future();
}

SEASTAR_TEST_CASE(kafka_connection_successful_write_read_routine_test) {
    const std::string correct_response {"\x00\x00\x01\x1C\x00\x00\x00\x00\x00\x00\x00\x00\x00\x2d\x00\x00"
                                        "\x00\x00\x00\x07\x00\x01\x00\x00\x00\x0b\x00\x02\x00\x00\x00\x05"
                                        "\x00\x03\x00\x00\x00\x08\x00\x04\x00\x00\x00\x02\x00\x05\x00\x00"
                                        "\x00\x01\x00\x06\x00\x00\x00\x05\x00\x07\x00\x00\x00\x02\x00\x08"
                                        "\x00\x00\x00\x07\x00\x09\x00\x00\x00\x05\x00\x0a\x00\x00\x00\x02"
                                        "\x00\x0b\x00\x00\x00\x05\x00\x0c\x00\x00\x00\x03\x00\x0d\x00\x00"
                                        "\x00\x02\x00\x0e\x00\x00\x00\x03\x00\x0f\x00\x00\x00\x03\x00\x10"
                                        "\x00\x00\x00\x02\x00\x11\x00\x00\x00\x01\x00\x12\x00\x00\x00\x02"
                                        "\x00\x13\x00\x00\x00\x03\x00\x14\x00\x00\x00\x03\x00\x15\x00\x00"
                                        "\x00\x01\x00\x16\x00\x00\x00\x01\x00\x17\x00\x00\x00\x03\x00\x18"
                                        "\x00\x00\x00\x01\x00\x19\x00\x00\x00\x01\x00\x1a\x00\x00\x00\x01"
                                        "\x00\x1b\x00\x00\x00\x00\x00\x1c\x00\x00\x00\x02\x00\x1d\x00\x00"
                                        "\x00\x01\x00\x1e\x00\x00\x00\x01\x00\x1f\x00\x00\x00\x01\x00\x20"
                                        "\x00\x00\x00\x02\x00\x21\x00\x00\x00\x01\x00\x22\x00\x00\x00\x01"
                                        "\x00\x23\x00\x00\x00\x01\x00\x24\x00\x00\x00\x01\x00\x25\x00\x00"
                                        "\x00\x01\x00\x26\x00\x00\x00\x01\x00\x27\x00\x00\x00\x01\x00\x28"
                                        "\x00\x00\x00\x01\x00\x29\x00\x00\x00\x01\x00\x2a\x00\x00\x00\x01"
                                        "\x00\x2b\x00\x00\x00\x00\x00\x2c\x00\x00\x00\x00\x00\x00\x00\x00",
                                        18 * 16
    };

    temporary_buffer<char> message {message_str, message_len};

    return kafka::tcp_connection::connect(BROKER_ADDRESS).then(
        [message = std::move(message), correct_response] (lw_shared_ptr<kafka::tcp_connection> conn) {
            return conn->write(message.clone()).then([conn, correct_response] {
                return conn->read(18 * 16).then([conn, correct_response] (temporary_buffer<char> buff) {
                    std::string response {buff.get(), buff.size()};
                    BOOST_CHECK_EQUAL(response, correct_response);
                    return make_ready_future();
                }).then([conn] {
                    return conn->close().finally([conn] {});
                });
            });
        }).then_wrapped([] (auto&& f) {
        try {
            f.get();
        } catch (std::exception& ex) {
            BOOST_FAIL(ex.what());
        }
    });
}

SEASTAR_TEST_CASE(kafka_send_poc) {
    std::string key_to_send = "marek";
    std::string value_to_send = "zochowsky";
    std::string topic_name = "test5";
    auto partition_num = 0;

    // Prepareing data to send

    kafka::produce_request req;
    req.set_acks(kafka::kafka_int16_t(-1));
    req.set_timeout_ms(kafka::kafka_int32_t(30000));

    kafka::produce_request_topic_produce_data topic_data;
    topic_data.set_name(kafka::kafka_string_t(topic_name));

    kafka::produce_request_partition_produce_data partition_data;
    partition_data.set_partition_index(kafka::kafka_int32_t(partition_num));

    kafka::kafka_records records;
    kafka::kafka_record_batch record_batch;

    record_batch._base_offset = 0;
    record_batch._partition_leader_epoch = -1;
    record_batch._magic = 2;
    record_batch._compression_type = kafka::kafka_record_compression_type::NO_COMPRESSION;
    record_batch._timestamp_type = kafka::kafka_record_timestamp_type::CREATE_TIME;
    record_batch._first_timestamp = 0x16e5b6eba2c; // todo it should be a real time
    record_batch._producer_id = -1;
    record_batch._producer_epoch = -1;
    record_batch._base_sequence = -1;
    record_batch._is_transactional = false;
    record_batch._is_control_batch = false;

    kafka::kafka_record record;
    record._timestamp_delta = 0;
    record._offset_delta = 0;
    record._key = key_to_send;
    record._value = value_to_send;

    record_batch._records.push_back(record);
    records._record_batches.push_back(record_batch);

    partition_data.set_records(records);

    kafka::kafka_array_t<kafka::produce_request_partition_produce_data> partitions{
            std::vector<kafka::produce_request_partition_produce_data>()};
    partitions->push_back(partition_data);
    topic_data.set_partitions(partitions);

    kafka::kafka_array_t<kafka::produce_request_topic_produce_data> topics{
            std::vector<kafka::produce_request_topic_produce_data>()};
    topics->push_back(topic_data);

    req.set_topics(topics);

    // Serialize data
    std::vector<char> serialized;
    boost::iostreams::back_insert_device<std::vector<char>> serialized_sink{serialized};
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>>> serialized_stream{serialized_sink};

    // Write request header
    kafka::kafka_int16_t api_key(0);
    kafka::kafka_int16_t api_version(7);
    kafka::kafka_int32_t correlation_id(6969);
    kafka::kafka_nullable_string_t client_id("seastar-kafka");

    api_key.serialize(serialized_stream, 0);
    api_version.serialize(serialized_stream, 0);
    correlation_id.serialize(serialized_stream, 0);
    client_id.serialize(serialized_stream, 0);

    req.serialize(serialized_stream, 7); // todo hardcoded version 7
    serialized_stream.flush();

    // Serialized data is header + produce request payload
    // But we need to prepend size

    std::vector<char> message;
    boost::iostreams::back_insert_device<std::vector<char>> message_sink{message};
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>>> message_stream{message_sink};

    kafka::kafka_int32_t message_size(serialized.size());
    message_size.serialize(message_stream, 0);
    message_stream.write(serialized.data(), serialized.size());
    message_stream.flush();

    temporary_buffer<char> message_buffer{message.data(), message.size()};

    // Sendin the data!
    return kafka::tcp_connection::connect(BROKER_ADDRESS).then(
            [message_buffer = std::move(message_buffer)] (lw_shared_ptr<kafka::tcp_connection> conn) {
                return conn->write(message_buffer.clone())
                .then([conn] {
                    return conn->read(4); // read response size
                })
                .then([conn] (temporary_buffer<char> response) {
                    boost::iostreams::stream<boost::iostreams::array_source> response_size_stream
                        (response.get(), response.size());

                    kafka::kafka_int32_t response_size;

                    response_size.deserialize(response_size_stream, 0);
                    return *response_size;
                })
                .then([conn] (int32_t response_size) {
                    return conn->read(response_size);
                })
                .then([conn] (temporary_buffer<char> response) {
                    boost::iostreams::stream<boost::iostreams::array_source> response_stream
                            (response.get(), response.size());

                    kafka::kafka_int32_t correlation_id;
                    correlation_id.deserialize(response_stream, 0);

                    printf("received correlation id of: %d\n", *correlation_id);

                    kafka::produce_response produce_response;
                    produce_response.deserialize(response_stream, 7);

                    printf("response has error code of: %d\n",
                           *produce_response.get_responses()[0].get_partitions()[0].get_error_code());

                    printf("response has base offset of: %ld\n",
                           *produce_response.get_responses()[0].get_partitions()[0].get_base_offset());
                })
                .then([conn] {
                    /*return conn->read(18 * 16).then([conn, correct_response] (temporary_buffer<char> buff) {
                        std::string response {buff.get(), buff.size()};
                        BOOST_CHECK_EQUAL(response, correct_response);
                        return make_ready_future();
                    }).then([conn] {
                        return conn->close().finally([conn] {});
                    });*/
                    return conn->close().finally([conn] {});
                });
            }).then_wrapped([] (auto&& f) {
        try {
            f.get();
        } catch (std::exception& ex) {
            BOOST_FAIL(ex.what());
        }
    });
}
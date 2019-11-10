#include "report_util.hh"
#include <map>
#include <sstream>
#include <ctime>
#include <iomanip>

std::string report_util::name_for_api_key(int16_t key) {
    static std::map<int16_t, std::string> name_mapping;
    if (name_mapping.empty()) {
        name_mapping[0] = "Produce";
        name_mapping[1] = "Fetch";
        name_mapping[2] = "ListOffsets";
        name_mapping[3] = "Metadata";
        name_mapping[4] = "LeaderAndIsr";
        name_mapping[5] = "StopReplica";
        name_mapping[6] = "UpdateMetadata";
        name_mapping[7] = "ControlledShutdown";
        name_mapping[8] = "OffsetCommit";
        name_mapping[9] = "OffsetFetch";
        name_mapping[10] = "FindCoordinator";
        name_mapping[11] = "JoinGroup";
        name_mapping[12] = "Heartbeat";
        name_mapping[13] = "LeaveGroup";
        name_mapping[14] = "SyncGroup";
        name_mapping[15] = "DescribeGroups";
        name_mapping[16] = "ListGroups";
        name_mapping[17] = "SaslHandshake";
        name_mapping[18] = "ApiVersions";
        name_mapping[19] = "CreateTopics";
        name_mapping[20] = "DeleteTopics";
        name_mapping[21] = "DeleteRecords";
        name_mapping[22] = "InitProducerId";
        name_mapping[23] = "OffsetForLeaderEpoch";
        name_mapping[24] = "AddPartitionsToTxn";
        name_mapping[25] = "AddOffsetsToTxn";
        name_mapping[26] = "EndTxn";
        name_mapping[27] = "WriteTxnMarkers";
        name_mapping[28] = "TxnOffsetCommit";
        name_mapping[29] = "DescribeAcls";
        name_mapping[30] = "CreateAcls";
        name_mapping[31] = "DeleteAcls";
        name_mapping[32] = "DescribeConfigs";
        name_mapping[33] = "AlterConfigs";
        name_mapping[34] = "AlterReplicaLogDirs";
        name_mapping[35] = "DescribeLogDirs";
        name_mapping[36] = "SaslAuthenticate";
        name_mapping[37] = "CreatePartitions";
        name_mapping[38] = "CreateDelegationToken";
        name_mapping[39] = "RenewDelegationToken";
        name_mapping[40] = "ExpireDelegationToken";
        name_mapping[41] = "DescribeDelegationToken";
        name_mapping[42] = "DeleteGroups";
        name_mapping[43] = "ElectPreferredLeaders";
        name_mapping[44] = "IncrementalAlterConfigs";
    }
    auto it = name_mapping.find(key);
    return it != name_mapping.end() ? it->second : "Unknown API!";
}

std::string report_util::epoch_to_string(long double secs_since_epoch) {
    std::stringstream buffer;
    std::time_t secs_since_epoch_time_t = static_cast<std::time_t>(secs_since_epoch);
    long double fractional = secs_since_epoch - secs_since_epoch_time_t;
    buffer << std::put_time(std::localtime(&secs_since_epoch_time_t), "%H:%M:%S");
    buffer << ":" << std::setw(3) << std::setfill('0') << static_cast<int>(fractional * 1000);
    return buffer.str();
}

std::string report_util::hexdump(const std::vector<char> &data) {
    std::string res;
    char buf[8] = {0};
    for (size_t i = 0; i < data.size(); i++) {
        std::sprintf(buf, "%02x", static_cast<unsigned char>(data[i]));
        res += buf;
        if (i % 2 == 1) res += " ";
        if (i % 16 == 15) res += "\n";
    }
    return res;
}

std::string report_util::asciidump(const std::vector<char> &data) {
    std::string res;
    for (size_t i = 0; i < data.size(); i++) {
        if (std::isprint(data[i])) {
            res += data[i];
        } else {
            res += ".";
        }
        if (i % 2 == 1) res += " ";
        if (i % 16 == 15) res += "\n";
    }
    return res;
}

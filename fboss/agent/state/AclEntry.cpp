/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/state/AclEntry.h"
#include "fboss/agent/state/NodeBase-defs.h"
#include "fboss/agent/state/StateUtils.h"
#include <folly/Conv.h>
#include <folly/MacAddress.h>

using folly::IPAddress;

namespace {
constexpr auto kPriority = "priority";
constexpr auto kName = "name";
constexpr auto kActionType = "actionType";
constexpr auto kSrcIp = "srcIp";
constexpr auto kDstIp = "dstIp";
constexpr auto kL4SrcPort = "l4SrcPort";
constexpr auto kL4DstPort = "l4DstPort";
constexpr auto kProto = "proto";
constexpr auto kTcpFlagsBitMap = "tcpFlagsBitMap";
constexpr auto kSrcPort = "srcPort";
constexpr auto kDstPort = "dstPort";
constexpr auto kSrcL4PortRange = "srcL4PortRange";
constexpr auto kDstL4PortRange = "dstL4PortRange";
constexpr auto kPktLenRange = "pktLenRange";
constexpr auto kIpFrag = "ipFrag";
constexpr auto kIcmpCode = "icmpCode";
constexpr auto kIcmpType = "icmpType";
constexpr auto kDscp = "dscp";
constexpr auto kPortName = "portName";
constexpr auto kDstMac = "dstMac";
constexpr auto kIpType = "IpType";
constexpr auto kTtl = "ttl";
constexpr auto kTtlValue = "value";
constexpr auto kTtlMask = "mask";
constexpr auto kAclAction = "aclAction";
constexpr auto kDstIpLocal = "dstIpLocal";
}

namespace facebook { namespace fboss {

folly::dynamic AclTtl::toFollyDynamic() const {
  folly::dynamic ttl = folly::dynamic::object;
  ttl[kTtlValue] = static_cast<uint16_t>(value_);
  ttl[kTtlMask] = static_cast<uint16_t>(mask_);
  return ttl;
}

AclTtl AclTtl::fromFollyDynamic(const folly::dynamic& ttlJson) {
  if (ttlJson.find(kTtlValue) == ttlJson.items().end()) {
    throw FbossError("ttl should have a value set");
  }
  if (ttlJson.find(kTtlMask) == ttlJson.items().end()) {
    throw FbossError("ttl should have a mask set");
  }
  return AclTtl(ttlJson[kTtlValue].asInt(), ttlJson[kTtlMask].asInt());
}

folly::dynamic AclEntryFields::toFollyDynamic() const {
  folly::dynamic aclEntry = folly::dynamic::object;
  if (srcIp.first) {
    aclEntry[kSrcIp] = IPAddress::networkToString(srcIp);
  }
  if (dstIp.first) {
    aclEntry[kDstIp] = IPAddress::networkToString(dstIp);
  }
  if (dstMac) {
    aclEntry[kDstMac] = dstMac->toString();
  }
  if (proto) {
    aclEntry[kProto] = static_cast<uint8_t>(proto.value());
  }
  if (tcpFlagsBitMap) {
    aclEntry[kTcpFlagsBitMap] = static_cast<uint8_t>(tcpFlagsBitMap.value());
  }
  if (srcPort) {
    aclEntry[kSrcPort] = static_cast<uint16_t>(srcPort.value());
  }
  if (dstPort) {
    aclEntry[kDstPort] = static_cast<uint16_t>(dstPort.value());
  }
  if (srcL4PortRange) {
    aclEntry[kSrcL4PortRange] = srcL4PortRange.value().toFollyDynamic();
  }
  if (dstL4PortRange) {
    aclEntry[kDstL4PortRange] = dstL4PortRange.value().toFollyDynamic();
  }
  if (pktLenRange) {
    aclEntry[kPktLenRange] = pktLenRange.value().toFollyDynamic();
  }
  if (ipFrag) {
    auto itr_ipFrag = cfg::_IpFragMatch_VALUES_TO_NAMES.find(ipFrag.value());
    CHECK(itr_ipFrag != cfg::_IpFragMatch_VALUES_TO_NAMES.end());
    aclEntry[kIpFrag] = itr_ipFrag->second;
  }
  if (icmpCode) {
    aclEntry[kIcmpCode] = static_cast<uint8_t>(icmpCode.value());
  }
  if (icmpType) {
    aclEntry[kIcmpType] = static_cast<uint8_t>(icmpType.value());
  }
  if (dscp) {
    aclEntry[kDscp] = static_cast<uint8_t>(dscp.value());
  }
  if (ipType) {
    aclEntry[kIpType] = static_cast<uint16_t>(ipType.value());
  }
  if (ttl) {
    aclEntry[kTtl] = ttl.value().toFollyDynamic();
  }
  if (dstIpLocal) {
    aclEntry[kDstIpLocal] = static_cast<bool>(dstIpLocal.value());
  }
  auto itr_action = cfg::_AclActionType_VALUES_TO_NAMES.find(actionType);
  CHECK(itr_action != cfg::_AclActionType_VALUES_TO_NAMES.end());
  aclEntry[kActionType] = itr_action->second;
  if (aclAction) {
    aclEntry[kAclAction] = aclAction.value().toFollyDynamic();
  }
  aclEntry[kPriority] = priority;
  aclEntry[kName] = name;
  return aclEntry;
}

AclEntryFields AclEntryFields::fromFollyDynamic(
    const folly::dynamic& aclEntryJson) {
  AclEntryFields aclEntry(aclEntryJson[kPriority].asInt(),
      aclEntryJson[kName].asString());
  if (aclEntryJson.find(kSrcIp) != aclEntryJson.items().end()) {
    aclEntry.srcIp = IPAddress::createNetwork(
      aclEntryJson[kSrcIp].asString());
  }
  if (aclEntryJson.find(kDstIp) != aclEntryJson.items().end()) {
    aclEntry.dstIp = IPAddress::createNetwork(
      aclEntryJson[kDstIp].asString());
  }
  if (aclEntry.srcIp.first && aclEntry.dstIp.first &&
      aclEntry.srcIp.first.isV4() != aclEntry.dstIp.first.isV4()) {
    throw FbossError(
      "Unmatched ACL IP versions ",
      aclEntryJson[kSrcIp].asString(),
      " vs ",
      aclEntryJson[kDstIp].asString()
    );
  }
  if (aclEntryJson.find(kDstMac) != aclEntryJson.items().end()) {
    aclEntry.dstMac = folly::MacAddress(aclEntryJson[kDstMac].asString());
  }
  if (aclEntryJson.find(kProto) != aclEntryJson.items().end()) {
    aclEntry.proto = aclEntryJson[kProto].asInt();
  }
  if (aclEntryJson.find(kTcpFlagsBitMap) != aclEntryJson.items().end()) {
    aclEntry.tcpFlagsBitMap = aclEntryJson[kTcpFlagsBitMap].asInt();
  }
  if (aclEntryJson.find(kSrcPort) != aclEntryJson.items().end()) {
    aclEntry.srcPort = aclEntryJson[kSrcPort].asInt();
  }
  if (aclEntryJson.find(kDstPort) != aclEntryJson.items().end()) {
    aclEntry.dstPort = aclEntryJson[kDstPort].asInt();
  }
  if (aclEntryJson.find(kSrcL4PortRange) != aclEntryJson.items().end()) {
    aclEntry.srcL4PortRange = AclL4PortRange::fromFollyDynamic(
      aclEntryJson[kSrcL4PortRange]);
  }
  if (aclEntryJson.find(kDstL4PortRange) != aclEntryJson.items().end()) {
    aclEntry.dstL4PortRange = AclL4PortRange::fromFollyDynamic(
      aclEntryJson[kDstL4PortRange]);
  }
  if (aclEntryJson.find(kPktLenRange) != aclEntryJson.items().end()) {
    aclEntry.pktLenRange = AclPktLenRange::fromFollyDynamic(
      aclEntryJson[kPktLenRange]);
  }
  if (aclEntryJson.find(kIpFrag) != aclEntryJson.items().end()) {
    auto itr_ipFrag = cfg::_IpFragMatch_NAMES_TO_VALUES.find(
      aclEntryJson[kIpFrag].asString().c_str());
    aclEntry.ipFrag = cfg::IpFragMatch(itr_ipFrag->second);
  }
  if (aclEntryJson.find(kIcmpType) != aclEntryJson.items().end()) {
    aclEntry.icmpType = aclEntryJson[kIcmpType].asInt();
  }
  if (aclEntryJson.find(kIcmpCode) != aclEntryJson.items().end()) {
    aclEntry.icmpCode = aclEntryJson[kIcmpCode].asInt();
  }
  if (aclEntryJson.find(kDscp) != aclEntryJson.items().end()) {
    aclEntry.dscp = aclEntryJson[kDscp].asInt();
  }
  if (aclEntryJson.find(kIpType) != aclEntryJson.items().end()) {
    aclEntry.ipType = static_cast<cfg::IpType>(aclEntryJson[kIpType].asInt());
  }
  if (aclEntryJson.find(kDstIpLocal) != aclEntryJson.items().end()) {
    aclEntry.dstIpLocal = static_cast<bool>(aclEntryJson[kDstIpLocal].asBool());
  }
  if (aclEntryJson.find(kTtl) != aclEntryJson.items().end()) {
    aclEntry.ttl = AclTtl::fromFollyDynamic(aclEntryJson[kTtl]);
  }
  aclEntry.actionType =
      cfg::_AclActionType_NAMES_TO_VALUES
          .find(aclEntryJson[kActionType].asString().c_str())
          ->second;
  if (aclEntryJson.find(kAclAction) != aclEntryJson.items().end()) {
    aclEntry.aclAction = MatchAction::fromFollyDynamic(
      aclEntryJson[kAclAction]);
  }

  return aclEntry;
}

void AclEntryFields::checkFollyDynamic(const folly::dynamic& aclEntryJson) {
  // check src ip and dst ip are of the same type
  if(aclEntryJson.find(kSrcIp) != aclEntryJson.items().end() &&
     aclEntryJson.find(kDstIp) != aclEntryJson.items().end()) {
    auto src = IPAddress::createNetwork(aclEntryJson[kSrcIp].asString());
    auto dst = IPAddress::createNetwork(aclEntryJson[kDstIp].asString());
    if (src.first.isV4() != dst.first.isV4()) {
      throw FbossError(
        "Unmatched ACL IP versions ",
        aclEntryJson[kSrcIp].asString(),
        " vs ",
        aclEntryJson[kDstIp].asString(),
        "; source and destination IPs must be of the same type"
      );
    }
  }
  // check ipFrag is valid
  if (aclEntryJson.find(kIpFrag) != aclEntryJson.items().end()) {
    const auto ipFragName = aclEntryJson[kIpFrag].asString();
    if (cfg::_IpFragMatch_NAMES_TO_VALUES.find(ipFragName.c_str()) ==
        cfg::_IpFragMatch_NAMES_TO_VALUES.end()) {
      throw FbossError("Unsupported ACL IP fragmentation option ",
          aclEntryJson[kIpFrag].asString());
    }
  }
  // check action is valid
  if (cfg::_AclActionType_NAMES_TO_VALUES.find(
        aclEntryJson[kActionType].asString().c_str()) ==
      cfg::_AclActionType_NAMES_TO_VALUES.end()) {
    throw FbossError(
        "Unsupported ACL action ", aclEntryJson[kActionType].asString());
  }
  // check icmp type exists when icmp code exist
  if (aclEntryJson.find(kIcmpCode) != aclEntryJson.items().end() &&
      aclEntryJson.find(kIcmpType) == aclEntryJson.items().end()) {
    throw FbossError("icmp type must be set when icmp code is set");
  }
  // the value of icmp type must be 0~255
  if (aclEntryJson.find(kIcmpType) != aclEntryJson.items().end() &&
      (aclEntryJson[kIcmpType].asInt() < 0 ||
       aclEntryJson[kIcmpType].asInt() > kMaxIcmpType)) {
    throw FbossError("icmp type value must be between 0 and ",
      std::to_string(kMaxIcmpType));
  }
  // the value of icmp code must be 0~255
  if (aclEntryJson.find(kIcmpCode) != aclEntryJson.items().end() &&
      (aclEntryJson[kIcmpCode].asInt() < 0 ||
       aclEntryJson[kIcmpCode].asInt() > kMaxIcmpCode)) {
    throw FbossError("icmp code value must be between 0 and ",
      std::to_string(kMaxIcmpCode));
  }
  // check the "proto" is either "icmp" or "icmpv6" when icmptype is set
  if (aclEntryJson.find(kIcmpType) != aclEntryJson.items().end() &&
      (aclEntryJson.find(kProto) == aclEntryJson.items().end() ||
       !(aclEntryJson[kProto] == kProtoIcmp ||
         aclEntryJson[kProto] == kProtoIcmpv6))) {
    throw FbossError("proto must be either icmp or icmpv6 ",
      "if icmp type is set");
  }
}

AclEntry::AclEntry(int priority, const std::string& name)
  : NodeBaseT(priority, name) {
}

template class NodeBaseT<AclEntry, AclEntryFields>;

}} // facebook::fboss

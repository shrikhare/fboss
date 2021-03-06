#!/usr/bin/env python3
#
#  Copyright (c) 2004-present, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under the BSD-style license found in the
#  LICENSE file in the root directory of this source tree. An additional grant
#  of patent rights can be found in the PATENTS file in the same directory.
#

from fboss.cli.utils import utils
from fboss.thrift_clients import FbossAgentClient, QsfpServiceClient
import pickle
import ipaddress


class FlushType:
        arp, ndp = range(2)


# Parent Class for all commands
class FbossCmd(object):
    def __init__(self, cli_opts):
        ''' initialize; client will be created in subclasses with the specific
            client they need '''

        self._hostname = cli_opts.hostname
        self._port = cli_opts.port
        self._timeout = cli_opts.timeout
        self._snapshot_file = cli_opts.snapshot_file
        self._client = None

    def _create_agent_client(self):
        args = [self._hostname, self._port]
        if self._timeout:
            args.append(self._timeout)

        if self._snapshot_file is not None:
            snap_client = pickle.load(open(self._snapshot_file, "rb"))
            try:
                return snap_client[self._hostname]['agent']
            except KeyError:
                print("Please specify the host the snapshot was taken of")
                exit(0)

        return FbossAgentClient(*args)

    def _create_qsfp_client(self):
        args = [self._hostname, self._port]
        if self._timeout:
            args.append(self._timeout)
        return QsfpServiceClient(*args)

# --- All generic commands below -- #


class NeighborFlushSubnetCmd(FbossCmd):

    def _flush_entry(self, ip, vlan):
        bin_ip = utils.ip_to_binary(ip)
        vlan_id = vlan
        with self._create_agent_client() as client:
            num_entries = client.flushNeighborEntry(bin_ip, vlan_id)
        print('Flushed {} entries'.format(num_entries))

    def run(self, flushType, network, vlan):
        if (isinstance(network, ipaddress.IPv6Network) and
                network.prefixlen == 128) or                    \
           (isinstance(network, ipaddress.IPv4Network) and
                network.prefixlen == 32):
            self._flush_entry(str(network.network_address), vlan)
            return

        with self._create_agent_client() as client:
            if flushType == FlushType.arp:
                table = client.getArpTable()
            elif flushType == FlushType.ndp:
                table = client.getNdpTable()
            else:
                print("Invaid flushType")
                exit(1)

            num_entries = 0
            for entry in table:
                if (ipaddress.ip_address(utils.ip_ntop(entry.ip.addr)) in
                    ipaddress.ip_network(network)) and                           \
                        (vlan is 0 or vlan == entry.vlanID):
                    num_entries += client.flushNeighborEntry(entry.ip,
                            entry.vlanID)

        print('Flushed {} entries'.format(num_entries))

class PrintNeighborTableCmd(FbossCmd):
    def print_table(self, entries, name, width, client):
        tmpl = "{:" + str(width) + "} {:18} {:<10}  {:18} {!s:12} {}"
        print(tmpl.format(
            "IP Address", "MAC Address", "Port", "VLAN", "State", "TTL"))
        ports = client.getAllPortInfo()

        for entry in sorted(entries, key=lambda e: e.ip.addr):
            port_identifier = entry.port
            if entry.port in ports and ports[entry.port].name:
                port_identifier = ports[entry.port].name
            ip = utils.ip_ntop(entry.ip.addr)
            vlan_field = '{} ({})'.format(entry.vlanName, entry.vlanID)
            ttl = '{}s'.format(entry.ttl // 1000) if entry.ttl else '?'
            state = entry.state if entry.state else 'NA'
            print(tmpl.format(ip, entry.mac, port_identifier, vlan_field,
                              state, ttl))


class VerbosityCmd(FbossCmd):
    def run(self, verbosity):
        with self._create_agent_client() as client:
            client.setOption('v', verbosity)

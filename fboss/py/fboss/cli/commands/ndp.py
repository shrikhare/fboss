#!/usr/bin/env python3
#
#  Copyright (c) 2004-present, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under the BSD-style license found in the
#  LICENSE file in the root directory of this source tree. An additional grant
#  of patent rights can be found in the PATENTS file in the same directory.
#

from fboss.cli.commands import commands as cmds

class NdpTableCmd(cmds.PrintNeighborTableCmd):
    def run(self):
        with self._create_agent_client() as client:
            resp = client.getNdpTable()
            name = 'NDP'
            width = 40
            self.print_table(resp, name, width, client)

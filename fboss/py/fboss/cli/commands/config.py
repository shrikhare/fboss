#!/usr/bin/env python3
#
#  Copyright (c) 2004-present, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under the BSD-style license found in the
#  LICENSE file in the root directory of this source tree. An additional grant
#  of patent rights can be found in the PATENTS file in the same directory.
#

import json

from fboss.cli.utils.utils import AGENT_KEYWORD
from fboss.cli.commands import commands as cmds
from neteng.fboss.ttypes import FbossBaseError

class GetConfigCmd(cmds.FbossCmd):
    def run(self, config_type):
        if config_type != AGENT_KEYWORD:
            print("No Config Info Found")

        with self._create_agent_client() as client:
            resp = client.getRunningConfig()

        if not resp:
            print("No Config Info Found")
            return
        parsed = json.loads(resp)
        print(json.dumps(parsed, indent=4, sort_keys=True,
                         separators=(',', ': ')))


class ReloadConfigCmd(cmds.FbossCmd):
    """
    Command to instruct agent to reload its own config, as if it is restarting,
    but without restarting.
    """
    def run(self):
        with self._create_agent_client() as client:
            try:
                client.reloadConfig()
                print("Config reloaded")
                return 0
            except FbossBaseError as e:
                print('Fboss Error: ' + e)
                return 2

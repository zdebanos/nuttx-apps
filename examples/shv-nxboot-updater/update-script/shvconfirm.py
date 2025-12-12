############################################################################
# apps/examples/shv-nxboot-updater/update-script/shvconfirm.py
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

import asyncio
from enum import Enum

from shv.rpcurl import RpcUrl
from shv.rpcapi.valueclient import SHVValueClient

class ConfirmStep(Enum):
    END_OK = 0
    END_ERROR = 1,

async def shv_confirm(
    connection: str, path_to_root: str, queue: asyncio.Queue,
    method_timeout:float
):
    try:
        url = RpcUrl.parse(connection)
    except ValueError:
        queue.put_nowait([ConfirmStep.END_ERROR, "The supplied URL is invalid!"])

    try:
        client = await asyncio.wait_for(SHVValueClient.connect(url), timeout=method_timeout)
        if client is None:
            _shv_flasher_cleanup("Could not connect to the broker.", queue, client)
            return
    except Exception as e:
        queue.put_nowait([ConfirmStep.END_ERROR, "Could not connect to the broker. " +
                          "Is the broker running or is the url OK?"], queue, client)
        return

    try:
        await asyncio.wait_for(client.call(f"{path_to_root}/fwStable", "confirm"), timeout=method_timeout)
    except TimeoutError as e:
        queue.put_nowait([ConfirmStep.END_ERROR, "confirm timeout"])
    except Exception:
        queue.put_nowait([ConfirmStep.END_ERROR, "Failed calling confirm"])

    await client.disconnect()
    queue.put_nowait([ConfirmStep.END_OK, 0])

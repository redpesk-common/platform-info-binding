#!/usr/bin/env bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"


echo "--- Detecting hardware specification ---"
cat /etc/platform-info/core.json
cat /etc/platform-info/os.json

echo "--- creating test's namespace & startintg tests ---"
LD_LIBRARY_PATH=${SCRIPT_DIR}/../build SCRIPTS_PATH=./scripts/ unshare -U -r -m bash -c "mount --bind ${SCRIPT_DIR}/fakeplatforminfo/ /etc/platform-info/; python ${SCRIPT_DIR}/tests.py -vvv --tap"


#!/bin/bash

set -euo pipefail

"$(dirname "$0")"/download/download_nsisshellexecasuser.sh

cp ShellExecAsUser.dll /usr/share/nsis/Plugins/x86-ansi

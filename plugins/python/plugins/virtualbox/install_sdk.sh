#!/bin/bash

set -e

export VBOX_INSTALL_PATH="/Applications/VirtualBox.app/Contents/MacOS"
. "$HOME/Library/Application Support/albert/python/venv/bin/activate"
cd "${VBOX_INSTALL_PATH}/sdk/installer/python"
pip install setuptools
python3 vboxapisetup.py install
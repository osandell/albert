# Albert plugin: VirtualBox

## Features

- Search VirtualBox VMs
- VM item actions:
  - (PoweredOff) Start
  - (Running) Save/PowerOff/Stop/Pause
  - (Saved) Restore/Discard
  - (Paused) Resume


## Technical details

Downloads and installs the VirtualBox SDK into the venv if not already installed.
The heuristic used to set VBOX_INSTALL_PATH is naive. 
Please post an issue if you encounter problems.

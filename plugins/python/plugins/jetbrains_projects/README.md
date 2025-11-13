# Albert plugin: Jetbrains projects

## Features

* Search and open projects of the Jetbrains IDEs.
* Supported IDEs:
  * Android Studio
  * Aqua
  * CLion
  * DataGrip
  * DataSpell
  * GoLand
  * IntelliJ IDEA
  * PhpStorm
  * PyCharm
  * Rider
  * RubyMine
  * RustRover
  * WebStorm
  * Writerside.
* The projects are sorted by recency.
* Optionally match the project path in addition to the project name. 

# Setup

For this plugin to find the IDEs, a commandline launcher in $PATH is required.
Open the IDE and click Tools -> Create Command-line Launcher to add one.
See also the [JetBrains documentation](https://www.jetbrains.com/help/pycharm/2025.2/working-with-the-ide-features-from-command-line.html#toolbox).

# Notes

The plugin reads the recent projects from the configuration directory of the IDEs.
On Linux, this is usually `~/.config/JetBrains/<product><version>`.
On macOS, this is usually `~/Library/Application Support/JetBrains/<product><version>`.
If you have a custom config directory, the best solution is to create a symlink in the default location.

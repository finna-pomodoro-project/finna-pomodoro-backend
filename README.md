# Finna Pomodoro

The finna pomodoro project aims to bring a modern pomodoro implementation to
your desktop.

The _modern_ feature comes from a complete separation of the logic (backend)
from the user interfaces. Did you ever wonder why you couldn't use different
interfaces (cli commands associated with key bindings, guis, desktop
notifications, web, deep integration within your desktop environment) to control
the **same** pomodoro? With Finna Pomodoro, you can!

This repository will host the Finna Pomodoro Daemon. It's the application that
handle the pomodoro logic and communication among the interested parties.

# Features

- Complete separation of the logic (backend) from the user interfaces.
- Backend written in C++11 using robust timing abstractions.
- Real-time pomodoro with robust numerical methods.
- A DBus-based protocol designed to minimize possible bugs in clients.
- Ninjas! No, just kidding, but the protocol is so well designed that allows
  flexible implementation by defining optional parts that would restrict
  implementers' creativity.
- DRM-free!

# Dependencies

This daemon is written in C++11 and depends on the glibmm library. You'll also
need CMake to build the project. For short, CMake is the meta-build system and
is used to generate a build system that your platform supports.

D-Bus is used for communication. D-Bus is pretty much a central component in
GNU/Linux nowadays and you'll likely find in any GNU/Linux system.

## Building

You strongly suggest you to ask packagers from your distribution to create an
user-friendly and easy-to-install package. This section is aimed to more
experienced users that wish to build from scratch (packagers and developers).

There are two types of building in CMake: in-source and shadow building.

In in-source mode, all intermediate files used to generate the final executable
will be generated in the same folder of your source code, polluting the project.
This mode is unrecommended if you plan to develop, but is fine for packagers.

In shadow building mode, all intermediate files used to generate the final
executable will be generated in an isolated folder from the source code. To use
this mode, just create a folder for the build and run cmake from there.

```sh
cd /path/to/build/dir
cmake /path/to/root/source/dir
make # under GNU/Linux, a Makefile is generated
```

# License

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

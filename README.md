NVIDIA Flow - 1.0.1
===================

Flow is a sparse grid-based fluid simulation library for real-time applications.
Please see the manual included in this release package for more information on
the API and usage.

Flow 2
-------------------

Flow 2 is now available and open source [here](https://github.com/NVIDIA-Omniverse/PhysX).

Supported Platforms
-------------------

Windows 32/64 bit

Requirements
------------

* A DX11 capable graphics card and driver
* Visual Studio 2015 and the Windows 10 SDK

Demo
====

nobuild_run.bat
Bin\x64\DemoAppRelease_x64.exe

Controls
--------

left mouse - Rotate camera
middle mouse - Pan camera
right mouse - Zoom camera

space - Shoot projectile
p - Pause simulation
F10 - Toggle DX11/DX12
F11 - Toggle fullscreen
Esc - Quit

Known Issues
============

* Volume Tiled Resources are slow in debug builds, due to D3D debug runtime warnings.
* The default memory limit might be excessive on cards with less than 2 GB of VRAM.

Acknowledgements
================

* SDL is licensed under the zlib license
* stb_truetype by Sean Barrett is public domain
* imgui by Mikko Mononen is licensed under the ZLib license

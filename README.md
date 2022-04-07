# Doom Eternal Model Importer v1.2
A cross-platform GUI tool for converting .OBJ format models into the .LWO format used by DOOM Eternal.

## Usage
See the guide here: https://wiki.eternalmods.com/books/2-how-to-create-mods/page/importing-custom-models

## Changelog
 - v1.2: Major code cleanup. Some bug fixes & optimizations. Source code is now public and compiles for both Windows and Linux.
 - v1.1: Fixed a problem with the way vertex normals were calculated, which caused dark-colored marks to appear on some models.

## Known Issues/Limitations:
- This tool can only be used to import .lwo format models. It cannot replace md6mesh / animated models.
- Currently the tool can only import models with a single mesh/material. Multi-mesh support may be added later.
- You cannot import a model with more than 65,535 vertices. This is a hard cap - the DOOM Eternal .lwo format does not allow for more than this without splitting meshes (something the tool might support in the future, but currently does not). If you try to import a model with too many vertices, you will get an error message. You can reduce vertex count using the decimation tool in Blender.
- Models with a glow effect (especially item pickups) have a noticeable lighting issue. This can be minimized by disabling the surface emissive / surface sheen via .decl files. Otherwise, I don't know how to fix this yet.

## Special Thanks:
- [Zwip-Zwap Zapony](https://github.com/ZwipZwapZapony) for extensive support and advice re: 3d modeling in general.
- [MikeyRay](https://github.com/MikeyRay) and [Scobalula](https://github.com/Scobalula) for assistance with the model format.
- [Elizabethany](https://github.com/elizabethany), [PowerBall253](https://github.com/PowerBall253), [Zwip-Zwap Zapony](https://github.com/ZwipZwapZapony) and others for help testing the tool.
- [PowerBall253](https://github.com/PowerBall253) for help with Linux compatibility.

## License:

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with the SAMUEL program.  If not, see <https://www.gnu.org/licenses/>.

## Building from Source:

There is no need to build/compile this program from source code. Simply download and use the included binaries for Windows or Linux (under "Releases").

If you *want* to build/compile from source, you will need a copy of the [Qt development library](https://www.qt.io/). This program uses Qt for its cross-platform GUI features. Please note that usage of Qt is subject to a separate licensing agreement. This program uses Qt under the [Qt for Open-Source Development](https://www.qt.io/download-open-source). The Qt source code can be acquired here: https://www.qt.io/offline-installers.

This program is tested and compiled using a static build of Qt version 6.1.2.

## Contributing:

Contributions are welcomed. There is lots of room for code cleanup/improvement. All issues and pull requests will be considered. Please note I have limited time, so my response may not be immediate.

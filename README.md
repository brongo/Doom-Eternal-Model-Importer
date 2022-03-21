# Doom Eternal Model Importer v1.1
A GUI tool for converting .OBJ format models into the .LWO format used by DOOM Eternal.

This tool is currently for Windows only. Linux support will be added in the near future. 

## New in version 1.1
This update fixes a problem with the way vertex normals were calculated, which caused dark-colored marks to appear on some models.

## Usage
See the guide here: https://wiki.eternalmods.com/books/2-how-to-create-mods/page/importing-custom-models

## Known Issues/Limitations:
- This tool can only be used to import .lwo format models. It cannot replace md6mesh / animated models.
- Currently the tool can only import models with a single mesh/material. Multi-mesh support may be added later.
- You cannot import a model with more than 65,535 vertices. This is a hard cap - the DOOM Eternal .lwo format does not allow for more than this without splitting meshes (something the tool might support in the future, but currently does not). If you try to import a model with too many vertices, you will get an error message. You can reduce vertex count using the decimation tool in Blender.
- Models with a glow effect (especially item pickups) have a noticeable lighting issue. This can be minimized by disabling the surface emissive / surface sheen via .decl files. Otherwise, I don't know how to fix this yet.

In general, this is new territory so it is likely more issues will be found. Please be kind and plan accordingly.

## Special Thanks:
- Zwip-Zwap Zapony for extensive support and advice re: 3d modeling in general.
- Ray for assistance with the .lwo model format.
- Elizabethany, PowerBall253, Zwip-Zwap Zapony and others for help testing the tool.

## Source:

I only created this repo to host the download somewhere outside of Discord for now. 

I'll clean up and publish the source code sometime soon.

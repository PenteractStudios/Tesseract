# Tesseract Game Engine

Basic engine created for UPC's Master in Advanced Programming for AAA Videogames.

![Photo of the Engine](https://imgur.com/NPpU4XW.jpeg)

## Features

- 3D Scene view with Unity-like controls
- Basic console log
- Loading and saving scenes
- Custom file format to save the information loaded
- Module Resource to manage the import of different type of files
- Implementation of an editable Quadtree
- Frustum Culling
- PBR Phong implementation
- Three types of lights (directional, point, spot)
- Ray tracing (to select objects in the scene)
- Event System to capture events
- Animated models 
- Creation of UI Canvas
- A Gameplay System with the capacity to create scripts
- A Build System to export a game

## Additional features

- Basic templated pool data structure to allow for faster allocation/deallocation of resources

## How to use

### Loading scenes, models and textures

- Drag the files inside the Panel Scene window and the files will be loaded in the Panel Project
- You can also copy the files to the Assets folder and the Engine will import the files and show them in the Panel Project

### Menu items

- File
  - New: Create new empty scene
  - Load: Load a scene 
  - Save: Save a scene
  - Quit: Exit the engine
- Assets
  - Create material
- View
  - Open/Close different panels from here
- Help
  - Documentation: Go to wiki page in github
  - Download latest: Go to releases page in github
  - Report a bug: Go to issues page in github
  - About: Open/Close about panel

### Scene view navigation

Left click to focus on the scene window first.

- Right-click to enter flythrough mode
  - WASD + QE to move around
  - Mouse to look around
- Alt + Left-click to orbit around the camera focus
- Alt + Right-click to zoom with the mouse (moves the camera focus)
- Mouse wheel to zoom (moves the camera focus)
- Arrow keys to move around
- F key to focus around the geometry
- Shift to move faster (5x)
- W/E/R to translate/rotate/scale selected object

Movement and zoom speed depend on the distance to the focus point (zooming out makes moving and zooming faster).

### Configuration panel

This panel contains engine information and configuration variables for multiple modules:

- Application
  - Application name and organization
- Time
  - FPS and MS graph
  - Change Framerate Limit
  - Activate/Deactivate Framerate Limit
  - Activate/Deactivate VSync
- Hardware
  - Information about library versions
  - Information about CPU, RAM and GPU
- Window
  - Change window mode
    - Windowed
    - Borderless
    - Fullscreen (Not working properly)
    - Desktop Fullscreen
  - Change brightness
  - Resize window
- Scene
  - Configure Quadtree
  - Change the background color
  - Change the ambient color

### Control Editor panel
- Translate/Rotate/Scale controller
- Play/Stop/Pause controller

### Scene panel
- Shading mode
- Engine camera parameters
- Gizmos parameters
- Stats
- Preview of the scene

### Inspector panel
Contains information about the currently loaded model

- Transformation (postition, rotation and scale)
- Geometry (meshes and bounding box)
- Textures (shader configuration, size and preview)
- Lights component
- Camera component
- Skybox component
- Script component
- Animation component
- UI component (Canvas, Image, Text, Button, Event System...)


### Console panel

Prints all logged information from the Engine.

### Project panel

A finder of the Assets folder with all the files imported

## Libraries used

- SDL 2 (https://www.libsdl.org/download-2.0.php)
- Glew (https://github.com/nigels-com/glew)
- MatGeoLib (https://github.com/juj/MathGeoLib)
- Dear ImGui (https://github.com/ocornut/imgui)
- DeviL (http://openil.sourceforge.net/)
- assimp (https://github.com/assimp/assimp)
- ImGuizmo (https://github.com/CedricGuillemet/ImGuizmo)
- RapidJason (https://github.com/Tencent/rapidjson)
- Fmt (https://github.com/fmtlib/fmt)
- Libsndfile (https://github.com/libsndfile/libsndfile)
- OpenAl-soft (https://github.com/kcat/openal-soft)

## About us

Find all the information about the organization in the web page:
https://penteractstudios.github.io/

## Github

https://github.com/PenteractStudios/Tesseract

## License

[MIT](./LICENSE)


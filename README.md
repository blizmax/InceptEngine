# Inception Engine
An independent work attempting to implement some components of a game engine from scratch. A demo can be found at https://www.bilibili.com/video/av95683434/

**Enabled Features (from new to old):**

* Animation controller

* Capsule collision

* Generating terrain using height map and terrain collision detection

* Cube map and skybox

* Randomly generating terrain using Perlin Noise and rendering using Marching Cube

* Marching Cube algorithm for rendering predefined surface

* IK animation of the right arm of the character, using FABRIK algorithm

* Socket system which enables actors to be attached to a socket or bone so that the attached actor will move
  with its parent
    
* Play skeleton animation
      
* Load skeleton hierarchy, bone information, and key framing animation data

* Load model to scene

* Camera control and follow character

* Character control

* Point light

* Play audio

* Base color texture

* Indexed rendering



**Implementing Features:**
  
  * Depth detection
  
  * Animation notify
  
  * Multisampling
  
  * Multitexturing
  
  * Simple ray tracer

  * UMG support
  
  
**Reference:**
  
  * Vulkan: for rendering
  
  * GLFW: for window handling
  
  * GLM: for math computation
  
  * Assimp: for reading FBX files
  
  * Fast Noise: for generating Perlin Noise

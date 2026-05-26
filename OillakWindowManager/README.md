# Oma Graphics Engine -projekti

## ?? Projektin yleiskuva
Tämä projekti on henkilökohtainen oppimismatka, jonka tarkoituksena on virkistää C++-ohjelmointitaitoja ja rakentaa yksinkertainen graphics engine alusta asti.  
Tavoitteena on toteuttaa peruspalikat vaiheittain: window management, input handling ja lopuksi Vulkan-renderöinti.

---

## ? Nykyinen tilanne
- **Window Manager**  
  - Toteutettu oma Win32-ikkunakäsittelijä (`Window`-luokka).  
  - Tarjoaa selkeän API:n ikkunan luontiin ja hallintaan.  
  - Hoitaa Windows message loopin (`processMessages`).  
  - Palauttaa `HWND`-kahvan Vulkan-integraatiota varten.

---

## ?? Seuraavat askeleet
1. **Input Manager**
   - Kaappaa keyboard ja mouse -tapahtumat Win32-message loopista.
   - Tarjoaa helpon API:n syötteen tilan kysymiseen:
     - `isKeyPressed(key)`
     - `getMousePosition()`
   - Tallentaa tilan sisäisesti (esim. `std::unordered_map<int,bool>` näppäimille).

2. **Vulkan-integraatio**
   - Alusta Vulkan instance tarvittavilla extensioneilla (`VK_KHR_surface`, `VK_KHR_win32_surface`).  
   - Luo Vulkan surface ikkunan kahvasta.  
   - Valitse physical device (GPU) ja queue familyt.  
   - Luo logical device ja swapchain.  
   - Toteuta klassinen **Hello Triangle** -renderöintipipeline.

3. **Renderer-abstraktio**
   - Kapseloi Vulkan-setup `VulkanRenderer`-luokkaan.  
   - Hoitaa initializationin, render loopin ja cleanupin.  
   - Tarjoaa `drawFrame()`-metodin pääloopille.

---

## ?? Roadmap
- [x] Window Manager  
- [ ] Input Manager  
- [ ] Vulkan Initialization (Instance, Surface, Device, Swapchain)  
- [ ] Graphics Pipeline (Shaders, Render Pass, Framebuffers)  
- [ ] Hello Triangle Demo  
- [ ] Resource Management (textures, buffers, shaders)  
- [ ] Scene Graph basics  

---

## ??? Kehitysympäristö
- **IDE:** Visual Studio  
- **Kieli:** C++17  
- **Alusta:** Windows (Win32 API + Vulkan SDK)  
- **Riippuvuudet:** Vulkan SDK, valinnainen GLM matematiikkaan

---

## ? Visio
Projektin lopussa tavoitteena on saada aikaan minimaalinen mutta toimiva graphics engine, joka pystyy:
- Luomaan ja hallitsemaan ikkunoita ilman ulkoisia kirjastoja kuten GLFW.  
- Käsittelemään käyttäjän inputin suoraan.  
- Renderöimään grafiikkaa Vulkanilla.  
- Toimimaan pohjana 3D-renderöinnin, resurssien hallinnan ja engine-suunnittelun kokeiluille.


# Custom Graphics Engine Project

## ?? Project Overview
This project is a personal learning journey to refresh C++ programming skills and build a simple graphics engine from scratch.  
The goal is to implement the fundamental building blocks step by step: window management, input handling, and finally Vulkan rendering.

---

## ? Current Progress
- **Window Manager**  
  - Implemented a custom Win32 window handler (`Window` class).  
  - Provides clean API for creating and managing a window.  
  - Handles the Windows message loop (`processMessages`).  
  - Exposes `HWND` handle for Vulkan integration.

---

## ?? Next Steps
1. **Input Manager**
   - Capture keyboard and mouse events from the Win32 message loop.
   - Provide easy API for querying input state:
     - `isKeyPressed(key)`
     - `getMousePosition()`
   - Store state internally (e.g., `std::unordered_map<int,bool>` for keys).

2. **Vulkan Integration**
   - Initialize Vulkan instance with required extensions (`VK_KHR_surface`, `VK_KHR_win32_surface`).
   - Create a Vulkan surface from the window handle.
   - Select physical device (GPU) and queue families.
   - Create logical device and swapchain.
   - Implement the classic **Hello Triangle** rendering pipeline.

3. **Renderer Abstraction**
   - Encapsulate Vulkan setup in a `VulkanRenderer` class.
   - Handle initialization, rendering loop, and cleanup.
   - Provide `drawFrame()` method for main loop.

---

## ?? Roadmap
- [x] Window Manager  
- [ ] Input Manager  
- [ ] Vulkan Initialization (Instance, Surface, Device, Swapchain)  
- [ ] Graphics Pipeline (Shaders, Render Pass, Framebuffers)  
- [ ] Hello Triangle Demo  
- [ ] Resource Management (textures, buffers, shaders)  
- [ ] Scene Graph basics  

---

## ??? Development Environment
- **IDE:** Visual Studio  
- **Language:** C++17  
- **Platform:** Windows (Win32 API + Vulkan SDK)  
- **Dependencies:** Vulkan SDK, optional GLM for math

---

## ? Vision
By the end of this project, the goal is to have a minimal but functional graphics engine that can:
- Create and manage windows without external libraries like GLFW.
- Handle user input directly.
- Render graphics using Vulkan.
- Serve as a foundation for experimenting with 3D rendering, resource management, and engine design.

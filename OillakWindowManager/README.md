# Oillak Engine

Henkilökohtainen oppimisprojekti, jonka tavoitteena on rakentaa minimalistinen graafinen moottori alusta asti ilman ulkoisia kirjastoja (kuten GLFW tai SDL).
Koska ottaa päähän opetella muiden tekemien kirjastojen API:ta, haluan oppia syvällisesti, miten kaikki toimii "raaka" C++:lla ja Vulkanilla. Tämä projekti on siis enemmänkin matka kohti syvällisempää ymmärrystä kuin nopea kehitystyökalujen käyttö.
nyt jos haluan jotain luoda niin teen sen itse, enkä käytä valmiita kirjastoja, koska haluan oppia syvällisesti miten kaikki toimii. Tämä projekti on siis enemmänkin matka kohti syvällisempää ymmärrystä kuin nopea kehitystyökalujen käyttö.
usko että siinä vaiheessa kun moottorilla voi tehdä jotain hauskaa, olen kerryttänyt tarpeeksi tietoa ja taitoa, että voin käyttää sitä luovasti ilman, että minun tarvitsee käyttää valmiita työkaluja, jotka vain peittävät kaiken alleen. Haluan ymmärtää kaiken, mitä tapahtuu, ja se vaatii aikaa ja vaivaa, mutta se on sen arvoista oppimisen kannalta.

## Projektiyhteenveto (Vision)
Tavoitteena on oppia syvällisesti C++-ohjelmointia ja modernia grafiikkarajapintaa. 
* **Ikkunointi:** Oma Win32-toteutus.
* **Input:** Suora syötteenkäsittely Win32-viestien kautta.
* **Grafiikka:** Vulkan-pohjainen renderöinti alusta asti.

## Nykytilanne (Päivitetty 7.6.2026)

### Valmiit osat:
- [x] **Window Manager:** Toteutettu oma Win32-ikkunakäsittelijä, tarjoaa selkeän API:n ja `HWND`-kahvan.
- [x] **Vulkan Instance:** Luotu ja konfiguroitu (peruslaajennukset).
- [x] **Physical Device:** Enumerointi, laitteiden haku ja Queue Family -tarkistukset valmiit.
- [x] **Logical Device:** Luotu onnistuneesti yhdessä grafiikka- ja present-jonojen (Queues) kanssa.
- [x] **Virheen käsittely:** `chk`-makro (`__FILE__`, `__LINE__`) käytössä `VulkanUtils.h`:ssa.
- [X] **Surface:** `VkSurfaceKHR`-luonti ikkunaa varten. *(Huom: Win32-surface vaatii `HWND`:n lisäksi `HINSTANCE`-kahvan, joka pitää hakea Window-luokasta!)*
- [X] Swapchain: Laitteen tuen tarkistus (formaatit, esitystilat) ja VkSwapchainKHR:n luonti.
- [x] Window Manager  
- [x] Vulkan Initialization (Instance, Surface, Device, Swapchain)  
- [x] Graphics Pipeline (Shaders, Render Pass, Framebuffers)  
- [x] Hello Triangle Demo 
- [x] Logger, Debugging Tools 
- [ ] 
### Työn alla / Seuraavat askeleet:
- [ ] **Fences ja Semaphores:** Synkronointimekanismit renderöintisilmukan hallintaan.
- [ ] **Resource Management:** Resurssien (tekstuurit, buffereiden, shaderit) luonti ja vapautus.
- [ ] **Input Manager:** Win32-viestien kaappaus, näppäin- ja hiiritilojen tallennus.

## Roadmap
- [x] Window Manager  
- [x] Vulkan Initialization (Instance, Surface, Device, Swapchain)  
- [x] Graphics Pipeline (Shaders, Render Pass, Framebuffers)  
- [x] Hello Triangle Demo 
- [x] Logger, Debugging Tools 
- [ ] Fences ja Semaphores 
- [ ] Input Manager  
- [ ] Resource Management (textures, buffers, shaders)  
- [ ] Scene Graph basics  
- [ ] Basic 3D rendering (models, transformations)
- [ ] Basic lighting (Phong, Blinn-Phong)
- [ ] Basic animation (keyframe, skeletal)
- [ ] Basic physics (collision detection, simple dynamics)
- [ ] Sandbox: simple game loop, input handling, basic physics
- [ ] Sandbox: more complex scenes, lighting, shadows
- [ ] Sandbox: performance optimizations, multi-threading
- [ ] More advanced graphics features (post-processing, compute shaders)
- [ ] Game development UI (editor tools, debug overlays)

##Graphics engine
  - [ ] triangle demo (done)
  - [ ] smooth rendering loop with synchronization (fences, semaphores)
  - [ ] swapchain recreation (window resizing)
  - [ ] resource management (textures, buffers, shaders)
  - [ ] performance testing
  - [ ] optimizations (command buffers, memory management)
  - [ ] performance testing
  - [ ] basic 3D rendering (models, transformations)
  - [ ] basic lighting (Phong, Blinn-Phong)
  - [ ] basic animation (keyframe, skeletal)

## Physics engine (optional, stretch goal)
- [ ] physics basic: movement
- [ ] physics basic: collision detection
- [ ] physics basic: simple dynamics (gravity, forces)
- [ ] physics intermediate: collision response (impulse-based eg. push objects apart, basic friction)
- [ ] physics intermediate: constraints (joints, springs)
- [ ] physics intermediate: basic ragdoll physics (simple jointed bodies)
- [ ] physics advanced: more complex collision shapes (convex hulls, compound shapes)
- [ ] physics advanced: more complex dynamics (soft bodies, cloth simulation)
- [ ] physics advanced: performance optimizations (broadphase, spatial partitioning)
- [ ] physics advanced: multi-threading (parallel collision detection, physics updates)
- [ ] physics advanced: integration with graphics engine (visualization of physics debug info, physics-based animation)
- [ ] physics advanced: performance testing and optimizations (profiling, memory management)

## Sandbox (optional, after graphics engine and physics engine basic stage is done)
## after graphics and physics basics are implemented, create a simple sandbox environment to test and demonstrate the engine's capabilities. This can be a simple 3D scene with basic gameplay mechanics, allowing for iterative development and testing of new features.
- [ ] sandbox: simple scenes, basic rendering features
- [ ] sandbox: simple game loop, input handling, basic physics
## after the basic sandbox is functional, expand it with more complex features and optimizations, creating a more complete and polished demo environment for the engine.
- [ ] sandbox: more complex scenes, lighting, shadows
- [ ] sandbox: basic gameplay mechanics (object interaction, simple AI)
## after the sandbox is more feature-complete, focus on polishing the experience, fixing bugs, and optimizing performance to create a smooth and enjoyable demo environment for showcasing the engine's capabilities.
- [ ] sandbox: more complex gameplay mechanics (inventory, crafting, quests)
- [ ] sandbox: polish, bug fixing, performance optimizations

## Kehitysympäristö
- **IDE:** Visual Studio  
- **Kieli:** C++20 (Standardi päivitetty!)
- **Alusta:** Windows (Win32 API + Vulkan SDK)  
- **Tekniset huomiot:** - Käytössä brace-initialization `{}` roskadatan välttämiseksi.
    - `cleanup()`-metodi hoitaa resurssien vapauttamisen käänteisessä järjestyksessä.

    ## Jatka tästä (Seuraava koodisessio)

**Ensimmäinen asia huomenna:**
- **swaphain fences ja semaphores:** Näiden avulla hallitaan renderöintisilmukan synkronointia, mikä on kriittistä sujuvan grafiikkasuorituksen varmistamiseksi. Tämä vaatii syvällistä ymmärrystä Vulkanin synkronointimekanismeista ja niiden oikeasta käytöstä renderöintiputkessa.
- recreate swapchain: Swapchainin uudelleenluonti on tärkeää, kun ikkunan koko muuttuu tai laitteessa tapahtuu muutoksia. Tämä vaatii swapchainin ja siihen liittyvien resurssien (kuten framebuffers) asianmukaista tuhoamista ja uudelleenluontia.



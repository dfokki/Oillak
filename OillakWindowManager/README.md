# Oillak Engine

Henkilökohtainen oppimisprojekti, jonka tavoitteena on rakentaa minimalistinen graafinen moottori alusta asti ilman ulkoisia kirjastoja (kuten GLFW tai SDL).

## Projektiyhteenveto (Vision)
Tavoitteena on oppia syvällisesti C++-ohjelmointia ja modernia grafiikkarajapintaa. 
* **Ikkunointi:** Oma Win32-toteutus.
* **Input:** Suora syötteenkäsittely Win32-viestien kautta.
* **Grafiikka:** Vulkan-pohjainen renderöinti alusta asti.

## Nykytilanne (Päivitetty 27.5.2026)

### Valmiit osat:
- [x] **Window Manager:** Toteutettu oma Win32-ikkunakäsittelijä, tarjoaa selkeän API:n ja `HWND`-kahvan.
- [x] **Vulkan Instance:** Luotu ja konfiguroitu (peruslaajennukset).
- [x] **Physical Device:** Enumerointi, laitteiden haku ja Queue Family -tarkistukset valmiit.
- [x] **Logical Device:** Luotu onnistuneesti yhdessä grafiikka- ja present-jonojen (Queues) kanssa.
- [x] **Virheen käsittely:** `chk`-makro (`__FILE__`, `__LINE__`) käytössä `VulkanUtils.h`:ssa.
- [X] **Surface:** `VkSurfaceKHR`-luonti ikkunaa varten. *(Huom: Win32-surface vaatii `HWND`:n lisäksi `HINSTANCE`-kahvan, joka pitää hakea Window-luokasta!)*
### Työn alla / Seuraavat askeleet:
- [ ] Swapchain: Laitteen tuen tarkistus (formaatit, esitystilat) ja VkSwapchainKHR:n luonti.
- [ ] **Input Manager:** Win32-viestien kaappaus, näppäin- ja hiiritilojen tallennus.

## Roadmap
- [x] Window Manager  
- [ ] Vulkan Initialization (Instance, Surface, Device, Swapchain)  
- [ ] Graphics Pipeline (Shaders, Render Pass, Framebuffers)  
- [ ] Hello Triangle Demo 
- [ ] Input Manager  
- [ ] Resource Management (textures, buffers, shaders)  
- [ ] Scene Graph basics  

## Kehitysympäristö
- **IDE:** Visual Studio  
- **Kieli:** C++20 (Standardi päivitetty!)
- **Alusta:** Windows (Win32 API + Vulkan SDK)  
- **Tekniset huomiot:** - Käytössä brace-initialization `{}` roskadatan välttämiseksi.
    - `cleanup()`-metodi hoitaa resurssien vapauttamisen käänteisessä järjestyksessä.

    ## Jatka tästä (Seuraava koodisessio)

**Ensimmäinen asia huomenna:**


*(Vinkki: Surface pitää luoda heti `createInstance()`-kutsun jälkeen, ennen laitteiden valintaa, jotta koodin rakenne on valmis tulevaa Swapchainia varten!)*
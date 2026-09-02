// Capa de dibujo con SDL2. Es comun a las dos versiones y se mantiene
// secuencial a proposito: SDL_Renderer no se puede usar desde varios hilos a
// la vez, asi que paralelizar aqui daria resultados indefinidos. Lo que si se
// reparte entre hilos es el calculo previo (fisica y constelacion), que vive
// en src/<version>/physics.cpp.
//
// El aspecto "neon" sale de dos decisiones:
//   - Mezcla aditiva para los elementos y las lineas: sobre fondo negro los
//     colores se suman, asi que donde se cruzan varias cosas la imagen se
//     aclara sola, como una luz real.
//   - Cada elemento se dibuja dos veces, un halo grande y translucido y un
//     nucleo pequeno y solido, ambos como abanicos de triangulos cuyo borde
//     se desvanece. Eso da el resplandor sin usar texturas ni shaders.

#include <cmath>
#include <cstdio>
#include <ctime>

#include "render.hpp"
#include "text.hpp"

namespace {

// El halo es este multiplo del nucleo.
constexpr float kGlowScale = 2.6f;
// Lados del poligono del halo. Mas que los del nucleo, para que se vea redondo.
constexpr int kGlowSides = 12;

// Opacidades del degradado de cada abanico: centro solido, borde transparente.
constexpr Uint8 kCoreCenterAlpha = 255;
constexpr Uint8 kCoreEdgeAlpha = 235;
constexpr Uint8 kGlowCenterAlpha = 45;
constexpr Uint8 kGlowEdgeAlpha = 0;

// Opacidad de una linea de conexion cuando los dos elementos estan pegados.
constexpr float kLinkMaxAlpha = 150.0f;

// Panel del reloj y del HUD.
constexpr int kClockScale = 4;           // Escala de la fuente del reloj.
constexpr int kHudScale = 2;             // Escala de la fuente del HUD.
constexpr int kPanelPaddingX = 22;       // Margen horizontal dentro del panel.
constexpr int kPanelPaddingY = 14;       // Margen vertical dentro del panel.
constexpr int kPanelCornerRadius = 12;   // Radio de las esquinas redondeadas.
constexpr int kPanelCornerSegments = 5;  // Tramos por esquina; mas = mas suave.
constexpr int kClockMarginTop = 18;      // Separacion del reloj respecto del borde.
constexpr int kHudMargin = 16;           // Separacion del HUD respecto de la esquina.

// Atenua una opacidad por el factor de resplandor de la configuracion.
Uint8 scaleAlpha(Uint8 alpha, float factor) {
    return static_cast<Uint8>(static_cast<float>(alpha) * factor);
}

// Agrega al buffer un poligono regular relleno, como abanico de triangulos que
// salen del centro. El color del centro y el del borde son distintos, asi que
// la interpolacion de SDL entre vertices produce el degradado del resplandor.
void pushFan(std::vector<SDL_Vertex>& vertices, std::vector<int>& indices,
             float centerX, float centerY, float radius, float startAngle, int sides,
             Color color, Uint8 centerAlpha, Uint8 edgeAlpha) {
    const int base = static_cast<int>(vertices.size());
    const SDL_FPoint noTexture{0.0f, 0.0f};

    vertices.push_back(SDL_Vertex{SDL_FPoint{centerX, centerY},
                                  SDL_Color{color.r, color.g, color.b, centerAlpha},
                                  noTexture});

    const float step = 2.0f * kPi / static_cast<float>(sides);
    for (int i = 0; i < sides; ++i) {
        const float angle = startAngle + step * static_cast<float>(i);
        vertices.push_back(SDL_Vertex{
            SDL_FPoint{centerX + radius * std::cos(angle), centerY + radius * std::sin(angle)},
            SDL_Color{color.r, color.g, color.b, edgeAlpha},
            noTexture});
    }

    // Un triangulo por lado: centro, vertice actual y el siguiente. El modulo
    // cierra el abanico uniendo el ultimo vertice con el primero.
    for (int i = 0; i < sides; ++i) {
        indices.push_back(base);
        indices.push_back(base + 1 + i);
        indices.push_back(base + 1 + (i + 1) % sides);
    }
}

// Agrega un rectangulo de esquinas redondeadas, tambien como abanico de
// triangulos alrededor de su centro. Se usa para el panel del reloj.
void pushRoundedRect(std::vector<SDL_Vertex>& vertices, std::vector<int>& indices,
                     float x, float y, float width, float height, float radius,
                     SDL_Color color) {
    // El radio no puede pasar de la mitad del lado mas corto, o las esquinas
    // se cruzarian entre si.
    const float maxRadius = 0.5f * (width < height ? width : height);
    if (radius > maxRadius) radius = maxRadius;

    const float centerX = x + width * 0.5f;
    const float centerY = y + height * 0.5f;
    const SDL_FPoint noTexture{0.0f, 0.0f};

    const int base = static_cast<int>(vertices.size());
    vertices.push_back(SDL_Vertex{SDL_FPoint{centerX, centerY}, color, noTexture});

    // Centros de los arcos de las cuatro esquinas, empezando por la de abajo a
    // la derecha y girando en el sentido de las manecillas en pantalla.
    const float cornerX[4] = {x + width - radius, x + radius, x + radius, x + width - radius};
    const float cornerY[4] = {y + height - radius, y + height - radius, y + radius, y + radius};
    const float cornerStart[4] = {0.0f, 0.5f * kPi, kPi, 1.5f * kPi};

    int perimeterCount = 0;
    for (int corner = 0; corner < 4; ++corner) {
        for (int step = 0; step <= kPanelCornerSegments; ++step) {
            const float angle = cornerStart[corner] +
                                0.5f * kPi * static_cast<float>(step) /
                                    static_cast<float>(kPanelCornerSegments);
            vertices.push_back(SDL_Vertex{
                SDL_FPoint{cornerX[corner] + radius * std::cos(angle),
                           cornerY[corner] + radius * std::sin(angle)},
                color, noTexture});
            ++perimeterCount;
        }
    }

    for (int i = 0; i < perimeterCount; ++i) {
        indices.push_back(base);
        indices.push_back(base + 1 + i);
        indices.push_back(base + 1 + (i + 1) % perimeterCount);
    }
}

// Hora local del sistema formateada como HH:MM:SS.
void formatLocalTime(char* buffer, std::size_t size) {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::snprintf(buffer, size, "%02d:%02d:%02d", local.tm_hour, local.tm_min, local.tm_sec);
}

} // namespace

bool pollQuitRequest() {
    // Hay que vaciar la cola cada cuadro aunque no interese el evento: si no,
    // el sistema operativo da la ventana por colgada.
    SDL_Event event;
    bool quit = false;

    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            quit = true;
        } else if (event.type == SDL_KEYDOWN) {
            const SDL_Keycode key = event.key.keysym.sym;
            if (key == SDLK_ESCAPE || key == SDLK_q) quit = true;
        }
    }

    return quit;
}

Renderer::~Renderer() {
    // Se libera en el orden inverso al de creacion, y se tolera que alguno sea
    // nulo porque el destructor tambien corre si init() fallo a la mitad.
    if (renderer_ != nullptr) SDL_DestroyRenderer(renderer_);
    if (window_ != nullptr) SDL_DestroyWindow(window_);
    if (SDL_WasInit(SDL_INIT_VIDEO) != 0) SDL_Quit();
}

bool Renderer::init(const Config& config) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "Error: no se pudo iniciar SDL (%s).\n", SDL_GetError());
        return false;
    }

    window_ = SDL_CreateWindow("Screensaver paralelo - UVG",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               config.width, config.height,
                               SDL_WINDOW_SHOWN);
    if (window_ == nullptr) {
        std::fprintf(stderr, "Error: no se pudo crear la ventana (%s).\n", SDL_GetError());
        return false;
    }

    Uint32 flags = SDL_RENDERER_ACCELERATED;
    if (config.vsync) flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer_ = SDL_CreateRenderer(window_, -1, flags);
    if (renderer_ == nullptr) {
        // Si no hay aceleracion disponible se intenta por software antes de
        // rendirse: mas lento, pero el programa corre igual.
        std::fprintf(stderr, "Aviso: sin renderer acelerado (%s), se usara software.\n", SDL_GetError());
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer_ == nullptr) {
        std::fprintf(stderr, "Error: no se pudo crear el renderer (%s).\n", SDL_GetError());
        return false;
    }

    // Fondo negro inicial. De aqui en adelante no se vuelve a borrar la
    // pantalla: cada cuadro solo la oscurece un poco, y eso deja la estela.
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    SDL_RenderPresent(renderer_);

    return true;
}

void Renderer::setTitle(const std::string& title) {
    if (window_ != nullptr) SDL_SetWindowTitle(window_, title.c_str());
}

bool Renderer::saveScreenshot(const char* path) const {
    if (renderer_ == nullptr) return false;

    int width = 0;
    int height = 0;
    if (SDL_GetRendererOutputSize(renderer_, &width, &height) != 0) {
        std::fprintf(stderr, "Error: no se pudo consultar el tamano del renderer (%s).\n", SDL_GetError());
        return false;
    }

    // Se pide una superficie con el mismo formato que usa el renderer para que
    // SDL no tenga que convertir los pixeles al leerlos.
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (surface == nullptr) {
        std::fprintf(stderr, "Error: no se pudo reservar la imagen (%s).\n", SDL_GetError());
        return false;
    }

    bool ok = true;
    if (SDL_RenderReadPixels(renderer_, nullptr, surface->format->format,
                             surface->pixels, surface->pitch) != 0) {
        std::fprintf(stderr, "Error: no se pudieron leer los pixeles (%s).\n", SDL_GetError());
        ok = false;
    } else if (SDL_SaveBMP(surface, path) != 0) {
        std::fprintf(stderr, "Error: no se pudo escribir '%s' (%s).\n", path, SDL_GetError());
        ok = false;
    }

    SDL_FreeSurface(surface);
    return ok;
}

void Renderer::drawFrame(const Simulation& sim, const Config& config, const Metrics& metrics) {
    drawTrail(config);
    drawLinks(sim);
    drawElements(sim, config);
    if (config.showClock) drawClock(config);
    if (config.showHud) drawHud(config, metrics);

    SDL_RenderPresent(renderer_);
}

void Renderer::drawTrail(const Config& config) {
    // En vez de borrar, se pinta un velo negro semitransparente encima del
    // cuadro anterior. Lo dibujado antes se va apagando poco a poco y eso es
    // lo que produce la cola de movimiento.
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, static_cast<Uint8>(config.trailFade * 255.0f));
    SDL_RenderFillRect(renderer_, nullptr);
}

void Renderer::drawLinks(const Simulation& sim) {
    if (sim.links.empty()) return;

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_ADD);

    for (const Link& link : sim.links) {
        const Element& a = sim.elements[link.a];
        const Element& b = sim.elements[link.b];

        // La linea toma el promedio de los dos colores, para que la conexion
        // se lea como algo que pertenece a los dos elementos y no a uno solo.
        const Uint8 red = static_cast<Uint8>((static_cast<int>(a.color.r) + b.color.r) / 2);
        const Uint8 green = static_cast<Uint8>((static_cast<int>(a.color.g) + b.color.g) / 2);
        const Uint8 blue = static_cast<Uint8>((static_cast<int>(a.color.b) + b.color.b) / 2);
        const Uint8 alpha = static_cast<Uint8>(link.strength * kLinkMaxAlpha);

        SDL_SetRenderDrawColor(renderer_, red, green, blue, alpha);
        SDL_RenderDrawLine(renderer_,
                           static_cast<int>(a.x), static_cast<int>(a.y),
                           static_cast<int>(b.x), static_cast<int>(b.y));
    }
}

void Renderer::drawElements(const Simulation& sim, const Config& config) {
    // Los buffers se vacian pero conservan su capacidad, asi que despues de
    // los primeros cuadros ya no se pide memoria nueva.
    vertices_.clear();
    indices_.clear();

    for (const Element& e : sim.elements) {
        // Pulso: el tamano late con un seno. La fase propia de cada elemento
        // hace que no laten todos al mismo tiempo.
        const float pulse = 1.0f + config.pulseAmount *
                                       std::sin(config.pulseSpeed * sim.time + e.phase);
        const float coreRadius = e.radius * pulse;

        pushFan(vertices_, indices_, e.x, e.y, coreRadius * kGlowScale, e.angle, kGlowSides,
                e.color, scaleAlpha(kGlowCenterAlpha, config.glow), kGlowEdgeAlpha);
        pushFan(vertices_, indices_, e.x, e.y, coreRadius, e.angle, config.coreSides,
                e.color, scaleAlpha(kCoreCenterAlpha, config.glow),
                scaleAlpha(kCoreEdgeAlpha, config.glow));
    }

    if (indices_.empty()) return;

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_ADD);
    SDL_RenderGeometry(renderer_, nullptr,
                       vertices_.data(), static_cast<int>(vertices_.size()),
                       indices_.data(), static_cast<int>(indices_.size()));
}

void Renderer::drawClock(const Config& config) {
    char clockText[16];
    formatLocalTime(clockText, sizeof(clockText));

    const int textW = textWidth(clockText, kClockScale);
    const int textH = textHeight(kClockScale);
    const int panelW = textW + 2 * kPanelPaddingX;
    const int panelH = textH + 2 * kPanelPaddingY;
    const int panelX = (config.width - panelW) / 2;
    const int panelY = kClockMarginTop;

    // El panel va con mezcla normal, no aditiva: tiene que TAPAR lo que pasa
    // por detras, no sumarse a ello, o el reloj se perderia entre las
    // particulas.
    vertices_.clear();
    indices_.clear();
    pushRoundedRect(vertices_, indices_,
                    static_cast<float>(panelX - 2), static_cast<float>(panelY - 2),
                    static_cast<float>(panelW + 4), static_cast<float>(panelH + 4),
                    static_cast<float>(kPanelCornerRadius + 2),
                    SDL_Color{90, 100, 115, 150});
    pushRoundedRect(vertices_, indices_,
                    static_cast<float>(panelX), static_cast<float>(panelY),
                    static_cast<float>(panelW), static_cast<float>(panelH),
                    static_cast<float>(kPanelCornerRadius),
                    SDL_Color{12, 14, 18, 225});

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(renderer_, nullptr,
                       vertices_.data(), static_cast<int>(vertices_.size()),
                       indices_.data(), static_cast<int>(indices_.size()));

    // La hora se dibuja sin resplandor, en gris claro: es informacion, no
    // parte de la animacion.
    drawText(renderer_, clockText, panelX + kPanelPaddingX, panelY + kPanelPaddingY,
             kClockScale, SDL_Color{225, 228, 235, 255});
}

void Renderer::drawHud(const Config& config, const Metrics& metrics) {
    // Durante el primer intervalo todavia no hay una medida de FPS: se muestran
    // guiones en vez de un cero que se leeria como si el programa no avanzara.
    char fpsText[16];
    if (metrics.fps > 0.0) {
        std::snprintf(fpsText, sizeof(fpsText), "%.1f", metrics.fps);
    } else {
        std::snprintf(fpsText, sizeof(fpsText), "--");
    }

    char line[96];
    std::snprintf(line, sizeof(line), "N %d   FPS %s   HILOS %d",
                  config.n, fpsText, metrics.threads);

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    drawText(renderer_, line,
             kHudMargin, config.height - textHeight(kHudScale) - kHudMargin,
             kHudScale, SDL_Color{150, 160, 175, 210});
}

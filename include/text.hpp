#ifndef TEXT_HPP
#define TEXT_HPP

#include <SDL.h>

// ---------------------------------------------------------------------------
// Dibujo de texto con una fuente de mapa de bits de 5x7 incluida en el propio
// programa (src/common/text.cpp).
//
// Se hace asi, y no con SDL2_ttf, a proposito: evita agregar una dependencia
// mas (y un archivo de fuente que habria que distribuir) solo para poner el
// reloj y un par de contadores en pantalla. Cada pixel de la fuente se dibuja
// como un rectangulo relleno, en un solo lote por cadena.
//
// Caracteres soportados: 0-9, A-Z, espacio y  : . , - = / % ( ) . Las
// minusculas se dibujan como mayusculas; cualquier otro caracter sale en
// blanco.
// ---------------------------------------------------------------------------

// Alto en pixeles de una linea de texto a la escala dada.
int textHeight(int scale);

// Ancho en pixeles que ocupara 'text' a la escala dada.
int textWidth(const char* text, int scale);

// Dibuja 'text' con su esquina superior izquierda en (x, y). Respeta el modo
// de mezcla que tenga puesto el renderer.
void drawText(SDL_Renderer* renderer, const char* text, int x, int y, int scale, SDL_Color color);

#endif // TEXT_HPP

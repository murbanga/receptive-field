#include "font.h"
#include <windows.h>
#include <gl/GL.h>

using namespace std;

Font::Font(const char *family, int size)
    :family(family), size(size), initialized(false), metrics(nullptr)
{

}

Font::~Font()
{
    delete[] metrics;
}

void Font::measure(const string &s, float &width, float &height)
{
    if (!initialized)init();
    int n = s.length();

    width = 0;
    height = 0;

    for (int i = 0; i < n; ++i) {
        width += metrics[s[i]].gmfCellIncX;
        height = max(height, metrics[s[i]].gmfBlackBoxY);
    }
    width *= size;
    height *= size;
}

void Font::draw(const string &s)
{
    if (!initialized)init();
    GLint old_list_base;
    glGetIntegerv(GL_LIST_BASE, &old_list_base);
    glPushMatrix();
    glScalef(size, size, 1);
    glListBase(1000);
    glCallLists(s.length(), GL_UNSIGNED_BYTE, s.c_str());
    glPopMatrix();
    glListBase(old_list_base);
}

void Font::init()
{
    metrics = new GLYPHMETRICSFLOAT[256];
    HDC hdc = wglGetCurrentDC();
    HFONT font = CreateFontA(10, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, family.c_str());
    HFONT prev = (HFONT)SelectObject(hdc, font);
    wglUseFontOutlines(hdc, 0, 255, 1000, 0.0f, 0.01f, WGL_FONT_LINES, metrics);
    SelectObject(hdc, prev);
    DeleteObject(font);
    initialized = true;
}

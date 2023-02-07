#include "font.h"
#include <glad/gl.h>
#ifdef WIN32
#include <windows.h>
#else
// FIXME
#endif
#include "GLFW/glfw3.h"

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/glfw3native.h"

using namespace std;

Font::Font(const char *family, float size)
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
    int n = (int)s.length();

    width = 0;
    height = 0;
#ifdef WIN32
    for (int i = 0; i < n; ++i) {
        width += metrics[s[i]].gmfCellIncX;
        height = max(height, metrics[s[i]].gmfBlackBoxY);
    }
#endif
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
    glListBase(starting_display_list);
    glCallLists((GLsizei)s.length(), GL_UNSIGNED_BYTE, s.c_str());
    glPopMatrix();
    glListBase(old_list_base);
}

void Font::init()
{
#ifdef WIN32
        auto *window = glfwGetCurrentContext();
        auto win32_window = glfwGetWin32Window(window);

        if(!metrics)
                metrics = new GLYPHMETRICSFLOAT[256];

        HDC hdc = GetDC(win32_window);

        HFONT font = CreateFontA(10, 0, 0, 0, FW_NORMAL, false, false, false,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, family.c_str());

        if (font)
        {
                HFONT prev = (HFONT)SelectObject(hdc, font);
                if (!wglUseFontOutlines(hdc, 0, 255, starting_display_list, 0.0f, 0.01f, WGL_FONT_LINES, metrics))
                {
                        printf("fail to initialize font '%s' with error %08x\n", family.c_str(), GetLastError());
                }

                SelectObject(hdc, prev);
                DeleteObject(font);
        }
#else
        // FIXME
#endif
        initialized = true;
}

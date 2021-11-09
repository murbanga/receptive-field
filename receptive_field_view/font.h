#pragma once
#include <string>
#include <memory>

typedef struct _GLYPHMETRICSFLOAT GLYPHMETRICSFLOAT;

class Font
{
public:
    Font(const char *family, float size);
    ~Font();

    void measure(const std::string &s, float &width, float &height);
    void draw(const std::string &s);

private:
    void init();

    bool initialized;
    std::string family;
    float size;
    GLYPHMETRICSFLOAT *metrics;
    const int starting_display_list = 10000;
};

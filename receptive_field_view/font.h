#pragma once
#include <string>
#include <memory>

typedef struct _GLYPHMETRICSFLOAT GLYPHMETRICSFLOAT;

class Font
{
public:
    Font(const char *family, int size);
    ~Font();

    void measure(const std::string &s, float &width, float &height);
    void draw(const std::string &s);

private:
    void init();

    bool initialized;
    std::string family;
    int size;
    GLYPHMETRICSFLOAT *metrics;
};

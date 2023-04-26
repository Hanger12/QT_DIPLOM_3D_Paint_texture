#pragma once
#include <string>
#include <map>
#include <memory>
#include "glfunctions.h"

class Texture
{
public:
    static std::shared_ptr<Texture> createTexture(GLuint _id, GLenum _target);
    static void setAnisotropicFilteringAll(float _anisotropicFiltering);

    Texture(const Texture &) = delete;
    Texture(const Texture &&) = delete;
    Texture &operator= (const Texture &) = delete;
    Texture &operator= (const Texture &&) = delete;
    ~Texture();
    GLuint getId() const;
    GLenum getTarget() const;

private:
    static std::map<std::string, std::weak_ptr<Texture>> textureMap;
    static float anisotropicFiltering;
    QOpenGLFunctions_3_3_Core *funcs;
    std::string filepath;
    GLuint id;
    GLenum target;


    explicit Texture(GLuint _id, GLenum _target);
};

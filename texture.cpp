#include "texture.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <functional>
#include <stb/stb_image.h>
#include <cassert>
#include "utility.h"


std::map<std::string, std::weak_ptr<Texture>> Texture::textureMap;
float Texture::anisotropicFiltering = 1.0f;

std::shared_ptr<Texture> Texture::createTexture(GLuint _id, GLenum _target)
{
    std::string idStr = std::to_string(_id);
    if (ContainerUtility::contains(textureMap, idStr))
    {
        return std::shared_ptr<Texture>(textureMap[idStr]);
    }
    else
    {
        std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture(_id, _target));
        textureMap[idStr] = texture;
        return texture;
    }
}


Texture::Texture(GLuint _id, GLenum _target)
    :funcs(getGLFunctions()),
      filepath(std::to_string(_id)),
      id(_id),
      target(_target)
{
}


Texture::~Texture()
{
    ContainerUtility::remove(textureMap, filepath);
    glDeleteTextures(1, &id);
}

GLuint Texture::getId() const
{
    return id;
}

GLenum Texture::getTarget() const
{
    return target;
}

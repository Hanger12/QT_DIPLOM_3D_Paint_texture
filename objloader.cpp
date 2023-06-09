#include "objloader.h"
#include <string>
#include <fstream>
#include <cassert>
#include <map>
#include <set>
#include <sstream>

std::vector<std::string> splitFaceString(const std::string &_input)
{
    std::vector<std::string> words;
    std::string word;
    std::stringstream stream(_input);

    // get '/' delimited sequences
    while (std::getline(stream, word, '/'))
    {
        bool found = false;

        // search for first non-empty, ' ' delimited sequence
        if (!word.empty())
        {
            std::string tmpWord;
            std::stringstream tmpStream(word);

            while (std::getline(tmpStream, tmpWord, ' '))
            {
                if (!tmpWord.empty())
                {
                    words.push_back(tmpWord);
                    found = true;
                    break;
                }
            }
        }

        // if sequence is empty or only contained ' ' store empty string
        if (!found)
        {
            words.push_back("");
        }
    }

    return words;
}

std::vector<std::string> splitLineString(const std::string &_input)
{
    std::vector<std::string> words;
    std::string word;
    std::stringstream stream(_input);

    while (std::getline(stream, word, ' '))
    {
        if (!word.empty())
        {
            words.push_back(word);
        }
    }

    return words;
}

IndexedMesh OBJLoader::loadOBJ(const char *_filepath, Error &_error)
{
    GLuint currentIndex = 0;
    std::map<std::string, GLuint> vertexToIndexMap;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<QVector3D> positions;
    std::vector<QVector3D> normals;
    std::vector<QVector2D> texCoords;

    _error = Error::SUCCESS;

    // parse file
    std::ifstream objFile(_filepath);
    bool objectFound = false;
    std::string line;
    while (std::getline(objFile, line))
    {
        // skip empty lines and comments
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::vector<std::string> lineParts = splitLineString(line);

        // assert for debugging, error code for release error reporting to user
        assert(!lineParts.empty());
        if (lineParts.empty())
        {
            _error = Error::FAILURE;
            return {};
        }

        // object
        if (lineParts[0] == "o")
        {
            // we only support one sub-mesh/object per file
            if (objectFound)
            {
                //break;
            }
            objectFound = true;
        }
        else if (lineParts[0] == "v")
        {
            assert(lineParts.size() == 4);
            if (lineParts.size() != 4)
            {
                _error = Error::FAILURE;
                return {};
            }
            positions.push_back(QVector3D(std::stof(lineParts[1]), std::stof(lineParts[2]), std::stof(lineParts[3])));
        }
        // normal
        else if (lineParts[0] == "vn")
        {
            assert(lineParts.size() == 4);
            if (lineParts.size() != 4)
            {
                _error = Error::FAILURE;
                return {};
            }
            normals.push_back(QVector3D(std::stof(lineParts[1]), std::stof(lineParts[2]), std::stof(lineParts[3])));
        }
        // tex coord
        else if (lineParts[0] == "vt")
        {
            assert(lineParts.size() >= 3);
            if (lineParts.size() < 3)
            {
                _error = Error::FAILURE;
                return {};
            }
            texCoords.push_back(QVector2D(std::stof(lineParts[1]), std::stof(lineParts[2])));
        }
        // face
        else if (lineParts[0] == "f")
        {
            assert(lineParts.size() >= 4);
            if (lineParts.size() < 4)
            {
                _error = Error::FAILURE;
                return {};
            }
            std::vector<uint32_t> localIndices;

            for (unsigned int i = 1; i < lineParts.size(); ++i)
            {
                std::vector<std::string> faceParts = splitFaceString(lineParts[i]);// Utility::split(lineParts[i], "/");

                if (faceParts.empty() || faceParts[0].empty())
                {
                    _error = Error::FAILURE;
                    return {};
                }

                // add empty records if none are present
                while (faceParts.size() < 3)
                {
                    faceParts.push_back("");
                }

                int posIndex = std::stoi(faceParts[0]);
                int texCoordIndex = faceParts[1].empty() ? 0 : std::stoi(faceParts[1]);
                int normalIndex = faceParts[2].empty() ? 0 : std::stoi(faceParts[2]);

                Vertex vertex = {};

                // position
                {
                    if (posIndex < 0)
                    {
                        assert(-posIndex <= positions.size());
                        if (-posIndex > positions.size())
                        {
                            _error = Error::FAILURE;
                            return {};
                        }
                        posIndex = positions.size() + posIndex;
                    }
                    else
                    {
                        --posIndex;
                    }
                    assert(posIndex >= 0 && posIndex < positions.size());
                    if (posIndex < 0 || posIndex >= positions.size())
                    {
                        _error = Error::FAILURE;
                        return {};
                    }
                    vertex.position = positions[posIndex];
                }


                // texcoord
                {
                    if (texCoordIndex < 0)
                    {
                        assert(-texCoordIndex <= texCoords.size());
                        if (-texCoordIndex > texCoords.size())
                        {
                            _error = Error::FAILURE;
                            return {};
                        }
                        texCoordIndex = texCoords.size() + texCoordIndex;
                    }
                    else
                    {
                        --texCoordIndex;
                    }

                    // vertex will have default constructed texcoord if no texcoord is present in file
                    if (!faceParts[1].empty())
                    {
                        assert(texCoordIndex >= 0 && texCoordIndex < texCoords.size());
                        if (texCoordIndex < 0 || texCoordIndex >= texCoords.size())
                        {
                            _error = Error::FAILURE;
                            return {};
                        }
                        vertex.texCoord = texCoords[texCoordIndex];
                    }
                }


                // normal
                {
                    if (normalIndex < 0)
                    {
                        assert(-normalIndex <= normals.size());
                        if (-normalIndex > normals.size())
                        {
                            _error = Error::FAILURE;
                            return {};
                        }
                        normalIndex = normals.size() + normalIndex;
                    }
                    else
                    {
                        --normalIndex;
                    }

                    // vertex will have default constructed normal if no normal is present in file
                    if (!faceParts[2].empty())
                    {
                        assert(normalIndex >= 0 && normalIndex < normals.size());
                        if (normalIndex < 0 || normalIndex >= normals.size())
                        {
                            _error = Error::FAILURE;
                            return {};
                        }
                        vertex.normal = normals[normalIndex];
                    }
                }

                std::string vertexStr = std::to_string(posIndex) + " " + std::to_string(texCoordIndex) + " " + std::to_string(normalIndex);

                auto it = vertexToIndexMap.find(vertexStr);
                if (it != vertexToIndexMap.end())
                {
                    localIndices.push_back(it->second);
                }
                else
                {
                    vertexToIndexMap[vertexStr] = currentIndex;
                    localIndices.push_back(currentIndex);
                    vertices.push_back(vertex);
                    ++currentIndex;
                }
            }

            for (unsigned int i = 2; i < localIndices.size(); ++i)
            {
                indices.push_back(localIndices[0]);
                indices.push_back(localIndices[i - 1]);
                indices.push_back(localIndices[i]);
            }
        }
    }

    assert(currentIndex == vertices.size());

#ifdef _DEBUG
    std::set<uint32_t> indexSet(indices.begin(), indices.end());
    assert(indexSet.size() == vertices.size());
#endif // _DEBUG

    if (vertices.empty() || indices.empty())
    {
        _error = Error::FAILURE;
        return {};
    }

    return { vertices, indices };
}

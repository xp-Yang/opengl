#ifndef Renderer_hpp
#define Renderer_hpp
#include "Shader.hpp"
#include "Mesh.hpp"
#include "View.hpp"
#include <GLFW/glfw3.h>

enum class DrawMode {
    Indices,
    Arrays
};
class Renderer {
public:
    Renderer() = default;
    void draw(const Shader& shader, GLuint vao_id, DrawMode mode, int indices_count = 0, int array_count = 0) const;
    void render(const View& view);
};

#endif
#include "Renderer.hpp"
#include "GamePlay/ECS/Components.hpp"
#include "GamePlay/ECS/System.hpp"
#include <windows.h>

void Renderer::drawIndex(const Shader& shader, GLuint vao_id, size_t indices_count)
{
    shader.start_using();
    glBindVertexArray(vao_id);
    glDrawElements(GL_TRIANGLES, (int)indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::drawTriangle(const Shader& shader, GLuint vao_id, size_t array_count)
{
    shader.start_using();
    glBindVertexArray(vao_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)array_count);
    glBindVertexArray(0);
}

void Renderer::render_picking_border()
{
    glEnable(GL_STENCIL_TEST);// 为了渲染border
    glStencilMask(0xFF);

    auto& world = ecs::World::get();

    glm::mat4 camera_view = glm::mat4(1.0f);
    glm::mat4 camera_projection;
    for (auto entity : world.entityView<ecs::CameraComponent>()) {
        ecs::CameraComponent& camera = *world.getComponent<ecs::CameraComponent>(entity);
        camera_view = camera.view;
        camera_projection = camera.projection;
    }

    for (auto entity : world.entityView<ecs::PickedComponent, ecs::RenderableComponent, ecs::TransformComponent>()) {
        auto& renderable = *world.getComponent<ecs::RenderableComponent>(entity);
        auto& model_matrix = *world.getComponent<ecs::TransformComponent>(entity);

        // render border
        glClear(/*GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | */GL_STENCIL_BUFFER_BIT);
        // TODO 去掉所有地方的static Shader*
        static Shader* border_shader = new Shader("resource/shader/border.vs", "resource/shader/border.fs");

        glStencilFunc(GL_ALWAYS, 1, 0xFF); //总是通过模板测试
        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE); //如果模板测试通过了，设置模板缓冲区为1
        glStencilMask(0xFF);

        for (int i = 0; i < renderable.primitives.size(); i++) {
            auto& mesh = renderable.primitives[i].mesh;
            auto& material = renderable.primitives[i].material;
            Shader* shader = material.shader; // TODO 怎么不是const Shader *, 编译器没报错
            shader->start_using();

            drawIndex(*shader, mesh.get_VAO(), mesh.get_indices_count());
            shader->stop_using();
        }

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF); //指定什么情况下通过模板测试，这里只有当前模板缓冲区不为1的部分才通过测试
        //API void glStencilFunc(GLenum func, GLint ref, GLuint mask);
        //func同深度测试一样，func 参数为 GL_NEVER、GL_LESS、GL_LEQUAL、GL_GREATER、GL_GEQUAL、GL_EQUAL、GL_NOTEQUAL、GL_ALWAYS。
        //ref是和当前模板缓冲中的值stencil进行比较的指定值，这个比较方式使用了第三个参数mask，例如GL_LESS通过，当且仅当
        //满足 : (ref & mask) < (stencil & mask).GL_GEQUAL通过，当且仅当(ref & mask) >= (stencil & mask)。
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //指定模板测试和深度测试通过或失败时执行的操作，这里什么都不做
        //API void glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);
        //sfail表示深度测试失败，dpfail表示模板测试通过但是深度测试失败，dppass表示模板测试和深度测试都通过时采取的行为。
        //参数是对应条件下执行的动作，例如GL_KEEP表示保留缓冲区中值，GL_REPLACE表示使用glStencilFunc设置的ref值替换。
        glStencilMask(0x00);

        border_shader->start_using();
        auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.05f));
        border_shader->setMatrix("model", 1, model_matrix.transform() * scale * glm::mat4(1.0f));
        border_shader->setMatrix("view", 1, camera_view);
        border_shader->setMatrix("projection", 1, camera_projection);
        for (int i = 0; i < renderable.primitives.size(); i++) {
            auto& mesh = renderable.primitives[i].mesh;
            drawIndex(*border_shader, mesh.get_VAO(), mesh.get_indices_count());
        }
        glStencilMask(0xFF);
    }
}

void Renderer::render_normal() {
    static Shader* normal_shader = new Shader("resource/shader/model.vs", "resource/shader/normal.fs", "resource/shader/normal.gs");

    auto& world = ecs::World::get();
    glm::mat4 camera_view;
    glm::mat4 camera_projection;
    for (auto entity : world.entityView<ecs::CameraComponent>()) {
        ecs::CameraComponent& camera = *world.getComponent<ecs::CameraComponent>(entity);
        camera_view = camera.view;
        camera_projection = camera.projection;
    }

    normal_shader->start_using();
    normal_shader->setMatrix("view", 1, camera_view);
    normal_shader->setMatrix("projection", 1, camera_projection);
    for (auto entity : world.entityView<ecs::RenderableComponent, ecs::TransformComponent>()) {
        auto& renderable = *world.getComponent<ecs::RenderableComponent>(entity);
        auto& model_matrix = *world.getComponent<ecs::TransformComponent>(entity);

        normal_shader->start_using();
        normal_shader->setMatrix("model", 1, model_matrix.transform());
        for (int i = 0; i < renderable.primitives.size(); i++) {
            auto& mesh = renderable.primitives[i].mesh;
            drawIndex(*normal_shader, mesh.get_VAO(), mesh.get_indices_count());
        }
    }
}

void Renderer::render_shadow_map() {
    glEnable(GL_DEPTH_TEST);
    // TODO 想办法放进view开启shadow_map的逻辑
    auto& world = ecs::World::get();

    static Shader* depth_shader = new Shader("resource/shader/depth.vs", "resource/shader/depth.fs");

    glm::mat4 light_ref_matrix;
    for (auto entity : world.entityView<ecs::LightComponent>()) {
        light_ref_matrix = world.getComponent<ecs::LightComponent>(entity)->lightReferenceMatrix();
    }

    depth_shader->start_using();
    depth_shader->setMatrix("view", 1, light_ref_matrix);

    for (auto entity : world.entityView<ecs::RenderableComponent, ecs::TransformComponent>()) {
        auto& renderable = *world.getComponent<ecs::RenderableComponent>(entity);
        auto& model_matrix = *world.getComponent<ecs::TransformComponent>(entity);

        depth_shader->start_using();
        depth_shader->setMatrix("model", 1, model_matrix.transform());
        for (int i = 0; i < renderable.primitives.size(); i++) {
            auto& mesh = renderable.primitives[i].mesh;
            drawIndex(*depth_shader, mesh.get_VAO(), mesh.get_indices_count());
        }
    }
}
#include "RenderPipeline.hpp"
#include "Pass/ShadowPass.hpp"
#include "Pass/GBufferPass.hpp"
#include "Pass/LightingPass.hpp"
#include "Pass/MainCameraPass.hpp"
#include "Pass/PickingPass.hpp"
#include "Pass/ScreenPass.hpp"
#include "Pass/RayTracingPass.hpp"
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>

void RenderPipeline::init()
{
    m_main_camera_pass = std::make_unique<MainCameraPass>();
    m_shadow_pass = std::make_unique<ShadowPass>();
    m_gbuffer_pass = std::make_unique<GBufferPass>();
    m_lighting_pass = std::make_unique<LightingPass>();
    m_picking_pass = std::make_unique<PickingPass>();
    m_screen_pass = std::make_unique<ScreenPass>();
    m_shadow_pass->init();
    m_main_camera_pass->init();
    m_gbuffer_pass->init();
    m_lighting_pass->init();
    m_picking_pass->init();
    m_screen_pass->init();

    m_ray_tracing_pass = std::make_unique<RayTracingPass>();
    m_ray_tracing_pass->init();
}

void RenderPipeline::setRenderParams(const RenderParams& params)
{
    m_render_params = params;
}

void RenderPipeline::render()
{
    m_ray_tracing_pass->draw();

    if (ImGui::GetIO().WantPassThroughMouse) {
        m_picking_pass->draw();
    }
    if (m_render_params.shadow) {
        static_cast<ShadowPass*>(m_shadow_pass.get())->configSamples(m_render_params.shadow_map_sample_count);
        m_shadow_pass->draw();
    }
    if (m_render_params.pipeline_type == PIPELINE_TYPE::FORWARD) {
        if (m_render_params.shadow) {
            m_main_camera_pass->prepare(m_shadow_pass->getFrameBuffer());
        }
        else {
            m_main_camera_pass->prepare(nullptr);
        }
        static_cast<MainCameraPass*>(m_main_camera_pass.get())->configShader(m_render_params.reflection, m_render_params.normal_debug, m_render_params.wireframe);
        static_cast<MainCameraPass*>(m_main_camera_pass.get())->configSamples(m_render_params.msaa_sample_count);
        m_main_camera_pass->draw();

        m_screen_pass->prepare(m_main_camera_pass->getFrameBuffer());
        m_screen_pass->draw();
    }
    else if (m_render_params.pipeline_type == PIPELINE_TYPE::DEFERRED) {
        m_gbuffer_pass->draw();

        if (m_render_params.shadow) {
            static_cast<LightingPass*>(m_lighting_pass.get())->prepare(m_gbuffer_pass->getFrameBuffer(), m_shadow_pass->getFrameBuffer());
        }
        else {
            m_lighting_pass->prepare(m_gbuffer_pass->getFrameBuffer());
        }
        m_lighting_pass->draw();

        m_screen_pass->prepare(m_lighting_pass->getFrameBuffer());
        m_screen_pass->draw();
    }
}

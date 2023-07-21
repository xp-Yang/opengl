#include "MyCamera.hpp"

static glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
static const float CameraMovementSpeed = 1.0f;
static const float Sensitivity = 0.01f;

static void matrix_log(const glm::mat4 mat) 
{
    printf("%.3f %.3f %.3f %.3f \n", mat[0].x, mat[1].x, mat[2].x, mat[3].x);
    printf("%.3f %.3f %.3f %.3f \n", mat[0].y, mat[1].y, mat[2].y, mat[3].y);
    printf("%.3f %.3f %.3f %.3f \n", mat[0].z, mat[1].z, mat[2].z, mat[3].z);
    printf("%.3f %.3f %.3f %.3f \n", mat[0].w, mat[1].w, mat[2].w, mat[3].w);
   //printf("%.3f %.3f %.3f %.3f \n", mat[0][0], mat[1][0], mat[2][0], mat[3][0]);
   //printf("%.3f %.3f %.3f %.3f \n", mat[0][1], mat[1][1], mat[2][1], mat[3][1]);
   //printf("%.3f %.3f %.3f %.3f \n", mat[0][2], mat[1][2], mat[2][2], mat[3][2]);
   //printf("%.3f %.3f %.3f %.3f \n", mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
    std::cout << std::endl;
}

MyCamera::MyCamera(const glm::vec3& position, const glm::vec3& target)
    : m_pos (position)
    , m_target (target)
{
    //m_direction = m_pos - m_target;
    //m_view_matrix = glm::lookAt(m_pos, m_target, up);

    m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
    m_view_matrix = glm::lookAt(m_pos, m_pos + m_direction, up);
}

void MyCamera::surround_with_target(const float radius) {
    //float camX = sin(glfwGetTime()) * radius;
    //float camZ = cos(glfwGetTime()) * radius;
    //m_view_matrix = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}

void MyCamera::key_process(int key, float frame_time)
{
    // 每一帧持续时间越长，意味着上一帧的渲染花费了越多时间，所以这一帧的速度应该越大，来平衡渲染所花去的时间
    float frame_speed = CameraMovementSpeed * frame_time;

    switch (key) {
    case GLFW_KEY_W:
        m_pos -= glm::vec3(0.0f, 0.0f, 1.0f) * frame_speed;
        break;
    case GLFW_KEY_A:
        m_pos += glm::vec3(1.0f, 0.0f, 0.0f) * frame_speed;
        break;
    case GLFW_KEY_D:
        m_pos -= glm::vec3(1.0f, 0.0f, 0.0f) * frame_speed;
        break;
    case GLFW_KEY_S:
        m_pos += glm::vec3(0.0f, 0.0f, 1.0f) * frame_speed;
        break;
    case GLFW_KEY_Z:
        m_pos -= glm::vec3(0.0f, 1.0f, 0.0f) * frame_speed;
        break;
    case GLFW_KEY_C:
        m_pos += glm::vec3(0.0f, 1.0f, 0.0f) * frame_speed;
        break;
    case GLFW_KEY_1:
        break;
    case GLFW_KEY_2:
        break;
    case GLFW_KEY_3:
        break;
    case GLFW_KEY_4:
        break;
    case GLFW_KEY_SPACE:

        break;
    default:
        break;
    }

    m_view_matrix = glm::lookAt(m_pos, m_pos + m_direction, up);

    std::cout << key << std::endl;
    matrix_log(m_view_matrix);
}


void MyCamera::mouse_process(double delta_x, double delta_y)
{
    // get pitch
    m_pitch += delta_y * Sensitivity;
    // get yaw
    m_yaw += delta_x * Sensitivity;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    // update direction
    m_direction.x = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));
    m_direction.y = sin(glm::radians(m_pitch));
    m_direction.z = -cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    m_direction = glm::normalize(m_direction);

    m_view_matrix = glm::lookAt(m_pos, m_pos + m_direction, up);
}

void MyCamera::mouse_scroll_process(double yoffset)
{
    // adjust fov by zoom
}

const glm::vec3& MyCamera::get_position() {
    return m_pos;
}

const glm::vec3& MyCamera::get_target() {
    return m_target;
}

const glm::vec3& MyCamera::get_direction() {
    return m_direction;
}

const glm::vec3& MyCamera::get_right_direction() {
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, get_direction()));
    return cameraRight;
}

const glm::vec3& MyCamera::get_up_direction() {
    glm::vec3 cameraUp = glm::cross(m_direction, get_right_direction());
    return cameraUp;
}

const glm::mat4& MyCamera::get_view() {
    return m_view_matrix;
}
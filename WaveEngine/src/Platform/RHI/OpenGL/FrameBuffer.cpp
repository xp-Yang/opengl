#include "FrameBuffer.hpp"
#include <glad/glad.h>
#include <assert.h>

FrameBuffer::FrameBuffer(int width, int height, int samples)
{
    m_width = width;
    m_height = height;
    m_samples = samples;
    m_is_multi_sample = (m_samples > 1);
}

void FrameBuffer::create(const std::vector<AttachmentType>& attachments_type)
{
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    int color_attachment_index = -1;
    for (int i = 0; i < attachments_type.size(); i++) {
        if (attachments_type[i] == AttachmentType::RGBA || attachments_type[i] == AttachmentType::RGB16F) {
            color_attachment_index++;
        }
        Attachment attachment;
        attachment.create(attachments_type[i], color_attachment_index, m_samples, m_width, m_height);
        m_attachments.push_back(attachment);
    }

    int color_attachment_size = color_attachment_index + 1;
    if (color_attachment_size > 1) { //有2个及以上
        unsigned int* attachments = new unsigned int[color_attachment_size];
        for (unsigned int i = 0; i < color_attachment_size; i++) {
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(color_attachment_size, attachments);
        delete[] attachments;
        //glDrawBuffers函数并不是一个Draw Call,而是一个状态机参数设置的函数,它的作用是告诉OpenGL,把绘制output put填充到这些Attachment对应的Buffer里,所以这个函数在创建Framebuffer的时候就可以被调用了。
    }
}

void FrameBuffer::createDefault()
{
    m_fbo = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

unsigned int FrameBuffer::getFBO() const
{
    return m_fbo;
}

int FrameBuffer::getWidth() const
{
    return m_width;
}

int FrameBuffer::getHeight() const
{
    return m_height;
}

const std::vector<Attachment>& FrameBuffer::getAttachments() const
{
    return m_attachments;
}

const Attachment& FrameBuffer::getFirstAttachmentOf(AttachmentType type) const
{
    for (auto& attachment : m_attachments) {
        if (attachment.getType() == type)
        {
            return attachment;
        }
    }
    assert(false);
    return Attachment();
}

void FrameBuffer::setSamples(int samples)
{
    // 如果创建的时候纹理对象不是multi-sample，就不能更改采样数
    if (!isMultiSampled())
        return;

    m_samples = samples;
    for (auto& attachment : m_attachments) {
        if (attachment.getType() == AttachmentType::RGBA) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, attachment.getMap());
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGBA, m_width, m_height, GL_TRUE);
        }
        if (attachment.getType() == AttachmentType::DEPTH24STENCIL8) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, attachment.getMap());
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_DEPTH24_STENCIL8, m_width, m_height, GL_TRUE);
        }
        if (attachment.getType() == AttachmentType::DEPTH) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, attachment.getMap());
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_DEPTH_COMPONENT, m_width, m_height, GL_TRUE);
        }
    }
}

bool FrameBuffer::isMultiSampled() const
{
    return m_is_multi_sample;
}

void FrameBuffer::blitColorMapTo(FrameBuffer* dest)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, getFBO());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->getFBO());
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, dest->getWidth(), dest->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::blitDepthMapTo(FrameBuffer* dest)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, getFBO());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->getFBO());
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, dest->getWidth(), dest->getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // 默认视口变换填满framebuffer，(未填满的部分是clearColor)
    // frameBuffer 的大小总是整个窗口大小 (其实小点没啥关系，只是会糊)
    glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::unBind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::clear()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Attachment::create(AttachmentType type, int color_attachment_index, int samples, int width, int height)
{
    m_type = type;
    switch (m_type)
    {
    case AttachmentType::RGB16F: {
        glGenTextures(1, &m_map);
        glBindTexture(GL_TEXTURE_2D, m_map);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachment_index, GL_TEXTURE_2D, m_map, 0);
        break;
    }
    case AttachmentType::RGBA: {
        if (samples > 1) {
            glGenTextures(1, &m_map);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_map);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_TRUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachment_index, GL_TEXTURE_2D_MULTISAMPLE, m_map, 0);
        }
        else {
            glGenTextures(1, &m_map);
            glBindTexture(GL_TEXTURE_2D, m_map);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            //使用GL_FLOAT和GL_UNSIGNED_BYTE区别
            //glTexImage2D 是旧接口，可以使用glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachment_index, GL_TEXTURE_2D, m_map, 0);
        }
        break;
    }
    case AttachmentType::DEPTH24STENCIL8: {
        // 统一用texture附件，不使用rbo附件
        //glGenRenderbuffers(1, &rbo);
        //glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        if (samples > 1) {
            glGenTextures(1, &m_map);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_map);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, GL_TRUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_map, 0);
        }
        else {
            glGenTextures(1, &m_map);
            glBindTexture(GL_TEXTURE_2D, m_map);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_map, 0);
        }
        break;
    }
    case AttachmentType::DEPTH: {
        if (samples > 1) {
            glGenTextures(1, &m_map);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_map);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH_COMPONENT, width, height, GL_TRUE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_map, 0);
        }
        else {
            glGenTextures(1, &m_map);
            glBindTexture(GL_TEXTURE_2D, m_map);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_map, 0);
        }

        // if depth only:
        //不包含颜色缓冲的帧缓冲对象是不完整的，所以我们需要显式告诉OpenGL我们不适用任何颜色数据进行渲染。
        //glDrawBuffer(GL_NONE);
        //glReadBuffer(GL_NONE);
        break;
    }
    default:
        break;
    }
}

AttachmentType Attachment::getType() const
{
    return m_type;
}

unsigned int Attachment::getMap() const
{
    return m_map;
}

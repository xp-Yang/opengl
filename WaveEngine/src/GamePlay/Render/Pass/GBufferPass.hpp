#ifndef GBufferPass_hpp
#define GBufferPass_hpp

#include "../RenderPass.hpp"

class GBufferPass : public RenderPass {
public:
    void init() override;
    void prepare(FrameBuffer* framebuffer) override;
    void draw() override;
    FrameBuffer* getFrameBuffer() override;
};

#endif // !GBufferPass_hpp

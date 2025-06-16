/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <studentSolution/gpu.hpp>
#include <algorithm>
#include <cmath>

// Helper function to get vertex ID (with indexing support)
uint32_t getVertexID(GPUMemory const& mem, uint32_t vertexIndex) {
    if (mem.activatedVertexArray >= mem.maxVertexArrays) return vertexIndex;
    
    auto const& vao = mem.vertexArrays[mem.activatedVertexArray];
    
    if (vao.indexBufferID >= 0 && vao.indexBufferID < (int32_t)mem.maxBuffers) {
        // Indexed drawing
        auto const& indexBuffer = mem.buffers[vao.indexBufferID];
        if (!indexBuffer.data) return vertexIndex;
        
        auto const* indexData = (uint8_t*)indexBuffer.data + vao.indexOffset;
        
        switch (vao.indexType) {
            case IndexType::U8:
                if (vao.indexOffset + vertexIndex * sizeof(uint8_t) < indexBuffer.size)
                    return ((uint8_t*)indexData)[vertexIndex];
                break;
            case IndexType::U16:
                if (vao.indexOffset + vertexIndex * sizeof(uint16_t) < indexBuffer.size)
                    return ((uint16_t*)indexData)[vertexIndex];
                break;
            case IndexType::U32:
                if (vao.indexOffset + vertexIndex * sizeof(uint32_t) < indexBuffer.size)
                    return ((uint32_t*)indexData)[vertexIndex];
                break;
        }
    }
    
    return vertexIndex; // Non-indexed drawing
}

// Vertex assembly function
void assembleVertex(InVertex& inVertex, GPUMemory const& mem, uint32_t vertexID) {
    if (mem.activatedVertexArray >= mem.maxVertexArrays) return;
    
    auto const& vao = mem.vertexArrays[mem.activatedVertexArray];
    
    inVertex.gl_VertexID = vertexID;
    
    for (uint32_t i = 0; i < maxAttribs; ++i) {
        auto const& attrib = vao.vertexAttrib[i];
        
        if (attrib.type == AttribType::EMPTY || attrib.bufferID < 0 || attrib.bufferID >= (int32_t)mem.maxBuffers) {
            continue;
        }
        
        auto const& buffer = mem.buffers[attrib.bufferID];
        if (!buffer.data) continue;
        
        uint64_t offset = attrib.offset + attrib.stride * vertexID;
        if (offset >= buffer.size) continue;
        
        auto const* data = (uint8_t*)buffer.data + offset;
        
        switch (attrib.type) {
            case AttribType::FLOAT:
                inVertex.attributes[i].v1 = *(float*)data;
                break;
            case AttribType::VEC2:
                inVertex.attributes[i].v2 = *(glm::vec2*)data;
                break;
            case AttribType::VEC3:
                inVertex.attributes[i].v3 = *(glm::vec3*)data;
                break;
            case AttribType::VEC4:
                inVertex.attributes[i].v4 = *(glm::vec4*)data;
                break;
            case AttribType::UINT:
                inVertex.attributes[i].u1 = *(uint32_t*)data;
                break;
            case AttribType::UVEC2:
                inVertex.attributes[i].u2 = *(glm::uvec2*)data;
                break;
            case AttribType::UVEC3:
                inVertex.attributes[i].u3 = *(glm::uvec3*)data;
                break;
            case AttribType::UVEC4:
                inVertex.attributes[i].u4 = *(glm::uvec4*)data;
                break;
            default:
                break;
        }
    }
}

// Check if triangle is front-facing
bool isFrontFacing(glm::vec3 const& v0, glm::vec3 const& v1, glm::vec3 const& v2, 
                   BackfaceCulling const& backfaceCulling) {
    auto edge1 = v1 - v0;
    auto edge2 = v2 - v0;
    auto normal = glm::cross(edge1, edge2);
    
    bool isCounterClockwise = normal.z > 0;
    
    if (backfaceCulling.frontFaceIsCounterClockWise) {
        return isCounterClockwise;
    } else {
        return !isCounterClockwise;
    }
}

// Clipping against near plane
struct ClippedTriangle {
    OutVertex vertices[3];
    bool valid = false;
};

struct ClippedTriangleList {
    ClippedTriangle triangles[2]; // Maximum 2 triangles from clipping
    int count = 0;
};

bool isInsideNearPlane(glm::vec4 const& pos) {
    return pos.z >= -pos.w;
}

float getIntersectionT(glm::vec4 const& a, glm::vec4 const& b) {
    float denominator = (b.w - a.w) + (b.z - a.z);
    if (std::abs(denominator) < 1e-6f) return 0.0f; // Avoid division by zero
    return (-a.w - a.z) / denominator;
}

OutVertex interpolateVertex(OutVertex const& a, OutVertex const& b, float t, Program const& program) {
    OutVertex result;
    result.gl_Position = a.gl_Position + t * (b.gl_Position - a.gl_Position);
    
    for (uint32_t i = 0; i < maxAttribs; ++i) {
        if (program.vs2fs[i] == AttribType::EMPTY) continue;
        
        switch (program.vs2fs[i]) {
            case AttribType::FLOAT:
                result.attributes[i].v1 = a.attributes[i].v1 + t * (b.attributes[i].v1 - a.attributes[i].v1);
                break;
            case AttribType::VEC2:
                result.attributes[i].v2 = a.attributes[i].v2 + t * (b.attributes[i].v2 - a.attributes[i].v2);
                break;
            case AttribType::VEC3:
                result.attributes[i].v3 = a.attributes[i].v3 + t * (b.attributes[i].v3 - a.attributes[i].v3);
                break;
            case AttribType::VEC4:
                result.attributes[i].v4 = a.attributes[i].v4 + t * (b.attributes[i].v4 - a.attributes[i].v4);
                break;
            default:
                result.attributes[i] = a.attributes[i]; // Don't interpolate integers
                break;
        }
    }
    
    return result;
}

ClippedTriangleList clipTriangle(OutVertex const& v0, OutVertex const& v1, OutVertex const& v2, 
                                          Program const& program) {
    ClippedTriangleList result;
    result.count = 0;
    
    bool inside0 = isInsideNearPlane(v0.gl_Position);
    bool inside1 = isInsideNearPlane(v1.gl_Position);
    bool inside2 = isInsideNearPlane(v2.gl_Position);
    
    int insideCount = inside0 + inside1 + inside2;
    
    if (insideCount == 3) {
        // All inside - no clipping needed
        result.triangles[0].vertices[0] = v0;
        result.triangles[0].vertices[1] = v1;
        result.triangles[0].vertices[2] = v2;
        result.triangles[0].valid = true;
        result.count = 1;
    } else if (insideCount == 2) {
        // Two vertices inside - create two triangles
        OutVertex const* insideVerts[2];
        OutVertex const* outsideVert;
        int insideIdx = 0;
        
        if (inside0) insideVerts[insideIdx++] = &v0;
        if (inside1) insideVerts[insideIdx++] = &v1;
        if (inside2) insideVerts[insideIdx++] = &v2;
        
        if (!inside0) outsideVert = &v0;
        else if (!inside1) outsideVert = &v1;
        else outsideVert = &v2;
        
        float t1 = getIntersectionT(insideVerts[0]->gl_Position, outsideVert->gl_Position);
        float t2 = getIntersectionT(insideVerts[1]->gl_Position, outsideVert->gl_Position);
        
        OutVertex newV1 = interpolateVertex(*insideVerts[0], *outsideVert, t1, program);
        OutVertex newV2 = interpolateVertex(*insideVerts[1], *outsideVert, t2, program);
        
        result.triangles[0].vertices[0] = *insideVerts[0];
        result.triangles[0].vertices[1] = *insideVerts[1];
        result.triangles[0].vertices[2] = newV1;
        result.triangles[0].valid = true;
        
        result.triangles[1].vertices[0] = *insideVerts[1];
        result.triangles[1].vertices[1] = newV1;
        result.triangles[1].vertices[2] = newV2;
        result.triangles[1].valid = true;
        
        result.count = 2;
    } else if (insideCount == 1) {
        // One vertex inside - create one triangle
        OutVertex const* insideVert;
        OutVertex const* outsideVerts[2];
        int outsideIdx = 0;
        
        if (inside0) insideVert = &v0;
        else if (inside1) insideVert = &v1;
        else insideVert = &v2;
        
        if (!inside0) outsideVerts[outsideIdx++] = &v0;
        if (!inside1) outsideVerts[outsideIdx++] = &v1;
        if (!inside2) outsideVerts[outsideIdx++] = &v2;
        
        float t1 = getIntersectionT(insideVert->gl_Position, outsideVerts[0]->gl_Position);
        float t2 = getIntersectionT(insideVert->gl_Position, outsideVerts[1]->gl_Position);
        
        OutVertex newV1 = interpolateVertex(*insideVert, *outsideVerts[0], t1, program);
        OutVertex newV2 = interpolateVertex(*insideVert, *outsideVerts[1], t2, program);
        
        result.triangles[0].vertices[0] = *insideVert;
        result.triangles[0].vertices[1] = newV1;
        result.triangles[0].vertices[2] = newV2;
        result.triangles[0].valid = true;
        
        result.count = 1;
    }
    // If insideCount == 0, triangle is completely outside, return empty list
    
    return result;
}

// Perspective division and viewport transform
void transformToScreenSpace(OutVertex& vertex, uint32_t width, uint32_t height) {
    // Perspective division
    if (vertex.gl_Position.w != 0.0f) {
        vertex.gl_Position.x /= vertex.gl_Position.w;
        vertex.gl_Position.y /= vertex.gl_Position.w;
        vertex.gl_Position.z /= vertex.gl_Position.w;
    }
    
    // Viewport transformation
    vertex.gl_Position.x = (vertex.gl_Position.x + 1.0f) * 0.5f * width;
    vertex.gl_Position.y = (vertex.gl_Position.y + 1.0f) * 0.5f * height;
}

// Calculate 2D barycentric coordinates
glm::vec3 calculateBarycentric(glm::vec2 const& p, glm::vec2 const& a, glm::vec2 const& b, glm::vec2 const& c) {
    glm::vec2 v0 = c - a;
    glm::vec2 v1 = b - a;
    glm::vec2 v2 = p - a;
    
    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);
    
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    return glm::vec3(1.0f - u - v, v, u);
}

// Check if point is inside triangle
bool isInsideTriangle(glm::vec3 const& baryCoords) {
    return baryCoords.x >= 0.0f && baryCoords.y >= 0.0f && baryCoords.z >= 0.0f;
}

// Stencil test
bool performStencilTest(uint8_t stencilValue, StencilSettings const& settings) {
    if (!settings.enabled) return true;
    
    switch (settings.func) {
        case StencilFunc::NEVER:    return false;
        case StencilFunc::LESS:     return stencilValue < settings.refValue;
        case StencilFunc::LEQUAL:   return stencilValue <= settings.refValue;
        case StencilFunc::GREATER:  return stencilValue > settings.refValue;
        case StencilFunc::GEQUAL:   return stencilValue >= settings.refValue;
        case StencilFunc::EQUAL:    return stencilValue == settings.refValue;
        case StencilFunc::NOTEQUAL: return stencilValue != settings.refValue;
        case StencilFunc::ALWAYS:   return true;
    }
    return true;
}

// Apply stencil operation
uint8_t applyStencilOp(uint8_t currentValue, StencilOp op, uint32_t refValue) {
    switch (op) {
        case StencilOp::KEEP:      return currentValue;
        case StencilOp::ZERO:      return 0;
        case StencilOp::REPLACE:   return refValue;
        case StencilOp::INCR:      return currentValue == 255 ? 255 : currentValue + 1;
        case StencilOp::INCR_WRAP: return (currentValue + 1) % 256;
        case StencilOp::DECR:      return currentValue == 0 ? 0 : currentValue - 1;
        case StencilOp::DECR_WRAP: return currentValue == 0 ? 255 : currentValue - 1;
        case StencilOp::INVERT:    return ~currentValue;
    }
    return currentValue;
}

// Clear framebuffer functions
void clearColor(GPUMemory& mem, ClearColorCommand const& cmd) {
    auto& fbo = mem.framebuffers[mem.activatedFramebuffer];
    if (!fbo.color.data) return;
    
    for (uint32_t y = 0; y < fbo.height; ++y) {
        for (uint32_t x = 0; x < fbo.width; ++x) {
            void* pixel = getPixel(fbo.color, x, y);
            
            if (fbo.color.format == Image::U8) {
                uint8_t* colorPtr = (uint8_t*)pixel;
                for (uint32_t c = 0; c < fbo.color.channels; ++c) {
                    colorPtr[c] = (uint8_t)(cmd.value[c] * 255.0f);
                }
            } else if (fbo.color.format == Image::F32) {
                float* colorPtr = (float*)pixel;
                for (uint32_t c = 0; c < fbo.color.channels; ++c) {
                    colorPtr[c] = cmd.value[c];
                }
            }
        }
    }
}

void clearDepth(GPUMemory& mem, ClearDepthCommand const& cmd) {
    auto& fbo = mem.framebuffers[mem.activatedFramebuffer];
    if (!fbo.depth.data) return;
    
    for (uint32_t y = 0; y < fbo.height; ++y) {
        for (uint32_t x = 0; x < fbo.width; ++x) {
            float* depthPtr = (float*)getPixel(fbo.depth, x, y);
            *depthPtr = cmd.value;
        }
    }
}

void clearStencil(GPUMemory& mem, ClearStencilCommand const& cmd) {
    auto& fbo = mem.framebuffers[mem.activatedFramebuffer];
    if (!fbo.stencil.data) return;
    
    for (uint32_t y = 0; y < fbo.height; ++y) {
        for (uint32_t x = 0; x < fbo.width; ++x) {
            uint8_t* stencilPtr = (uint8_t*)getPixel(fbo.stencil, x, y);
            *stencilPtr = cmd.value;
        }
    }
}

// Rasterization and fragment processing
void rasterizeTriangle(GPUMemory& mem, OutVertex const& v0, OutVertex const& v1, OutVertex const& v2) {
    auto& program = mem.programs[mem.activatedProgram];
    auto& fbo = mem.framebuffers[mem.activatedFramebuffer];
    
    // Get screen coordinates
    glm::vec2 p0(v0.gl_Position.x, v0.gl_Position.y);
    glm::vec2 p1(v1.gl_Position.x, v1.gl_Position.y);
    glm::vec2 p2(v2.gl_Position.x, v2.gl_Position.y);
    
    // Bounding box
    int minX = std::max(0, (int)std::floor(std::min({p0.x, p1.x, p2.x})));
    int maxX = std::min((int)fbo.width - 1, (int)std::ceil(std::max({p0.x, p1.x, p2.x})));
    int minY = std::max(0, (int)std::floor(std::min({p0.y, p1.y, p2.y})));
    int maxY = std::min((int)fbo.height - 1, (int)std::ceil(std::max({p0.y, p1.y, p2.y})));
    
    // Check if triangle is front-facing (for stencil operations)
    bool frontFacing = isFrontFacing(glm::vec3(p0, 0), glm::vec3(p1, 0), glm::vec3(p2, 0), mem.backfaceCulling);
    
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            glm::vec2 pixelCenter(x + 0.5f, y + 0.5f);
            glm::vec3 baryCoords = calculateBarycentric(pixelCenter, p0, p1, p2);
            
            if (!isInsideTriangle(baryCoords)) continue;
            
            // Create fragment
            InFragment fragment;
            fragment.gl_FragCoord = glm::vec4(x + 0.5f, y + 0.5f, 0.0f, 1.0f);
            
            // Interpolate depth using 2D barycentric coordinates
            fragment.gl_FragCoord.z = baryCoords.x * v0.gl_Position.z +
                                     baryCoords.y * v1.gl_Position.z +
                                     baryCoords.z * v2.gl_Position.z;
            
            // Perspective-correct attribute interpolation
            float invW = baryCoords.x / v0.gl_Position.w +
                        baryCoords.y / v1.gl_Position.w +
                        baryCoords.z / v2.gl_Position.w;
            
            if (invW != 0.0f) {
                float perspectiveCorrect = 1.0f / invW;
                glm::vec3 perspectiveBary = glm::vec3(
                    baryCoords.x / v0.gl_Position.w * perspectiveCorrect,
                    baryCoords.y / v1.gl_Position.w * perspectiveCorrect,
                    baryCoords.z / v2.gl_Position.w * perspectiveCorrect
                );
                
                for (uint32_t i = 0; i < maxAttribs; ++i) {
                    if (program.vs2fs[i] == AttribType::EMPTY) continue;
                    
                    switch (program.vs2fs[i]) {
                        case AttribType::FLOAT:
                            fragment.attributes[i].v1 = perspectiveBary.x * v0.attributes[i].v1 +
                                                       perspectiveBary.y * v1.attributes[i].v1 +
                                                       perspectiveBary.z * v2.attributes[i].v1;
                            break;
                        case AttribType::VEC2:
                            fragment.attributes[i].v2 = perspectiveBary.x * v0.attributes[i].v2 +
                                                       perspectiveBary.y * v1.attributes[i].v2 +
                                                       perspectiveBary.z * v2.attributes[i].v2;
                            break;
                        case AttribType::VEC3:
                            fragment.attributes[i].v3 = perspectiveBary.x * v0.attributes[i].v3 +
                                                       perspectiveBary.y * v1.attributes[i].v3 +
                                                       perspectiveBary.z * v2.attributes[i].v3;
                            break;
                        case AttribType::VEC4:
                            fragment.attributes[i].v4 = perspectiveBary.x * v0.attributes[i].v4 +
                                                       perspectiveBary.y * v1.attributes[i].v4 +
                                                       perspectiveBary.z * v2.attributes[i].v4;
                            break;
                        default:
                            // Integer attributes use provoking vertex (v0)
                            fragment.attributes[i] = v0.attributes[i];
                            break;
                    }
                }
            }
            
            // Early per-fragment operations
            bool stencilPassed = true;
            bool depthPassed = true;
            
            // Stencil test
            uint8_t stencilValue = 0;
            if (fbo.stencil.data) {
                stencilValue = *(uint8_t*)getPixel(fbo.stencil, x, y);
                stencilPassed = performStencilTest(stencilValue, mem.stencilSettings);
                
                if (!stencilPassed && mem.stencilSettings.enabled) {
                    // Apply sfail operation
                    auto& ops = frontFacing ? mem.stencilSettings.frontOps : mem.stencilSettings.backOps;
                    stencilValue = applyStencilOp(stencilValue, ops.sfail, mem.stencilSettings.refValue);
                    if (!mem.blockWrites.stencil) {
                        *(uint8_t*)getPixel(fbo.stencil, x, y) = stencilValue;
                    }
                    continue;
                }
            }
            
            // Depth test
            if (fbo.depth.data) {
                float currentDepth = *(float*)getPixel(fbo.depth, x, y);
                depthPassed = fragment.gl_FragCoord.z < currentDepth;
                
                if (!depthPassed) {
                    // Apply dpfail operation
                    if (mem.stencilSettings.enabled && fbo.stencil.data) {
                        auto& ops = frontFacing ? mem.stencilSettings.frontOps : mem.stencilSettings.backOps;
                        stencilValue = applyStencilOp(stencilValue, ops.dpfail, mem.stencilSettings.refValue);
                        if (!mem.blockWrites.stencil) {
                            *(uint8_t*)getPixel(fbo.stencil, x, y) = stencilValue;
                        }
                    }
                    continue;
                }
            }
            
            // Fragment shader
            OutFragment outFragment;
            ShaderInterface si;
            si.uniforms = mem.uniforms;
            si.textures = mem.textures;
            si.gl_DrawID = mem.gl_DrawID;
            
            if (program.fragmentShader) {
                program.fragmentShader(outFragment, fragment, si);
            }
            
            // Late per-fragment operations
            if (outFragment.discard) continue;
            
            // Apply dppass stencil operation
            if (mem.stencilSettings.enabled && fbo.stencil.data) {
                auto& ops = frontFacing ? mem.stencilSettings.frontOps : mem.stencilSettings.backOps;
                stencilValue = applyStencilOp(stencilValue, ops.dppass, mem.stencilSettings.refValue);
                if (!mem.blockWrites.stencil) {
                    *(uint8_t*)getPixel(fbo.stencil, x, y) = stencilValue;
                }
            }
            
            // Write depth
            if (fbo.depth.data && !mem.blockWrites.depth) {
                *(float*)getPixel(fbo.depth, x, y) = fragment.gl_FragCoord.z;
            }
            
            // Write color with alpha blending
            if (fbo.color.data && !mem.blockWrites.color) {
                void* pixel = getPixel(fbo.color, x, y);
                
                if (fbo.color.format == Image::U8) {
                    uint8_t* colorPtr = (uint8_t*)pixel;
                    float alpha = outFragment.gl_FragColor.a;
                    
                    for (uint32_t c = 0; c < fbo.color.channels; ++c) {
                        float currentColor = colorPtr[c] / 255.0f;
                        float newColor = outFragment.gl_FragColor[c];
                        float blendedColor = currentColor * (1.0f - alpha) + newColor * alpha;
                        colorPtr[c] = (uint8_t)(glm::clamp(blendedColor, 0.0f, 1.0f) * 255.0f);
                    }
                } else if (fbo.color.format == Image::F32) {
                    float* colorPtr = (float*)pixel;
                    float alpha = outFragment.gl_FragColor.a;
                    
                    for (uint32_t c = 0; c < fbo.color.channels; ++c) {
                        float currentColor = colorPtr[c];
                        float newColor = outFragment.gl_FragColor[c];
                        colorPtr[c] = currentColor * (1.0f - alpha) + newColor * alpha;
                    }
                }
            }
        }
    }
}

// Draw function
void drawTriangles(GPUMemory& mem, uint32_t nofVertices) {
    auto& program = mem.programs[mem.activatedProgram];
    auto& fbo = mem.framebuffers[mem.activatedFramebuffer];
    
    ShaderInterface si;
    si.uniforms = mem.uniforms;
    si.textures = mem.textures;
    si.gl_DrawID = mem.gl_DrawID;
    
    for (uint32_t i = 0; i < nofVertices; i += 3) {
        // Vertex assembly and vertex shader for triangle
        OutVertex outVertices[3];
        
        for (int v = 0; v < 3; ++v) {
            if (i + v >= nofVertices) break;
            
            InVertex inVertex;
            uint32_t vertexID = getVertexID(mem, i + v);
            assembleVertex(inVertex, mem, vertexID);
            
            if (program.vertexShader) {
                program.vertexShader(outVertices[v], inVertex, si);
            }
        }
        
        if (i + 2 >= nofVertices) break;
        
        // Backface culling check before clipping
        if (mem.backfaceCulling.enabled) {
            glm::vec3 screenPos0 = glm::vec3(outVertices[0].gl_Position) / outVertices[0].gl_Position.w;
            glm::vec3 screenPos1 = glm::vec3(outVertices[1].gl_Position) / outVertices[1].gl_Position.w;
            glm::vec3 screenPos2 = glm::vec3(outVertices[2].gl_Position) / outVertices[2].gl_Position.w;
            
            if (!isFrontFacing(screenPos0, screenPos1, screenPos2, mem.backfaceCulling)) {
                continue; // Skip back-facing triangle
            }
        }
        
        // Clipping
        auto clippedTriangles = clipTriangle(outVertices[0], outVertices[1], outVertices[2], program);
        
        // Process clipped triangles
        for (int t = 0; t < clippedTriangles.count; ++t) {
            auto& clippedTri = clippedTriangles.triangles[t];
            if (!clippedTri.valid) continue;
            
            // Perspective division and viewport transform
            for (int v = 0; v < 3; ++v) {
                transformToScreenSpace(clippedTri.vertices[v], fbo.width, fbo.height);
            }
            
            // Rasterize triangle
            rasterizeTriangle(mem, clippedTri.vertices[0], clippedTri.vertices[1], clippedTri.vertices[2]);
        }
    }
}

// Main GPU run function
void student_GPU_run(GPUMemory&mem,CommandBuffer const&cb){
    for (uint32_t i = 0; i < cb.nofCommands && i < CommandBuffer::maxCommands; ++i) {
        auto const& cmd = cb.commands[i];
        
        switch (cmd.type) {
            case CommandType::BIND_FRAMEBUFFER:
                if (cmd.data.bindFramebufferCommand.id < mem.maxFramebuffers)
                    mem.activatedFramebuffer = cmd.data.bindFramebufferCommand.id;
                break;
                
            case CommandType::BIND_PROGRAM:
                if (cmd.data.bindProgramCommand.id < mem.maxPrograms)
                    mem.activatedProgram = cmd.data.bindProgramCommand.id;
                break;
                
            case CommandType::BIND_VERTEXARRAY:
                if (cmd.data.bindVertexArrayCommand.id < mem.maxVertexArrays)
                    mem.activatedVertexArray = cmd.data.bindVertexArrayCommand.id;
                break;
                
            case CommandType::BLOCK_WRITES_COMMAND:
                mem.blockWrites = cmd.data.blockWritesCommand.blockWrites;
                break;
                
            case CommandType::SET_BACKFACE_CULLING_COMMAND:
                mem.backfaceCulling.enabled = cmd.data.setBackfaceCullingCommand.enabled;
                break;
                
            case CommandType::SET_FRONT_FACE_COMMAND:
                mem.backfaceCulling.frontFaceIsCounterClockWise = cmd.data.setFrontFaceCommand.frontFaceIsCounterClockWise;
                break;
                
            case CommandType::SET_STENCIL_COMMAND:
                mem.stencilSettings = cmd.data.setStencilCommand.settings;
                break;
                
            case CommandType::SET_DRAW_ID:
                mem.gl_DrawID = cmd.data.setDrawIdCommand.id;
                break;
                
            case CommandType::USER_COMMAND:
                if (cmd.data.userCommand.callback) {
                    cmd.data.userCommand.callback(cmd.data.userCommand.data);
                }
                break;
                
            case CommandType::CLEAR_COLOR:
                clearColor(mem, cmd.data.clearColorCommand);
                break;
                
            case CommandType::CLEAR_DEPTH:
                clearDepth(mem, cmd.data.clearDepthCommand);
                break;
                
            case CommandType::CLEAR_STENCIL:
                clearStencil(mem, cmd.data.clearStencilCommand);
                break;
                
            case CommandType::DRAW:
                if (mem.activatedProgram < mem.maxPrograms && mem.activatedFramebuffer < mem.maxFramebuffers)
                    drawTriangles(mem, cmd.data.drawCommand.nofVertices);
                mem.gl_DrawID++;
                break;
                
            case CommandType::SUB_COMMAND:
                if (cmd.data.subCommand.commandBuffer) {
                    student_GPU_run(mem, *cmd.data.subCommand.commandBuffer);
                }
                break;
                
            default:
                break;
        }
    }
}
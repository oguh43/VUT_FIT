/*!
 * @file
 * @brief This file contains functions for model rendering
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <studentSolution/prepareModel.hpp>
#include <studentSolution/gpu.hpp>
#include <solutionInterface/uniformLocations.hpp>
#include <studentSolution/shaderFunctions.hpp>
#include <iostream>

static uint32_t drawCallCounter = 0;

void processNode(GPUMemory& mem, CommandBuffer& commandBuffer, Node const& node, 
                Model const& model, glm::mat4 const& parentMatrix) {
    // Combine current node's transformation with parent
    glm::mat4 currentMatrix = parentMatrix * node.modelMatrix;
    
    // If this node has a mesh, create draw commands
    if (node.mesh >= 0 && node.mesh < (int32_t)model.nofMeshes) {
        auto const& mesh = model.meshes[node.mesh];
        
        // Create vertex array for this mesh
        VertexArray vao = {};
        
        // Set up indexing if present
        if (mesh.indexBufferID >= 0) {
            vao.indexBufferID = mesh.indexBufferID;
            vao.indexOffset = mesh.indexOffset;
            vao.indexType = mesh.indexType;
        } else {
            vao.indexBufferID = -1;
        }
        
        // Set up vertex attributes
        if (mesh.position.type != AttribType::EMPTY) {
            vao.vertexAttrib[0] = mesh.position;
        }
        if (mesh.normal.type != AttribType::EMPTY) {
            vao.vertexAttrib[1] = mesh.normal;
        }
        if (mesh.texCoord.type != AttribType::EMPTY) {
            vao.vertexAttrib[2] = mesh.texCoord;
        }
        
        // Store vertex array in GPU memory
        mem.vertexArrays[drawCallCounter] = vao;
        
        // Calculate matrices for uniforms
        glm::mat4 modelMatrix = currentMatrix;
        glm::mat4 invTransposeModelMatrix = glm::transpose(glm::inverse(modelMatrix));
        
        // Set up uniforms for this draw call
        uint32_t modelMatrixLocation = getUniformLocation(drawCallCounter, MODEL_MATRIX);
        uint32_t invTransposeLocation = getUniformLocation(drawCallCounter, INVERSE_TRANSPOSE_MODEL_MATRIX);
        uint32_t diffuseColorLocation = getUniformLocation(drawCallCounter, DIFFUSE_COLOR);
        uint32_t textureIdLocation = getUniformLocation(drawCallCounter, TEXTURE_ID);
        uint32_t doubleSidedLocation = getUniformLocation(drawCallCounter, DOUBLE_SIDED);
        
        mem.uniforms[modelMatrixLocation].m4 = modelMatrix;
        mem.uniforms[invTransposeLocation].m4 = invTransposeModelMatrix;
        mem.uniforms[diffuseColorLocation].v4 = mesh.diffuseColor;
        mem.uniforms[textureIdLocation].i1 = mesh.diffuseTexture;
        mem.uniforms[doubleSidedLocation].v1 = mesh.doubleSided ? 1.0f : 0.0f;
        
        // Add commands to command buffer
        pushBindVertexArrayCommand(commandBuffer, drawCallCounter);
        pushSetBackfaceCullingCommand(commandBuffer, !mesh.doubleSided);
        pushDrawCommand(commandBuffer, mesh.nofIndices);
        
        drawCallCounter++;
    }
    
    // Recursively process children
    for (size_t i = 0; i < node.nofChildren; ++i) {
        processNode(mem, commandBuffer, node.children[i], model, currentMatrix);
    }
}

/**
 * @brief This function prepares model into memory and creates command buffer
 *
 * @param mem gpu memory
 * @param commandBuffer command buffer
 * @param model model structure
 */
void student_prepareModel(GPUMemory&mem,CommandBuffer&commandBuffer,Model const&model){
    drawCallCounter = 0;
    
    // Copy buffers to GPU memory
    for (size_t i = 0; i < model.nofBuffers; ++i) {
        mem.buffers[i] = model.buffers[i];
    }
    
    // Copy textures to GPU memory
    for (size_t i = 0; i < model.nofTextures; ++i) {
        mem.textures[i] = model.textures[i];
    }
    
    // Process all root nodes with identity matrix
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    for (size_t i = 0; i < model.nofRoots; ++i) {
        processNode(mem, commandBuffer, model.roots[i], model, identityMatrix);
    }
}

/**
 * @brief This function represents vertex shader of texture rendering method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param si shader interface
 */
void student_drawModel_vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&si){
    // Get input attributes
    glm::vec3 position = inVertex.attributes[0].v3;  // position in model space
    glm::vec3 normal = inVertex.attributes[1].v3;    // normal in model space
    glm::vec2 texCoord = inVertex.attributes[2].v2;  // texture coordinates
    
    // Get transformation matrices from uniforms
    glm::mat4 projectionViewMatrix = si.uniforms[getUniformLocation(si.gl_DrawID, PROJECTION_VIEW_MATRIX)].m4;
    glm::mat4 lightProjectionViewMatrix = si.uniforms[getUniformLocation(si.gl_DrawID, USE_SHADOW_MAP_MATRIX)].m4;
    glm::mat4 modelMatrix = si.uniforms[getUniformLocation(si.gl_DrawID, MODEL_MATRIX)].m4;
    glm::mat4 invTransposeModelMatrix = si.uniforms[getUniformLocation(si.gl_DrawID, INVERSE_TRANSPOSE_MODEL_MATRIX)].m4;
    
    // Transform position to world space
    glm::vec4 worldPosition = modelMatrix * glm::vec4(position, 1.0f);
    
    // Transform normal to world space
    glm::vec4 worldNormal = invTransposeModelMatrix * glm::vec4(normal, 0.0f);
    
    // Set output attributes
    outVertex.attributes[0].v3 = glm::vec3(worldPosition);           // world space position
    outVertex.attributes[1].v3 = glm::vec3(worldNormal);             // world space normal
    outVertex.attributes[2].v2 = texCoord;                           // texture coordinates
    outVertex.attributes[3].v4 = lightProjectionViewMatrix * worldPosition; // light space position
    
    // Transform to clip space for rasterization
    outVertex.gl_Position = projectionViewMatrix * worldPosition;
}

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param si shader interface
 */
void student_drawModel_fragmentShader(OutFragment&outFragment,InFragment const&inFragment,ShaderInterface const&si){
    // Get fragment attributes
    glm::vec3 worldPosition = inFragment.attributes[0].v3;
    glm::vec3 worldNormal = inFragment.attributes[1].v3;
    glm::vec2 texCoord = inFragment.attributes[2].v2;
    glm::vec4 lightSpacePosition = inFragment.attributes[3].v4;
    
    // Get uniforms
    glm::vec3 lightPosition = si.uniforms[getUniformLocation(si.gl_DrawID, LIGHT_POSITION)].v3;
    glm::vec3 cameraPosition = si.uniforms[getUniformLocation(si.gl_DrawID, CAMERA_POSITION)].v3;
    int32_t shadowMapId = si.uniforms[getUniformLocation(si.gl_DrawID, SHADOWMAP_ID)].i1;
    glm::vec3 ambientLightColor = si.uniforms[getUniformLocation(si.gl_DrawID, AMBIENT_LIGHT_COLOR)].v3;
    glm::vec3 lightColor = si.uniforms[getUniformLocation(si.gl_DrawID, LIGHT_COLOR)].v3;
    glm::vec4 diffuseColor = si.uniforms[getUniformLocation(si.gl_DrawID, DIFFUSE_COLOR)].v4;
    int32_t textureId = si.uniforms[getUniformLocation(si.gl_DrawID, TEXTURE_ID)].i1;
    float doubleSided = si.uniforms[getUniformLocation(si.gl_DrawID, DOUBLE_SIDED)].v1;
    
    // Normalize normal
    glm::vec3 normal = glm::normalize(worldNormal);
    
    // Handle double-sided surfaces
    if (doubleSided > 0.0f) {
        glm::vec3 viewDirection = glm::normalize(cameraPosition - worldPosition);
        if (glm::dot(normal, viewDirection) < 0.0f) {
            normal = -normal;
        }
    }
    
    // Get material diffuse color
    glm::vec3 materialColor;
    if (textureId >= 0) {
        // Use texture
        glm::vec4 textureColor = student_read_texture(si.textures[textureId], texCoord);
        materialColor = glm::vec3(textureColor);
    } else {
        // Use uniform diffuse color
        materialColor = glm::vec3(diffuseColor);
    }
    
    // Calculate lighting direction
    glm::vec3 lightDirection = glm::normalize(lightPosition - worldPosition);
    
    // Lambert diffuse lighting
    float diffuseFactor = std::max(0.0f, glm::dot(normal, lightDirection));
    
    // Shadow mapping
    float shadowFactor = 1.0f;
    if (shadowMapId >= 0) {
        // Perspective divide for light space position
        glm::vec3 projCoords = glm::vec3(lightSpacePosition) / lightSpacePosition.w;
        
        // Transform from [-1,1] to [0,1] range (assuming bias matrix is already applied)
        // If bias matrix is not applied, uncomment the following lines:
        // projCoords = projCoords * 0.5f + 0.5f;
        
        // Check if fragment is in shadow map bounds
        if (projCoords.x >= 0.0f && projCoords.x <= 1.0f && 
            projCoords.y >= 0.0f && projCoords.y <= 1.0f && 
            projCoords.z >= 0.0f && projCoords.z <= 1.0f) {
            
            // Sample shadow map
            float shadowMapDepth = student_read_textureClamp(si.textures[shadowMapId], glm::vec2(projCoords.x, projCoords.y)).r;
            
            // Compare depths with bias to prevent shadow acne
            float bias = 0.0001f;
            if (projCoords.z > shadowMapDepth + bias) {
                shadowFactor = 0.5f; // In shadow - reduce lighting by 50%
            }
        }
    }
    
    // Combine lighting
    glm::vec3 ambient = ambientLightColor * materialColor;
    glm::vec3 diffuse = lightColor * materialColor * diffuseFactor * shadowFactor;
    
    glm::vec3 finalColor = ambient + diffuse;
    
    // Set output
    outFragment.gl_FragColor = glm::vec4(finalColor, diffuseColor.a);
    outFragment.discard = false;
}
#version 450

layout (binding = 0) uniform GammaUBO {
    float gamma;
};

layout(input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput resolvedImage;

layout(location = 0) out vec4 outColor;

// const float gamma = 2.2;


void main() {
    vec3 color = subpassLoad(resolvedImage).rgb;

    color = pow(color, vec3(1.0 / gamma));

    outColor = vec4(color, 1.0);
}

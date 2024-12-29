#version 450

// Input Attachments로 Geometry Pass 데이터 받기
layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionAttachment; // Position 데이터
layout(input_attachment_index = 1, binding = 1) uniform subpassInput normalAttachment;   // Normal 데이터
layout(input_attachment_index = 2, binding = 2) uniform subpassInput albedoAttachment;   // Albedo 데이터

layout(binding = 3) uniform LightingInfo {
    vec3 lightPos;
    vec3 lightColor;
    vec3 cameraPos;
} lighting;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 fragPosition = subpassLoad(positionAttachment).rgb;
    vec3 fragNormal = normalize(subpassLoad(normalAttachment).rgb);
    vec3 albedo = subpassLoad(albedoAttachment).rgb;

    if (gl_FragCoord.z == 1.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 lightDir = normalize(lighting.lightPos - fragPosition);
    float diff = max(dot(fragNormal, lightDir), 0.0);

    vec3 viewDir = normalize(lighting.cameraPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient = 0.1 * albedo;
    vec3 diffuse = diff * albedo * lighting.lightColor;
    vec3 specular = spec * lighting.lightColor;

    outColor = vec4(ambient + diffuse + specular, 1.0);
    // outColor = vec4(fragPosition, 1.0);
    // outColor = vec4(fragNormal, 1.0);
    // outColor = vec4(albedo, 1.0);
}

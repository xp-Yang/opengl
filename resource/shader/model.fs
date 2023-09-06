#version 330 core

in GS_OUT{
    vec3 pass_color;
    vec3 pass_pos;
    vec2 pass_uv;
    vec3 pass_normal;
    vec4 FragPosLightSpace;
} fs_in;

struct Light {
    vec3 position;
    vec3 color;
};

struct Material {
    float ambient;
    sampler2D diffuse_map;
    sampler2D specular_map;
    sampler2D normal_map;
    sampler2D height_map;
    float shininess;
};

uniform Material material;
uniform Light light;
uniform vec3 camera_pos;

uniform sampler2D shadow_map;
uniform samplerCube skybox;
uniform bool enable_skybox_sample;

out vec4 out_color;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 1.还在裁剪空间，执行透视除法，变换到NDC空间
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 2.变换到[0,1]的范围, 便于采样
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0)
        return 0.0;

    float bias = 0.005;
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float closestDepth = texture(shadow_map, projCoords.xy).r;
    // 检查当前片段是否在阴影中
    float shadow = projCoords.z - closestDepth > bias ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    vec3 ambient_light = light.color * material.ambient * vec3(texture(material.diffuse_map, fs_in.pass_uv));

    //TODO normal需要变换成世界空间，但要注意不能带平移
    vec3 normal = normalize(fs_in.pass_normal);
    vec3 light_direction = normalize(light.position - fs_in.pass_pos);
    float diffuse_coef = max(dot(light_direction, normal), 0.0);
    vec3 diffuse_light = light.color * diffuse_coef * vec3(texture(material.diffuse_map, fs_in.pass_uv));

    vec3 view_direction = normalize(camera_pos - fs_in.pass_pos);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec_coef = pow(max(dot(view_direction, reflect_direction), 0.001), material.shininess);
    vec3 specular_light = light.color * spec_coef * vec3(texture(material.specular_map, fs_in.pass_uv));

    // 计算阴影
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);       
    vec3 lighting = (ambient_light + (1.0 - shadow) * (diffuse_light + specular_light)) * fs_in.pass_color;    
    out_color = vec4(lighting, 1.0);

    if(enable_skybox_sample){
        vec3 I = normalize(fs_in.pass_pos - camera_pos);
        vec3 R = reflect(I, normalize(normal));
        out_color = 0.33 * vec4(lighting, 1.0) + 0.66 * vec4(texture(skybox, R).rgb, 1.0);
    }

    //debug
    //out_color = vec4(fs_in.pass_color * (ambient_light + diffuse_light + specular_light), 1.0);
    //out_color = vec4(1.0, 0.0, 0.0, 1.0);
    //out_color = vec4(fs_in.pass_uv, 0.0, 1.0);
    //out_color = vec4(vec3(shadow), 1.0);
    //out_color = vec4(normal, 1.0);
    //out_color = vec4(abs(spec_coef),abs(spec_coef),abs(spec_coef), 1.0f);
    //out_color = vec4(abs(view_direction), 1.0f);
}
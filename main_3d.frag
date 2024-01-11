#version 330 core
out vec4 FragColor;

in vec3 chNormal;  
in vec3 chFragPos;  
in vec2 chUV;

uniform vec3 viewPos; 

// Direkciono svetlo
struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform DirLight dirLight;

// Tackasto svetlo
struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

// Reflektor svetlo
struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;  
};
uniform SpotLight spotLight;

uniform sampler2D uDiffMap1;
uniform sampler2D uSpecMap1;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
void main()
{
    // properties
    vec3 norm = normalize(chNormal);
    vec3 viewDir = normalize(viewPos - chFragPos);

    // phase 1: Directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        result += CalcPointLight(pointLights[i], norm, chFragPos, viewDir);  
    }
    // phase 3: Spot light
    result += CalcSpotLight(spotLight, norm, chFragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(uDiffMap1, chUV));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(uDiffMap1, chUV));
    vec3 specular = texture(uSpecMap1, chUV).r * spec * light.specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 

    // ambinet
    vec3 ambient = light.ambient * vec3(texture(uDiffMap1, chUV));
    ambient *= attenuation;
        
    // diffuse 
    vec3 diffuse = light.diffuse * diff * vec3(texture(uDiffMap1, chUV));
    diffuse *= attenuation;

    // specular
    vec3 specular = light.specular * spec * vec3(texture(uSpecMap1, chUV));
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
    
    // ambinet
    vec3 ambient = light.ambient * vec3(texture(uDiffMap1, chUV));
    ambient *= attenuation;

    // diffuse 
    vec3 diffuse = light.diffuse * diff * vec3(texture(uDiffMap1, chUV));
    diffuse *= attenuation;

    // specular
    vec3 specular = light.specular * spec * vec3(texture(uSpecMap1, chUV));
    specular *= attenuation;

    float theta = acos(dot(-lightDir, normalize(light.direction)));
    float epsilon = radians(light.cutOff);

    vec3 result = ambient;
    if (theta < epsilon) {
      result += diffuse;
      result += specular;
    }
    return result;
}
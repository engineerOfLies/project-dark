#version 110

uniform sampler2D colorMap;
uniform sampler2D normalMap;

varying vec3 lightDir;
varying vec3 halfVector;

void main()
{
  vec3 n = normalize(texture2D(normalMap, gl_TexCoord[0].st).xyz * 2.0 - 1.0);
  vec3 l = normalize(lightDir);
  vec3 h = normalize(halfVector);

  float nDotL = max(0.0, dot(n, l));
  float nDotH = max(0.0, dot(n, h));
  float power = (nDotL == 0.0) ? 0.0 : pow(nDotH, gl_FrontMaterial.shininess);
    
  vec4 ambient = gl_FrontLightProduct[0].ambient;
  vec4 diffuse = gl_FrontLightProduct[0].diffuse * nDotL;
  vec4 specular = gl_FrontLightProduct[0].specular * power;
  vec4 color = gl_FrontLightModelProduct.sceneColor + ambient + diffuse + specular;
    
  gl_FragColor = color * texture2D(colorMap, gl_TexCoord[0].st);
} 

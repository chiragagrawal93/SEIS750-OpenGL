#version 150
in vec4 color;
out vec4  fColor;
in vec2 fTexCoord;
uniform sampler2D cloudTexture;	// Calling it day texture, but its actually cloud texture
uniform vec4 ambient_light;
in vec4 fvAmbientDiffuseColor;
uniform vec4 light_color;
uniform vec4 light_position;
in float fvSpecularExponent;
in vec4 fvSpecularColor;
in vec3 vN;
in vec4 position;


void main()
{
	vec3 L = normalize(light_position.xyz -position.xyz);
	vec3 E = normalize (-position.xyz);
	vec3 N = normalize (vN);
	vec3 H = normalize (L+E);

	vec4 tmpfvAmbientDiffuseColor = texture2D(cloudTexture, fTexCoord);	
	vec4 tmpfvSpecularColor = vec4(0,0,0,0);

	vec4 ambient = ambient_light*tmpfvAmbientDiffuseColor;
	vec4 diffuse = light_color * tmpfvAmbientDiffuseColor * max(0.0, dot(L, N));
	vec4 specular = light_color * tmpfvSpecularColor * pow(max(0.0, dot(N, H)), fvSpecularExponent); // Specular

	fColor = ambient + diffuse;				// Not actually using specular, so omit it from here for now
	fColor.w = tmpfvAmbientDiffuseColor.w;
}
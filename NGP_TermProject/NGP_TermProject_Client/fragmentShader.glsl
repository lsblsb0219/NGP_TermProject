#version 330
in vec3 FragPos; //--- 위치값
in vec3 Normal; //--- 버텍스 세이더에서 받은 노멀값
in vec2 out_TexCoord;
out vec4 FragColor; //--- 최종 객체의 색 저장

uniform sampler2D outTexture1;
uniform sampler2D outTexture2;
uniform sampler2D outTexture3;
uniform sampler2D outTexture4;
uniform sampler2D outTexture5;
uniform sampler2D outTexture6;
uniform sampler2D outTexture7;
uniform sampler2D outTexture8;
uniform sampler2D outTexture9;
uniform sampler2D outTexture10;
uniform sampler2D outTexture11;
uniform sampler2D outTexture12;
uniform sampler2D outTexture13;
uniform sampler2D outTexture14;
uniform sampler2D outTexture15;
uniform sampler2D outTexture16;
uniform sampler2D outTexture17;
uniform sampler2D outTexture18;
uniform sampler2D outTexture19;
uniform sampler2D outTexture20;
uniform sampler2D outTexture21;
uniform sampler2D outTexture22;
uniform sampler2D outTexture23;
uniform sampler2D outTexture24;

uniform int index;
uniform vec3 objectColor;
uniform vec3 lightPos; //--- 조명의 위치
uniform vec3 lightColor; //--- 응용 프로그램에서 설정한 조명 색상

void main(void) 
{	
	FragColor = vec4(0.0, 0.0, 0.0, 1.0); // 기본값 (혹시 case 안 들어갈 때 대비)
	float ambientLight = 0.75; //--- 주변 조명 계수
	vec3 ambient = ambientLight * lightColor; //--- 주변 조명 값
	
	vec3 normalVector = normalize (Normal);
	vec3 lightDir = normalize (lightPos - FragPos); //--- 표면과 조명의 위치로 조명의 방향을 결정한다.
	float diffuseLight = max (dot (normalVector, lightDir), 0.0); //--- N과 L의 내적 값으로 강도 조절: 음수 방지
	vec3 diffuse = diffuseLight * lightColor; //--- 산란 반사 조명값: 산란반사값 * 조명색상값
	
	vec3 result = (ambient + diffuse) * objectColor; //--- 최종 조명 설정된 픽셀 색상: (주변+산란반사+거울반사조명)*객체 색상

	switch(index){
	case 0:
		FragColor = vec4 (result, 1.0);
		break;
	case 1:
		FragColor = ( texture (outTexture1, out_TexCoord) + 
		texture (outTexture2, out_TexCoord) + 
		texture (outTexture3, out_TexCoord) + 
		texture (outTexture4, out_TexCoord) + 
		texture (outTexture5, out_TexCoord) +
		texture (outTexture6, out_TexCoord) +
		texture (outTexture7, out_TexCoord) + 
		texture (outTexture8, out_TexCoord) + 
		texture (outTexture9, out_TexCoord) + 
		texture (outTexture10, out_TexCoord) + 
		texture (outTexture11, out_TexCoord) + 
		texture (outTexture12, out_TexCoord) + 
		texture (outTexture13, out_TexCoord) + 
		texture (outTexture14, out_TexCoord) + 
		texture (outTexture15, out_TexCoord) + 
		texture (outTexture16, out_TexCoord) +
		texture (outTexture18, out_TexCoord) + 
		texture (outTexture19, out_TexCoord)) *  vec4 (result, 1.0);
		break;

	case 2:
		FragColor = texture(outTexture17, vec2(1.0 - out_TexCoord.x, out_TexCoord.y));
		break;

	case 3:
		FragColor = texture(outTexture20, vec2(1.0 - out_TexCoord.x, out_TexCoord.y));
		break;

	case 4:
		FragColor = texture(outTexture21, vec2(1.0 - out_TexCoord.x, out_TexCoord.y));
		break;

	case 5:
		FragColor = texture(outTexture22, vec2(1.0 - out_TexCoord.x, out_TexCoord.y));
		break;

	case 6:
		FragColor = texture(outTexture23, vec2(1.0 - out_TexCoord.x, out_TexCoord.y));
		break;

	case 7:
		FragColor = texture(outTexture24, vec2(1.0 - out_TexCoord.x, out_TexCoord.y));
		break;
	}
}

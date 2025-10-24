#version 330
in vec3 FragPos; //--- ��ġ��
in vec3 Normal; //--- ���ؽ� ���̴����� ���� ��ְ�
in vec2 out_TexCoord;
out vec4 FragColor; //--- ���� ��ü�� �� ����

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

uniform int index;
uniform vec3 objectColor;
uniform vec3 lightPos; //--- ������ ��ġ
uniform vec3 lightColor; //--- ���� ���α׷����� ������ ���� ����

void main(void) 
{	
	float ambientLight = 0.75; //--- �ֺ� ���� ���
	vec3 ambient = ambientLight * lightColor; //--- �ֺ� ���� ��
	
	vec3 normalVector = normalize (Normal);
	vec3 lightDir = normalize (lightPos - FragPos); //--- ǥ��� ������ ��ġ�� ������ ������ �����Ѵ�.
	float diffuseLight = max (dot (normalVector, lightDir), 0.0); //--- N�� L�� ���� ������ ���� ����: ���� ����
	vec3 diffuse = diffuseLight * lightColor; //--- ��� �ݻ� ����: ����ݻ簪 * �������
	
	vec3 result = (ambient + diffuse) * objectColor; //--- ���� ���� ������ �ȼ� ����: (�ֺ�+����ݻ�+�ſ�ݻ�����)*��ü ����

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
		texture (outTexture16, out_TexCoord) ) *  vec4 (result, 1.0);
		break;
	}
}

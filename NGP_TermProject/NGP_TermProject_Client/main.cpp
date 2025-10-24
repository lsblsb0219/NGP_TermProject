#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
//#include <gl/glm/glm.hpp>
//#include <gl/glm/ext.hpp>
//#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/glm/glm.hpp>
#include <gl/glm/glm/ext.hpp>
#include <gl/glm/glm/gtc/matrix_transform.hpp>
#include <Windows.h>
#include <ctime>
#include <random>

typedef struct Bounding_Box {
	GLfloat x1, z1, x2, z2;
}BB;

struct Robot {
	GLfloat size{}, x{}, z{}, road[2][2]{},
		speed = 0.0f,
		shake = 1, y_radian = 180.0f, // shake = (��,�ٸ�)ȸ�� ����, radian = �� y�� ȸ�� ����
		y{};
	BB bb{}; //���� ���, ������ �ϴ�
	int shake_dir{}, dir{};
	bool move = false; // �����̰� �ִ���(��� �� �̵�)
};
Robot player_robot, block_robot[19];

GLvoid drawScene();
GLvoid KeyBoard(unsigned char key, int x, int y);
GLvoid SpecialKeyBoard(int key, int x, int y);
GLvoid Reshape(int w, int h);
GLvoid TimerFunc(int x);
GLvoid Bump(int index);

int read_ten(int num);
BB get_bb(Robot robot);
bool collision(BB obj_a, BB obj_b);

void InitBuffer();
void InitTextures();
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading 
	if (!fptr) // Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
	length = ftell(fptr); // Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
	fclose(fptr); // Close the file 
	buf[length] = 0; // Null terminator 
	return buf; // Return the buffer 
}
GLubyte* LoadDIBitmap(const char* filename, BITMAPINFO** info)
{
	FILE* fp;
	GLubyte* bits;
	int bitsize{}, infosize{};
	BITMAPFILEHEADER header;
	//--- ���̳ʸ� �б� ���� ������ ����
	if ((fp = fopen(filename, "rb")) == NULL)
		return NULL;
	//--- ��Ʈ�� ���� ����� �д´�.
	if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1) {
		fclose(fp); return NULL;
	}
	//--- ������ BMP �������� Ȯ���Ѵ�.
	if (header.bfType != 'MB') {
		fclose(fp); return NULL;
	}
	//--- BITMAPINFOHEADER ��ġ�� ����.
	infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
	//--- ��Ʈ�� �̹��� �����͸� ���� �޸� �Ҵ��� �Ѵ�.
	if ((*info = (BITMAPINFO*)malloc(infosize)) == NULL) {
		fclose(fp); return NULL;
	}
	//--- ��Ʈ�� ���� ����� �д´�.
	if (fread(*info, 1, infosize, fp) < (unsigned int)infosize) {
		free(*info);
		fclose(fp); return NULL;
	}
	//--- ��Ʈ���� ũ�� ����
	if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
		bitsize = ((*info)->bmiHeader.biWidth *
			(*info)->bmiHeader.biBitCount + 7) / 8 *
		abs((*info)->bmiHeader.biHeight);
	//--- ��Ʈ���� ũ�⸸ŭ �޸𸮸� �Ҵ��Ѵ�.
	if ((bits = (GLubyte*)malloc(bitsize)) == NULL) {
		free(*info);
		fclose(fp); return NULL;
	}
	//--- ��Ʈ�� �����͸� bit(GLubyte Ÿ��)�� �����Ѵ�.
	if (fread(bits, 1, bitsize, fp) < (unsigned int)bitsize) {
		free(*info); free(bits);
		fclose(fp); return NULL;
	}
	fclose(fp);
	return bits;
}

GLint background_width, background_height;
float vertexPosition[] = {
	-1.0f, 1.0f, 1.0f, //�ո�
	-1.0f,-1.0f, 1.0f,
	 1.0f,-1.0f, 1.0f,
	 1.0f, 1.0f, 1.0f,

	-1.0f, 1.0f,-1.0f, //����
	-1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f, 1.0f,
	 1.0f, 1.0f,-1.0f,

	-1.0f, 1.0f,-1.0f, //���ʿ�
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,

	 1.0f, 1.0f,-1.0f, //�޸�
	 1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,

	-1.0f,-1.0f, 1.0f, //�Ʒ���
	-1.0f,-1.0f,-1.0f,
	 1.0f,-1.0f,-1.0f,
	 1.0f,-1.0f, 1.0f,

	 1.0f, 1.0f, 1.0f, //������ ��
	 1.0f,-1.0f, 1.0f,
	 1.0f,-1.0f,-1.0f,
	 1.0f, 1.0f,-1.0f,//24

	 1.0f, 0.0f, 0.0f, //x,y,z ��
	-1.0f, 0.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,
	 0.0f,-1.0f, 0.0f,
	 0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f,-1.0f, //30

	-1.0f, 0.0f, 1.0f, //�ٴ�
	-1.0f, 0.0f,-1.0f,
	 1.0f, 0.0f,-1.0f,
	 1.0f, 0.0f, 1.0f// 34
};//������ü, ��,�����ü ���͵�
float vertexNormal[] = {
	 0.0f, 0.0f, 1.0f,//�ո�
	 0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 1.0f,

	 0.0f, 1.0f, 0.0f,//����
	 0.0f, 1.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,

	-1.0f, 0.0f, 0.0f,//�޸�
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,

	 0.0f, 0.0f,-1.0f,//�޸�
	 0.0f, 0.0f,-1.0f,
	 0.0f, 0.0f,-1.0f,
	 0.0f, 0.0f,-1.0f,

	 0.0f,-1.0f, 0.0f,//�Ʒ�
	 0.0f,-1.0f, 0.0f,
	 0.0f,-1.0f, 0.0f,
	 0.0f,-1.0f, 0.0f,

	 1.0f, 0.0f, 0.0f,//������
	 1.0f, 0.0f, 0.0f,
	 1.0f, 0.0f, 0.0f,
	 1.0f, 0.0f, 0.0f,

	 //��
	   1.0f, 0.0f, 0.0f,
	  -1.0f, 0.0f, 0.0f,
	   0.0f, 1.0f, 0.0f,
	   0.0f,-1.0f, 0.0f,
	   0.0f, 0.0f, 1.0f,
	   0.0f, 0.0f,-1.0f,

	   0.0f, 1.0f, 0.0f,//�Ʒ�
	   0.0f, 1.0f, 0.0f,
	   0.0f, 1.0f, 0.0f,
	   0.0f, 1.0f, 0.0f
};//������ü, ��,�����ü �����
float vertexTexture[] = {
	//�ո�
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	//����
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	//�޸�
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	//�޸�
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	//�Ʒ�
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	//������
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	//��
   1.0f, 0.0f,
   1.0f, 0.0f,
   1.0f, 0.0f,
   1.0f, 0.0f,
   1.0f, 0.0f,
   1.0f, 0.0f,

   //�Ʒ�
   1.0f, 1.0f,
   1.0f, 0.0f,
   0.0f, 0.0f,
   0.0f, 1.0f
};//������ü, ��,�����ü �����

GLchar* vertexSource, * fragmentSource;
GLuint shaderID;
GLuint vertexShader;
GLuint fragmentShader;
unsigned int texture_runmap[16];
BITMAPINFO* bmp;

int main(int argc, char** argv)
{
	background_width = 1200, background_height = 800;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(background_width, background_height);
	glutCreateWindow("2024_Computer_Graphics_Final_Project");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	make_vertexShaders();
	make_fragmentShaders();
	shaderID = make_shaderProgram();
	InitBuffer();
	InitTextures();
	glutKeyboardFunc(KeyBoard);
	glutSpecialFunc(SpecialKeyBoard);
	glutTimerFunc(10, TimerFunc, 1);
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}
void make_vertexShaders()
{
	vertexSource = filetobuf("vertexShader.glsl");
	//--- ���ؽ� ���̴� ��ü �����
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexShader);
	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragmentShader.glsl");
	//--- �����׸�Ʈ ���̴� ��ü �����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	//--- �����׸�Ʈ ���̴� ������
	glCompileShader(fragmentShader);
	//--- �������� ����� ���� ���� ���: ������ ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}
GLuint make_shaderProgram()
{
	GLuint ShaderProgramID;

	ShaderProgramID = glCreateProgram();
	glAttachShader(ShaderProgramID, vertexShader);
	glAttachShader(ShaderProgramID, fragmentShader);
	glLinkProgram(ShaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];

	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		exit(-1);
	}
	glUseProgram(ShaderProgramID);

	return ShaderProgramID;
}
GLuint VAO, VBO[3];
time_t start_time, finish_time;
void InitBuffer()
{
	glGenVertexArrays(1, &VAO); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glGenBuffers(3, VBO); //--- 2���� VBO�� �����ϰ� �Ҵ��ϱ�

	glBindVertexArray(VAO); //--- VAO�� ���ε��ϱ�
	//--- 1��° VBO�� Ȱ��ȭ�Ͽ� ���ε��ϰ�, ���ؽ� �Ӽ� (��ǥ��)�� ����
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	//--- ���� diamond ���� ���ؽ� ������ ���� ���ۿ� �����Ѵ�.
	//--- triShape �迭�� ������: 9 * float
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosition), vertexPosition, GL_STATIC_DRAW);
	//--- ��ǥ���� attribute �ε��� 0���� ����Ѵ�: ���ؽ� �� 3* float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute �ε��� 0���� ��밡���ϰ� ��
	glEnableVertexAttribArray(0);

	//--- 2��° VBO�� Ȱ��ȭ �Ͽ� ���ε� �ϰ�, ���ؽ� �Ӽ� (����)�� ����
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	//--- ���� colors���� ���ؽ� ������ �����Ѵ�.
	//--- colors �迭�� ������: 9 *float 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormal), vertexNormal, GL_STATIC_DRAW);
	//--- ������ attribute �ε��� 1���� ����Ѵ�: ���ؽ� �� 3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute �ε��� 1���� ��� �����ϰ� ��.
	glEnableVertexAttribArray(1);

	//--- 2��° VBO�� Ȱ��ȭ �Ͽ� ���ε� �ϰ�, ���ؽ� �Ӽ� (����)�� ����
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	//--- ���� colors���� ���ؽ� ������ �����Ѵ�.
	//--- colors �迭�� ������: 9 *float 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexture), vertexTexture, GL_STATIC_DRAW);
	//--- ������ attribute �ε��� 1���� ����Ѵ�: ���ؽ� �� 3*float
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//--- attribute �ε��� 1���� ��� �����ϰ� ��.
	glEnableVertexAttribArray(2);

	player_robot.move = false;
	player_robot.y_radian = 180.0f, player_robot.shake_dir = 1;
	player_robot.x = -201, player_robot.z = 150, player_robot.y=0.f;
	{
		block_robot[0].road[0][0] = -203,	block_robot[0].road[0][1] = 140;
		block_robot[0].road[1][0] = -203,	block_robot[0].road[1][1] = -150;

		block_robot[1].road[0][0] = -199,	block_robot[1].road[0][1] = 140;
		block_robot[1].road[1][0] = -199,	block_robot[1].road[1][1] = -150;

		block_robot[2].road[0][0] = -201,	block_robot[2].road[0][1] = 130;
		block_robot[2].road[1][0] = -201,	block_robot[2].road[1][1] = -150;

		block_robot[3].road[0][0] = -202,	block_robot[3].road[0][1] = 0;
		block_robot[3].road[1][0] = -202,	block_robot[3].road[1][1] = 150;

		block_robot[4].road[0][0] = -200,	block_robot[4].road[0][1] = 0;
		block_robot[4].road[1][0] = -200,	block_robot[4].road[1][1] = 150;

		block_robot[5].road[0][0] = -201,	block_robot[5].road[0][1] = 5;
		block_robot[5].road[1][0] = -201,	block_robot[5].road[1][1] = 150;

		block_robot[6].road[0][0] = -195,	block_robot[6].road[0][1] = -153;
		block_robot[6].road[1][0] = -195,	block_robot[6].road[1][1] = -147;
		
		block_robot[7].road[0][0] = 200,	block_robot[7].road[0][1] = -150;
		block_robot[7].road[1][0] = 190,	block_robot[7].road[1][1] = -150;

		block_robot[8].road[0][0] = 200,	block_robot[8].road[0][1] = -153;
		block_robot[8].road[1][0] = -200,	block_robot[8].road[1][1] = -148;

		block_robot[9].road[0][0] = 200,	block_robot[9].road[0][1] = -152;
		block_robot[9].road[1][0] = -200,	block_robot[9].road[1][1] = -152;

		block_robot[10].road[0][0] = 198,	block_robot[10].road[0][1] = -150;
		block_robot[10].road[1][0] = -198,	block_robot[10].road[1][1] = -150;

		block_robot[11].road[0][0] = 196,	block_robot[11].road[0][1] = -148;
		block_robot[11].road[1][0] = -196,	block_robot[11].road[1][1] = -148;
		//
		block_robot[12].road[0][0] = 198,	block_robot[12].road[0][1] = -110;
		block_robot[12].road[1][0] = 204,	block_robot[12].road[1][1] = -110;

		block_robot[13].road[0][0] = 204,	block_robot[13].road[0][1] = -40;
		block_robot[13].road[1][0] = 198,	block_robot[13].road[1][1] = -40;

		block_robot[14].road[0][0] = 203,	block_robot[14].road[0][1] = -40;
		block_robot[14].road[1][0] = 203,	block_robot[14].road[1][1] = -110;

		block_robot[15].road[0][0] = 199,	block_robot[15].road[0][1] = -40;
		block_robot[15].road[1][0] = 199,	block_robot[15].road[1][1] = 20;

		block_robot[16].road[0][0] = 200,	block_robot[16].road[0][1] = 50;
		block_robot[16].road[1][0] = 200,	block_robot[16].road[1][1] = 140;

		block_robot[17].road[0][0] = 202,	block_robot[17].road[0][1] = 140;
		block_robot[17].road[1][0] = 202,	block_robot[17].road[1][1] = 50;

		block_robot[18].road[0][0] = 201,	block_robot[18].road[0][1] = 148;
		block_robot[18].road[1][0] = 201,	block_robot[18].road[1][1] = 148;
	}
	for (int i = 0; i < 19; ++i) {
		block_robot[i].x = block_robot[i].road[0][0];
		block_robot[i].z = block_robot[i].road[0][1];
		block_robot[i].speed = 0.2f, block_robot[i].shake_dir = 1;
		if (block_robot[i].road[0][0] < block_robot[i].road[1][0])
			block_robot[i].y_radian = 90.0f;
		if (block_robot[i].road[0][0] > block_robot[i].road[1][0])
			block_robot[i].y_radian = -90.0f;
		if (block_robot[i].road[0][1] < block_robot[i].road[1][1])
			block_robot[i].y_radian = 0.0f;
		if (block_robot[i].road[0][1] > block_robot[i].road[1][1])
			block_robot[i].y_radian = 180.0f;
	}
	player_robot.bb = get_bb(player_robot);
	start_time = int(time(NULL));
}
void InitTextures() 
{
	glGenTextures(16, texture_runmap);
	glUseProgram(shaderID);

	//--- texture[0]
	int tLocation1 = glGetUniformLocation(shaderID, "outTexture1"); //--- outTexture1 ������ ���÷��� ��ġ�� ������
	glUniform1i(tLocation1, 0); //--- ���÷��� 0�� �������� ����
	glBindTexture(GL_TEXTURE_2D, texture_runmap[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data1 = LoadDIBitmap("front.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data1);

	//--- texture[1]
	int tLocation2 = glGetUniformLocation(shaderID, "outTexture2"); //--- outTexture2 ������ ���÷��� ��ġ�� ������
	glUniform1i(tLocation2, 1);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data2 = LoadDIBitmap("up.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data2);

	//--- texture[2]
	int tLocation3 = glGetUniformLocation(shaderID, "outTexture3"); //--- outTexture3 ������ ���÷��� ��ġ�� ������
	glUniform1i(tLocation3, 2);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data3 = LoadDIBitmap("left.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data3);

	//--- texture[3]
	int tLocation4 = glGetUniformLocation(shaderID, "outTexture4"); //--- outTexture4 ������ ���÷��� ��ġ�� ������
	glUniform1i(tLocation4, 3);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data4 = LoadDIBitmap("back.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data4);

	//--- texture[4]
	int tLocation5 = glGetUniformLocation(shaderID, "outTexture5"); //--- outTexture5 ������ ���÷��� ��ġ�� ������
	glUniform1i(tLocation5, 4);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data5 = LoadDIBitmap("bottom.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data5);

	//--- texture[5]
	int tLocation6 = glGetUniformLocation(shaderID, "outTexture6"); //--- outTexture6 ������ ���÷��� ��ġ�� ������
	glUniform1i(tLocation6, 5);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data6 = LoadDIBitmap("right.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data6);

	//--- texture[6] - 0
	int tLocation7 = glGetUniformLocation(shaderID, "outTexture7");
	glUniform1i(tLocation7, 6);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data7 = LoadDIBitmap("N0.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data7);

	//--- texture[7] - 1
	int tLocation8 = glGetUniformLocation(shaderID, "outTexture8");
	glUniform1i(tLocation8, 7);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[7]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data8 = LoadDIBitmap("N1.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data8);

	//--- texture[8] - 2
	int tLocation9 = glGetUniformLocation(shaderID, "outTexture9");
	glUniform1i(tLocation9, 8);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[8]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data9 = LoadDIBitmap("N2.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data9);

	//--- texture[9] - 3
	int tLocation10 = glGetUniformLocation(shaderID, "outTexture10");
	glUniform1i(tLocation10, 9);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[9]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data10 = LoadDIBitmap("N3.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data10);

	//--- texture[10] - 4
	int tLocation11 = glGetUniformLocation(shaderID, "outTexture11");
	glUniform1i(tLocation11, 10);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[10]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data11 = LoadDIBitmap("N4.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data11);

	//--- texture[11] - 5
	int tLocation12 = glGetUniformLocation(shaderID, "outTexture12");
	glUniform1i(tLocation12, 11);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[11]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data12 = LoadDIBitmap("N5.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data12);

	//--- texture[12 - 6
	int tLocation13 = glGetUniformLocation(shaderID, "outTexture13");
	glUniform1i(tLocation13, 12);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[12]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data13 = LoadDIBitmap("N6.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data13);

	//--- texture[13] - 7
	int tLocation14 = glGetUniformLocation(shaderID, "outTexture14");
	glUniform1i(tLocation14, 13);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[13]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data14 = LoadDIBitmap("N7.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data14);

	//--- texture[14] - 8
	int tLocation15 = glGetUniformLocation(shaderID, "outTexture15");
	glUniform1i(tLocation15, 14);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[14]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data15 = LoadDIBitmap("N8.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data15);

	//--- texture[15] - 9
	int tLocation16 = glGetUniformLocation(shaderID, "outTexture16");
	glUniform1i(tLocation16, 15);
	glBindTexture(GL_TEXTURE_2D, texture_runmap[15]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* data16 = LoadDIBitmap("N9.bmp", &bmp);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_BGR, GL_UNSIGNED_BYTE, data16);
}

GLfloat camera_move[3]{ 0.0f, 2.0f, 2.5f }, camera_look[3]{ 0.0f, 0.5f, 0.0f }, light_pos[3]{ 0.0f, 2.0f, 2.0f }, camera_radian = 0.0f;
int end_anime;
BB map_bb{ -204.0f,-153.f,-198.f,151.f }, map_bb2{ -204.f,-153.f,204.f,-147.f }, map_bb3{ 198.0f,-153.f,204.f,151.f }, goal{198.f,149.f,204.f,151.f};
bool end = false;

GLvoid drawScene()
{
	glUseProgram(shaderID);
	glBindVertexArray(VAO);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	//�׳� ��===================================================================================================================================================================================
	if(!end){
		glViewport(0, 0, background_width, background_height);

		unsigned int modelLocation = glGetUniformLocation(shaderID, "modelTransform");//���� ��ȯ ��İ��� ���̴��� uniform mat4 modelTransform���� �Ѱ���
		unsigned int viewLocation = glGetUniformLocation(shaderID, "viewTransform");//���� ����
		unsigned int projectionLocation = glGetUniformLocation(shaderID, "projectionTransform");//���� ����

		//���� ����
		glm::mat4 kTransform = glm::mat4(1.0f);
		kTransform = glm::perspective(glm::radians(45.0f), 4.0f/3.0f, 0.1f, 300.0f);
		kTransform = glm::translate(kTransform, glm::vec3(0.0f, 0.0f, -8.0f));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &kTransform[0][0]);

		//���� ��ȯ
		glm::mat4 vTransform = glm::mat4(1.0f);
		glm::vec3 cameraPos = glm::vec3(player_robot.x - 1.0f * sin(glm::radians(player_robot.y_radian)), 0.5f, player_robot.z - 1.0f * cos(glm::radians(player_robot.y_radian))); //--- ī�޶� ��ġ
		glm::vec3 cameraDirection = glm::vec3(player_robot.x, 0.0f, player_robot.z); //--- ī�޶� �ٶ󺸴� ����
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- ī�޶� ���� ����

		vTransform = glm::rotate(vTransform, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
		//vTransform = glm::rotate(vTransform, glm::radians(30.0f), glm::vec3(0.0, 1.0, 0.0));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &vTransform[0][0]);

		//��
		glm::mat4 axisTransForm = glm::mat4(1.0f);//��ȯ ��� ���� T
		axisTransForm = glm::rotate(axisTransForm, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		axisTransForm = glm::rotate(axisTransForm, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axisTransForm));//��ȯ ����� ���̴��� ����

		unsigned int indexLocation = glGetUniformLocation(shaderID, "index"); //--- object Color�� ����
		unsigned int lightPosLocation = glGetUniformLocation(shaderID, "lightPos"); //--- lightPos �� ����
		unsigned int lightColorLocation = glGetUniformLocation(shaderID, "lightColor"); //--- lightColor �� ����
		unsigned int objColorLocation = glGetUniformLocation(shaderID, "objectColor"); //--- object Color�� ����

		//���� ��ġ �� ��
		glUniform3f(lightPosLocation, 200.0f, 200.0f, 150.0f);
		glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);

		//������Ʈ �� ����
		glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);

		glUniform1i(indexLocation, 0);

		/*���⿡ �κ�*/
		{
			glm::mat4 shapeTransForm = glm::mat4(1.0f);//��ȯ ��� ���� T
			shapeTransForm = glm::translate(shapeTransForm, glm::vec3(player_robot.x, player_robot.y, player_robot.z));      //robot��ġ
			shapeTransForm = glm::rotate(shapeTransForm, glm::radians(player_robot.y_radian), glm::vec3(0.0f, 1.0f, 0.0f));                 //���� ����
			shapeTransForm = glm::scale(shapeTransForm, glm::vec3(2.0f, 2.0f, 2.0f));                                                                      //size
			/*�����ٸ�*/ {
				glUniform3f(objColorLocation, 0.0f, 0.6f, 0.6f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.05f, 0.2f, 0.0f));                                                                //�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(player_robot.shake), glm::vec3(1.0f, 0.0f, 0.0f));                                  //�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));                                                                //��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));                                                                      //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*�޴ٸ�*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.05f, 0.2f, 0.0f));                                                               //�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(-player_robot.shake), glm::vec3(1.0f, 0.0f, 0.0f));                                 //�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));                                                                //��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));                                                                      //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* ���� */ {
				glUniform3f(objColorLocation, 0.6f, 0.0f, 0.6f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
				model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.05f));                                                                      //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*������*/ {
				glUniform3f(objColorLocation, 0.6f, 0.6f, 0.0f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(-player_robot.shake), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));                                                                    //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*��  ��*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(player_robot.shake), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));                                                                    //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* �Ӹ� */ {
				glUniform3f(objColorLocation, 0.6f, 0.0f, 0.6f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.55f, 0.0f));
				model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));                                                                       //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			}
		}
		/*�̰� ��ֹ� �κ�*/
		glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);
		for (int i = 0; i < 19; ++i) {
			glm::mat4 shapeTransForm = glm::mat4(1.0f);//��ȯ ��� ���� T
			shapeTransForm = glm::translate(shapeTransForm, glm::vec3(block_robot[i].x, 0.0f, block_robot[i].z));      //robot��ġ
			shapeTransForm = glm::rotate(shapeTransForm, glm::radians(block_robot[i].y_radian), glm::vec3(0.0f, 1.0f, 0.0f));                 //���� ����
			shapeTransForm = glm::scale(shapeTransForm, glm::vec3(2.0f, 2.0f, 2.0f));                                                                      //size
			/*�����ٸ�*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.05f, 0.2f, 0.0f));                                                                //�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(block_robot[i].shake), glm::vec3(1.0f, 0.0f, 0.0f));                                  //�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));                                                                //��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));                                                                      //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*�޴ٸ�*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.05f, 0.2f, 0.0f));                                                               //�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(-block_robot[i].shake), glm::vec3(1.0f, 0.0f, 0.0f));                                 //�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));                                                                //��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));                                                                      //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* ���� */ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
				model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.05f));                                                                      //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*������*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(-block_robot[i].shake), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));                                                                    //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*��  ��*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(block_robot[i].shake), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));                                                                    //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* �Ӹ� */ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.55f, 0.0f));
				model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));                                                                       //size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			}
		}
		/*�̰� �ϴ� ��*/
		{
			glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
			model = axisTransForm * model;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			glUniform3f(objColorLocation, 0.2f, 0.2f, 0.2f);
			glDrawArrays(GL_LINES, 24, 6); //��
		}
		/*����� ��(�ٴ�)*/
		{
			/*Ʈ��1*/
			{
				glUniform3f(objColorLocation, 0.75f, 0.75f, 1.0f);

				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(200.0f + 1.0f - 2.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(1.0f, 0.0f, 150.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, -150.0f + 2.0f));
				model = glm::scale(model, glm::vec3(200.0f, 0.0f, 1.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-200.0f - 1.0f + 2.0f, 0.0f, 1.0f));
				model = glm::scale(model, glm::vec3(1.0f, 0.0f, 150.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
			}

			/*Ʈ��2*/
			{
				glUniform3f(objColorLocation, 0.5f, 0.5f, 1.0f);

				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(200.0f + 1.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f, 0.0f, 150.0f + 1.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, -150.0f));
				model = glm::scale(model, glm::vec3(200.0f + 1.0f, 0.0f, 1.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-200.0f - 1.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(1.0f, 0.0f, 150.0f + 1.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
			}

			/*Ʈ��3*/
			{
				glUniform3f(objColorLocation, 0.25f, 0.25f, 1.0f);

				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(200.0f + 1.0f + 2.0f, 0.0f, -1.0f));
				model = glm::scale(model, glm::vec3(1.0f, 0.0f, 150.0f + 1.0f + 1.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, -150.0f - 2.0f));
				model = glm::scale(model, glm::vec3(200.0f + 1.0f + 1.0f, 0.0f, 1.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-200.0f - 1.0f - 2.0f, 0.0f, -1.0f));
				model = glm::scale(model, glm::vec3(1.0f, 0.0f, 150.0f + 1.0f + 1.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
			}
		}
		/*���*/
		{
			/*���1*/
			{
				glUniform3f(objColorLocation, 0.5f, 0.25f, 0.25f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(203.9f, 2.0f, 149.f));
				model = glm::scale(model, glm::vec3(0.1f, 2.0f, 0.1f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24);
			}

			/*���2*/
			{
				glUniform3f(objColorLocation, 0.5f, 0.25f, 0.25f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(198.1f, 2.0f, 149.f));
				model = glm::scale(model, glm::vec3(0.1f, 2.0f, 0.1f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24);
			}

			/*���*/
			{
				glUniform3f(objColorLocation, 1.f, 1.f, 1.f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(201.f, 3.2f, 148.f));
				model = glm::scale(model, glm::vec3(3.0f, 0.8f, 1.f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 4);
			}
		}
		/*Start ����*/
		{
			glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);

			glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
			model = glm::translate(model, glm::vec3(-201.0f, 0.5f, 149.0f));
			model = glm::scale(model, glm::vec3(3.0f, 0.0f, 0.1f));
			model = axisTransForm * model;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
		}
		/*ť���*/
		{
			glUniform1i(indexLocation, 1);
			glActiveTexture(GL_TEXTURE0); //--- ���� 0�� Ȱ��ȭ
			glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
			model = glm::scale(model, glm::vec3(210.0f, 10.0f, 160.0f));
			model = axisTransForm * model;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			glBindTexture(GL_TEXTURE_2D, texture_runmap[0]);
			glDrawArrays(GL_QUADS, 0, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[1]);
			glDrawArrays(GL_QUADS, 4, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[2]);
			glDrawArrays(GL_QUADS, 8, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[3]);
			glDrawArrays(GL_QUADS, 12, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[4]);
			glDrawArrays(GL_QUADS, 16, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[5]);
			glDrawArrays(GL_QUADS, 20, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
		}
	}
	//�̴� ��===================================================================================================================================================================================
	if(!end)  {
		glViewport(0 * background_width / 4, 3 * background_height / 4, background_width / 4, background_height / 4); /*���� �����ʻ�� ��򰡿� ��ġ �ٶ��*/

		unsigned int modelLocation = glGetUniformLocation(shaderID, "modelTransform");//���� ��ȯ ��İ��� ���̴��� uniform mat4 modelTransform���� �Ѱ���
		unsigned int viewLocation = glGetUniformLocation(shaderID, "viewTransform");//���� ����
		unsigned int projectionLocation = glGetUniformLocation(shaderID, "projectionTransform");//���� ����

		unsigned int indexLocation = glGetUniformLocation(shaderID, "index"); //--- object Color�� ����
		unsigned int lightPosLocation = glGetUniformLocation(shaderID, "lightPos"); //--- lightPos �� ����
		unsigned int lightColorLocation = glGetUniformLocation(shaderID, "lightColor"); //--- lightColor �� ����
		unsigned int objColorLocation = glGetUniformLocation(shaderID, "objectColor"); //--- object Color�� ����

		// ����
		glm::mat4 kTransform = glm::mat4(1.0f);
		kTransform = glm::ortho(-8.5f, 8.5f, -8.0f, 8.0f, -5.0f, 5.0f);
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &kTransform[0][0]);

		//���� ��ȯ
		glm::mat4 vTransform = glm::mat4(1.0f);
		glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -1.0f); //--- ī�޶� ��ġ
		glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- ī�޶� �ٶ󺸴� ����
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- ī�޶� ���� ����

		vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &vTransform[0][0]);

		//��
		glm::mat4 axisTransForm = glm::mat4(1.0f);//��ȯ ��� ���� T
		axisTransForm = glm::rotate(axisTransForm, glm::radians(player_robot.y_radian - 180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		axisTransForm = glm::translate(axisTransForm, glm::vec3(0.0f, 0.0f, 2.0f));
		axisTransForm = glm::rotate(axisTransForm, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axisTransForm));//��ȯ ����� ���̴��� ����

		glUniform3f(lightPosLocation, 0.0f, 3.0f, 0.0f);
		glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);

		//������Ʈ �� ����
		glUniform1i(indexLocation, 0);
		glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);

		/*����� ��(�ٴ�)*/
		{
			/*Ʈ��1*/
			{
				glUniform3f(objColorLocation, 0.75f, 0.75f, 0.75f);

				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(200.0f + 25.0f - 50.0f, 0.0f, -25.0f));
				model = glm::scale(model, glm::vec3(25.0f, 0.0f, 150.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 150.0f - 50.0f));
				model = glm::scale(model, glm::vec3(200.0f, 0.0f, 25.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(-200.0f - 25.0f + 50.0f, 0.0f, -25.0f));
				model = glm::scale(model, glm::vec3(25.0f, 0.0f, 150.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
			}

			/*Ʈ��2*/
			{
				glUniform3f(objColorLocation, 0.5f, 0.5f, 0.5f);

				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(200.0f + 25.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(25.0f, 0.0f, 150.0f + 25.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 150.0f));
				model = glm::scale(model, glm::vec3(200.0f + 25.0f, 0.0f, 25.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(-200.0f - 25.0f, 0.0f, 0.0f));
				model = glm::scale(model, glm::vec3(25.0f, 0.0f, 150.0f + 25.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
			}

			/*Ʈ��3*/
			{
				glUniform3f(objColorLocation, 0.25f, 0.25f, 0.25f);

				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(200.0f + 25.0f + 50.0f, 0.0f, 25.0f));
				model = glm::scale(model, glm::vec3(25.0f, 0.0f, 150.0f + 25.0f + 25.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, 150.0f + 50.0f));
				model = glm::scale(model, glm::vec3(200.0f + 25.0f + 25.0f, 0.0f, 25.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

				model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				model = glm::translate(model, glm::vec3(-200.0f - 25.0f - 50.0f, 0.0f, 25.0f));
				model = glm::scale(model, glm::vec3(25.0f, 0.0f, 150.0f + 25.0f + 25.0f));
				model = axisTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

				glDrawArrays(GL_QUADS, 30, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
			}
		}
		/*�÷��̾�*/
		{
			glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);

			glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
			model = glm::scale(model, glm::vec3(0.022f, 1.0f, 0.02f));
			model = glm::translate(model, glm::vec3(-player_robot.x, 1.0f, -player_robot.z));
			model = glm::scale(model, glm::vec3(50.0f, 0.0f, 50.0f));
			model = glm::rotate(model, glm::radians(player_robot.y_radian), glm::vec3(0.0f, 1.0f, 0.0f));
			model = axisTransForm * model;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_QUADS, 0, 24); //������ü
		}
	}
	//���� â===================================================================================================================================================================================
	if (end) {
		glViewport(0, 0, background_width, background_height);

		unsigned int modelLocation = glGetUniformLocation(shaderID, "modelTransform");				//���� ��ȯ ��İ��� ���̴��� uniform mat4 modelTransform���� �Ѱ���
		unsigned int viewLocation = glGetUniformLocation(shaderID, "viewTransform");				//���� ����
		unsigned int projectionLocation = glGetUniformLocation(shaderID, "projectionTransform");	//���� ����

		//���� ����
		glm::mat4 kTransform = glm::mat4(1.0f);
		kTransform = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
		kTransform = glm::translate(kTransform, glm::vec3(0.0f, 0.0f, -8.0f));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &kTransform[0][0]);

		//���� ��ȯ
		glm::mat4 vTransform = glm::mat4(1.0f);
		glm::vec3 cameraPos = glm::vec3(camera_move[0], camera_move[1], camera_move[2]);		//--- ī�޶� ��ġ
		glm::vec3 cameraDirection = glm::vec3(camera_look[0], camera_look[1], camera_look[2]);	//--- ī�޶� �ٶ󺸴� ����
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- ī�޶� ���� ����

		vTransform = glm::rotate(vTransform, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
		vTransform = glm::rotate(vTransform, glm::radians(camera_radian), glm::vec3(0.0, 1.0, 0.0));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &vTransform[0][0]);

		//��
		glm::mat4 axisTransForm = glm::mat4(1.0f);//��ȯ ��� ���� T
		axisTransForm = glm::rotate(axisTransForm, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		axisTransForm = glm::rotate(axisTransForm, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(axisTransForm));	//��ȯ ����� ���̴��� ����

		unsigned int indexLocation = glGetUniformLocation(shaderID, "index");			//--- object Color�� ����
		unsigned int lightPosLocation = glGetUniformLocation(shaderID, "lightPos");		//--- lightPos �� ����
		unsigned int lightColorLocation = glGetUniformLocation(shaderID, "lightColor");	//--- lightColor �� ����
		unsigned int objColorLocation = glGetUniformLocation(shaderID, "objectColor");	//--- object Color�� ����

		glm::vec4 light_pos_location = axisTransForm * glm::vec4(light_pos[0], light_pos[1], light_pos[2], 1.0f);
		//���� ��ġ �� ��
		glUniform3f(lightPosLocation, light_pos_location.x, light_pos_location.y, light_pos_location.z);
		glUniform3f(lightColorLocation, 1.0f, 1.0f, 1.0f);

		//������Ʈ �� ����
		glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);

		glUniform1i(indexLocation, 0);

		/*���⿡ �κ�*/
		{
			glm::mat4 shapeTransForm = glm::mat4(1.0f);//��ȯ ��� ���� T
			shapeTransForm = glm::translate(shapeTransForm, glm::vec3(player_robot.x, player_robot.y, player_robot.z));						//robot��ġ
			shapeTransForm = glm::rotate(shapeTransForm, glm::radians(player_robot.y_radian), glm::vec3(0.0f, 1.0f, 0.0f));					//���� ����
			shapeTransForm = glm::scale(shapeTransForm, glm::vec3(2.0f, 2.0f, 2.0f));														//size
			/*�����ٸ�*/ {
				glUniform3f(objColorLocation, 0.0f, 0.6f, 0.6f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.05f, 0.2f, 0.0f));																//�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(player_robot.shake), glm::vec3(0.0f, 0.0f, 1.0f));									//�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));																//��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*�޴ٸ�*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.05f, 0.2f, 0.0f));																//�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(-player_robot.shake), glm::vec3(0.0f, 0.0f, 1.0f));									//�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));																//��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* ���� */ {
				glUniform3f(objColorLocation, 0.6f, 0.0f, 0.6f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
				model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*������*/ {
				glUniform3f(objColorLocation, 0.6f, 0.6f, 0.0f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(-player_robot.shake * 3), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*��  ��*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(-player_robot.shake * 3), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* �Ӹ� */ {
				glUniform3f(objColorLocation, 0.6f, 0.0f, 0.6f);
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.55f, 0.0f));
				model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			}
		}
		/*�̰� ��ֹ� �κ�*/
		glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);
		for (int i = 0; i < 19; ++i) {
			glm::mat4 shapeTransForm = glm::mat4(1.0f);//��ȯ ��� ���� T
			shapeTransForm = glm::translate(shapeTransForm, glm::vec3(block_robot[i].x, 0.0f, block_robot[i].z));							//robot��ġ
			shapeTransForm = glm::rotate(shapeTransForm, block_robot[i].y_radian, glm::vec3(0.0f, 1.0f, 0.0f));				//���� ����
			shapeTransForm = glm::scale(shapeTransForm, glm::vec3(2.0f, 2.0f, 2.0f));														//size
			/*�����ٸ�*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.05f, 0.2f, 0.0f));																//�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(block_robot[i].shake), glm::vec3(0.0f, 0.0f, 1.0f));								//�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));																//��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*�޴ٸ�*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.05f, 0.2f, 0.0f));																//�� ��ġ�� ���� ����
				model = glm::rotate(model, glm::radians(-block_robot[i].shake), glm::vec3(0.0f, 0.0f, 1.0f));								//�ٸ� ����
				model = glm::translate(model, glm::vec3(0.0f, -0.1f, 0.0f));																//��������
				model = glm::scale(model, glm::vec3(0.05f, 0.1f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* ���� */ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
				model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*������*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(block_robot[i].shake), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::rotate(model, glm::radians(-block_robot[i].shake * 2), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /*��  ��*/ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(-0.125f, 0.5f, 0.0f));
				model = glm::rotate(model, glm::radians(-block_robot[i].shake), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::rotate(model, glm::radians(-block_robot[i].shake * 2), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, -0.13f, 0.0f));
				model = glm::scale(model, glm::vec3(0.025f, 0.13f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			} /* �Ӹ� */ {
				glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
				model = glm::translate(model, glm::vec3(0.0f, 0.55f, 0.0f));
				model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));																	//size
				model = axisTransForm * shapeTransForm * model;
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_QUADS, 0, 24); //������ü
			}
		}
		/*ť���*/
		{
			glUniform1i(indexLocation, 1);
			glActiveTexture(GL_TEXTURE0); //--- ���� 0�� Ȱ��ȭ
			glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
			model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
			model = glm::scale(model, glm::vec3(26.0f, 5.0f, 26.0f));
			model = axisTransForm * model;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			glBindTexture(GL_TEXTURE_2D, texture_runmap[0]);
			glDrawArrays(GL_QUADS, 0, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[1]);
			glDrawArrays(GL_QUADS, 4, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[2]);
			glDrawArrays(GL_QUADS, 8, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[3]);
			glDrawArrays(GL_QUADS, 12, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[4]);
			glDrawArrays(GL_QUADS, 16, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0

			glBindTexture(GL_TEXTURE_2D, texture_runmap[5]);
			glDrawArrays(GL_QUADS, 20, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
		}
		/*����*/
		int print_num = finish_time - start_time;
		for (int i = 0; i < end_anime && i < read_ten(finish_time - start_time); ++i) {
			glActiveTexture(GL_TEXTURE0); //--- ���� 0�� Ȱ��ȭ
			glm::mat4 model = glm::mat4(1.0f);//��ȯ ��� ���� T
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
			model = glm::rotate(model, glm::radians(camera_radian), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(1.0f * read_ten(finish_time - start_time) / 2 - 1.0f * i, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.0f));
			model = axisTransForm * model;
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			glBindTexture(GL_TEXTURE_2D, texture_runmap[6 + print_num % 10]);
			glDrawArrays(GL_QUADS, 0, 4); //�簢�� ũ�� 1.0 x 0.0 x 1.0
			print_num /= 10;
		}
	}

	glutSwapBuffers();
}
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid KeyBoard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'm':
		if (player_robot.move)
			player_robot.move = false;
		else
			player_robot.move = true;
		if (player_robot.shake_dir == 0)
			player_robot.shake_dir = 1;
		break;
	case't':
		if (!end)
			player_robot.x = 201, player_robot.z = 140, player_robot.y = 0.f, player_robot.y_radian = 0.0f;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid SpecialKeyBoard(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		if (!end)
			player_robot.y_radian += 45.0f;
		break;
	case GLUT_KEY_RIGHT:
		if (!end)
			player_robot.y_radian -= 45.0f;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid TimerFunc(int x)
{
	if (end) {
		if (end_anime == 0) {
			camera_radian += 1.0f;
			if (camera_radian == 180.0f)
				end_anime++;
		}
		else if (end_anime < read_ten(finish_time - start_time)) {
			light_pos[1] += 0.1f;
			end_anime++;
		}
		else if (end_anime == read_ten(finish_time - start_time)) {
			camera_radian += 1.0f;
			camera_move[2] += 0.1f;
			if (camera_radian == 360.0f)
				end_anime++;
		}
		else {
			player_robot.shake += player_robot.shake_dir * 1.0f;
			if (player_robot.shake <= 0.0f || player_robot.shake >= 60.0f)
				player_robot.shake_dir *= -1;
			for (int i = 0; i < 19; ++i) {
				block_robot[i].shake += block_robot[i].shake_dir * 5.0f * player_robot.speed;
				if (block_robot[i].shake <= 0.0f || block_robot[i].shake >= 60.0f)
					block_robot[i].shake_dir *= -1;
			}
		}
	}
	else {
		if (player_robot.move) {
			if (collision(map_bb, player_robot.bb) || collision(map_bb2, player_robot.bb) || collision(map_bb3, player_robot.bb)) {
				player_robot.x += sin(glm::radians(player_robot.y_radian)) * player_robot.speed;
				player_robot.z += cos(glm::radians(player_robot.y_radian)) * player_robot.speed;
				player_robot.bb = get_bb(player_robot);
			}
			else {
				player_robot.y -= 0.1f;
				player_robot.speed = 0.0f;
				player_robot.move = false;
			}
			player_robot.shake += player_robot.shake_dir * 20 * player_robot.speed;
			if (player_robot.shake <= -60.0f || player_robot.shake >= 60.0f)
				player_robot.shake_dir *= -1;
			if (player_robot.speed < 0.25f)
				player_robot.speed += 0.001f;

			if (collision(goal, player_robot.bb)) {
				finish_time = int(time(NULL));
				player_robot.x = 0.0f, player_robot.z = 0.0f, player_robot.y = 0.0f, player_robot.y_radian = 0.0f, 
					player_robot.shake = 0.0f, player_robot.shake_dir = 1;
				player_robot.move = false;
				end = true;
				std::cout << finish_time - start_time << '\n';
				std::cout << read_ten(finish_time - start_time) <<'\n';
			}
		}
		if (player_robot.y < 0) {
			player_robot.y -= player_robot.speed;
			player_robot.speed += 0.01f;

			if (player_robot.y < -5.f) {
				player_robot.y_radian = 180.0f, player_robot.shake_dir = 0, player_robot.shake = false, player_robot.speed = 0.0f;;
				player_robot.x = -201, player_robot.z = 150, player_robot.y = 0.f;
				player_robot.bb = get_bb(player_robot);
			}
		}

		for (int i = 0; i < 19; ++i) {
			if (end) {
				if (i < 2) {
					block_robot[i].x = 1.0f * (i % 5) - 2.0f;
					block_robot[i].z = -1.0f * (i / 5);
				}
				else {
					block_robot[i].x = 1.0f * ((i + 1) % 5) - 2.0f;
					block_robot[i].z = -1.0f * ((i + 1) / 5);
				}
				block_robot[i].y_radian = atan2(player_robot.x - player_robot.x, player_robot.x - player_robot.x);
				block_robot[i].shake = 0.0f; block_robot[i].shake_dir = 1;
			}
			else {
				if (player_robot.move && collision(block_robot[i].bb, player_robot.bb)) {
					player_robot.move = false;
					player_robot.x -= sin(glm::radians(player_robot.y_radian)) * player_robot.speed;
					player_robot.z -= cos(glm::radians(player_robot.y_radian)) * player_robot.speed;
					player_robot.speed = 0;
					GLfloat radian = atan2(player_robot.x - block_robot[i].x, player_robot.z - block_robot[i].z);
					player_robot.road[0][0] = block_robot[i].x, player_robot.road[0][1] = block_robot[i].z;
					player_robot.road[1][0] = block_robot[i].x + 2.0f * sin(radian), player_robot.road[1][1] = block_robot[i].z + 2.0f * cos(radian);
					glutTimerFunc(10, Bump, i);
				}
				else {
					block_robot[i].x += sin(glm::radians(block_robot[i].y_radian)) * block_robot[i].speed;
					block_robot[i].z += cos(glm::radians(block_robot[i].y_radian)) * block_robot[i].speed;
					block_robot[i].bb = get_bb(block_robot[i]);
					block_robot[i].shake += block_robot[i].shake_dir * 20 * block_robot[i].speed;
					if (block_robot[i].shake <= -60.0f || block_robot[i].shake >= 60.0f)
						block_robot[i].shake_dir *= -1;
					if ((block_robot[i].road[0][0] < block_robot[i].x and block_robot[i].x < block_robot[i].road[1][0]) ||
						(block_robot[i].road[0][0] > block_robot[i].x and block_robot[i].x > block_robot[i].road[1][0]) ||
						(block_robot[i].road[0][1] < block_robot[i].z and block_robot[i].z < block_robot[i].road[1][1]) ||
						(block_robot[i].road[0][1] > block_robot[i].z and block_robot[i].z > block_robot[i].road[1][1]));
					else
						block_robot[i].y_radian += 180.0f;

					if (player_robot.move && collision(block_robot[i].bb, player_robot.bb)) {
						player_robot.move = false;
						block_robot[i].x -= sin(glm::radians(block_robot[i].y_radian)) * block_robot[i].speed;
						block_robot[i].z -= cos(glm::radians(block_robot[i].y_radian)) * block_robot[i].speed;
						player_robot.x -= sin(glm::radians(player_robot.y_radian)) * player_robot.speed;
						player_robot.z -= cos(glm::radians(player_robot.y_radian)) * player_robot.speed;
						player_robot.speed = 0;
						GLfloat radian = atan2(player_robot.x - block_robot[i].x, player_robot.z - block_robot[i].z);
						player_robot.road[0][0] = block_robot[i].x, player_robot.road[0][1] = block_robot[i].z;
						player_robot.road[1][0] = block_robot[i].x + 2.0f * sin(radian), player_robot.road[1][1] = block_robot[i].z + 2.0f * cos(radian);
						glutTimerFunc(10, Bump, i);
					}
				}
			}
		}
	}
	glutTimerFunc(10, TimerFunc, 1);
	glutPostRedisplay();
}
GLvoid Bump(int index)
{
	if (!end) {
		if (collision(map_bb, player_robot.bb) || collision(map_bb2, player_robot.bb) || collision(map_bb3, player_robot.bb)) {
			GLfloat radian = atan2(player_robot.road[1][0] - player_robot.road[0][0], player_robot.road[1][1] - player_robot.road[0][1]);
			player_robot.x += sin(radian) * player_robot.speed;
			player_robot.z += cos(radian) * player_robot.speed;
			player_robot.bb = get_bb(player_robot);
		}
		else {
			player_robot.y -= 0.1f;
			player_robot.speed = 0.0f;
		}

		if (player_robot.speed < 0.25f)
			player_robot.speed += 0.001f;

		glm::vec2 road_spots = glm::vec2(player_robot.road[0][0], player_robot.road[0][1]);
		glm::vec2 player_spots = glm::vec2(player_robot.x, player_robot.z);

		if (player_robot.y < 0.0f);
		else if (index < 19 && glm::distance(road_spots, player_spots) < 2.0f)
			glutTimerFunc(10, Bump, index);
		else
			player_robot.move = true;
	}
}

int read_ten(int num)
{
	int Vplace = 0;
	while (num > 0) {
		Vplace++;
		num /= 10;
	}
	return Vplace;
}
BB get_bb(Robot robot)
{
	glm::mat4 Transform = glm::mat4(1.0f);//��ȯ ��� ���� T
	Transform = glm::translate(Transform, glm::vec3(robot.x, 0.0f, robot.z));
	Transform = glm::rotate(Transform, glm::radians(robot.y_radian), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 spots[4]{ glm::vec3(-0.3f, 0.0f, 0.1f), glm::vec3(0.3f, 0.0f, 0.1f), glm::vec3(-0.3f, 0.0f, -0.1f), glm::vec3(0.3f, 0.0f, -0.1f) };
	for (int i = 0; i < 4; ++i)
		spots[i] = glm::vec3(Transform * glm::vec4(spots[i], 1.0f));

	BB bounding_box{ spots[0].x, spots[0].z, spots[0].x, spots[0].z };
	for (int i = 1; i < 3; ++i) {
		if (bounding_box.x1 > spots[i].x)
			bounding_box.x1 = spots[i].x;
		if (bounding_box.x2 < spots[i].x)
			bounding_box.x2 = spots[i].x;
		if (bounding_box.z1 > spots[i].z)
			bounding_box.z1 = spots[i].z;
		if (bounding_box.z2 < spots[i].z)
			bounding_box.z2 = spots[i].z;
	}

	return bounding_box;
}
bool collision(BB obj_a, BB obj_b) {
	if (obj_a.x1 > obj_b.x2) return false;
	if (obj_a.x2 < obj_b.x1) return false;
	if (obj_a.z1 > obj_b.z2) return false;
	if (obj_a.z2 < obj_b.z1)return false;

	return true;
}
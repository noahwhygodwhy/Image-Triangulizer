#include <iostream>

#include "Delaunay.hpp"
#include "Sobel.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "glad.h"
#include <GLFW/glfw3.h>
#include <random>
#include <chrono>

#include "Shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static const int NUMBER_OF_POINTS = 15000; //10k is good for a medium size image, larger images require larger counts, mess around with it.
static const int SOBEL_MINIMUM = 50; //the brightness of a pixel for it to be counted as a edge, and have a vertex
static const float RATIO_BRIGHT_TO_DARK = 0.95f; //the ratio of how many points have to have the sobel minimum, and how many don't

//static const string fileInPath = "C:/Users/noahm/source/repos/Image Triangulizer/Image1.png";  //INPUT FILE MUST BE A PNG
//static const string fileOutPath = "C:/Users/noahm/source/repos/Image Triangulizer/Image2.png"; //MUST BE A PNG CAUSE THE WRITING LIBRARY WILL WRITE A PNG EITHER WAY

static const string fileInPath = "C:/Users/noahm/desktop/cat.png";
static const string fileOutPath = "C:/Users/noahm/desktop/catout.png"; //MUST BE A PNG CAUSE THE WRITING LIBRARY WILL WRITE A PNG EITHER WAY
//static const string fileInPath = "C:/Users/noahm/desktop/Image1.png";
//static const string fileOutPath = "C:/Users/noahm/desktop/Image2.png"; //MUST BE A PNG CAUSE THE WRITING LIBRARY WILL WRITE A PNG EITHER WAY


using namespace std;
using namespace glm;


struct Vertex
{
	fvec2 coords;
	fvec3 color;
};

//returns the centroid of a triangle (the center point)
vec2 centroid(const Triangle& t)
{
	return vec2(((t.points[0].x + t.points[1].x + t.points[2].x) / 3), ((t.points[0].y + t.points[1].y + t.points[2].y) / 3));
}


//renders the vertices to a internal frame buffer, and return that frame as an array of pixels in the form of a unsigned byte array.
unsigned char* renderIt(vector<Vertex>& vertices, int width, int height, int nrChannels)
{

	//just some basic openGL context setup
	glfwInit();
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, 1);


	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(width, height, "Title", NULL, NULL);
	if (window == NULL)
	{
		cout << "Window creation failed" << endl;
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "GLAD init failed" << endl;
		exit(-1);
	}


	//set up a custom framebuffer so we're not restricted by screen size
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	glViewport(0, 0, width, height);

	//import shaders
	Shader shader = Shader("vertShader.glsl", "fragShader.glsl");

	//set options
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glFrontFace(GL_CCW);


	//setup buffers
	unsigned int VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);

	

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//actually render it
	shader.use();
	shader.setMatFour("transform", mat4(1));
	shader.setMatFour("view", lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 1, 0)));
	shader.setMatFour("projection", glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1000.0f, 1000.0f));
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	//grab the rendered pixels to an array
	unsigned char* data = new unsigned char[width * height * nrChannels];
	if (nrChannels == 4)
	{
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else
	{
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	//terminate the window context
	glfwTerminate();

	return data;
}



//get a 64 bit random integer
uint64_t getRand()
{
	mt19937_64 p(chrono::steady_clock::now().time_since_epoch().count());
	uint64_t toReturn = p();
	return toReturn;
}

//determines if a triangle is counter clockwise
bool counterClockwise(Triangle t)
{
	return (t.points[1].x - t.points[0].x) * (t.points[2].y - t.points[0].y) - (t.points[2].x - t.points[0].x) * (t.points[1].y - t.points[0].y);
}



int main(int argc, char** argv)
{
	int width = 0;
	int height = 0;
	int nrChannels = 0;
	//printf("file in: -%s-\n", fileInPath.c_str());
	unsigned char* data = stbi_load(fileInPath.c_str(), &width, &height, &nrChannels, 0);

	printf("width: %i, height: %i", width, height);
	printf("numchannels: %i\n", nrChannels);

	if (data == 0)
	{
		printf("Bad file in. Check path.\n");
		exit(0);
	}
	printf("Read in file. Processing now\n");

	unsigned char* sobelData = makeSobalImage(data, width, height, nrChannels);


	//stbi_write_png(fileOutPath.c_str(), width, height, nrChannels, sobelData, nrChannels * width);
	//exit(0);

	srand(time(0));
	int numPixels =  width* height;
	printf("num pixel: %i\n", numPixels);
	int i = 0;
	vector<vec2> points;


	points.push_back(vec2(0, 0));
	points.push_back(vec2(width-1, 0));
	points.push_back(vec2(0, height-1));
	points.push_back(vec2(width-1, height-1));
	

	//put points on the sobel image pixels that are whiter than SOBEL_MINIMUM
	while (i < NUMBER_OF_POINTS*(RATIO_BRIGHT_TO_DARK))
	{
		uint64_t pixelIndex = getRand()%(numPixels-1);
		if (*(sobelData + (pixelIndex * nrChannels)) > SOBEL_MINIMUM)
		{
			points.push_back(vec2(pixelIndex % width, pixelIndex / width));
			i++;
		}
	}
	i = 0;
	//put points anywhere
	while (i < NUMBER_OF_POINTS * (1.0f-RATIO_BRIGHT_TO_DARK))
	{
		uint64_t pixelIndex = getRand() % (numPixels - 1);
		//printf("pixel index: %ul\n", pixelIndex);
		points.push_back(vec2(pixelIndex % width, pixelIndex / width));
		i++;
		
	}
	
	i = 0;
	//put points on the sobel image pixels that are whiter 250, which are just on the edge
	while (i < NUMBER_OF_POINTS * 0.01f)
	{
		int pixelIndex = getRand() % (numPixels - 1);
		if (*(sobelData + (pixelIndex * nrChannels)) > 250)
		{
			points.push_back(vec2(pixelIndex % width, pixelIndex / width));
			i++;
		}

	}


	//find the delaunay triangulation of the points
	vector<Triangle> triangles = delaunay(points, vec2(0, height), vec2(width, 0));

	//ensure they're all counter clockwise for rendering purposes
	for (Triangle& t : triangles)
	{
		if (!counterClockwise(t))
		{
			vec2 temp = t.points[1];
			t.points[1] = t.points[2];
			t.points[2] = temp;
		}
	}


	//find the color of the center of each triangle
	//and add a 3 vertices each with that color
	vector<Vertex> vertices;
	for (const Triangle& t : triangles)
	{
		ivec2 c = centroid(t);
		int pixelOffset = ((c.y * width) + c.x) * nrChannels;
		fvec3 intColor = fvec3(*(data + pixelOffset + 0), *(data + pixelOffset + 1), *(data + pixelOffset + 2));
		fvec3 fColor = intColor / 255.0f;
		for (vec2 p : t.points)
		{
			vertices.push_back({ p, fColor });
		}
	}

	printf("Triangulized. About to render.\n");

	//render it
	unsigned char* triangulizedImage = renderIt(vertices, width, height, nrChannels);

	printf("Rendered, now saving as %s\n", fileOutPath.c_str());

	stbi_flip_vertically_on_write(true);
	stbi_write_png(fileOutPath.c_str(), width, height, nrChannels, triangulizedImage, nrChannels * width);
	printf("Saved. Press enter to exit\n");
	
}
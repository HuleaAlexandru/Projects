//-------------------------------------------------------------------------------------------------
// Descriere: fisier main
//
// Autor: student
// Data: today
//-------------------------------------------------------------------------------------------------

//incarcator de meshe
#include "lab_mesh_loader.hpp"
//incarcator de shadere
#include "lab_shader_loader.hpp"
//interfata cu glut, ne ofera fereastra, input, context opengl
#include "lab_glut.hpp"
#include "lab_camera.hpp"
#include "lab_texture_loader.hpp"

//time, min
#include <ctime>
#include <algorithm>
#include <vector>

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#define PI 3.14159265359

#define ROTATION_LIMIT 60
#define ROOT_MAGNITUTE 40
#define ROOT_RADIUS 3
#define RADIUS_PROPORTION 0.6
#define ROOT_CHILDREN 6
#define MAX_DEPTH 3
#define MIN_LEAF 3
#define TO_DIE_LEAVES 10

using namespace std;

struct treeLine {
	glm::vec3 startPoint;
	int rotationOX;
	int rotationOY;
	float magnitude;
	float maxRadius;
	float minRadius;
	int maxChildren;
	int nrLeafs;
	int depth;

	vector<void*> children;
	glm::mat4 model_matrix;
};

struct leaf {
	glm::vec3 startPoint;
	float length;
	int rotationOX;
	int rotationOY;
};

class Laborator : public lab::glut::WindowListener{

//variabile
private:
	glm::mat4 model_matrix, view_matrix, projection_matrix;								//matrici 4x4 pt modelare vizualizare proiectie
	unsigned int program_shader;														//id-ul de opengl al obiectului de tip program shader

	lab::Camera* camera;
	treeLine* root;
	lab::Mesh* line;
	lab::Mesh* volume;
	lab::Mesh* leaf_mesh;
	vector<treeLine*> lines;
	vector<glm::mat4> model_matrices;
	vector<glm::mat4> leaf_model_matrices;
	vector<glm::mat4> dead_leaves;
	int mod;
	int wireFrame;
	unsigned int treeTexture;
	vector<unsigned int> leafTextures;
	vector<unsigned int> alphas;
	int anotimp;
	glm::vec3 lightPos;

//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
	Laborator(){
		srand(time(NULL));

		//setari pentru desenare, clear color seteaza culoarea de clear pentru ecran (format R,G,B,A)
		glClearColor(0.5,0.5,0.5,1);
		glClearDepth(1);			//clear depth si depth test (nu le studiem momentan, dar avem nevoie de ele!)
		glEnable(GL_DEPTH_TEST);	//sunt folosite pentru a determina obiectele cele mai apropiate de camera (la curs: algoritmul pictorului, algoritmul zbuffer)
		
		//incarca un shader din fisiere si gaseste locatiile matricilor relativ la programul creat
		program_shader = lab::loadShader("shadere\\vertex.glsl", "shadere\\fragment.glsl");
		
		//matrici de modelare si vizualizare
		model_matrix = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
		camera = new lab::Camera(glm::vec3(0, 30, 100), glm::vec3(0, 40, 0), glm::vec3(0, 1, 0));

		//desenare wireframe
		wireFrame = GL_FILL;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame);

		mod = 0;
		generateLineTree();
		line = generateMeshForLine();
		leaf_mesh = generateMeshForLeaf();
		volume = generateVolume(root->maxRadius, root->minRadius);
		treeTexture = lab::loadTextureBMP("resurse/treeTexture.bmp");

		leafTextures.push_back(lab::loadTextureBMP("resurse/leaf1.bmp"));
		leafTextures.push_back(lab::loadTextureBMP("resurse/leaf2.bmp"));

		alphas.push_back(lab::loadTextureBMP("resurse/leaf1alpha.bmp"));
		alphas.push_back(lab::loadTextureBMP("resurse/leaf2alpha.bmp"));
	
		anotimp = 0;
		lightPos = glm::vec3(0, 20, 70);
	}

	//destructor .. e apelat cand e distrusa clasa
	~Laborator(){
		//distruge shader
		glDeleteProgram(program_shader);
	}

	void* generateLine(treeLine* parent) {
		treeLine* child = new treeLine();
		float proportion = (rand() % 100 + 1) / 100.;
		glm::vec3 endParent = glm::vec3(parent->model_matrix * glm::vec4(0, 0.9, 0, 1));
		glm::vec3 middleParent = glm::vec3(parent->model_matrix * glm::vec4(0, 0.4, 0, 1));
		child->startPoint = glm::vec3(middleParent.x * (1 - proportion) + endParent.x * proportion,
			middleParent.y * (1 - proportion) + endParent.y * proportion,
			middleParent.z * (1 - proportion) + endParent.z * proportion);

		int val = rand() % ROTATION_LIMIT;
		child->rotationOX = 75 - val;

		if (child->rotationOX == parent->rotationOX) {
			do {
				child->rotationOY = rand() % 360;
			} while (child->rotationOY == parent->rotationOY);
		}
		else {
			child->rotationOY = rand() % 360;
		}

		proportion = (rand() % 10 + 70) / 100.;
		child->magnitude = parent->magnitude * proportion;

		proportion = (rand() % 10 + 70) / 100.;
		child->maxRadius = parent->minRadius * proportion;
		child->minRadius = child->maxRadius * RADIUS_PROPORTION;

		child->depth = parent->depth - 1;

		int nr_children = parent->maxChildren - (rand() % 2);
		child->maxChildren = nr_children;

		child->model_matrix = generateMatrix(child);
		model_matrices.push_back(child->model_matrix);

		child->nrLeafs = parent->nrLeafs + 1;
		if (parent->depth != MAX_DEPTH) {
			for (int i = 0; i < child->nrLeafs; i++) {
				void* c = generateLeaf(child);
				child->children.push_back(c);
			}
		}

		if (child->depth != 0) {
			for (int i = 0; i < nr_children; i++) {
				void* c = generateLine(child);
				child->children.push_back(c);
			}
		}
		lines.push_back(child);
		return child;
	}

	void* generateLeaf(treeLine* parent) {
		leaf* child = new leaf();

		float proportion = (rand() % 100 + 1) / 100.;
		glm::vec3 endParent = glm::vec3(parent->model_matrix * glm::vec4(0, 0.9, 0, 1));
		glm::vec3 middleParent = glm::vec3(parent->model_matrix * glm::vec4(0, 0.2, 0, 1));
		child->startPoint = glm::vec3(middleParent.x * (1 - proportion) + endParent.x * proportion,
			middleParent.y * (1 - proportion) + endParent.y * proportion,
			middleParent.z * (1 - proportion) + endParent.z * proportion);

		proportion = (rand() % 101) / 100.;
		child->length = MIN_LEAF + proportion;

		do {
			int val = rand() % 91;
			child->rotationOX = val;
		} while (abs(child->rotationOX - parent->rotationOX) < 15);

		child->rotationOY = rand() % 360;

		leaf_model_matrices.push_back(generateMatrixForLeaf(child));
		return child;
	}

	void generateLineTree() {
		root = new treeLine();
		root->startPoint = glm::vec3(0, 0, 0);
		root->rotationOX = 0;
		root->rotationOY = 0;
		root->magnitude = ROOT_MAGNITUTE;
		root->maxRadius = ROOT_RADIUS;
		root->minRadius = ROOT_RADIUS * RADIUS_PROPORTION;
		root->depth = MAX_DEPTH;
		root->maxChildren = ROOT_CHILDREN;
		root->nrLeafs = 5;
		root->model_matrix = generateMatrix(root);
		model_matrices.push_back(root->model_matrix);
		for (int i = 0; i < ROOT_CHILDREN; i++) {
			void* child = generateLine(root);
			root->children.push_back(child);
		}
		lines.push_back(root);
	}

	glm::mat4 generateMatrix(treeLine* line) {
		float rotX = line->rotationOX * 1.;
		float rotY = line->rotationOY * 1.;

		glm::mat4 result = glm::mat4();
		result = glm::translate(result, glm::vec3(line->startPoint.x, line->startPoint.y, line->startPoint.z));
		result = glm::rotate(result, rotY, glm::vec3(0, 1, 0));
		result = glm::rotate(result, rotX, glm::vec3(1, 0, 0));
		result = glm::scale(result, glm::vec3(line->maxRadius, line->magnitude, line->maxRadius));
		return result;
	}

	glm::mat4 generateMatrixForLeaf(leaf* f) {
		float rotX = f->rotationOX * 1.;
		float rotY = f->rotationOY * 1.;

		glm::mat4 result = glm::mat4();
		result = glm::translate(result, glm::vec3(f->startPoint.x, f->startPoint.y, f->startPoint.z));
		result = glm::rotate(result, rotY, glm::vec3(0, 1, 0));
		result = glm::rotate(result, rotX, glm::vec3(1, 0, 0));
		result = glm::scale(result, glm::vec3(f->length, f->length, 1));
		return result;
	}

	lab::Mesh* generateMeshForLeaf() {
		//date mesh
		std::vector<lab::VertexFormat> vertices;

		vertices.push_back(lab::VertexFormat(-0.5, 0, 0.03, 0, 0, 1, 0, 0));
		vertices.push_back(lab::VertexFormat(0.5, 0, 0.03, 0, 0, 1, 1, 0));
		vertices.push_back(lab::VertexFormat(-0.5, 1, 0.03, 0, 0, 1, 0, 1));
		vertices.push_back(lab::VertexFormat(0.5, 1, 0.03, 0, 0, 1, 1, 1));

		vertices.push_back(lab::VertexFormat(-0.5, 0, -0.03, 0, 0, -1, 0, 0));
		vertices.push_back(lab::VertexFormat(0.5, 0, -0.03, 0, 0, -1, 1, 0));
		vertices.push_back(lab::VertexFormat(-0.5, 1, -0.03, 0, 0, -1, 0, 1));
		vertices.push_back(lab::VertexFormat(0.5, 1, -0.03, 0, 0, -1, 1, 1));

		std::vector<unsigned int> indices;
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(3);

		indices.push_back(4);
		indices.push_back(5);
		indices.push_back(6);
		indices.push_back(5);
		indices.push_back(6);
		indices.push_back(7);

		//obiecte OpenGL mesh
		unsigned int vbo, ibo, vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(lab::VertexFormat)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(2 * sizeof(float) * 3));
		return new lab::Mesh(vbo, ibo, vao, indices.size());
	}

	lab::Mesh* generateMeshForLine() {
		//date mesh
		std::vector<lab::VertexFormat> vertices;

		vertices.push_back(lab::VertexFormat(0, 0, 0));
		vertices.push_back(lab::VertexFormat(0, 1, 0));
		
		std::vector<unsigned int> indices;
		indices.push_back(0);
		indices.push_back(1);

		//obiecte OpenGL mesh
		unsigned int vbo, ibo, vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(lab::VertexFormat)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)0);
		return new lab::Mesh(vbo, ibo, vao, indices.size());
	}

	lab::Mesh* generateVolume(float maxRadius, float minRadius) {
		//date mesh
		std::vector<lab::VertexFormat> vertices;
		std::vector<unsigned int> indices;
		float radiusProportion = maxRadius / minRadius;
		for (int i = 0; i < 360; i++) {
			float x = cos(i * PI / 180);
			float y = 0;
			float z = sin(i * PI / 180);
			float u = 2 * (atan(z / x) + PI / 2) / (2 * PI);
			float v = y / 1;
			vertices.push_back(lab::VertexFormat(x, y, z, x, y, z, u, v));

			x = cos(i * PI / 180) / radiusProportion;
			y = 1;
			z = sin(i * PI / 180) / radiusProportion;
			u = 2 * (atan(z / x) + PI / 2) / (2 * PI);
			v = y / 1;
			glm::vec3 norm = glm::normalize(glm::vec3(x, 0, z));
			vertices.push_back(lab::VertexFormat(x, y, z, norm.x, norm.y, norm.z, u, v));
			
			indices.push_back(2 * i);
			indices.push_back(2 * i + 1);
			indices.push_back((2 * i + 2) % 720);
			
			indices.push_back(2 * i + 1);
			indices.push_back((2 * i + 2) % 720);
			indices.push_back((2 * i + 3) % 720);
		}

		int start = vertices.size();
		vertices.push_back(lab::VertexFormat(0, 0, 0, 0, -1, 0, 0, 0));
		for (int i = 0; i < 360; i++) {
			float x = cos(i * PI / 180);
			float y = 0;
			float z = sin(i * PI / 180);
			float u = 2 * (atan(z / x) + PI / 2) / (2 * PI);
			float v = y / 1;
			vertices.push_back(lab::VertexFormat(x, y, z, 0, -1, 0, u, v));
			indices.push_back(start);
			indices.push_back(start + i + 1);
			if (i != 359) {
				indices.push_back(start + i + 2);
			}
			else {
				indices.push_back(start + 1);
			}
		}

		start = vertices.size();
		vertices.push_back(lab::VertexFormat(0, 1, 0, 0, 1, 0, 0, 1));
		for (int i = 0; i < 360; i++) {
			float x = cos(i * PI / 180) / radiusProportion;
			float y = 1;
			float z = sin(i * PI / 180) / radiusProportion;
			float u = 2 * (atan(z / x) + PI / 2) / (2 * PI);
			float v = y / 1;
			vertices.push_back(lab::VertexFormat(x, y, z, 0, 1, 0, u, v));
			indices.push_back(start);
			indices.push_back(start + i + 1);
			if (i != 359) {
				indices.push_back(start + i + 2);
			}
			else {
				indices.push_back(start + 1);
			}
		}

		//obiecte OpenGL mesh
		unsigned int vbo, ibo, vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(lab::VertexFormat)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(2 * sizeof(float) * 3));
		return new lab::Mesh(vbo, ibo, vao, indices.size());
	}


	
	//--------------------------------------------------------------------------------------------
	//functii de cadru ---------------------------------------------------------------------------

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei ( modelam/simulam scena)
	void notifyBeginFrame(){
	}
	//functia de afisare (lucram cu banda grafica)
	void notifyDisplayFrame(){
		
		//pe tot ecranul
		anotimp = (anotimp + 1) % 61;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniform1i(glGetUniformLocation(program_shader, "mod"), mod);
		glUniform3f(glGetUniformLocation(program_shader, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(program_shader, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(program_shader, "viewPos"), camera->position.x, camera->position.y, camera->position.z);

		glUniform3f(glGetUniformLocation(program_shader, "constants"), 1.0f, 0.0014f, 0.000007f);

		glUniform1f(glGetUniformLocation(program_shader, "ambientStrength"), 0.5);
		glUniform1f(glGetUniformLocation(program_shader, "diffuseStrength"), 0.5);
		glUniform1f(glGetUniformLocation(program_shader, "specularStrength"), 0.3);
		glUniform1f(glGetUniformLocation(program_shader, "shininess"), 32);

		if (mod == 0) {
			glUseProgram(program_shader);
			glUniformMatrix4fv(glGetUniformLocation(program_shader,"view_matrix"),1,false,glm::value_ptr(camera->getViewMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(program_shader,"projection_matrix"),1,false,glm::value_ptr(projection_matrix));
		
			for (int i = 0; i < model_matrices.size(); i++) {
				glUniformMatrix4fv(glGetUniformLocation(program_shader, "model_matrix"), 1, false, glm::value_ptr(model_matrices[i]));
				line->draw();
			}
		}
		else if (mod == 1) {
			glUseProgram(program_shader);
			glUniformMatrix4fv(glGetUniformLocation(program_shader, "view_matrix"), 1, false, glm::value_ptr(camera->getViewMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(program_shader, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, treeTexture);
			glUniform1i(glGetUniformLocation(program_shader, "myTexture"), 0);
			for (int i = 0; i < model_matrices.size(); i++) {
				glUniformMatrix4fv(glGetUniformLocation(program_shader, "model_matrix"), 1, false, glm::value_ptr(model_matrices[i]));
				volume->draw(GL_TRIANGLES);
			}
		}
		else {
			glUseProgram(program_shader);
			glUniformMatrix4fv(glGetUniformLocation(program_shader, "view_matrix"), 1, false, glm::value_ptr(camera->getViewMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(program_shader, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, leafTextures[0]);
			glUniform1i(glGetUniformLocation(program_shader, "myTexture"), 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, alphas[0]);
			glUniform1i(glGetUniformLocation(program_shader, "myAlphaTexture"), 1);
			for (int i = 0; i < leaf_model_matrices.size(); i++) {
				glUniformMatrix4fv(glGetUniformLocation(program_shader, "model_matrix"), 1, false, glm::value_ptr(leaf_model_matrices[i]));
				leaf_mesh->draw(GL_TRIANGLES);
			}
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, leafTextures[1]);
			glUniform1i(glGetUniformLocation(program_shader, "myTexture"), 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, alphas[1]);
			glUniform1i(glGetUniformLocation(program_shader, "myAlphaTexture"), 1);
			for (int i = 0; i < dead_leaves.size(); i++) {
				glUniformMatrix4fv(glGetUniformLocation(program_shader, "model_matrix"), 1, false, glm::value_ptr(dead_leaves[i]));
				leaf_mesh->draw(GL_TRIANGLES);
			}

			if (anotimp / 60) {
				for (int i = 0; i < TO_DIE_LEAVES && leaf_model_matrices.size() != 0; i++) {
					int val = rand() % leaf_model_matrices.size();
					dead_leaves.push_back(leaf_model_matrices[val]);
					leaf_model_matrices.erase(leaf_model_matrices.begin() + val);
				}
			}

			glUniform1i(glGetUniformLocation(program_shader, "mod"), 1);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, treeTexture);
			glUniform1i(glGetUniformLocation(program_shader, "myTexture"), 0);
			for (int i = 0; i < model_matrices.size(); i++) {
				glUniformMatrix4fv(glGetUniformLocation(program_shader, "model_matrix"), 1, false, glm::value_ptr(model_matrices[i]));
				volume->draw(GL_TRIANGLES);
			}
		}
	}
	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previos_width, int previous_height){
		//reshape
		if(height==0) height=1;
		glViewport(0,0,width,height);
		float aspect = (float)width / (float)height;
		projection_matrix = glm::perspective(75.0f, aspect,0.1f, 10000.0f);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == 27) lab::glut::close();	//ESC inchide glut si 
		if(key_pressed == 32) {
			//SPACE reincarca shaderul si recalculeaza locatiile (offseti/pointeri)
			glDeleteProgram(program_shader);
			program_shader = lab::loadShader("shadere\\vertex.glsl", "shadere\\geometry.glsl", "shadere\\fragment.glsl");
		}
		
		if (key_pressed == 'w' || key_pressed == 'W'){
			camera->translateForward(1);
		}
		if (key_pressed == 's' || key_pressed == 'S'){
			camera->translateForward(-1);
		}
		if (key_pressed == 'a' || key_pressed == 'A'){
			camera->translateRight(-1);
		}
		if (key_pressed == 'd' || key_pressed == 'D'){
			camera->translateRight(1);
		}
		if (key_pressed == 'q' || key_pressed == 'Q'){
			camera->translateUpword(-1);
		}
		if (key_pressed == 'e' || key_pressed == 'E'){
			camera->translateUpword(1);
		}

		if (key_pressed == 't' || key_pressed == 'T'){
			camera->rotateFPSoZ(5 * PI / 180);
		}
		if (key_pressed == 'g' || key_pressed == 'G'){
			camera->rotateFPSoZ(-5 * PI / 180);
		}
		if (key_pressed == 'f' || key_pressed == 'F'){
			camera->rotateFPSoX(-5 * PI / 180);
		}
		if (key_pressed == 'h' || key_pressed == 'H'){
			camera->rotateFPSoX(5 * PI / 180);
		}
		if (key_pressed == 'r' || key_pressed == 'R'){
			camera->rotateFPSoY(-5 * PI / 180);
		}
		if (key_pressed == 'y' || key_pressed == 'Y'){
			camera->rotateFPSoY(5 * PI / 180);
		}

		if (key_pressed == 'i' || key_pressed == 'I'){
			camera->rotateFPSoZ(5 * PI / 180);
		}
		if (key_pressed == 'k' || key_pressed == 'K'){
			camera->rotateFPSoZ(-5 * PI / 180);
		}
		if (key_pressed == 'j' || key_pressed == 'J'){
			camera->rotateFPSoX(-5 * PI / 180);
		}
		if (key_pressed == 'l' || key_pressed == 'L'){
			camera->rotateFPSoX(5 * PI / 180);
		}
		if (key_pressed == 'u' || key_pressed == 'U'){
			camera->rotateFPSoY(-5 * PI / 180);
		}
		if (key_pressed == 'o' || key_pressed == 'O'){
			camera->rotateFPSoY(5 * PI / 180);
		}

		if (key_pressed == 'z' || key_pressed == 'Z'){
			mod = (mod + 1) % 3;
		}
		if (key_pressed == 'x' || key_pressed == 'X'){
			if (wireFrame == GL_LINE) {
				wireFrame = GL_FILL;
			}
			else {
				wireFrame = GL_LINE;
			}
			glPolygonMode(GL_FRONT_AND_BACK, wireFrame);
		}

		if (key_pressed == '8'){
			lightPos.z--;
		}
		if (key_pressed == '2'){
			lightPos.z++;
		}
		if (key_pressed == '4'){
			lightPos.x--;
		}
		if (key_pressed == '6'){
			lightPos.x++;
		}
		if (key_pressed == '7'){
			lightPos.y--;
		}
		if (key_pressed == '9'){
			lightPos.y++;
		}
	
	}
	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y){	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
		if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();
	}
	//tasta speciala ridicata
	void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y){}
	//drag cu mouse-ul
	void notifyMouseDrag(int mouse_x, int mouse_y){ }
	//am miscat mouseul (fara sa apas vreun buton)
	void notifyMouseMove(int mouse_x, int mouse_y){ }
	//am apasat pe un boton
	void notifyMouseClick(int button, int state, int mouse_x, int mouse_y){ }
	//scroll cu mouse-ul
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y){ std::cout<<"Mouse scroll"<<std::endl;}

};

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("lab shadere 2 - suprafete de baleiere"),800,600,100,100,true);
	lab::glut::ContextInfo context(3,3,false);
	lab::glut::FramebufferInfo framebuffer(true,true,true,true);
	lab::glut::init(window,context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
	glewExperimental = true;
	glewInit();
	std::cout<<"GLEW:initializare"<<std::endl;

	//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
	//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
	Laborator mylab;
	lab::glut::setListener(&mylab);

	//taste
	std::cout<<"Taste:"<<std::endl<<"\tESC ... iesire"<<std::endl<<"\tSPACE ... reincarca shadere"<<std::endl;
	std::cout<<"\t1 ... creste numar puncte generate"<<std::endl<<"\t2 ... scade numar puncte generate"<<std::endl;
	std::cout<<"\t3 ... creste numar instante"<<std::endl<<"\t4 ... scade numar instante"<<std::endl;


	//run
	lab::glut::run();

	return 0;
}
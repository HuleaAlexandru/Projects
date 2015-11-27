//-------------------------------------------------------------------------------------------------
// Descriere: fisier main
//
// Autor: student
// Data: today
//-------------------------------------------------------------------------------------------------

//incarcator de meshe
#include "lab_mesh_loader.hpp"
//geometrie: drawSolidCube, drawWireTeapot...
#include "lab_geometry.hpp"
//incarcator de shadere
#include "lab_shader_loader.hpp"
//interfata cu glut, ne ofera fereastra, input, context opengl
#include "lab_glut.hpp"
//texturi
#include "lab_texture_loader.hpp"
//time
#include <ctime>
#include <stdlib.h>
#include <time.h>
#include "lab_camera.hpp"

using namespace std;
using namespace glm;
using namespace lab;

const int ox1 = -580;
const int ox2 = 580;
const int oz1 = -580;
const int oz2 = 580;
const int size = 20;
const int minCub = -10;
const int maxCub = 10;

const float myPi = 3.14159265359;

const float myNear = 0.1f;
const float myFar = 2000.0f;

float aspect = 800. / 600;
float hNear = tan(75 / 2 * myPi / 180) * myNear * 2;
float hFar = tan(75 / 2 * myPi / 180) * myFar * 2;
float wNear = hNear * aspect;
float wFar = hFar * aspect;

std::vector<lab::Camera*> camere;
vector < glm::mat4 > oras;
std::vector<VertexFormat> cub;
int** visibleBuildings;
vector < vector <glm::vec4> > World_Positions;

int fromRealWorldToMatrix(int x) {
	// - 600 == 0
	// 600 == 60
	return (x + 600) / 20;
}

int fromMatrixToRealWorld(int x) {
	return x * 20 - 600;
}

int fromMatrixToIndex(int x, int z) {
	if (x % 2 == 1 && z % 2 == 1) {
		// 1 1 == 0          
		// 1 59 == 29  
		// 3 1 == 30
		// 3 59 == 59
		return 15 * (x - 1) + (z - 1) / 2;
	}
	return -1;
}

void fromIndexToMatrix(int i, int& x, int& z) {
	x = (i / 30) * 2 + 1;
	z = (i - 15 * (x - 1)) * 2 + 1;
}

glm::mat4 creeazaCladire(int translatex, int translatez) {
	int height = rand() % 10 + 5;
	int x = fromRealWorldToMatrix(translatex);
	int z = fromRealWorldToMatrix(translatez);

	glm::mat4 matrix = glm::mat4();
	matrix = glm::translate(glm::mat4(), glm::vec3(translatex, height * size / 2, translatez));
	matrix = glm::scale(matrix, glm::vec3(1, height, 1));

	vector<glm::vec4> cube_pos;
	for (int i = 0; i < cub.size(); i++) {
		glm::vec4 initialPosition = glm::vec4(cub[i].position_x, cub[i].position_y, cub[i].position_z, 1);
		glm::vec4 position = matrix * initialPosition;
		cube_pos.push_back(position);
	}
	World_Positions.push_back(cube_pos);

	return matrix;
}

vector <glm::mat4 > creeazaOras() {
	vector < glm::mat4 > orasulMeu;
	for (int i = ox1; i < ox2; i += 2 * size) {
		for (int j = oz1; j < oz2; j += 2 * size) {
			orasulMeu.push_back(creeazaCladire(i + size / 2, j + size / 2));
		}
	}
	return orasulMeu;
}

vector<glm::mat4> creeazaPodea() {
	vector<glm::mat4> podea;
	for (int i = ox1 - size; i < ox2 + size; i += size) {
		for (int j = oz1 - size; j < oz2 + size; j += size) {
			podea.push_back(glm::translate(glm::mat4(), glm::vec3(i, 0, j)));
		}
	}
	return podea;
}

// obtin cele 4 colturi ale frustumului
// obtin normalele la cele 6 plane
// fac dot product intre fiecare punct al cubului si normale

void intersect1() {
	wNear = hNear * aspect;
	wFar = hFar * aspect;
	// n - near, f - far, t - top, b - botton, l - left, r - right, c - center
	glm::vec3 nc = camere[0]->position + myNear * camere[0]->forward;
	glm::vec3 fc = camere[0]->position + myFar * camere[0]->forward;

	glm::vec3 ftl = fc + (camere[0]->up * hFar / 2.0f) - (camere[0]->right * wFar / 2.0f);
	glm::vec3 ftr = fc + (camere[0]->up * hFar / 2.0f) + (camere[0]->right * wFar / 2.0f);
	glm::vec3 fbl = fc - (camere[0]->up * hFar / 2.0f) - (camere[0]->right * wFar / 2.0f);
	glm::vec3 fbr = fc - (camere[0]->up * hFar / 2.0f) + (camere[0]->right * wFar / 2.0f);

	glm::vec3 ntl = nc + (camere[0]->up * hNear / 2.0f) - (camere[0]->right * wNear / 2.0f);
	glm::vec3 ntr = nc + (camere[0]->up * hNear / 2.0f) + (camere[0]->right * wNear / 2.0f);
	glm::vec3 nbl = nc - (camere[0]->up * hNear / 2.0f) - (camere[0]->right * wNear / 2.0f);
	glm::vec3 nbr = nc - (camere[0]->up * hNear / 2.0f) + (camere[0]->right * wNear / 2.0f);

	int nr_of_plans = 6;

	glm::vec3* normals = new glm::vec3[nr_of_plans];
	glm::vec3* centers = new glm::vec3[nr_of_plans];
	// normalele
	// fata far
	glm::vec3 v = glm::normalize(ftl - ftr);
	glm::vec3 u = glm::normalize(ftr - fbr);
	glm::vec3 n1 = glm::cross(v, u);
	if (glm::dot(n1, camere[0]->forward) > 0) {
		n1 = -n1;
	}
	normals[0] = n1;
	centers[0] = (ftl + fbr) / 2.0f;

	// fata near
	v = glm::normalize(ntl - ntr);
	u = glm::normalize(ntr - nbr);
	glm::vec3 n2 = glm::cross(v, u);
	if (glm::dot(n2, camere[0]->forward) < 0) {
		n2 = -n2;
	}
	normals[1] = n2;
	centers[1] = (ntl + nbr) / 2.0f; 

	// fata right
	v = glm::normalize(ntr - ftr);
	u = glm::normalize(ftr - fbr);
	glm::vec3 n3 = glm::cross(v, u);
	if (glm::dot(n3, camere[0]->right) > 0) {
		n3 = -n3;
	}
	normals[2] = n3;
	centers[2] = (ftr + nbr) / 2.0f;

	// fata left
	v = glm::normalize(ntl - ftl);
	u = glm::normalize(ftl - fbl);
	glm::vec3 n4 = glm::cross(v, u);
	if (glm::dot(n4, camere[0]->right) < 0) {
		n4 = -n4;
	}
	normals[3] = n4;
	centers[3] = (ftl + nbl) / 2.0f;

	// fata up 
	v = glm::normalize(ntl - ftl);
	u = glm::normalize(ftl - ftr);
	glm::vec3 n5 = glm::cross(v, u);
	if (glm::dot(n5, camere[0]->up) > 0) {
		n5 = -n5;
	}
	normals[4] = n5;
	centers[4] = (ftl + ntr) / 2.0f;

	// fata down
	v = glm::normalize(nbl - fbl);
	u = glm::normalize(fbl - fbr);
	glm::vec3 n6 = glm::cross(v, u);
	if (glm::dot(n6, camere[0]->up) < 0) {
		n6 = -n6;
	}
	normals[5] = n6;
	centers[5] = (fbl + nbr) / 2.0f;

	for (int i = 0; i < oras.size(); i++) {
		// oras[i] == o cladire
		bool inInterior = true;
		int x,z;
		fromIndexToMatrix(i, x, z);

		for (int plan = 0; plan < nr_of_plans; plan++) {
			
			glm::vec3 normal = normals[plan];
			glm::vec3 center = centers[plan];
			
			bool rightSide = false;

			for (int k = 0; k < cub.size(); k++) {
				//cub[k] este un varf al cubului initial
				glm::vec4 position = World_Positions[i][k];
				glm::vec3 actualPosition = glm::vec3(position.x, position.y, position.z) - center;

				float result = glm::dot(actualPosition, normal);

				if (result >= 0) {
					rightSide = true;
					break;
				}	
			}
			
			if (!rightSide) {
				inInterior = false;
				break;
			}
		}

		if (inInterior) {
			visibleBuildings[x][z] = true;
		}
		else {
			visibleBuildings[x][z] = false;
		}
	}
	delete centers;
	delete normals;
}


class Laborator : public lab::glut::WindowListener{

//variabile
private:
	glm::mat4 model_matrix_floor;
	glm::mat4  model_matrix_cube;														//matrici modelare pentru cele 3 obiecte
	glm::mat4 view_matrix, projection_matrix;											//matrici 4x4 pt modelare vizualizare proiectie
	unsigned int gl_program_shader;														//id-ul de opengl al obiectului de tip program shader
	unsigned int screen_width, screen_height;

	//meshe
	unsigned int cube_vbo, cube_ibo, cube_vao, cube_num_indices;		
	unsigned int floor_vbo, floor_ibo, floor_vao, floor_num_indices;

	//texturi
	unsigned int cube_texture;
	unsigned int floor_texture;
	unsigned int red_texture;
	unsigned int green_texture;

	int nrCam;
	
	vector < glm::mat4> podea;
//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
	Laborator(){
		//setari pentru desenare, clear color seteaza culoarea de clear pentru ecran (format R,G,B,A)
		glClearColor(0,0,0,1);
		glClearDepth(1);			//clear depth si depth test (nu le studiem momentan, dar avem nevoie de ele!)
		glEnable(GL_DEPTH_TEST);	//sunt folosite pentru a determina obiectele cele mai apropiate de camera (la curs: algoritmul pictorului, algoritmul zbuffer)
		
		//incarca un shader din fisiere si gaseste locatiile matricilor relativ la programul creat
		gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");
		
		red_texture = lab::loadTextureBMP("resurse\\red.bmp");
		green_texture = lab::loadTextureBMP("resurse\\green.bmp");

		//cube
		cub = lab::loadObj("resurse\\cube.obj", cube_vao, cube_vbo, cube_ibo, cube_num_indices);
		cube_texture = lab::loadTextureBMP("resurse\\windows.bmp");

		lab::loadObj("resurse\\floor.obj", floor_vao, floor_vbo, floor_ibo, floor_num_indices);
		floor_texture = lab::loadTextureBMP("resurse\\floor.bmp");
		model_matrix_floor = glm::mat4(60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 1);

		//matrici de modelare si vizualizare
		lab::Camera* cam1 = new lab::Camera();
		cam1->set(glm::vec3(0, 50, 0), glm::vec3(-100, 50, 0), glm::vec3(0, 1, 0));
		camere.push_back(cam1);

		lab::Camera* cam2 = new lab::Camera();
		cam2->set(glm::vec3(0, 1200, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
		camere.push_back(cam2);

		nrCam = 0;
		visibleBuildings = new int*[61];
		for (int i = 0; i < 61; i++) {
			visibleBuildings[i] = new int[61];
			for (int j = 0; j < 61; j++) {
				visibleBuildings[i][j] = false;
			}
		}

		srand(time(NULL));
		podea = creeazaPodea();
		oras = creeazaOras();
		//desenare wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	//destructor .. e apelat cand e distrusa clasa
	~Laborator(){
		//distruge shader
		glDeleteProgram(gl_program_shader);
		glDeleteBuffers(1,&cube_vbo);	glDeleteBuffers(1,&cube_ibo);	glDeleteVertexArrays(1,&cube_vao);		
		glDeleteBuffers(1, &floor_vbo);	glDeleteBuffers(1, &floor_ibo);	glDeleteVertexArrays(1, &floor_vao);
	}

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei ( modelam/simulam scena)
	void notifyBeginFrame(){
		//rotatie
	}
	//functia de afisare (lucram cu banda grafica)
	void notifyDisplayFrame(){
		
		//pe tot ecranul
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//foloseste shaderul
		glUseProgram(gl_program_shader);
				
		//trimite variabile uniforme la shader
		glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"view_matrix"),1,false,glm::value_ptr(camere[nrCam]->getViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(gl_program_shader, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
		glUniform3f(glGetUniformLocation(gl_program_shader, "light_position"), camere[0]->position.x, camere[0]->position.y, camere[0]->position.z);
		glUniform3f(glGetUniformLocation(gl_program_shader, "direction_position"), camere[0]->forward.x, camere[0]->forward.y, camere[0]->forward.z);
		glUniform1f(glGetUniformLocation(gl_program_shader, "cut_off"), myPi / 9);
		glUniform1i(glGetUniformLocation(gl_program_shader, "spot_exponent"), 30);
		glUniform1i(glGetUniformLocation(gl_program_shader, "camera"), nrCam);

		// desenare podea 
		glUniformMatrix4fv(glGetUniformLocation(gl_program_shader, "model_matrix"), 1, false, glm::value_ptr(model_matrix_floor));

		glActiveTexture(GL_TEXTURE0 + floor_texture);
		glBindTexture(GL_TEXTURE_2D, floor_texture);
		unsigned int floor_textura = glGetUniformLocation(gl_program_shader, "textura1");
		glUniform1i(floor_textura, floor_texture);

		glBindVertexArray(floor_vao);

		for (int i = 0; i < podea.size(); i++) {
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader, "model_matrix"), 1, false, glm::value_ptr(podea[i]));	
			glDrawElements(GL_TRIANGLES, floor_num_indices, GL_UNSIGNED_INT, 0);
		}

		// desenare oras
		glUniformMatrix4fv(glGetUniformLocation(gl_program_shader,"model_matrix"),1,false,glm::value_ptr(model_matrix_cube));
		intersect1();
		if (nrCam == 0) {
			glActiveTexture(GL_TEXTURE0 + cube_texture);
			glBindTexture(GL_TEXTURE_2D, cube_texture);
			unsigned int cub_textura = glGetUniformLocation(gl_program_shader, "textura1");
			glUniform1i(cub_textura, cube_texture);

			glBindVertexArray(cube_vao);

			for (int i = 0; i < oras.size(); i++) {
				int x, z;
				fromIndexToMatrix(i, x, z);

				if (visibleBuildings[x][z]) {
					glUniformMatrix4fv(glGetUniformLocation(gl_program_shader, "model_matrix"), 1, false, glm::value_ptr(oras[i]));
					glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_INT, 0);
				}
			}
		}
		else {
			glBindVertexArray(cube_vao);
		
			for (int i = 0; i < oras.size(); i++) {
				int x, z;
				fromIndexToMatrix(i, x, z);
				
				if (visibleBuildings[x][z]) {
					glActiveTexture(GL_TEXTURE0 + green_texture);
					glBindTexture(GL_TEXTURE_2D, green_texture);
					unsigned int green_textura = glGetUniformLocation(gl_program_shader, "textura1");
					glUniform1i(green_textura, green_texture);
				}
				else {
					glActiveTexture(GL_TEXTURE0 + red_texture);
					glBindTexture(GL_TEXTURE_2D, red_texture);
					unsigned int red_textura = glGetUniformLocation(gl_program_shader, "textura1");
					glUniform1i(red_textura, red_texture);
				}

				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader, "model_matrix"), 1, false, glm::value_ptr(oras[i]));
				glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_INT, 0);
				
			}
		}	
	}

	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previos_width, int previous_height){
		//reshape
		if(height==0) height=1;
		aspect = (float)width / (float)height;
		projection_matrix = glm::perspective(75.0f, aspect, myNear, myFar);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if (key_pressed == 27) lab::glut::close();	//ESC inchide glut si 
		if (key_pressed == 32) {
			//SPACE reincarca shaderul si recalculeaza locatiile (offseti/pointeri)
			glDeleteProgram(gl_program_shader);
			gl_program_shader = lab::loadShader("shadere\\shader_vertex.glsl", "shadere\\shader_fragment.glsl");
		}
		if (key_pressed == 'w'){
			static bool wire = true;
			wire = !wire;
			glPolygonMode(GL_FRONT_AND_BACK, (wire ? GL_LINE : GL_FILL));
		}

		//if (nrCam == 0) {
			if (key_pressed == 'p') { 
				camere[0]->translateForward(5);
			}
			if (key_pressed == 'u') { 
				camere[0]->translateRight(-5); 
			}
			if (key_pressed == 'o') {
				camere[0]->translateForward(-5); 
			}
			if (key_pressed == 'i') {
				camere[0]->translateRight(5);
			}
			if (key_pressed == 'y') {
				camere[0]->translateUpword(5); 
			}
			if (key_pressed == 't') { 
				camere[0]->translateUpword(-5);
			}
			if (key_pressed == 'j') { 
				camere[0]->rotateFPSoY(0.1f); 
			}

			if (key_pressed == 'h') { 
				camere[0]->rotateFPSoY(-0.1f);
			}
			if (key_pressed == 'k') { 
				camere[0]->rotateFPSoZ(-0.1f); 
			}
			if (key_pressed == 'l') { 
				camere[0]->rotateFPSoZ(0.1f); 
			}
			if (key_pressed == 'g') { 
				camere[0]->rotateFPSoX(0.1f);
			}
			if (key_pressed == 'f') { 
				camere[0]->rotateFPSoX(-0.1f);
			}
		//}
		if (key_pressed == 'c') {
			nrCam = (nrCam + 1) % 2;
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
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y){ }

};

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("lab shadere 5 - texturi"),800,600,100,100,true);
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
	std::cout<<"Taste:"<<std::endl<<"\tESC ... iesire"<<std::endl<<"\tSPACE ... reincarca shadere"<<std::endl<<"\tw ... toggle wireframe"<<std::endl;

	//run
	lab::glut::run();

	return 0;
}
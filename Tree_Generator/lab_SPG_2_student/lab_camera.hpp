//-------------------------------------------------------------------------------------------------
// Descriere: header in care este definita si implementata camera
// Nota:
//		camera este capabila de operatie de rotatie si translatie pe toate axele
//		camera este capabila de rotatii FP(first person) cat si TP(third person)
//
// Autor: Lucian Petrescu
// Data: 14 oct 2013
//-------------------------------------------------------------------------------------------------


#pragma once
#include "dependente\glm\glm.hpp"
#include "dependente\glm\gtc\type_ptr.hpp"
#include "dependente\glm\gtc\matrix_transform.hpp"

namespace lab{

	struct CamVertexFormat{ 
		glm::vec3 position, color; 
		CamVertexFormat(const glm::vec3 &p, const glm::vec3 &c){ 
			position=p; color=c;
		} 
	};


	class Camera{
	public:
		Camera(){
			//initializeaza camera
			position = glm::vec3(0,0,50);
			forward = glm::vec3(0,0,-1);
			up = glm::vec3(0,1,0);
			right = glm::vec3(1,0,0);
		}
		Camera(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up){
			//set camera
			set(position, center,up);
		}
		~Camera(){ }

		void set(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up){
			//update camera
			this->position = position;
			forward = glm::normalize(center-position);
			right = glm::cross(forward, up);
			this->up=glm::cross(right,forward);
		}

		void translateForward(float distance){
			//translatie
			this->position += glm::normalize(this->forward) * distance;
		}
		void translateUpword(float distance){
			//translatie
			this->position += glm::normalize(this->up) * distance;
		}
		void translateRight(float distance){
			//translatie
			this->position += glm::normalize(this->right) * distance;
		}
		void rotateFPSoX(float angle){
			//rotatie
			this->up = this->up * cos(angle) + this->forward * sin(angle);
			this->forward = glm::cross(this->up, this->right);
		}
		void rotateFPSoY(float angle){
			//rotatie
			this->forward = this->forward * cos(angle) + this->right * sin(angle);
			this->right = glm::cross(this->forward, this->up);
		}
		void rotateFPSoZ(float angle){
			//rotatie
			this->right = this->right * cos(angle) + this->up * sin(angle);
			this->up = glm::cross(this->right, this->forward);
		}
		void rotateTPSoX(float angle, float distance){
			//rotatie
			glm::vec3 last_forward = this->forward;
			this->rotateFPSoX(angle);
			this->position = this->position + distance * (last_forward - this->forward);
		}
		void rotateTPSoY(float angle, float distance){
			//rotatie
			glm::vec3 last_forward = this->forward;
			this->rotateFPSoY(angle);
			this->position = this->position + distance * (last_forward - this->forward);
		}
		void rotateTPSoZ(float angle, float distance){
			//rotatie
			glm::vec3 last_forward = this->forward;
			this->rotateFPSoZ(angle);
			this->position = this->position + distance * (last_forward - this->forward);
		}

		glm::mat4 getViewMatrix(){
			return glm::lookAt(position,position + glm::normalize(forward), up);
		}

	public:
		//camera
		glm::vec3 position;
		glm::vec3 up;
		glm::vec3 forward;
		glm::vec3 right;

		//geometrie camera
		unsigned int vao,vbo,ibo;
	};
}
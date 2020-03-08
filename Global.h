#ifndef  GLOBAL_H
#define GLOBAL_H


#include "Math.h"

const glm::mat4 FBX_Import_Mesh_Root_Transformation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
const glm::mat4 FBX_Import_Mesh_Root_Transformation_Inverse = glm::inverse(FBX_Import_Mesh_Root_Transformation);

const glm::mat4 swordSocket = glm::translate(glm::vec3(0, 2, 0)) * glm::rotate(glm::mat4(1.0), -0.2f, glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1.0), 0.2f, glm::vec3(1, 0, 0)) * glm::translate(glm::vec3(-5, 5, 0)) * glm::rotate(glm::mat4(1.0), 0.8f, glm::vec3(0, 0, 1)) * glm::translate(glm::vec3(0, 0, -5)) * glm::translate(glm::vec3(10, 0, 0)) * glm::translate(glm::vec3(0, -5, 0)) * glm::translate(glm::vec3(0, 0, -10)) * glm::translate(glm::vec3(-20, 5, 0));


const float lengthOfTerrainQuad = 1.0f;

const float terrainMaxHeight = 500.0f;


//const glm::mat4 swordSocketTransformation = glm::
//const glm::mat4 swordSocketTransformation = glm::translation([0, 2, 0]).times(Mat4.rotation(-0.2, Vec.of(0, 1, 0))).times(Mat4.rotation(0.2, Vec.of(1, 0, 0))).times(Mat4.translation([-5, 5, 0])).times(Mat4.rotation(0.8, Vec.of(0, 0, 1))).times(Mat4.translation([0, 0, -5])).times(Mat4.translation([10, 0, 0])).times(Mat4.translation([0, -5, 0])).times(Mat4.translation([0, 0, -10])).times(Mat4.translation([-20, 0, 0]));


#endif 


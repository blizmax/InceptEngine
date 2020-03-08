#pragma once

#include "Renderer.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "Camera.h"
#include "Player.h"






struct Data
{
    Renderer* r;
    Camera* c;
    bool* b;
    Actor* p;
    Actor* sword;
    void (*IkFAB)(std::vector<glm::mat4>& boneT, Skeleton& skeleton, glm::mat4 swordLocation);
    void (*audio)();
    std::vector<glm::mat4>* tPose;
    Skeleton* sk;
    glm::mat4 location;
    std::thread* music;
};




void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto data = reinterpret_cast<Data*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        data->IkFAB(*data->tPose, *data->sk, data->location);
    }
    else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT ))
    {
        data->p->translate(glm::vec3(data->p->getForwardVector()), 20.0f);
        
    }
    else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        data->p->translate(glm::vec3(data->p->getForwardVector()), -20.0f);
    }
    else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        data->p->translate(glm::vec3(data->p->getRightWardVector()), 20.0f);
    }
    else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        data->p->translate(glm::vec3(data->p->getRightWardVector()), -20.0f);
       
    }
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
       
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
       
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        data->sword->translate(data->sword->getForwardVector(), 10);
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        data->sword->translate(data->sword->getForwardVector(), -10);
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        data->sword->translate(data->sword->getUpWardVector(), 10);
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        data->sword->translate(data->sword->getUpWardVector(), -10);
    }
    else if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        data->sword->translate(data->sword->getRightWardVector(), 10);
    }
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        data->sword->translate(data->sword->getRightWardVector(), -10);
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (*data->b) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
       
        *data->b = ! *data->b;
    }
    else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }
    else if (key == GLFW_KEY_7 && action == GLFW_PRESS)
    {
        data->sword->rotate(data->sword->getRightWardVector(), 10);
    }
    else if (key == GLFW_KEY_8 && action == GLFW_PRESS)
    {
        data->sword->rotate(data->sword->getForwardVector(), 10);
    }
    else if (key == GLFW_KEY_9 && action == GLFW_PRESS)
    {
        data->sword->rotate(data->sword->getUpWardVector(), 10);
    }
    else if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        data->c->lightUp(30.0f);
    }
    else if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
        data->c->lightUp(-30.0f);
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        data->music = new std::thread(data->audio);
    }


}

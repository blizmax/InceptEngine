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
    Player* p;
};

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto data = reinterpret_cast<Data*>(glfwGetWindowUserPointer(window));
    data->r->resizeWindow(width, height);

}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto data = reinterpret_cast<Data*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        data->p->m_localFrame.rotate(data->p->m_localFrame.Z, 15.0f);
    }
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        data->p->m_localFrame.rotate(data->p->m_localFrame.X, 15.0f);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        data->p->m_localFrame.rotate(data->p->m_localFrame.Y, 15.0f);
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        data->c->rotateVertical(-15);
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        data->c->rotateVertical(15);
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        data->c->rotateHorizontal(15);
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        data->c->printCameraPramameter();
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        *data->b = ! *data->b;
    }
    else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }

}

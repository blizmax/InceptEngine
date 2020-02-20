#pragma once

#include "Renderer.h"
#include <iostream>
#include <chrono>
#include <thread>



void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    renderer->resizeWindow(width, height);

}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        renderer->m_mvp.model *= glm::rotate(glm::mat4(1.0f),  glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        renderer->m_mvp.model *= glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        renderer->m_mvp.model *= glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
       
    }
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
       
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
       
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        
    }
    else if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        
    }

}

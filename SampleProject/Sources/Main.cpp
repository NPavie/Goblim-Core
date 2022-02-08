/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

// For GLM 0.9.6.3


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <time.h>
#include <AntTweakBar/AntTweakBar.h>

#include "Utils/MyTime.h"

#include "GPUResources/GPUInfo.h"

#include "SampleEngine.h"

using namespace std;
#ifndef M_PI
#define M_PI 3.1415926
#endif

typedef enum {NODE,CAMERA} Interact_Target;
typedef enum {TRANSLATE,ROTATE,NO_MOVE} Interact_Mode;

int oldWheel = 0;
glm::vec3 oldMouse;
Interact_Mode iMode = NO_MOVE;
Interact_Target iTarget = CAMERA;


EngineGL *engine = NULL;
Scene *scene = NULL;
GLFWwindow* window = NULL;
int64 timeTotal = 0;
int frame_Num = 0;
bool init;


//GLFWGPUMCNoiseEditor *mcNoiseEditor;

int Wwidth,Wheight;
int64 start_time, current_time, last_time, fps_last_time;
unsigned int frame;

glm::vec3 projectOnSphere(glm::vec2 pos)
{
    
    glm::vec3 v;
    v[0] = pos[0];
    v[1] = pos[1];
    float d = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
    v[2] = (float)cos((M_PI / 2.0F) * ((d < 1.0F) ? d : 1.0F));
    float a = 1.0F / (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] *= a;
    v[1] *= a;
    v[2] *= a;
    
    
    return(v);
}
void trackBallCamera(Camera* cam, glm::vec3 nMouse, glm::vec3 &oMouse)
{
    
    
    float dx = nMouse[0] - oMouse[0];
    float dy = nMouse[1] - oMouse[1];
    float dz = nMouse[2] - oMouse[2];
    
    if (fabs(dx) > 0 || fabs(dy) > 0 || fabs(dz) > 0)
    {
        float angle = 3.14f * sqrt(dx*dx + dy*dy + dz*dz);
        glm::vec3 axis;
        axis[0] = oMouse[1] * nMouse[2] - oMouse[2] * nMouse[1];
        axis[1] = oMouse[2] * nMouse[0] - oMouse[0] * nMouse[2];
        axis[2] = oMouse[0] * nMouse[1] - oMouse[1] * nMouse[0];
        
        cam->rotate(axis, -angle);
        
    }
    
    oMouse = nMouse;
    
    
}
void trackBallFrame(Frame *f, Camera *cam, glm::vec3 nMouse, glm::vec3 &oMouse)
{
    glm::vec3 dis = nMouse - oMouse;
    if (fabs(dis.x) > 0 || fabs(dis.y) > 0 || fabs(dis.z) > 0)
    {
        if (iMode == ROTATE)
        {
            glm::vec3 v1(nMouse);
            glm::vec3 v2(oMouse);
            if (v1.x*v1.x + v1.y*v1.y <= 1.0f)
            v1.z = sqrt(1.0f - (v1.x*v1.x + v1.y*v1.y));
            else
            v1.z = (1.0f) / sqrt(v1.x*v1.x + v1.y*v1.y);
            if (v2.x*v2.x + v2.y*v2.y <= 1.0f)
            v2.z = sqrt(1.0f - (v2.x*v2.x + v2.y*v2.y));
            else
            v2.z = (1.0f) / sqrt(v2.x*v2.x + v2.y*v2.y);
            v1 = glm::normalize(v1);
            v2 = glm::normalize(v2);
            
            glm::vec3 rax = glm::normalize(glm::cross(v2, v1));
            float angle = acos(glm::dot(v1, v2));
            rax = cam->convertDirTo(rax, f);
            f->rotate(rax, (float)(angle));
            
        }
        else if (iMode == TRANSLATE)
        {
            glm::vec3 localCenter = cam->convertPtFrom(glm::vec3(0.0, 0.0, 0.0), f);
            glm::vec3 dir = cam->convertDirTo(glm::vec3(dis.x, dis.y, 0.0), f);
            f->translate(dir*-localCenter.z*0.5f);
        }
    }
    
    oMouse = nMouse;
}

void OnWindowSize(GLFWwindow* win, int width, int height)
{
    Wwidth = width;
    Wheight = height;
    
    if (engine != NULL)
    engine->onWindowResize(width, height);
    TwWindowSize(width, height);
    
}
void OnMouseButton(GLFWwindow* win, int glfwButton, int glfwAction, int modifiers)
{
    if (!TwEventMouseButtonGLFW(glfwButton, glfwAction))
    {
        
        if (glfwAction == GLFW_PRESS) // Send event to AntTweakBar
        {
            double x, y;
            glfwGetCursorPos(win, &x, &y);
            glm::vec2 v;
            v.x = (float)x / (float)Wwidth;
            v.y = 1.0f - (float)y / (float)Wheight;
            v = 2.0f*v - 1.0f;
            
            oldMouse = projectOnSphere(v);
            
            if (glfwButton == GLFW_MOUSE_BUTTON_RIGHT)
            iMode = TRANSLATE;
            else if (glfwButton == GLFW_MOUSE_BUTTON_LEFT)
            iMode = ROTATE;
        }
        else
        {
            iMode = NO_MOVE;
        }
    }
    
    
    
}
void OnMousePos(GLFWwindow* win, double mouseX, double mouseY)
{
    if (!TwEventMousePosGLFW((int)mouseX,(int) mouseY))  // Send event to AntTweakBar
    {
        glm::vec2 v;
        v.x = (float)mouseX / (float)Wwidth;
        v.y = 1.0f - (float)mouseY / (float)Wheight;
        v = 2.0f*v - 1.0f;
        glm::vec3 newMouse = projectOnSphere(v);
        if (iTarget == CAMERA && iMode != NO_MOVE)
        trackBallCamera(scene->camera(), newMouse, oldMouse);
        else if (iTarget == NODE && iMode != NO_MOVE)
        trackBallFrame(scene->getManipulatedNode()->frame(), scene->camera(), newMouse, oldMouse);
    }
}
void OnMouseWheel(GLFWwindow* win, double posx, double posy)
{
    if (!TwEventMouseWheelGLFW((int)posx))   // Send event to AntTweakBar
    {
        if (scene)
        {
            if (iTarget == CAMERA)
            {
                scene->camera()->translate(glm::vec3(0.0, 0.0, -0.25*posy));
            }
            else if (iTarget == NODE && scene->camera() != NULL)
            {
                //float modif = (posy) ? 1.0f : -1.0f;
                glm::vec3 dep = (float)(posy)*0.1f*scene->camera()->convertDirTo(glm::vec3(0.0, 0.0, -1.0), scene->getManipulatedNode()->frame());
                scene->getManipulatedNode()->frame()->translate(dep);
                
                
            }
        }
    }
    
}
void OnKey(GLFWwindow* win, int glfwKey, int scancode, int glfwAction, int modifiers)
{
    if (!TwEventKeyGLFW(glfwKey, glfwAction))  // Send event to AntTweakBar
    {
        
        if (glfwKey == GLFW_KEY_ESCAPE && glfwAction == GLFW_PRESS) // Want to quit?
        glfwSetWindowShouldClose(window, GL_TRUE);
        else
        {
            if (glfwAction == GLFW_PRESS)
            switch (glfwKey)
            {
                case GLFW_KEY_UP:
                scene->camera()->translate(glm::vec3(0.0, 0.0, -10.0));
                break;
                case GLFW_KEY_DOWN:
                scene->camera()->translate(glm::vec3(0.0, 0.0, 10.0));
                break;
                case GLFW_KEY_RIGHT:
                
                break;
                case GLFW_KEY_LEFT:
                
                break;
                case GLFW_KEY_F5:
                scene->nextCamera();
                
                break;
                case GLFW_KEY_F6:
                scene->nextManipulatedNode();
                break;
                case GLFW_KEY_F7:
                engine->drawLights = !engine->drawLights;
                if (engine->drawLights)
                LOG(INFO) << "Now drawing lights";
                else
                LOG(INFO) << "Now hiding lights";
                break;
                case GLFW_KEY_F8:
                engine->drawBoundingBoxes = !engine->drawBoundingBoxes;
                if (engine->drawBoundingBoxes)
                LOG(INFO) << "Now drawing BoundingBoxes";
                else
                LOG(INFO) << "Now hiding BoundingBoxes";
                break;
                
                
                
                
                case GLFW_KEY_SPACE:
                if (iTarget == CAMERA)
                {
                    iTarget = NODE;
                    LOG(INFO)  << "Now manipulating Node "<< scene->getManipulatedNode()->getName();
                }
                else
                {
                    iTarget = CAMERA;
                    LOG(INFO)  << "Now manipulating Camera "<< scene->camera()->getName();
                }
                
                break;
            }
            
        }
    }
}
void OnChar(GLFWwindow* win, unsigned int glfwChar)
{
    if( !TwEventCharGLFW(glfwChar, GLFW_PRESS) )    // Send event to AntTweakBar
    {
        
        switch(glfwChar)
        {
            case 'z':
            scene->camera()->translate(glm::vec3(0.0,0.0,-1.0));
            break;
            case 's':
            scene->camera()->translate(glm::vec3(0.0,0.0,1.0));
            break;
            case 'q':
            scene->camera()->translate(glm::vec3(-1.0,0.0,0.0));
            break;
            case 'd':
            scene->camera()->translate(glm::vec3(1.0,0.0,0.0));
            break;
            case 'i':
            scene->getManipulatedNode()->frame()->translate(glm::vec3(0.0,0.0,0.1));
            break;
            case 'k':
            scene->getManipulatedNode()->frame()->translate(glm::vec3(0.0,0.0,-0.1));
            break;
            case 'l':
            scene->getManipulatedNode()->frame()->translate(glm::vec3(0.1,0.0,0.0));
            break;
            case 'j':
            scene->getManipulatedNode()->frame()->translate(glm::vec3(-0.1,0.0,0.0));
            break;
            
            default :
            break;
            
        }
    }
    
    
}
void error_callback(int error, const char* description)
{
    LOG(ERROR)  << "GLFW : " << description << std::endl;
}
bool initSystem()
{
    
    if( !glfwInit() )
    {
        // A fatal error occurred
        LOG(ERROR)  << "GLFW initialization failed" << std::endl;
        return (false);
    }
    
    
    // FSAA Antialiasing : Possible Performance Killer
    glfwWindowHint (GLFW_REFRESH_RATE,0);
    glfwWindowHint (GLFW_SAMPLES,8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_TRUE);
    window = glfwCreateWindow(Wwidth, Wheight, "GobLim", NULL, NULL);
    //window = glfwCreateWindow(Wwidth, Wheight, "Goblim", glfwGetPrimaryMonitor(), NULL);
    
    if( !window )
    {
        // A fatal error occurred
        LOG(ERROR) << "Cannot open GLFW window" << std::endl;
        glfwTerminate();
        return (false);
    }
    glfwMakeContextCurrent(window);
    
    glfwSwapInterval(0);
    
    // Set GLFW event callbacks
    glfwSetFramebufferSizeCallback(window,OnWindowSize);
    glfwSetMouseButtonCallback(window,OnMouseButton);
    glfwSetCursorPosCallback(window,OnMousePos);
    glfwSetScrollCallback(window,OnMouseWheel);
    glfwSetKeyCallback(window,OnKey);
    glfwSetErrorCallback(error_callback);
    glfwSetCharCallback(window, OnChar);
    
    // Initialize AntTweakBar
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(Wwidth, Wheight);
    
    // Initialize GLEW (or glad)
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    //if (GLEW_OK != glewInit())
    {
        LOG(ERROR) << "OpenGL context initialization failed" << std::endl;
        glfwTerminate();
        return false;
    }
    
    
    //Initialize random generator
    srand((unsigned int)time(0));
    return(true);
}
void computeFps()
{
    
    frame_Num++;
    current_time = GetTimeMs64();
    
    if((current_time-fps_last_time) >= 1000)
    {
        
        stringstream ss (stringstream::in | stringstream::out);
        
        ss << 1000.0/engine->getFrameTime() << ", Frame avg time :  " << engine->getFrameTime() ;
        string title = "GobLim [FPS : " + ss.str() + "]" ;
        glfwSetWindowTitle(window, title.c_str());
        frame_Num=0;
        timeTotal = 0;
        fps_last_time = current_time;
    }
    
}

void shutDown()
{
    delete engine;
    scene->kill();
    glfwDestroyWindow(window);
    glfwTerminate();
    LOG(TRACE)  << "[Execution time] " << (current_time - start_time)/1000.0f <<"s" << endl;
    LOG(TRACE) << "Shutting Down ";
    exit(1);
}

void draw()
{
    engine->render();
}

void animate(const int elapsedTime)
{
    engine->animate(elapsedTime);
}
float eTime = 0;
void mainLoop()
{
    frame = 0;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        last_time = current_time;
        current_time = GetTimeMs64();
        
        if (init)
        {
            eTime += (current_time - last_time);
            {
                animate((const int ) (current_time - last_time));
                eTime = 0;
            }
            
            draw();
            computeFps();
        }
        ++frame;
        TwDraw();
        glfwSwapBuffers(window);
    }
}

int main()
{
    start_time = GetTimeMs64();
    
    Wwidth = 800;
    Wheight = 600;
    
    if (!initSystem())
    {
        LOG(ERROR)  << "Error : System initialization" << std::endl;
        shutDown();
        return 0;
    }
    
    OnWindowSize(window, Wwidth, Wheight);
    
    std::cout << "GPU : " << GPUInfo::Instance()->getGPUName() << std::endl;
    std::cout << "OpenGL : " << GPUInfo::Instance()->getOpenGLVersion() << std::endl;
    
    
    engine = new SampleEngine(Wwidth,Wheight);
    init = false;
    try
    {
        init = engine->init();
    }
    catch(const std::exception & e )
    {
        
        cerr << e.what() ;
    }
    
    scene = Scene::getInstance();
    current_time = GetTimeMs64();
    LOG(INFO)  << "[Initialization time] " << (current_time - start_time)/1000.0f <<"s" << endl;
    fps_last_time = GetTimeMs64();
    mainLoop();
    
    LOG(TRACE) << 1000/engine->getFrameTime() << " , " << engine->getFrameTime() << std::endl;
    shutDown();
    
    
}

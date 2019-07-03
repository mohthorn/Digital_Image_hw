//Homework12 Shading
//ChengyiMin
#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <cmath>

#include "imglib.h"
//#include "methods.h"

using namespace std; 

#define M 8
#define N 8
#define K_SIZE 4
#define RGBWHITE    1, 1, 1     // white for screen background
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================


img ppm;
img ppm_l;
img ppm_d;
img ppm_n;
img ppm_s;
img new_ppm;
img depth;

int global_x=0;
int global_y=0;

int tracking=0;


string md;
char * filename;
char *method;
// =============================================================================
// setPixels()
//
// This function stores the RGB values of each pixel to "pixmap."
// Then, "glutDisplayFunc" below will use pixmap to display the pixel colors.
// =============================================================================


static void windowResize(int w, int h)
{   
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,(w/2),0,(h/2),0,1); 
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}
static void windowDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0,0);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glDrawPixels(new_ppm.width, new_ppm.height, GL_RGB, GL_UNSIGNED_BYTE, new_ppm.pixels);
    glFlush();
}
static void processMouse(int button, int state, int x, int y)
{ 
    y = new_ppm.height - y;
    if(button != GLUT_LEFT_BUTTON)    // ignore all but left button
        return;
    if(state == GLUT_DOWN)
    {
        tracking=1;
    }
    else
        tracking=0;
    
}

static void mouseMotion( int x, int y)
{ 
    y = new_ppm.height - y;
    global_x=x;
    global_y=y;

}

static void timer( int i)
{ 
    new_ppm.diffuse(ppm_l,ppm_d,ppm_n,ppm_s,depth,global_x,global_y);
    //cout<<"1"<<endl;
    // string store_name=to_string(i);
    // string fname="anim/animation";
    // char *f = new char[fname.length() + 1];
    // std::strcpy(f, fname.c_str());
    //new_ppm.ppm_store(f,store_name.c_str());
    glutPostRedisplay();
    //glutPassiveMotionFunc(mouseMotion);
    glutTimerFunc( 1000/24.0, timer, 0 );
}


static void init(void)
{
    glClearColor(1,1,1,1); // Set background color.
}

// =============================================================================
// main() Program Entry
// =============================================================================


int main(int argc, char *argv[])
{

    //initialize the global variables
    if(argc!=5)
    {
        cout<<"usage: ./pr01 \'filename\' \'filename\' \'filename\' \'filename\'"<<endl;
        return 0;
    }
    else
    {
        ppm.setPixels(argv[1]);
        ppm_n.setPixels(argv[2]);
        ppm_s.setPixels(argv[3]);
        depth.setPixels(argv[4]);
        ppm_d=img(ppm.width,ppm.height,ppm.depth);
        ppm_l=img(ppm.width,ppm.height,ppm.depth);
        new_ppm=img(ppm.width,ppm.height,ppm.depth);
    }
    method=argv[2];

    md="Light";
    //cout<<"para1: "<<para1<<endl;
    //cout<<"para2: "<<para2<<endl;
    filename=argv[1];
    //new_ppm=img(ppm.width,ppm.height,ppm.depth);
    //actions
    string store_name=md;
    cout<<new_ppm.width<<"*"<<new_ppm.height<<endl;
    ppm_l=ppm.lighting(0);
    ppm_d=ppm.lighting(-100);
    new_ppm.diffuse(ppm_l,ppm_d,ppm_n,ppm_s,depth);
    new_ppm.ppm_store(filename,store_name.c_str());
 
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100); // Where the window will display on-screen.
    glutInitWindowSize(new_ppm.width, new_ppm.height);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Homework Zero");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    //glutMouseFunc(processMouse);
    //glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(mouseMotion);
    glutTimerFunc(1000/24.0, timer, 0);
    glClearColor(RGBWHITE, 1);
    glutMainLoop();    
    return 0; //This line never gets reached. We use it because "main" is type int.
}
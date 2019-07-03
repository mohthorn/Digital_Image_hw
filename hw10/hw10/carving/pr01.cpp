//Homework10 Stitching & Carving
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
// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================


img ppm;
img new_ppm;
img ctrl;

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
    if(state == GLUT_UP)
        exit(0);   
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
    if(argc!=4)
    {
        cout<<"usage: ./pr01 \'filename\' \'method\' \'cropped column number\'"<<endl;
        return 0;
    }
    else
    {
        ppm.setPixels(argv[1]);
        new_ppm=img(ppm.width,ppm.height,ppm.depth);
    }
    method=argv[2];

    md=method;
    //cout<<"para1: "<<para1<<endl;
    //cout<<"para2: "<<para2<<endl;
    filename=argv[1];
    int para =stof(argv[3]);
    //new_ppm=img(ppm.width,ppm.height,ppm.depth);
    new_ppm=img(ppm);
    //actions
    cout<<new_ppm.width<<"*"<<new_ppm.height<<endl;
    new_ppm=new_ppm.seamCarving(new_ppm,1,para);
    string store_name=md+to_string(para);
    cout<<new_ppm.width<<"*"<<new_ppm.height<<endl;
    new_ppm.ppm_store(filename,store_name.c_str());
    // OpenGL Commands:
    // Once "glutMainLoop" is executed, the program loops indefinitely to all
    // glut functions.  
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100); // Where the window will display on-screen.
    glutInitWindowSize(new_ppm.width, new_ppm.height);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Homework Zero");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    glutMouseFunc(processMouse);
    glutMainLoop();    
    return 0; //This line never gets reached. We use it because "main" is type int.
}
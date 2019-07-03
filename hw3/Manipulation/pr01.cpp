//Homework3 Color Manipulation Chengyi Min
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

using namespace std; 

// =============================================================================
// These variables will store the input ppm image's width, height, and color
// =============================================================================

class img{
public:
    int width;
    int height;
    int depth;
    unsigned char *pixels;

    img(int width,int height,int depth)
    {
        this->width=width;
        this->height=height;
        this->depth=depth;
        pixels=new unsigned char[width*height*3];
        memset(pixels,0,width*height*3);
    }
    img(void)
    {
        width=height=depth=0;
    }
};

img ppm;
double rm[3][100];
double h[3][100];
int len[3];

// =============================================================================
// setPixels()
//
// This function stores the RGB values of each pixel to "pixmap."
// Then, "glutDisplayFunc" below will use pixmap to display the pixel colors.
// =============================================================================
void setPixels(char* fname)
{

    ifstream ppm_file;
    
    ppm_file.open(fname,ios::binary | ios::in);
    if (ppm_file==NULL)
    {
        cout<<"File Open Error!"<<endl;
        return;
    }
    else
        {
            int feat[4];
            int counter=0;
            char start_flag=0;
            while(!ppm_file.eof())
            {       
                    string s;
                    ppm_file>>s;
                    if(s[0]=='#')   //eliminate comments
                    {
                        string s2;
                        getline(ppm_file,s2);
                    }
                    if(s[0]!='#'&&counter<4)    //get attributes of the img
                    {
                        if(counter==0)
                        {
                            feat[counter]=s[1]-'0';
                        }
                        else
                        {
                            feat[counter]=atoi(s.c_str());
                        }
                        counter++;                  
                }  
                if(counter>=4)
                    break;           
            }

 

            ppm=img(feat[1],feat[2],feat[3]);   //create img array
            int width=feat[1];
            int height=feat[2];
            int depth=feat[3];
            while(!ppm_file.eof())  //get to the start of pixel values
            { 

                    if(counter>=4)
                    {
                        char value=0;
                        ppm_file.read(reinterpret_cast<char*>(&value),1);

                        if(value+0==10) //end of magic number
                        {
                            start_flag=1;
                            break;
                        }
                        
                    }  
            }
            for(int j=height-1;j>=0;j--)    //read pixel values
                for(int i=0;i<width;i++)
                {
                    int k=(width*j+i)*3;
                    char color;
                    ppm_file.read(&color,1);               
                    ppm.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    ppm.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    ppm.pixels[k]=(unsigned char)color;
                }
            
            cout<<ppm.width<<' '<<ppm.height<<' '<<ppm.depth<<' '<<ppm.width*ppm.height<<endl;
        }   
}

int piecewise(int x,int channel)
{
    double result=0;
    
    for(int i=0;i<len[channel]-1;i++)
    {   
        double x_f=1.0*x/255.0;
        if(x_f>=h[channel][i]&&x_f<h[channel][i+1])
        {
            double dist =x_f-h[channel][i];
            result=1.0*rm[channel][i]+1.0*(rm[channel][i+1]-rm[channel][i])*(len[channel]-1)*dist;
            break;
        }
    }
    if(x==255)
        result=rm[channel][len[channel]-1];
    return result*255;
}



void manipulate()
{
    for(int i=0;i<ppm.width;i++)    //read pixel values
        for(int j=0;j<ppm.height;j++)
        {
            int k=(ppm.width*j+i)*3;
            char color;              
            ppm.pixels[k]=piecewise(ppm.pixels[k],0);
            ppm.pixels[k+1]=piecewise(ppm.pixels[k+1],1);
            ppm.pixels[k+2]=piecewise(ppm.pixels[k+2],2);
        }
}

double line(double x0, double y0, double a, double b,double x, double y)
{
  return a*(x-x0)+b*(y-y0);
}

double short_line(double x0, double y0, double a, double b,double x, double y,double x1)
{
    double max=std::max(fabs(line(x0, y0, a, b,x, y))-50,line(x0, 0, -1, 0, x, y));
    max=std::max(max,line(x1, 1, 1, 0, x, y));
}

void plot_curve()
{
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<len[i]-1;j++)
        {
            double a=(rm[i][j]-rm[i][j+1])*100.0;
            double b=(h[i][j+1]-h[i][j])*100.0;
            for(int p=100*i;p<100*(i+1);p++)
            {
                for(int q=0;q<100;q++)
                {
                    int k=(ppm.width*q+p)*3;
                    if(short_line(100.0*h[i][j]+i*100,100.0*rm[i][j],a,b,p,q,100.0*h[i][j+1]+i*100)<=0)
                    {
                        ppm.pixels[k]=(i==0)*255;
                        ppm.pixels[k+1]=(i==1)*255;
                        ppm.pixels[k+2]=(i==2)*255;
                    }

                }
            }
        }
    }
}

void pinput()
{
    ifstream fin;
    fin.open("points.txt");
    string color[3];

    int offset;
    for(int i=0;i<3;i++)
    {
        getline(fin,color[i]);
        int n=0;
        const char *input;
        input =color[i].c_str();
        while(sscanf(input,"%lf%n",&rm[i][n],&offset)==1)
        {
            input+=offset;            
        
            n++;
        }
        for(int j=0;j<n;j++)
            h[i][j]=j*1.0/(n-1);
        for(int j=0;j<n;j++)
            cout<<rm[i][j]<<" ";
        cout<<endl;
        for(int j=0;j<n;j++)
            cout<<h[i][j]<<" ";
        cout<<endl;
        len[i]=n;
    }
}

void ppm_store(char * name)
{
  ofstream fout;
  char *s=name;
  sprintf(s,"%s_boost.ppm",name);
  fout.open(s,ios::binary | ios::out);
  fout<<"P6"<<endl;
  fout<<ppm.width<<" "<<ppm.height<<endl;
  fout<<"255";
  char value=10;
  fout.write(reinterpret_cast<char*>(&value),1);   //end of magic 
  for(int j=ppm.height-1;j>=0;j--)
  {
    for(int i=0;i<ppm.width;i++)
    {
      int k=(ppm.width*j+i)*3;
    
      fout.write(reinterpret_cast<char*>(&ppm.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&ppm.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&ppm.pixels[k]),1);

    }
  }
    

}

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
    glDrawPixels(ppm.width, ppm.height, GL_RGB, GL_UNSIGNED_BYTE, ppm.pixels);
    glFlush();
}
static void processMouse(int button, int state, int x, int y)
{
    if(state == GLUT_UP)
    exit(0);               // Exit on mouse click.
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
    if(argc!=2)
    {
        cout<<"usage: ./pr01 \'filename\'"<<endl;
        return 0;
    }
    else
        setPixels(argv[1]);
    pinput();
    manipulate();
    plot_curve();
    ppm_store(argv[1]);
    // OpenGL Commands:
    // Once "glutMainLoop" is executed, the program loops indefinitely to all
    // glut functions.  
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100); // Where the window will display on-screen.
    glutInitWindowSize(ppm.width, ppm.height);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Homework Zero");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    glutMouseFunc(processMouse);
    glutMainLoop();    
    return 0; //This line never gets reached. We use it because "main" is type int.
}
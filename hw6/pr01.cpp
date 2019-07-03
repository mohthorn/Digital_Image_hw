//Homework6 Inverse Mapping
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

using namespace std; 

#define M 8
#define N 8
#define K_SIZE 9
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
img new_ppm;
img ctrl;
// =============================================================================
// setPixels()
//
// This function stores the RGB values of each pixel to "pixmap."
// Then, "glutDisplayFunc" below will use pixmap to display the pixel colors.
// =============================================================================
void setPixels(char* fname,img &pic)
{

    ifstream ppm_file;  
    ppm_file.open(fname,ios::binary | ios::in);
    if (!ppm_file)
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

 

            pic=img(feat[1],feat[2],feat[3]);   //create img array
            new_ppm=img(feat[1],feat[2],feat[3]);
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
                    pic.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    pic.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    pic.pixels[k]=(unsigned char)color;
                }
            
            cout<<pic.width<<' '<<pic.height<<' '<<pic.depth<<' '<<pic.width*pic.height<<endl;
        }   
}


void scale(double u,double v, int &x, int &y,double u_para,double v_para)
{
    double m[2]={1.0/u_para,1.0/v_para};
    double x_f,y_f;
    x_f=u*m[0];
    y_f=v*m[1];
    x=x_f;
    y=y_f;
}

void rotata(double u,double v, int &x, int &y,double theta)
{
    double m[2][2];
    double t[2]={u,v};
    theta=-theta;
    m[0][0]=cos(theta);
    m[0][1]=-sin(theta);
    m[1][0]=sin(theta);
    m[1][1]=cos(theta);
    x=0;    y=0;
    double x_f=0,y_f=0;
    for(int i=0;i<2;i++)
    {
        x_f+=t[i]*m[0][i];
        y_f+=t[i]*m[1][i];
    }
    x=x_f;
    y=y_f;
}

void shear(double u,double v, int &x, int &y,double para1, double para2)
{
    double m[2][2];
    double t[2]={u,v};
    m[0][0]=-1/(para1*para2-1);
    m[0][1]=para1/(para1*para2-1);
    m[1][0]=para2/(para1*para2-1);
    m[1][1]=-1/(para1*para2-1);
    x=0;    y=0;
    double x_f=0,y_f=0;
    for(int i=0;i<2;i++)
    {
        x_f+=t[i]*m[0][i];
        y_f+=t[i]*m[1][i];
    }
    x=x_f;
    y=y_f;
}

void mirror(double u,double v, int &x, int &y)
{
    double m[2][2];
    double t[2]={u,v};
    m[0][0]=-1;
    m[0][1]=0;
    m[1][0]=0;
    m[1][1]=1;
    x=0;    y=0;
    double x_f=0,y_f=0;
    for(int i=0;i<2;i++)
    {
        x_f+=t[i]*m[0][i];
        y_f+=t[i]*m[1][i];
    }
    x=x_f;
    y=y_f;
}

void translation(double u,double v, int &x, int &y, double para1,double para2)
{
    double m[3][3]={1,  0,  -para1,
                    0,  1,  -para2,
                    0,  0,  1};
    double t[3]={u,v,1};
    double x_f=0,y_f=0;
    for(int i=0;i<3;i++)
    {
        x_f+=t[i]*m[0][i];
        y_f+=t[i]*m[1][i];
    }
    x=x_f;
    y=y_f;
}




void perspective(double u,double v, int &x, int &y, double para1,double para2)
{
    double w=(para1*u+para2*v+1)/ppm.width;
    double m[3][3]={w,  0,  0,
                    0,  w,  0,
                    0,  0,  1};
    double t[3]={u,v,1};
    double x_f=0,y_f=0;
    for(int i=0;i<3;i++)
    {
        x_f+=t[i]*m[0][i];
        y_f+=t[i]*m[1][i];
    }
    x=x_f;
    y=y_f;
}

void transformation(img &pic, img &new_pic, string md, double para1=1,double para2=1)
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0, 1);
    if(md=="scale")
    {
        //cout<<para1<<" "<<para2<<endl;
        double new_width=pic.width*1.0*(para1);
        double new_height=pic.height*1.0*(para2);
        new_ppm=img((int)(new_width),(int)new_height,pic.depth);
        
    }
    if(md=="rotate")
    {
        para1=para1/180*M_PI;
        double new_width=pic.width*1.0*cos(para1)+pic.height*1.0*sin(para1);
        double new_height=pic.width*1.0*sin(para1)+pic.height*1.0*cos(para1);
        cout<<new_width<<" "<<new_height;
        new_ppm=img((int)(new_width),(int)new_height,pic.depth);
        
    }   
    if(md=="shear")
    {
        double new_width=pic.width*1.0*2;
        double new_height=pic.width*1.0*2;
        cout<<new_width<<" "<<new_height;
        new_ppm=img((int)(new_width),(int)new_height,pic.depth);       
    } 
    if(md=="mirror")
    {
        double new_width=pic.width*1.0;
        double new_height=pic.width*1.0;
        cout<<new_width<<" "<<new_height;
        new_ppm=img((int)(new_width),(int)new_height,pic.depth);       
    } 
    if(md=="translation")
    {
        double new_width=pic.width*1.0*2;
        double new_height=pic.width*1.0*2;
        cout<<new_width<<" "<<new_height;
        new_ppm=img((int)(new_width),(int)new_height,pic.depth);       
    } 

    if(md=="perspective")
    {
        double new_width=pic.width*1.0;
        double new_height=pic.width*1.0;
        cout<<new_width<<" "<<new_height;
        new_ppm=img((int)(new_width),(int)new_height,pic.depth);       
    } 
    for(int i=0;i<new_pic.width;i++)
    {
        for(int j=0;j<new_pic.height;j++)
        {
            int k=(new_pic.width*j+i)*3;
            int new_i,new_j;
            double ri=dis(gen);
            double rj=dis(gen);
            double new_val[3]={};
            for(int m=0;m<M;m++)
                for(int n=0;n<N;n++)
                {
                    double u_in=i+(m*1.0+ri)/(1.0*M);
                    double v_in=j+(n*1.0+rj)/(1.0*N);
                    if(md=="scale")
                        scale(u_in,v_in,new_i,new_j,para1,para2);
                    if(md=="rotate")
                    {
                        rotata(u_in-new_pic.width*0.5,v_in-new_pic.height*0.5,new_i,new_j,para1);
                        new_i+=pic.width/2;
                        new_j+=pic.height/2;
                    }
                    if(md=="shear")
                    {
                        shear(u_in,v_in,new_i,new_j,para1,para2);
                    }
                    if(md=="mirror")
                    {
                        mirror(u_in-new_pic.width*0.5,v_in-new_pic.height*0.5,new_i,new_j);
                        new_i+=pic.width/2;
                        new_j+=pic.height/2;
                    }
                    if(md=="translation")
                    {
                        translation(u_in,v_in,new_i,new_j,para1,para2);
                    }
                    if(md=="perspective")
                    {
                        perspective(u_in,v_in,new_i,new_j,para1,para2);
                    }                    
                    if(new_i<0)
                        new_i=-1;
                    if(new_j<0)
                        new_j=-1;
                    if(new_i>=pic.width)
                        new_i=-1;
                    if(new_j>=pic.height)
                            new_j=-1;
                    int k2=(pic.width*new_j+new_i)*3;
                    for(int count=0;count<3;count++)
                    {
                        if(new_i!=-1 && new_j!=-1)
                            new_val[count]+=(double)(pic.pixels[k2+count]*1.0/(1.0*(M*N)));
                    }
                }
            for(int count=0;count<3;count++)
                new_pic.pixels[k+count]=new_val[count];
        }
    }   
}


void ppm_store(char * name,char * method,img pic)
{
  ofstream fout;
  char *s=name;
  sprintf(s,"%s_%s.ppm",name,method);
  fout.open(s,ios::binary | ios::out);
  fout<<"P6"<<endl;
  fout<<pic.width<<" "<<pic.height<<endl;
  cout<<pic.width<<" "<<pic.height<<endl;
  fout<<"255";
  char value=10;
  fout.write(reinterpret_cast<char*>(&value),1);   //end of magic 
  for(int j=pic.height-1;j>=0;j--)
  {
    for(int i=0;i<pic.width;i++)
    {
      int k=(pic.width*j+i)*3;
    
      fout.write(reinterpret_cast<char*>(&pic.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&pic.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&pic.pixels[k]),1);

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
    glDrawPixels(new_ppm.width, new_ppm.height, GL_RGB, GL_UNSIGNED_BYTE, new_ppm.pixels);
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
    if(argc!=5)
    {
        cout<<"usage: ./pr01 \'filename\' \'method\' \'parameter1\' \'parameter2\'"<<endl;
        return 0;
    }
    else
    {
        setPixels(argv[1],ppm);
    }
    char *method=argv[2];

    string md=method;
    double para1 =atof(argv[3]);
    double para2 =atof(argv[4]);
    cout<<"para1: "<<para1<<endl;
    cout<<"para2: "<<para2<<endl;
    transformation(ppm,new_ppm,md,para1,para2);
    ppm_store(argv[1],method,new_ppm);
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
#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
#include <cstring>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <cmath>
#include <algorithm>
#include <string>
#include <ctime>
#include <random>

using namespace std;
#define IMG_SIZE 800
#define X 4

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

img shape;



void ppm_store(img shape)
{
  ofstream fout;
  fout.open("procedural_result.ppm",ios::binary | ios::out);
  fout<<"P6"<<endl;
  fout<<IMG_SIZE<<" "<<IMG_SIZE<<endl;
  fout<<"255";
  char value=10;
  fout.write(reinterpret_cast<char*>(&value),1);   //end of magic 
  for(int j=shape.height-1;j>=0;j--)
  {
    for(int i=0;i<shape.width;i++)
    {
      int k=(shape.width*j+i)*3;
      fout.write(reinterpret_cast<char*>(&shape.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&shape.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&shape.pixels[k]),1);

    }
  }
    

}

double circle(double x0,double y0,double r,double x,double y)
{
    
  return (x-x0)*(x-x0)+(y-y0)*(y-y0)-r*r;
}

double line(double x0, double y0, double a, double b,double x, double y)
{
  return a*(x-x0)+b*(y-y0);
}

double function_curve(double x, double y)
{
  double r=10+0.3*(x-400)+20*sin(0.05*(x-400))-y+180;
  return -r;
}

double blobby(double x, double y)
{
  double sigmoid=200/(1+exp(-(0.02*x-8)))-y+200;
  double sigmoid2=-200/(1+exp(-(0.02*x-8)))-y+600;
  double l2=line(200,163,-1,-1.5,x,y);
  double l3=line(200,637,-1,1.5,x,y);
  sigmoid2 =-sigmoid2;
  double circ=circle(370,400,330, x, y);
  double circ2=circle(217,480,140, x, y);
  double circ3=circle(217,320,140, x, y);
  double max=std::max(sigmoid,sigmoid2);
  max=std::max(max,l2);
  max=std::max(max,l3);
  max=std::max(circ,max);
  max=std::min(circ2,max);
  max=std::min(circ3,max);
  return max;
}

double shaded(double x, double y, double r)
{
  double circ=circle(400,400,r, x, y);
  return circ;
}

//convex square
double convex(double x, double y)
{
  double a[4]={-3,1,8,1};
  double b[4]={1,2,-1,-7};
  double x0[4]={20,200,700,200};
  double y0[4]={20,700,400,200};
  double max=IMG_SIZE*IMG_SIZE*IMG_SIZE;
  max=-max;
  for(int k=0;k<4;k++)
  {
    if(line(x0[k],y0[k],a[k],b[k],x,y)>max)
    {
      max=line(x0[k],y0[k],a[k],b[k],x,y);
    }
  }
  return max;
}


double star(double x, double y)
{
  double a[5]={-1,8,-1,1,1,};
  double b[5]={1,-1,-1,2,-4,};
  double x0[5]={20,600,600,100,100};
  double y0[5]={20,400,200,700,200};
  double max=IMG_SIZE*IMG_SIZE*IMG_SIZE;
  
  for(int i=0;i<5;i++)
  {
    max=IMG_SIZE*IMG_SIZE*IMG_SIZE;
    max=-max;
    for(int k=i;k<i+3;k++)
    {
      if(line(x0[k%5],y0[k%5],a[k%5],b[k%5],x,y)>max)
      {
        max=line(x0[k%5],y0[k%5],a[k%5],b[k%5],x,y);
        //if(max*max<100)
          //return max;
      }
      
    }
    if(max<0)
      return max;
  }
  return max;
}

double super_sampling(int x, int y,char * form)
{
  string f=form;
  int sum=0;
  double max=0;
  
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(-0.5, 0.5);

   
  for(int i=0;i<X;i++)
    for(int j=0;j<X;j++)
    {
      double ri=dis(gen);
      double rj=dis(gen);
      double m=x+(i+0.5)*1.0/X+ri/X;  //jittering
      double n=y+(j+0.5)*1.0/X+rj/X;
      if(f=="star")
          max=star(m,n);
      if(f=="convex")
          max=convex(m,n);
      if(f=="function")
          max=function_curve(m,n);
      if(f=="blobby")
          max=blobby(m,n);
      if(f=="shaded")
          max=circle(400,400,200,m,n);
      if(max<0)
      {
        sum++;
      }
    }
  return sum*0.9/(X*X)+(X*X-sum)*0.1/(X*X);
}


void setPixels(char * formation)
{
  string f=formation;
  shape=img(800,800,255);
   for(int y = 0; y < shape.height ; y++) 
   {
     for(int x = 0; x < shape.width; x++) 
     {
        int i = (y * shape.width + x) * 3; 

        
        double max=circle(400,400,200, x, y);
        //cout<<formation<<endl;
        double coef=super_sampling(x,y,formation);
        shape.pixels[i] = int(255*(1-coef));
        shape.pixels[i+1] =int(255*(1-coef)); 
        shape.pixels[i+2] = int(255*coef); 
        

        if(f=="shaded")
          {
            int max2=shaded(x+70,y+70,200);
            if( max< 0 && max2<=0)
            {
              int p=i;
              double increase=shape.pixels[p]+(-max2)/300;
              if(increase>255)
                shape.pixels[p]=255;
              else
                shape.pixels[p]=int(increase);
            }
          }
        

          if(f=="shaded")
          {
            int max3=shaded(x-400,y-400,0);
            if(max>=0 && max3>=0 )
            {
              for(int p=i+2; p>=i;p--)
              {
                double increase;
                increase=shape.pixels[p]-1.0/5000*max3;
                if(increase<0)
                  shape.pixels[p]=0;
                else
                  shape.pixels[p]=int(increase);
              }
            }
          }
  
        

     }
   }
}

void ppm_store(char * name)
{
  ofstream fout;
  char *s=name;
  sprintf(s,"%s_%dX.ppm",name,X);
  fout.open(s,ios::binary | ios::out);
  fout<<"P6"<<endl;
  fout<<IMG_SIZE<<" "<<IMG_SIZE<<endl;
  fout<<"255";
  char value=10;
  fout.write(reinterpret_cast<char*>(&value),1);   //end of magic 
  for(int j=shape.height-1;j>=0;j--)
  {
    for(int i=0;i<shape.width;i++)
    {
      int k=(shape.width*j+i)*3;
    
      fout.write(reinterpret_cast<char*>(&shape.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&shape.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&shape.pixels[k]),1);

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
  glDrawPixels(shape.width, shape.height, GL_RGB, GL_UNSIGNED_BYTE, shape.pixels);
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
  //srand (time(NULL));
  if(argc!=2)
    {
        cout<<"usage: ./pr01 \'shape name\'"<<endl;
        return 0;
    }
  else
  {
        setPixels(argv[1]);
        ppm_store(argv[1]);
  }

 
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100); // Where the window will display on-screen.
  glutInitWindowSize(shape.width, shape.height);
  glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
  glutCreateWindow("Homework Zero");
  init();
  glutReshapeFunc(windowResize);
  glutDisplayFunc(windowDisplay);
  glutMouseFunc(processMouse);
  glutMainLoop();

  return 0; //This line never gets reached. We use it because "main" is type int.
}


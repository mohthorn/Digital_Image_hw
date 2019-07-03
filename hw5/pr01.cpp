//Homework5 Non Stationary Filters
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

#define M 4
#define N 4
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


double line(double x0, double y0, double a, double b,double x, double y,double T=1)
{
  return 2*fabs(a*(x-x0)+b*(y-y0))-T<0;
}

void W(img &pic, int c_i, int c_j,double (&W_m)[K_SIZE][K_SIZE])
{
    // cout<<c_i<<" ";
    // cout<<c_j<<endl;
    int k=(pic.width*c_j+c_i)*3;
    int r=pic.pixels[k];
    int g=pic.pixels[k+1];
    double x=2*r-1;
    double y=2*g-1;
    double len=sqrt(x*x+y*y);
    x=x/len;
    y=y/len;
    //calculate a kernel
    double sum=0;
    double T=1;
    for(int i=0;i<K_SIZE;i++)
    {
        for(int j=0;j<K_SIZE;j++)
        {
            W_m[i][j]=0;
            for(int m=0;m<M;m++)
                for(int n=0;n<N;n++)
                {                   
                    W_m[i][j]+=line(c_i+0.5,c_j+0.5,x,y,c_i+i-(K_SIZE)/2+m/M+0.5,c_j+j+n/N+0.5-(K_SIZE)/2,1)*1.0/(M*N);                  
                }
            sum+=W_m[i][j];               
        }
    }
    for(int i=0;i<K_SIZE;i++)
    {
        for(int j=0;j<K_SIZE;j++)
        {
            W_m[i][j]/=sum;
            //cout<<W_m[i][j]<<" ";
        }
        //cout<<endl;
    }
    //cout<<endl;
}

void find_point(int x,int y, int &a,int &b)
{
    a=x;
    b=y;
    if(a<0)
        a=-a;
    if(b<0)
        b=-b;
    if(a>=ppm.width)
        a=2*(ppm.width-1)-a;
    if(b>=ppm.height)
        b=2*(ppm.height-1)-b;
}

double motion_blur( double T)    //diamond
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0, 1);
    double result=0;
    int center_x=K_SIZE/2;
    int center_y=K_SIZE/2;
    for(int i=0;i<ctrl.width;i++)
        for(int j=0;j<ctrl.height;j++)
        {
            double w[K_SIZE][K_SIZE];
            W(ctrl,i,j,w);
            double new_val[3]={0,0,0};
            int k=(ppm.width*j+i)*3;
            double ri=dis(gen);
            double rj=dis(gen);
            for(int m=0;m<K_SIZE;m++)
                for(int n=0;n<K_SIZE;n++)
                {
                    int p=i+m-center_x;
                    int q=j+n-center_y;
                    int m1,n1;
                    find_point(p,q,m1,n1);
                    int k2=(ctrl.width*n1+m1)*3;
                    for(int channel=0;channel<3;channel++)
                        new_val[channel]+=(ppm.pixels[k2+channel]*1.0*w[m][n]);                 
                }
            for(int channel=0;channel<3;channel++)
            {
                new_ppm.pixels[k+channel]=new_val[channel];

                if(new_val[channel]<0)
                    new_ppm.pixels[k+channel]=0;
                if(new_val[channel]>255)
                    new_ppm.pixels[k+channel]=255;
            }
        }
    
    return result;
}


// void morph_kernel(img &pic, int c_i, int c_j, int x_size, int y_size, double (&W_m)[int x_size=1][int y_size=1], int s)
// {
//     ;
// }


double morph( string method)    //diamond
{
    int x_size=1;
    int y_size=1;
    double result=0;
    for(int i=0;i<ctrl.width;i++)
        for(int j=0;j<ctrl.height;j++)
        {
            int k=(ctrl.width*j+i)*3;
            int R=ctrl.pixels[k];
            int G=ctrl.pixels[k+1];
            int B=ctrl.pixels[k+2];
            x_size=(R/51)*2+1;    //3-13
            y_size=(G/51)*2+1;
            int shape=B/86;//0-2
            //generate kernel
            double w[x_size][y_size]={};
            int center_x=x_size/2;
            int center_y=y_size/2;
            if(shape==0)
            {
                for(int m=0;m<x_size;m++)
                    for(int n=0;n<y_size;n++)
                    {
                        if(m==x_size/2 || n==y_size/2)
                            w[m][n]=1;
                    }
            }
            if(shape==1)
            {
                for(int m=0;m<x_size;m++)
                    for(int n=0;n<y_size;n++)
                    {
                        int x=abs(x_size/2-m);
                        int y=abs(y_size/2-n);
                        double h=y_size*1.0;
                        h/=x_size;
                        h*=(x_size/2.0-x*1.0);
                        if(y<=h)
                            w[m][n]=1;
                        //cout<<h<<endl;
                        
                    }               
            }
            if(shape==2)
            {
                for(int m=0;m<x_size;m++)
                    for(int n=0;n<y_size;n++)
                    {
                        int x=abs(x_size/2-m);
                        int y=abs(y_size/2-n);
                        if(x*x*1.0+y*y*1.0<=x_size*y_size*1.0/4)
                            w[m][n]=1;
                    }  
            }
            double new_val[3]={0,0,0};

            if(method=="erosion")   
                for(int m=0;m<3;m++)
                    new_val[m]=255;
            for(int m=0;m<x_size;m++)
                for(int n=0;n<y_size;n++)
                {
                    int p=i+m-center_x;
                    int q=j+n-center_y;
                    int m1,n1;
                    find_point(p,q,m1,n1);
                    int k2=(ppm.width*n1+m1)*3;
                    if(method=="erosion")
                        for(int channel=0;channel<3;channel++)
                        {
                            if(w[m][n]>0)
                                if((int)(ppm.pixels[k2+channel])<new_val[channel])
                                    new_val[channel]=(ppm.pixels[k2+channel]);   

                        }  
                    else
                        for(int channel=0;channel<3;channel++)
                        {
                            if(w[m][n]>0)
                                if((int)(ppm.pixels[k2+channel])>new_val[channel])
                                    new_val[channel]=(ppm.pixels[k2+channel]);   
                        }              
                }
            for(int channel=0;channel<3;channel++)
            {
                new_ppm.pixels[k+channel]=new_val[channel];

                // if(new_ppm.pixels[k+channel]<0)
                //     new_ppm.pixels[k+channel]=0;
                // if(new_ppm.pixels[k+channel]>255)
                //     new_ppm.pixels[k+channel]=255;
            }
        }
    
    return result;
}

void pnoise(img &pic)
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(-10, 10);
    for(int i=0;i<pic.width;i++)
        for(int j=0;j<pic.height;j++)
        {
            if(dis(gen)<-5)
            {
                int k=(pic.width*j+i)*3;
                
                for(int channel=0;channel<3;channel++)
                {
                    int r=dis(gen);
                    pic.pixels[k+channel]=(r+10)*20;
                    if(pic.pixels[k+channel]<0)
                        pic.pixels[k+channel]=0;
                    if(pic.pixels[k+channel]>255)
                        pic.pixels[k+channel]=255;
                }
            }
        }
}

void ppm_store(char * name,char * method)
{
  ofstream fout;
  char *s=name;
  sprintf(s,"%s_%s.ppm",name,method);
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
    
      fout.write(reinterpret_cast<char*>(&new_ppm.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&new_ppm.pixels[k++]),1);
      fout.write(reinterpret_cast<char*>(&new_ppm.pixels[k]),1);

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
    glDrawPixels(ppm.width, ppm.height, GL_RGB, GL_UNSIGNED_BYTE, new_ppm.pixels);
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
    if(argc!=4)
    {
        cout<<"usage: ./pr01 \'filename\' \'filename\' \'method\'"<<endl;
        return 0;
    }
    else
    {
        setPixels(argv[1],ppm);
        setPixels(argv[2],ctrl);
    }
    char *method=argv[3];
    double para=1;

    string md=method;
    if(md=="blur")
    {
        pnoise(ppm);
        motion_blur(1);
    }
    else
        morph(md);
    ppm_store(argv[1],method);
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
//Homework4 Filters Chengyi Min
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
img new_ppm;
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

 

            ppm=img(feat[1],feat[2],feat[3]);   //create img array
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
                    ppm.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    ppm.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    ppm.pixels[k]=(unsigned char)color;
                }
            
            cout<<ppm.width<<' '<<ppm.height<<' '<<ppm.depth<<' '<<ppm.width*ppm.height<<endl;
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

double gaussian(double x, double y, double stdev )
{
    double expo=(x*x+y*y)*1.0/(2*stdev*stdev);
    double deno=pow(M_E, -expo);
    
    double dom=2*M_PI*stdev*stdev*1.0;
    //cout<<deno/dom<<" ";
    return deno/dom;
}

double laplacian(double x, double y, double para,int a, int b)
{
    double result=0;
    //cout<<x<<" ";
    if(x>0)
    {
        result=1;
    }
    else
        result=-1;
    if(fabs(x)<0.0000001 || fabs(y)<0.0000001)
        result=0;
    else
        result=1;
    int base=a/2;
    if(fabs(x)<0.0000001 && fabs(y)<0.0000001)
        result=-(base*base*4);
    result/=2;
    //if(fabs(y)<0.0000001)
        //result*=1;
    return result;

}


double sobel(double x, double y, double para,int a, int b)
{
    double result=0;
    //cout<<x<<" ";
    if(x>0)
    {
        result=1;
    }
    else
        result=-1;
    if(fabs(x)<0.0000001 )
        result=0;
    int base=a/2;
    if(fabs(y)<0.0000001 )
        result*=(base+base);
    //if(fabs(y)<0.0000001)
        //result*=1;
    return result;

}

double erosion_filter(double x, double y, double para,int a, int b)    //diamond
{
    double result=0;
    //cout<<x<<" ";
    if((x*x+y*y)<=(a/2*a/2) )
        result=1;
    else
        result=0;
    return result;
}


double emboss(double x, double y, double para,int a, int b)    //diamond
{
    double result=0;
    //cout<<x<<" ";
    if(fabs(x-y)<0.000001)
        if(x<0)
            result=1;
        else
            if(fabs(x)>0.000001)
                result=-1;
    return result;
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

void morph(string method,int a, int b,double para)
{
    double matrix[a][b]={0};
    int center_x=a/2;
    int center_y=b/2;

        for(int i=0;i<a;i++)
            for(int j=0;j<b;j++)
            {
                double x=center_x-i*1.0;
                double y=center_y-j*1.0;
                //cout<<x<<","<<y<<endl;
                matrix[i][j]=erosion_filter(x,y,para,a,b);
            }


    for(int i=0;i<a;i++)
    {
        for(int j=0;j<b;j++)
        {
            cout<<matrix[i][j]<<"  ";          
        }
        cout<<endl;
    }
    for(int j=ppm.height-1;j>=0;j--)    //read pixel values
        for(int i=0;i<ppm.width;i++)
        {
            double new_val[3]={255,255,255};
            if (method=="dilation")
                for(int i=0;i<3;i++)
                    new_val[i]=0;
            int k=(ppm.width*j+i)*3;
            for(int m=0;m<a;m++)
                for(int n=0;n<b;n++)
                {
                    int p=i+m-center_x;
                    int q=j+n-center_y;
                    int m1,n1;
                    find_point(p,q,m1,n1);
                    int k2=(ppm.width*n1+m1)*3;
                    for(int channel=0;channel<3;channel++)
                    {
                        if(fabs(matrix[m][n])>0.000001)
                        {
                            if (method=="dilation")
                            {
                                if(new_val[channel]<int(ppm.pixels[k2+channel]*matrix[m][n]))
                                    new_val[channel]=ppm.pixels[k2+channel]; 
                            }
                            else
                                if(new_val[channel]>int(ppm.pixels[k2+channel]*matrix[m][n]))
                                    new_val[channel]=ppm.pixels[k2+channel];   
                        }  
                    }            
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

}



void convo(string method,int a, int b,double para)  //the size should be odd
{
    double matrix[a][b]={0};
    int center_x=a/2;
    int center_y=b/2;
    if(method == "blur")
    {
        double sum=0;
        for(int i=0;i<a;i++)
        {
            for(int j=0;j<b;j++)
            {
                double x=i*1.0-center_x;
                double y=j*1.0-center_y;
                matrix[i][j]=gaussian(x,y,para);
                sum+=matrix[i][j];
            }
            //cout<<endl;
        }
        //cout<<endl;
        for(int i=0;i<a;i++)
        {
            for(int j=0;j<b;j++)
            {
                matrix[i][j]/=sum;
                //cout<<matrix[i][j]<<" ";
                
            }
            //cout<<endl;
        }
    }

    if(method=="edge")
    {
        for(int i=0;i<a;i++)
            for(int j=0;j<b;j++)
            {
                double x=center_x-i*1.0;
                double y=center_y-j*1.0;
                //cout<<x<<","<<y<<endl;
                matrix[i][j]=sobel(x,y,para,a,b);
            }
    }
    if(method=="emboss")
    {
        for(int i=0;i<a;i++)
            for(int j=0;j<b;j++)
            {
                double x=center_x-i*1.0;
                double y=center_y-j*1.0;
                //cout<<x<<","<<y<<endl;
                matrix[i][j]=emboss(x,y,para,a,b);
            }
    }
    if(method=="laplacian")
    {
        for(int i=0;i<a;i++)
            for(int j=0;j<b;j++)
            {
                double x=center_x-i*1.0;
                double y=center_y-j*1.0;
                //cout<<x<<","<<y<<endl;
                matrix[i][j]=laplacian(x,y,para,a,b);
            }
    }

    for(int i=0;i<a;i++)
    {
        for(int j=0;j<b;j++)
        {
            cout<<matrix[i][j]<<"  ";          
        }
        cout<<endl;
    }
    for(int j=ppm.height-1;j>=0;j--)    //read pixel values
        for(int i=0;i<ppm.width;i++)
        {
            double new_val[3]={0,0,0};
            int k=(ppm.width*j+i)*3;
            for(int m=0;m<a;m++)
                for(int n=0;n<b;n++)
                {
                    int p=i+m-center_x;
                    int q=j+n-center_y;
                    int m1,n1;
                    find_point(p,q,m1,n1);
                    int k2=(ppm.width*n1+m1)*3;
                    for(int channel=0;channel<3;channel++)
                        new_val[channel]+=(ppm.pixels[k2+channel]*1.0*matrix[m][n]);                 
                }
            for(int channel=0;channel<3;channel++)
            {
                new_ppm.pixels[k+channel]=new_val[channel];

                if(new_val[channel]<0)
                    new_ppm.pixels[k+channel]=0;
                if(new_val[channel]>255)
                    new_ppm.pixels[k+channel]=255;
                // if(method=="edge")
                // {
                //     new_ppm.pixels[k+channel]+=ppm.pixels[k+channel];
                // if(new_ppm.pixels[k+channel]<0)
                //      new_ppm.pixels[k+channel]=0;
                //  if(new_ppm.pixels[k+channel]>255)
                //      new_ppm.pixels[k+channel]=255;
                //  }
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
        cout<<"usage: ./pr01 \'filename\' \'filter\' \'kernel size\'"<<endl;
        return 0;
    }
    else
        setPixels(argv[1]);
    char *method=argv[2];
    double para=1;
    int a=stoi(argv[3]);
    int b=stoi(argv[3]);
    string md=method;
    if(md=="dilation"||md=="erosion")
        morph(method,a,b,para);
    else
        convo(method,a,b,para);
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
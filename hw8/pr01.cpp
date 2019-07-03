//Homework8 Compositing
//ChengyiMin
#include <cstdlib>
#include <iostream>
#include <GL/glut.h>
//#include <png.h>
//#include <Magick++.h> 
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
#define K_SIZE 10
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
        pixels=new unsigned char[width*height*4];   //with alpha channel
        memset(pixels,0,width*height*4);
    }
    img(void)
    {
        width=height=depth=0;
    }
};



img ppm;
img new_ppm;
img edge;

string md;
char * filename;
char *method;
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
            //new_ppm=img(feat[1],feat[2],feat[3]);
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
                    int k=(width*j+i)*4;
                    char color;
                    ppm_file.read(&color,1);               
                    pic.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    pic.pixels[k++]=(unsigned char)color;
                    ppm_file.read(&color,1);
                    pic.pixels[k++]=(unsigned char)color;
                    pic.pixels[k]=255;
                }

        }   
}

void multiply(img &pic1, img &pic2)
{
    for(int j=pic1.height-1;j>=0;j--)
        for(int i=0;i<pic1.width;i++)
        {
            int k=(pic1.width*j+i)*4;
            for(int c=0;c<4;c++)
            {
                double a =pic1.pixels[k+c]/255.0;
                double b =pic2.pixels[k+c]/255.0;
                pic2.pixels[k+c]=a*b*255.0;
            }
        }
}

void maxpix(img &pic1, img &pic2)
{
    for(int j=pic1.height-1;j>=0;j--)
        for(int i=0;i<pic1.width;i++)
        {
            int k=(pic1.width*j+i)*4;
            for(int c=0;c<4;c++)
            {
                pic2.pixels[k+c]=max(pic1.pixels[k+c],pic2.pixels[k+c]);
            }
        }
}


void minpix(img &pic1, img &pic2)
{
    for(int j=pic1.height-1;j>=0;j--)
        for(int i=0;i<pic1.width;i++)
        {
            int k=(pic1.width*j+i)*4;
            for(int c=0;c<4;c++)
            {
                pic2.pixels[k+c]=min(pic1.pixels[k+c],pic2.pixels[k+c]);
            }
        }
}

void subtraction(img &pic1, img &pic2)
{
    for(int j=pic1.height-1;j>=0;j--)
        for(int i=0;i<pic1.width;i++)
        {
            int k=(pic1.width*j+i)*4;
            for(int c=0;c<3;c++)
            {
                double result=pic1.pixels[k+c]-pic2.pixels[k+c];
                if(result<0)
                    result=0;
                pic2.pixels[k+c]=result;
            }
        }
}


void over(img &pic1, img &pic2)
{
    for(int j=pic2.height-1;j>=0;j--)
        for(int i=0;i<pic2.width;i++)
        {
            int k=(pic2.width*j+i)*4;
            int p=i-pic2.width/2+pic1.width/2;
            int q=j;//-pic2.height/2+pic1.height/2;     //grounded
            int k2=(pic1.width*q+p)*4;
            for(int c=0;c<3;c++)
            {              
                double background=pic2.pixels[k+c];
                double front=0;
                double alpha=0;
                if(p>=0 && p<pic1.width && q>0 && q<pic1.height)
                {
                    front=pic1.pixels[k2+c];
                    alpha=pic1.pixels[k2+3]/255.0;
                }
                pic2.pixels[k+c]=(front-background)*(alpha)+background;
            }
        }

}

void globalAlphaSet(img &pic,int alpha)
{
    for(int j=pic.height-1;j>=0;j--)
        for(int i=0;i<pic.width;i++)
        {
            int k=(pic.width*j+i)*4;
            pic.pixels[k+3]=alpha;
        }
}

void find_point(int x,int y, int &a,int &b)
{
    a=x;
    b=y;
    if(a<0)
        a=0;
    if(b<0)
        b=0;
    if(a>=ppm.width)
        a=ppm.width-1;
    if(b>=ppm.height)
        b=ppm.height-1;
}

void greenScreenAlpha(img & pic1, img & pic2, img & edge, int ksize , int interval)
{
    if(ksize<3)
        ksize=K_SIZE;
    for(int j=pic1.height-1;j>=0;j--)
        for(int i=0;i<pic1.width;i++)
        {
            int k=(pic1.width*j+i)*4;
            double cf[3]={0},cb[3]={0},co[3]={0};
            double a_result=0;
            double range=0;
            double b_count=0,f_count=0;
            if(edge.pixels[k]>0 && edge.pixels[k]<255 )
            {
                for(int p=0;p<2*ksize+1;p++)
                    for(int q=0;q<2*ksize+1;q++)
                    {
                        int p1=i+p-ksize,q1=j+q-ksize;
                        find_point(i+p-ksize,j+q-ksize,p1,q1);
                        int k2=(pic1.width*q1+p1)*4;
                        for(int channel=0;channel<3;channel++)
                        {
                            cb[channel]+=(double)pic1.pixels[k2+channel]*(edge.pixels[k2]==0);
                            cf[channel]+=(double)pic1.pixels[k2+channel]*(edge.pixels[k2]==255);
                        }
                        b_count+=(edge.pixels[k2]==0);
                        f_count+=(edge.pixels[k2]==255);
                    }
                for(int channel=0;channel<3;channel++)
                {
                    co[channel]=pic1.pixels[k+channel];
                    cb[channel]=cb[channel]/(b_count*1.0);
                    cf[channel]=cf[channel]/(f_count*1.0);
                    if( f_count==0)          // background
                    {
                        a_result=0;                    
                        cf[channel]=0;
                    }
                    if(b_count==0)
                    {
                        a_result=1;
                        cb[channel]=0;
                    }
                    if(f_count!=0 && b_count!=0)
                    {
                    double a_channel=(co[channel]-cb[channel])*1.0/(cf[channel]-cb[channel]);

                    //if(a_channel>0 && a_channel<1)
                        if(fabs(cf[channel]-cb[channel])>range)
                        {
                            range=fabs(cf[channel]-cb[channel]);
                            a_result=a_channel;
                        }
                    }
                }
            }

            if(a_result<0)
                a_result=0;
            if(a_result>1)
                a_result=1;
            int back_flag=1;
            if(edge.pixels[k]>0 && edge.pixels[k]<255  )
            {
                for(int channel=0;channel<3;channel++)
                {
                    if(fabs((double)pic1.pixels[k+channel]-cb[channel])>=interval)
                        back_flag=0;
                }
                if(back_flag)
                {
                    for(int channel=0;channel<3;channel++)
                    {
                    pic1.pixels[k+channel]=cf[channel];
                    }
                }
                else
                    a_result=1;
            }
            if(edge.pixels[k]==255)
                a_result=1;
            if(edge.pixels[k]==0)
                a_result=0;
            pic1.pixels[k+3]=255*a_result;
        }
    over(pic1, pic2);

}
constexpr unsigned int str2int(const char* str, int h=0)
{
    return !str[h] ? 5381 :(str2int(str,h+1)*33)^str[h];
}

void composite(img &pic1, img &pic2, string md, int ksize , int interval)
{
    const char *m=md.c_str();
    char ef[50]="edge.ppm";
    switch (str2int(m))
    {
        case str2int("multiply"):
            multiply(ppm,new_ppm);
            break;
        case str2int("max"):
            maxpix(ppm,new_ppm);
            break;
        case str2int("min"):
            minpix(ppm,new_ppm);
            break;
        case str2int("over"):
            globalAlphaSet(ppm,180);
            over(ppm,new_ppm);
            break;
        case str2int("subtraction"):
            subtraction(ppm,new_ppm);
            break;     
        case str2int("greenscreen"):
            setPixels(ef, edge);
            greenScreenAlpha(ppm,new_ppm,edge,ksize , interval);
            break;
        default:
            break;

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
  //cout<<pic.width<<" "<<pic.height<<endl;
  fout<<"255";
  char value=10;
  fout.write(reinterpret_cast<char*>(&value),1);   //end of magic 
  for(int j=pic.height-1;j>=0;j--)
  {
    for(int i=0;i<pic.width;i++)
    {
      int k=(pic.width*j+i)*4;
    
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
    glDrawPixels(new_ppm.width, new_ppm.height, GL_RGBA, GL_UNSIGNED_BYTE, new_ppm.pixels);
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
    int argv_num=4;
    int ksize=0;
    int interval=0;
    if(argc<4)
    {
        cout<<"usage: ./pr01 \'filename\' \'filename\' \'method\' "<<endl;
        return 0;
    }
    string isGreenscreen=argv[3];
    //initialize the global variables
    if(isGreenscreen=="greenscreen")
    {
        argv_num=6;
        if(argc!=argv_num)
        {
            cout<<"usage: ./pr01 \'filename\' \'filename\' \'greenscreen\' \'parameter1\' \'parameter2\'"<<endl;
            return 0;
        }
        else
        {
            setPixels(argv[1],ppm);
            setPixels(argv[2],new_ppm);
            ksize=atof(argv[4]);
            interval=atof(argv[5]);
        }
    }
    else
        if(argc!=argv_num)
        {
            cout<<"usage: ./pr01 \'filename\' \'filename\' \'method\' "<<endl;
            return 0;
        }
        else
        {
            setPixels(argv[1],ppm);
            setPixels(argv[2],new_ppm);
        }
    method=argv[3];
    
    md=method;
    composite(ppm,new_ppm,md,ksize,interval);
    double para1 =1;
    double para2 =1;
    //cout<<"para1: "<<para1<<endl;
    //cout<<"para2: "<<para2<<endl;
    filename=argv[1];
    ppm_store(argv[1],method,new_ppm);
    // OpenGL Commands:
    // Once "glutMainLoop" is executed, the program loops indefinitely to all
    // glut functions.  
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100); // Where the window will display on-screen.
    glutInitWindowSize(new_ppm.width, new_ppm.height);
    glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
    glutCreateWindow("Homework Zero");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    glutMouseFunc(processMouse);
    glutMainLoop();    
    return 0; //This line never gets reached. We use it because "main" is type int.
}
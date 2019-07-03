//Homework3 Hue Swap Chengyi Min
#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <cstring>
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

img origin;
img hsv;
double rm[3][100];
double h[3][100];
int len[3];

// =============================================================================
// setPixels()
//
// This function stores the RGB values of each pixel to "pixmap."
// Then, "glutDisplayFunc" below will use pixmap to display the pixel colors.
// =============================================================================
#define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z))) 
#define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z)))

void RGBtoHSV(int r, int g, int b, double &h, double &s, double &v)
{
  double red, green, blue; 
  double max, min, delta;

  red = r / 255.0; green = g / 255.0; blue = b / 255.0;  /* r, g, b to 0 - 1 scale */

  max = maximum(red, green, blue); 
  min = minimum(red, green, blue);

  v = max;        /* value is maximum of r, g, b */

  if(max == 0){    /* saturation and hue 0 if value is 0 */ 
    s = 0; 
    h = 0; 
  } 
  else{ 
    s = (max - min) / max;           /* saturation is color purity on scale 0 - 1 */

    delta = max - min; 
    if(delta == 0)                    /* hue doesn't matter if saturation is 0 */ 
      h = 0; 
    else
    { 
      if(red == max)                  /* otherwise, determine hue on scale 0 - 360 */ 
        h = (green - blue) / delta; 
      else if(green == max) 
        h = 2.0 + (blue - red) / delta; 
      else /* (blue == max) */ 
        h = 4.0 + (red - green) / delta; 
      h = h * 60.0; 
      if(h < 0) 
        h = h + 360.0; 
    } 
  } 
}

void HSVtoRGB(int &r, int &g, int &b, double h, double s, double v)
{
    double c =v*s;
    double x= c*(1-fabs(fmod(h/60.0,2)-1));
    double m= v-c;
    double rf,gf,bf;
    if(h>=0 && h<60)
    {
        rf=c;gf=x;bf=0;
    }
    else if(h<120)
    {
        rf=x;gf=c;bf=0;
    }
    else if (h<180)
    {
        rf=0;gf=c;bf=x;
    }
    else if (h<240)
    {
        rf=0;gf=x;bf=c;
    }
    else if (h<300)
    {
        rf=x;gf=0;bf=c;
    }
    else if(h<360)
    {
        rf=c;gf=0;bf=x;
    }
    r=(rf+m)*255.0;
    g=(gf+m)*255.0;
    b=(bf+m)*255.0;
}

void setPixels(char* fname, img &pic)
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

 

            pic=img(feat[1],feat[2],feat[3]);   //create img array
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


void hue_swap(char * ppm_name, char * base)
{
    setPixels(ppm_name,origin);
    setPixels(base,hsv);
    for(int j=origin.height-1;j>=0;j--)    //read pixel values
        for(int i=0;i<origin.width;i++)
        {
            
            double h_o,h_b,s_o,s_b,v_o,v_b;
            int k=(origin.width*j+i)*3;
            int r=origin.pixels[k],g=origin.pixels[k+1],b=origin.pixels[k+2];
            char color;             
            RGBtoHSV(origin.pixels[k],origin.pixels[k+1],origin.pixels[k+2],h_o,s_o,v_o);
            RGBtoHSV(hsv.pixels[k],hsv.pixels[k+1],hsv.pixels[k+2],h_b,s_b,v_b);
            HSVtoRGB(r,g,b,h_b,s_o,v_o);
            origin.pixels[k]=r;
            origin.pixels[k+1]=g;
            origin.pixels[k+2]=b;
        }
}

void ppm_store(img shape)
{
  ofstream fout;
  fout.open("procedural_result.ppm",ios::binary | ios::out);
  fout<<"P6"<<endl;
  fout<<shape.width<<" "<<shape.height<<endl;
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
    glDrawPixels(origin.width, origin.height, GL_RGB, GL_UNSIGNED_BYTE, origin.pixels);
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
    if(argc!=3)
    {
        cout<<"usage: ./pr01 \'filename\' \'base_image\'"<<endl;
        return 0;
    }
    else
        hue_swap(argv[1],argv[2]);
    ppm_store(origin);
   // pinput();
    //manipulate();
    // OpenGL Commands:
    // Once "glutMainLoop" is executed, the program loops indefinitely to all
    // glut functions.  
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100); // Where the window will display on-screen.
    glutInitWindowSize(origin.width, origin.height);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Homework Zero");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    glutMouseFunc(processMouse);
    glutMainLoop();    
    return 0; //This line never gets reached. We use it because "main" is type int.
}
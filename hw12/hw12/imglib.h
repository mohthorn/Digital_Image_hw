#include <cstdlib>
#include <iostream>
#include <GL/glut.h>

#include <fstream>
#include <cassert>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cmath>

using namespace std; 


class img{
public:
    int width;
    int height;
    int depth;
    unsigned char *pixels;
    int **energy;
    int **path;
    unsigned char *alphas;

    img(int width,int height,int depth)
    {
        this->width=width;
        this->height=height;
        this->depth=depth;
        pixels=new unsigned char[width*height*3];
        
        energy =new int *[width];
        for(int i=0;i<width;i++)
        {
            energy[i]=new int[height];
        }
        path =new int *[width];
        for(int i=0;i<width;i++)
        {
            path[i]=new int[height];
        }
        for(int i=0;i<width;i++)
            for(int j=0;j<height;j++)
            {
                energy[i][j]=0;
                path[i][j]=0;
            }
        memset(pixels,0,width*height*3);
        alphas = new unsigned char[width*height];
        memset(alphas,255,width*height);
    }
    img(void)
    {
        width=height=depth=0;
    }

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

       

                this->width=feat[1];
                this->height=feat[2];   
                this->depth=feat[3];
                //generate energy
                energy =new int *[width];
                for(int i=0;i<width;i++)
                {
                    energy[i]=new int[height];
                }
                path =new int *[width];
                for(int i=0;i<width;i++)
                {
                    path[i]=new int[height];
                }
                for(int i=0;i<width;i++)
                    for(int j=0;j<height;j++)
                    {
                        energy[i][j]=0;
                        path[i][j]=0;
                    }
                this->pixels=new unsigned char[this->width*this->height*3];
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
                        this->pixels[k++]=(unsigned char)color;
                        ppm_file.read(&color,1);
                        this->pixels[k++]=(unsigned char)color;
                        ppm_file.read(&color,1);
                        this->pixels[k]=(unsigned char)color;
                    }
                alphas = new unsigned char[width*height];
                memset(alphas,255,width*height);
                //cout<<pic.width<<' '<<pic.height<<' '<<pic.depth<<' '<<pic.width*pic.height<<endl;
            }  
    } 

    void ppm_store(char * name,const char * method)
    {
        ofstream fout;
        char *s=name;
        sprintf(s,"%s_%s.ppm",name,method);
        fout.open(s,ios::binary | ios::out);
        fout<<"P6"<<endl;
        fout<<this->width<<" "<<this->height<<endl;
        //cout<<pic.width<<" "<<pic.height<<endl;
        fout<<"255";
        char value=10;
        fout.write(reinterpret_cast<char*>(&value),1);   //end of magic 
        for(int j=this->height-1;j>=0;j--)
        {
            for(int i=0;i<this->width;i++)
            {
              int k=(this->width*j+i)*3;
              fout.write(reinterpret_cast<char*>(&this->pixels[k++]),1);
              fout.write(reinterpret_cast<char*>(&this->pixels[k++]),1);
              fout.write(reinterpret_cast<char*>(&this->pixels[k]),1);
            }
        }
    }
    //copy function
    img(const img &pic)
    {
        width=pic.width;
        height=pic.height;
        depth=pic.depth;
        for(int i=0;i<width*height*3;i++)
        {
            pixels[i]=pic.pixels[i];
        }
        for(int i=0;i<width*height;i++)
        {
            alphas[i]=pic.alphas[i];
        }
        for(int i=0;i<width;i++)
        {
            for(int j=0;j<height;j++)
            {
                energy[i][j]=pic.energy[i][j];
                //cout<<pic.energy[i][j]<<endl;
                path[i][j]=pic.path[i][j];
            }
        }
    
    }

    void find_point(int x,int y, int &a,int &b,const img &pic)
    {
        a=x;
        b=y;
        if(a<0)
            a=0;
        if(b<0)
            b=0;
        if(a>=pic.width)
            a=pic.width-1;
        if(b>=pic.height)
            b=pic.height-1;
    }

    img lighting(int up)
    {
        img result=img(width,height,depth);
        for(int j=height-1;j>=0;j--)
        {
            for(int i=0;i<width;i++)
            {
                int k=(width*j+i)*3;
                for(int channel=0;channel<3;channel++)
                {
                    double res=double(pixels[k+channel])+up;
                    if(res>255)
                        res=255;
                    if(res<0)
                        res=0;
                    result.pixels[k+channel]=res;
                }
            }
        }
        return result;
    }



    void diff(img &pic1)
    {
        for(int j=pic1.height-1;j>=0;j--)
            for(int i=0;i<pic1.width;i++)
            {
                int k=(pic1.width*j+i)*3;
                for(int c=0;c<3;c++)
                {
                    double result=pic1.pixels[k+c]-pixels[k+c];
                    if(result<0)
                        result=0;
                    if(result>255)
                        result=255;
                    pixels[k+c]=result;
                }
            }
    }


    double dotProduct(double v1[3], double v2[3])
    {
        double dot1 = v1[0] * v2[0];
        double dot2 = v1[1] * v2[1];
        double dot3 = v1[2] * v2[2];
        return dot1+dot2+dot3;
    }

    double* mPlus(double v1[3],double v2[3])
    {
        double *result=new double[3];
        result[0]=v1[0]+v2[0];
        result[1]=v1[1]+v2[1];
        result[2]=v1[2]+v2[2];
        return result;
    }

    double* Product(double a, double v1[3])
    {
        double *result=new double[3];
        result[0]=v1[0]*a;
        result[1]=v1[1]*a;
        result[2]=v1[2]*a;
        return result;
    }

    double *norm(double v[3])
    {
        double *result=new double[3];
        double sum=0;
        for(int i=0;i<3;i++)
        {
            sum+=(v[i]*v[i]);
        }
        sum=sqrt(sum);
        result[0]=v[0]/sum;
        result[1]=v[1]/sum;
        result[2]=v[2]/sum;
        return result;
    }


    double callShadow(img &depth, int i, int j, double xl=0, double yl=0 , double zl=0)
    {
        //mock 3D
        double eps=0.01;
        double diff_x=xl-i;
        double diff_y=yl-j;
        int nsh=1;
        int steps = (int)max(diff_x,diff_y);
        int p=0;
 

        int x=i;
        int y=j;
        int depth_po=(depth.width*y+x)*3;
        double d=depth.pixels[depth_po];
        double z=d;
        int counter =0;
        while(depth.pixels[depth_po]<z && z<zl) 
        {
            x= i+p*(xl-i)*1.0/(double)steps;
            y= j+p*(yl-i)*1.0/(double)steps;
            z=d+p*(zl-d)*1.0/(double)steps;
            depth_po=(depth.width*y+x)*3;
            if(depth_po>width*height*3-1)
                depth_po=width*height*3-1;
            if(depth_po<0)
                depth_po=0;
        }
        if(fabs(x-xl)<eps && fabs(y-yl)<eps)
            nsh=1;
        else
            nsh=0;
        return nsh;
    }

    void diffuse(img &cl, img &cd, img &cn, img&cs, img& depth, int m_x=0, int m_y=0)
    {
        double l[3]={-(double)m_x,-(double)m_y,-(double)width};

        double e[3]={0,0,1};

        for(int j=height-1;j>=0;j--)
        {
            for(int i=0;i<width;i++)
            {
                int k=(width*j+i)*3;
                double x=2*double(cn.pixels[k]/255.0)-1;
                double y=2*double(cn.pixels[k+1]/255.0)-1;
                double z=double(cn.pixels[k+2]/255.0);

                double norm_v[3]={x,y,z};
                double *n = new double[3];
                n=norm(norm_v);
                double *new_l=new double[3];
                double cpoint[3]={(double)i,(double)j,0};

        
                new_l=mPlus(cpoint,l);

                new_l=norm(new_l);

                double *r=new double[3];
                double alpha =0.5*dotProduct(n,new_l)+0.5;
                alpha =callShadow(depth,i,j,m_x,m_y,width)*alpha;
                r=mPlus(Product(-1,new_l),Product(2*dotProduct(n,new_l),n)); 
                double s=0.5*r[2]+0.5;
                for(int channel=0;channel<3;channel++)
                {
                    double bright = cl.pixels[k+channel];
                    double dark = cd.pixels[k+channel];
                    double spec =cs.pixels[k+channel];
                   // cout<<alpha<<endl;
                    double res=(bright-dark)*(alpha)+dark;
                    if(res>255)
                        res=255;
                    if(res<0)
                        res=0;


                    res=(res-spec)*(1-0.5*s)+spec;

                    pixels[k+channel]=res;
                    // if(fabs(i-m_x)<10 && fabs(j-m_y)<10)
                    //     pixels[k+channel]=255;
                }
            }
        }
    }
};












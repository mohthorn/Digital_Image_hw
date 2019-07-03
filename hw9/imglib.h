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
    }

    void find_point(int x,int y, int &a,int &b,const img &pic)
    {
        a=x;
        b=y;
        if(a<0)
            a=-a;
        if(b<0)
            b=-b;
        if(a>=pic.width)
            a=2*(pic.width-1)-a;
        if(b>=pic.height)
            b=2*(pic.height-1)-b;
    }



    img floydDithering(const img&pic,int para)
    {
        width=pic.width;
        height=pic.height;
        depth=pic.depth;
        int size=pic.width*pic.height*3;
        int divide=1;
        double mul=1;
        if(para>1)
        {
            divide=256/(para-1);
            mul = 255.0/(para-1);
        }
        float mat[size];
        int directions[4][2]={{1,0},{-1,1},{0,1},{1,1}};
        double res[4]={7.0/16.0,3.0/16.0,5.0/16.0,1.0/16.0};
        for(int i=0;i<width*height*3;i++)
        {
            mat[i]=pic.pixels[i];
        }       
        for(int j=height-1;j>=0;j--)
        {
            for(int i=0;i<width;i++)
            {
                int k=(width*j+i)*3;
                for(int channel=0;channel<3;channel++)
                {
                    double oldpixel = mat[k+channel];
                    double newpixel =floor(oldpixel*1.0/(double)divide+0.5)*mul;
                    mat[k+channel]=newpixel;
                    
                    //cout<<(int)(mat[k+channel])<<endl;
                    double qerror = oldpixel-newpixel;

                    for(int p=0;p<4;p++)
                    {
                        int new_i=0,new_j=0;
                        find_point(i+directions[p][0],j+directions[p][1],new_i,new_j,pic);
                        int k2=(width*new_j+new_i)*3;
                        mat[k2+channel]+=qerror*1.0*res[p];
                        //cout<<mat[k2+channel]<<endl;

                    }
                }
            }
        }
        for(int i=0;i<width*height*3;i++)
        {
            if(mat[i]>255)
                mat[i]=255;
            if(mat[i]<0)
                mat[i]=0;
            pixels[i]=floor(mat[i]/(double)divide+0.5)*mul;
        }

    }

    img orderedDithering(const img&pic , int para, int dimension)
    {
        width=pic.width;
        height=pic.height;
        depth=pic.depth;
        int ditherMapSize=dimension;
        int divide=1;
        double mul=1;
        if(para>1)
        {
            divide=256/(para-1);
            mul = 255.0/(para-1);
        }
        int ditherBase[2][2]={{0,2},
                              {3,1},}  ;
        int ditherMap[ditherMapSize][ditherMapSize]={0};
        int ditherPrev[ditherMapSize][ditherMapSize]={0};
        for(int i =0;i<2;i++)
            for(int j=0;j<2;j++)
            {
                ditherMap[i][j]=ditherBase[i][j];
            }
        cout<<endl;
        
        for(int size = 4; size<=ditherMapSize; size*=2)
        {
            int times=size/2;
            memcpy(ditherPrev,ditherMap, ditherMapSize*ditherMapSize*sizeof(int));
        for(int i=0;i<ditherMapSize;i++)
        {
            // for(int j=0;j<ditherMapSize;j++)
            //     cout<< ditherPrev[i][j]<<" ";
            // cout<<endl;
        }
        cout<<endl;
            for(int i=0;i<size;i++)
                for(int j=0;j<size;j++)
                {
                    ditherMap[i][j]=4*ditherPrev[i%times][j%times]+ditherBase[i/times][j/times];
                }
        }
        for(int i=0;i<ditherMapSize;i++)
        {
            for(int j=0;j<ditherMapSize;j++)
                cout<< ditherMap[i][j]<<" ";
            cout<<endl;
        }
        for(int j=height-1;j>=0;j--)
        {
            for(int i=0;i<width;i++)
            {
                int k=(width*j+i)*3;
                for(int channel=0;channel<3;channel++)
                {
                    double qerror=pixels[k+channel]*1.0/divide-floor(pixels[k+channel]*1.0/divide);
                    double result;
                    if(qerror>ditherMap[i%ditherMapSize][j%ditherMapSize]/(double)(ditherMapSize*ditherMapSize))
                        result=ceil(pixels[k+channel]*1.0/divide)*mul;
                    else
                        result=floor(pixels[k+channel]*1.0/divide)*mul;
                    if(result>255)
                        result=255;
                    if(result<0)
                        result=0;
                    pixels[k+channel]=result;
                }
            }
        }

    }
};












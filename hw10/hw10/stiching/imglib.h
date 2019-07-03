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

    double energyGain(int i, int j, int p_i, int p_j)
    {
        int k=(this->width*j+i)*3;
        // int p_k=(this->width*p_j+p_i)*3;
        // double sum_y=0;
        // double sum_x=0;
        double result=0;
        // double max=0;
        // int m[3][3]={{1,2,1},
        //             {0,0,0},
        //             {-1,-2,-1},
        //             };
        // int n[3][3]={{1,0,-1},
        //             {2,0,-2},
        //             {1,0,-1},
        //             };
        // for(int i_k=0;i_k<3;i_k++)
        // {
        //     for(int j_k=0;j_k<3;j_k++ )
        //     {
        //         int i_real,j_real;
        //         find_point(p_i+i_k-1,p_j+j_k-1,i_real,j_real,*this);
        //         int k=(this->width*j_real+i_real)*3;
        //         //int p_k=(this->width*p_j+p_i)*3;
        //         for(int channels=0;channels<3;channels++)
        //         {
        //             double dist=m[i_k][j_k]*1.0*this->pixels[k+channels];
        //             sum_y+=dist;
        //         }  
        //     }      
        // }  
 
        // result=sqrt(sum_x*sum_x+sum_y*sum_y);
        //cout<<sum<<endl;
        for(int channel=0; channel<3;channel++)
        {
            result+=pixels[k+channel];
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

    void findSeam(int*boundary,int min=1)
    {
        int i_min=min;
        //cout<<pic.path[1][pic.height-1]<<endl;
        for(int j=height-1;j>=0;j--)
        {
            boundary[j]=i_min;
            i_min=path[i_min][j];
        }
    }

    int minEnergy(int start, int finish)
    {
        int min_index=0;
        int min_total_energy=10e8;
        //int new_width=finish;
        int prev[3]={-1,0,1};
        for(int i=start;i<finish;i++)
        {
            energy[i][0]=0;
            path[i][0]=0;             
        }
            
        for(int j=1;j<height;j++)
        {
            for(int i=start;i<finish;i++)
            {
                int prev_i;
                double min_energy=10e8;
                for(int p=0;p<3;p++)
                {
                    int i_p=i+prev[p];
                    if(i_p<start)
                        i_p=start;
                    if(i_p>=finish)
                        i_p=finish-1;
                    double energy_gain=energyGain(i,j,i_p,j-1);
                    double total_energy=energy_gain+energy[i_p][j-1];
                    if(total_energy<min_energy)
                    {
                        min_energy=total_energy;
                        prev_i=i_p;
                    }
                }
                energy[i][j]=min_energy;
                path[i][j]=prev_i;                 
            }
        }
        for(int i=start;i<finish;i++)
        {
            //cout<<i<<":"<<result.energy[i][result.height-1]<<endl;
            if(energy[i][height-1]<min_total_energy)
            {
                min_index=i;
                //cout<<i<<":"<<result.energy[i][result.height-1]<<endl;
                min_total_energy=energy[i][height-1];
            }
        }  
        return min_index;     
    }

    void stitch(img &pic1, img &pic2,int start, int finish)
    {
        int i_min=minEnergy(start, finish);
        int *boundary=new int[height];
        memset(boundary, 0, height);
        findSeam(boundary,i_min);
        int kernel[3][3]={{1,1,1},
                    {1,3,1},
                    {1,1,1}
                    };
        for(int j=height-1;j>=0;j--)
        {
            for(int i=0;i<width;i++)
            {
                img *foreground_pic=&pic1;
                int k=(width*j+i)*3;
                if(i>boundary[j])
                {
                    foreground_pic=&pic2;
                }
                for(int channel=0;channel<3;channel++)
                {
                    pixels[k+channel]=foreground_pic->pixels[k+channel];
                }
            }
        }
        for(int j=height-1;j>=0;j--)
        {
            for(int i=0;i<width;i++)
            {
                if(abs(i-boundary[j])<2)
                {
                    int k=(width*j+i)*3;
                    double sum[3]={0};
                    for(int p=0;p<3;p++)
                        for(int q=0;q<3;q++)
                        {
                            int i_real=0;
                            int j_real=0;
                            find_point(i+p-1,j+q-1,i_real,j_real,*this);
                            int k2=(this->width*j_real+i_real)*3;
                            for(int channel=0;channel<3;channel++)
                            {
                                sum[channel]+=kernel[p][q]*pixels[k2+channel]/11.0;
                            }
                        }
                    for(int channel=0;channel<3;channel++)
                    {
                        pixels[k+channel]=sum[channel];
                    }
                }
                
            }
        }
    }
};












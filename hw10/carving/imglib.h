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
        int p_k=(this->width*p_j+p_i)*3;
        double sum_y=0;
        double sum_x=0;
        double result=0;
        double max=0;
        int m[3][3]={{1,2,1},
                    {0,0,0},
                    {-1,-2,-1},
                    };
        int n[3][3]={{1,0,-1},
                    {2,0,-2},
                    {1,0,-1},
                    };
        for(int i_k=0;i_k<3;i_k++)
        {
            for(int j_k=0;j_k<3;j_k++ )
            {
                int i_real,j_real;
                find_point(p_i+i_k-1,p_j+j_k-1,i_real,j_real,*this);
                int k=(this->width*j_real+i_real)*3;
                //int p_k=(this->width*p_j+p_i)*3;
                for(int channels=0;channels<3;channels++)
                {
                    double dist=m[i_k][j_k]*1.0*this->pixels[k+channels];
                    sum_y+=dist;
                }  
            }      
        }  
 
        result=sqrt(sum_x*sum_x+sum_y*sum_y);
        //cout<<sum<<endl;
        return result;
    }


    img removeASeam(const img&pic,int direction=1,int min=1)
    {
        int new_width=pic.width-1;
        img result=img(new_width,pic.height,pic.depth);
        int i_min=min;
        int energ[pic.width][pic.height]={0};
        int path[pic.width][pic.height]={0};
        //cout<<pic.path[1][pic.height-1]<<endl;
        for(int j=pic.height-1;j>=1;j--)
        {
            energy[i_min][j]=-1;
            i_min=pic.path[i_min][j];
        }
        pic.energy[i_min][0]=-1;
        for(int j=pic.height-1;j>=0;j--)
        {
            int i_r=0;
            for(int i=0;i<pic.width;i++)
            {
                int k=(pic.width*j+i)*3;
                if(pic.energy[i][j]!=-1)
                {
                    int k2=(new_width*j+i_r)*3;
                    for(int channel=0;channel<3;channel++)
                    {
                        result.pixels[k2+channel]=pic.pixels[k+channel];
                    }
                    i_r+=1;
                }
            }
        }   
        return result;    
    }

    img seamCarving(const img&pic, int direction=1, int para=0)
    {
        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<> dis(0, 1);

        width=pic.width;
        height=pic.height;
        depth=pic.depth;
        img result=pic;
        result=pic;
        for(int epoc=0;epoc<para;epoc++)
        {
            int min_index=0;
            int min_total_energy=10e8;
            int new_width=result.width;
            int prev[3]={-1,0,1};
            // if(dis(gen)<0.5)
            //     for(int i=0;i<3;i++)
            //         prev[i]=-prev[i];
            for(int i=0;i<new_width;i++)
            {
                result.energy[i][0]=0;
                result.path[i][0]=0;             
            }
            
            for(int j=1;j<result.height;j++)
            {
                for(int i=0;i<new_width;i++)
                {
                    //int k=(new_width*j+i)*3;
                    int prev_i;
                    double min_energy=10e8;
                    for(int p=0;p<3;p++)
                    {
                        int i_p=i+prev[p];
                        if(i_p<0)
                            i_p=0;
                        if(i_p>=new_width)
                            i_p=new_width-1;
                        double energy_gain=result.energyGain(i,j,i_p,j-1);
                        double total_energy=energy_gain+result.energy[i_p][j-1];
                        if(total_energy<min_energy)
                        {
                            min_energy=total_energy;
                            prev_i=i_p;
                        }
                    }
                    result.energy[i][j]=min_energy;
                    result.path[i][j]=prev_i;
                    //cout<<result.path[i][j]<<endl;
                }
            }
            for(int i=0;i<result.width;i++)
            {
                //cout<<i<<":"<<result.energy[i][result.height-1]<<endl;
                if(result.energy[i][result.height-1]<min_total_energy)
                {
                    min_index=i;
                    //cout<<i<<":"<<result.energy[i][result.height-1]<<endl;
                    min_total_energy=result.energy[i][result.height-1];
                }
            }
            cout<<"epoc: "<<epoc<<endl;
            //cout<<min_index<<endl;
            //cout<<min_total_energy<<endl;
            result=result.removeASeam(result,direction,min_index);
        }


        return result;
    }
};












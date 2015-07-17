//#################################################################//
#include "Bmp.h"
//#include "error.h"
#include <IL/devil_cpp_wrapper.hpp>
//#################################################################//
void init_ilu_lib()
{
	static bool ini=false;

	if(!ini)
	{
		ilInit();
		iluInit();
		ini=true;
	}
}
//#################################################################//
Bmp::Bmp()
{
	init_ilu_lib();
	width=height=0;
	data=NULL;
}
//#################################################################//
Bmp::Bmp(const char*filename, bool convert32)
{
	init_ilu_lib();
	width=height=0;
	data=NULL;
	Load(filename,convert32);
}
//#################################################################//
Bmp::Bmp(int x,int y,int b,unsigned char*buffer)
{
	width=height=0;
	data=NULL;
	Set(x,y,b,buffer);
}
//#################################################################//
Bmp::~Bmp()
{
	if (data) free(data);
}
//#################################################################//
void Bmp::Save(const char*filename)
{
	unsigned char bmp[58]=
			{0x42,0x4D,0x36,0x30,0,0,0,0,0,0,0x36,0,0,0,0x28,0,0,0,
	           	0x40,0,0,0, // X-Size
	           	0x40,0,0,0, // Y-Size
                   	1,0,0x18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	bmp[18]	=width;
	bmp[19]	=width>>8;
	bmp[22]	=height;
	bmp[23]	=height>>8;
	bmp[28]	=bpp;

	FILE* fn;
	if ((fn = fopen (filename,"wb")) != NULL)
	{
		fwrite(bmp ,1,54   ,fn);
		fwrite(data,1,width*height*(bpp/8),fn);
		fclose(fn);
	}
	else error_stop("Bmp::save %s",filename);
	/*
		ILuint imageID;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);

		ilTexImage(imageWidth, imageHeight, 1, bytesperpixel, IL_RGB, IL_UNSIGNED_BYTE, pixels); //the third agrument is usually always 1 (its depth. Its never 0 and I can't think of a time when its 2)

		ilSaveImage("image.jpg");	
	*/
}
//#################################################################//
void  Bmp::Blur(int count)
{
	int x,y,b,c;
	int bytes=bpp/8;
	for(c=0;c<count;c++)
		for(x=0;x<width-1;x++)
			for(y=0;y<height-1;y++)
				for(b=0;b<bytes;b++)
					data[(y*width+x)*bytes+b]=
					    (	(int)data[((y+0)*width+x+0)*bytes+b]+
					      (int)data[((y+0)*width+x+1)*bytes+b]+
					      (int)data[((y+1)*width+x+0)*bytes+b]+
					      (int)data[((y+1)*width+x+1)*bytes+b] ) /4;

}
//#################################################################//
void Bmp::Crop(int x,int y)
{
	if(data==NULL)return;

	unsigned char* newdata;
	int i,j;

	int bytes=bpp/8;

	newdata=(unsigned char*)malloc(x*y*bytes);

	if(!newdata) error_stop("Bmp::crop : out of memory");

	memset(newdata,0,x*y*bytes);

	for(i=0;i<y;i++)
		if(i<height)
			for(j=0;j<x*bytes;j++)
				if(j<width*bytes)
					newdata[i*x*bytes+j]=data[i*width*bytes+j];
	free(data);
	data=NULL;
	Set(x,y,bpp,newdata);
}
//#################################################################//
void Bmp::Scale(int x,int y)
{
	if(data==NULL)return ;
	if(x==0)return ;
	if(y==0)return ;

	unsigned char* newdata;
	int i,j,k;

	int bytes=bpp/8;
	newdata=(unsigned char*)malloc(x*y*bytes);
	if(!newdata) error_stop("Bmp::scale : out of memory");

	memset(newdata,0,x*y*bytes);

	for(i=0;i<y;i++)
		for(j=0;j<x;j++)
			for(k=0;k<bytes;k++)
				newdata[i*x*bytes+j*bytes+k]=data[(i*height/y)*(width*bytes)+(j*width/x)*bytes+k];

	free(data);
	data=NULL;
	Set(x,y,bpp,newdata);
}
//#################################################################//
void Bmp::Set(int x,int y,int b,unsigned char*buffer)
{
	width=x;
	height=y;
	bpp=b;
	if(data) free(data);

	data=(unsigned char*) malloc(width*height*(bpp/8));
	if(!data) error_stop("Bmp::set : out of memory");

	if(buffer==0)
		memset(data,0,width*height*(bpp/8));
	else
		memmove(data,buffer,width*height*(bpp/8));
}
//#################################################################//
void Bmp::Load(const char *filename, bool convert32)
{
	ilImage i;

	if(!i.Load(filename))	error_stop("Bmp::load file %s not found\n",filename);
	if(i.GetData()==0)		error_stop("Bmp::load %s : 0 pointer\n",filename);
	if(i.Format()==IL_RGBA || convert32) i.Convert(IL_RGBA); else i.Convert(IL_RGB);

	printf("Bmp::loaded %s : %dx%dx%d \n",filename,i.Width(),i.Height(),i.Bpp());

	Set(i.Width(),i.Height(),i.Bpp()*8,i.GetData());
}
//#################################################################//
void Bmp::MakeBump()
{
	int stride=bpp/8;
	std::vector<vec3f> normals;normals.resize(width*height);
	loopi(0,width)loopj(0,height)
	{
		float h =data[( i+j*width)*stride+0];
		float hx=data[((i+1)%width+j*width)*stride+0];
		float hy=data[( i+((j+1)%height)*width)*stride+0];
		vec3f d1(32,0,hx-h);
		vec3f d2(0,32,hy-h);
		vec3f n;n.cross(d1,d2);n.norm();
		normals[i+j*width]=n;
	}
	loopi(0,width)loopj(0,height)
	{
		vec3f n=normals[i+j*width];
		data[(i+j*width)*stride+0] = n.x*127.5+127.5;
		data[(i+j*width)*stride+1] = n.y*127.5+127.5;
		data[(i+j*width)*stride+2] = n.z*127.5+127.5;
	}
}
//#################################################################//

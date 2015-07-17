///////////////////////////////////////////
#pragma once
///////////////////////////////////////////
#include "core.h"
#include "mathlib/Vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
///////////////////////////////////////////
#pragma comment(lib,"DevIL.lib")
#pragma comment(lib,"ILU.lib")
#pragma comment(lib,"ILUT.lib")
///////////////////////////////////////////
class Bmp
{
public:

	Bmp();
	Bmp(int x,int y,int bpp,unsigned char*data=0);
	Bmp(const char*filename, bool convert32=0);
	~Bmp();

	void Load(const char*filename, bool convert32=0);
	void Save(const char*filename);
	void Set(int x,int y,int bpp,unsigned char*data);
	void Crop(int x,int y);
	void Scale(int x,int y);
	void Blur(int count);
	void MakeBump();
	
	unsigned char*data;
	int width;
	int height;
	int bpp;
};


#include "CCreatureAnimation.h"
#include "../CGameInfo.h"
#include "../hch/CLodHandler.h"
int CCreatureAnimation::getType() const
{
	return type;
}

void CCreatureAnimation::setType(int type)
{
	this->type = type;
	curFrame = 0;
	if(type!=-1)
	{
		if(SEntries[curFrame].group!=type) //rewind
		{
			int j=-1; //first frame in displayed group
			for(int g=0; g<SEntries.size(); ++g)
			{
				if(SEntries[g].group==type && j==-1)
				{
					j = g;
					break;
				}
			}
			if(curFrame!=-1)
				curFrame = j;
		}
	}
	else
	{
		if(curFrame>=frames)
			curFrame = 0;
	}
}

CCreatureAnimation::CCreatureAnimation(std::string name) : RLEntries(NULL), RWEntries(NULL)
{
	//load main file
	std::string data2 = CGI->spriteh->getTextFile(name);
	if(data2.size()==0)
		throw new std::string("no such def!");
	FDef = new unsigned char[data2.size()];
	for(int g=0; g<data2.size(); ++g)
	{
		FDef[g] = data2[g];
	}

	//init anim data
	int i,j, totalInBlock;
	char Buffer[13];
	defName=name;
	int andame = data2.size();
	i = 0;
	DEFType = readNormalNr(i,4); i+=4;
	fullWidth = readNormalNr(i,4); i+=4;
	fullHeight = readNormalNr(i,4); i+=4;
	i=0xc;
	totalBlocks = readNormalNr(i,4); i+=4;

	i=0x10;
	for (int it=0;it<256;it++)
	{
		palette[it].R = FDef[i++];
		palette[it].G = FDef[i++];
		palette[it].B = FDef[i++];
		palette[it].F = 0;
	}
	i=0x310;
	totalEntries=0;
	for (int z=0; z<totalBlocks; z++)
	{
		int unknown1 = readNormalNr(i,4); i+=4;
		totalInBlock = readNormalNr(i,4); i+=4;
		for (j=SEntries.size(); j<totalEntries+totalInBlock; j++)
			SEntries.push_back(SEntry());
		int unknown2 = readNormalNr(i,4); i+=4;
		int unknown3 = readNormalNr(i,4); i+=4;
		for (j=0; j<totalInBlock; j++)
		{
			for (int k=0;k<13;k++) Buffer[k]=FDef[i+k]; 
			i+=13;
			SEntries[totalEntries+j].name=Buffer;
		}
		for (j=0; j<totalInBlock; j++)
		{ 
			SEntries[totalEntries+j].offset = readNormalNr(i,4);
			int unknown4 = readNormalNr(i,4); i+=4;
		}
		//totalEntries+=totalInBlock;
		for(int hh=0; hh<totalInBlock; ++hh)
		{
			SEntries[totalEntries].group = z;
			++totalEntries;
		}
	}
	for(j=0; j<SEntries.size(); ++j)
	{
		SEntries[j].name = SEntries[j].name.substr(0, SEntries[j].name.find('.')+4);
	}
	//pictures don't have to be readed here
	//for(int i=0; i<SEntries.size(); ++i)
	//{
	//	Cimage nimg;
	//	nimg.bitmap = getSprite(i);
	//	nimg.imName = SEntries[i].name;
	//	nimg.groupNumber = SEntries[i].group;
	//	ourImages.push_back(nimg);
	//}
	//delete FDef;
	//FDef = NULL;

	//init vars
	curFrame = 0;
	type = -1;
	frames = totalEntries;
}

int CCreatureAnimation::readNormalNr (int pos, int bytCon, unsigned char * str, bool cyclic)
{
	int ret=0;
	int amp=1;
	if (str)
	{
		for (int i=0; i<bytCon; i++)
		{
			ret+=str[pos+i]*amp;
			amp*=256;
		}
	}
	else 
	{
		for (int i=0; i<bytCon; i++)
		{
			ret+=FDef[pos+i]*amp;
			amp*=256;
		}
	}
	if(cyclic && bytCon<4 && ret>=amp/2)
	{
		ret = ret-amp;
	}
	return ret;
}
int CCreatureAnimation::nextFrameMiddle(SDL_Surface *dest, int x, int y, bool attacker, bool incrementFrame, bool yellowBorder, SDL_Rect * destRect)
{
	return nextFrame(dest,x-fullWidth/2,y-fullHeight/2,attacker,incrementFrame,yellowBorder,destRect);
}
int CCreatureAnimation::nextFrame(SDL_Surface *dest, int x, int y, bool attacker, bool incrementFrame, bool yellowBorder, SDL_Rect * destRect)
{
	if(dest->format->BytesPerPixel<3)
		return -1; //not enough depth

	//increasing frame numer
	int SIndex = -1;
	if(incrementFrame)
	{
		SIndex = curFrame++;
		if(type!=-1)
		{
			if(SEntries[curFrame].group!=type) //rewind
			{
				int j=-1; //first frame in displayed group
				for(int g=0; g<SEntries.size(); ++g)
				{
					if(SEntries[g].group==type && j==-1)
					{
						j = g;
						break;
					}
				}
				if(curFrame!=-1)
					curFrame = j;
			}
		}
		else
		{
			if(curFrame>=frames)
				curFrame = 0;
		}
	}
	else
	{
		SIndex = curFrame;
	}
	//frame number increased

	long BaseOffset, 
		SpriteWidth, SpriteHeight, //sprite format
		LeftMargin, RightMargin, TopMargin,BottomMargin,
		i, add, FullHeight,FullWidth,
		TotalRowLength, // length of read segment
		RowAdd;
	unsigned char SegmentType, SegmentLength;
	
	i=BaseOffset=SEntries[SIndex].offset;
	int prSize=readNormalNr(i,4,FDef);i+=4;
	int defType2 = readNormalNr(i,4,FDef);i+=4;
	FullWidth = readNormalNr(i,4,FDef);i+=4;
	FullHeight = readNormalNr(i,4,FDef);i+=4;
	SpriteWidth = readNormalNr(i,4,FDef);i+=4;
	SpriteHeight = readNormalNr(i,4,FDef);i+=4;
	LeftMargin = readNormalNr(i,4,FDef);i+=4;
	TopMargin = readNormalNr(i,4,FDef);i+=4;
	RightMargin = FullWidth - SpriteWidth - LeftMargin;
	BottomMargin = FullHeight - SpriteHeight - TopMargin;
	
	add = 4 - FullWidth%4;

	int BaseOffsetor = BaseOffset = i;

	int ftcp = 0;

	if (defType2==1) //as it should be allways in creature animations
	{
		if (TopMargin>0)
		{
			for (int i=0;i<TopMargin;i++)
			{
				ftcp+=FullWidth+add;
			}
		}
		RLEntries = new int[SpriteHeight];
		for (int i=0;i<SpriteHeight;i++)
		{
			RLEntries[i]=readNormalNr(BaseOffset,4,FDef);BaseOffset+=4;
		}
		for (int i=0;i<SpriteHeight;i++)
		{
			BaseOffset=BaseOffsetor+RLEntries[i];
			if (LeftMargin>0)
			{
				ftcp+=LeftMargin;
			}
			TotalRowLength=0;
			do
			{
				SegmentType=FDef[BaseOffset++];
				SegmentLength=FDef[BaseOffset++];
				if (SegmentType==0xFF)
				{
					for (int k=0;k<=SegmentLength;k++)
					{
						int xB = (attacker ? ftcp%(FullWidth+add) : (FullWidth+add) - ftcp%(FullWidth+add) - 1) + x;
						int yB = ftcp/(FullWidth+add) + y;
						if(xB>=0 && yB>=0 && xB<dest->w && yB<dest->h)
						{
							if(!destRect || (destRect->x <= xB && destRect->x + destRect->w > xB && destRect->y <= yB && destRect->y + destRect->h > yB))
								putPixel(dest, xB + yB*dest->w, palette[FDef[BaseOffset+k]], FDef[BaseOffset+k], yellowBorder);
						}
						ftcp++; //increment pos
						if ((TotalRowLength+k+1)>=SpriteWidth)
							break;
					}
					BaseOffset+=SegmentLength+1;////
					TotalRowLength+=SegmentLength+1;
				}
				else
				{
					for (int k=0;k<SegmentLength+1;k++)
					{
						int xB = (attacker ? ftcp%(FullWidth+add) : (FullWidth+add) - ftcp%(FullWidth+add) - 1) + x;
						int yB = ftcp/(FullWidth+add) + y;
						if(xB>=0 && yB>=0 && xB<dest->w && yB<dest->h)
						{
							if(!destRect || (destRect->x <= xB && destRect->x + destRect->w > xB && destRect->y <= yB && destRect->y + destRect->h > yB))
								putPixel(dest, xB + yB*dest->w, palette[SegmentType], SegmentType, yellowBorder);
						}
						ftcp++; //increment pos
					}
					TotalRowLength+=SegmentLength+1;
				}
			}while(TotalRowLength<SpriteWidth);
			RowAdd=SpriteWidth-TotalRowLength;
			if (RightMargin>0)
			{
				ftcp+=RightMargin;
			}
			if (add>0)
			{
				ftcp+=add+RowAdd;
			}
		}
		delete [] RLEntries;
		RLEntries = NULL;
		if (BottomMargin>0)
		{
			ftcp += BottomMargin * (FullWidth+add);
		}
	}

	//SDL_UpdateRect(dest, x, y, FullWidth+add, FullHeight);

	return 0;
}

int CCreatureAnimation::framesInGroup(int group) const
{
	int ret = 0; //number of frames in given group
	for(int g=0; g<SEntries.size(); ++g)
	{
		if(SEntries[g].group == group)
			++ret;
	}
	return ret;
}

CCreatureAnimation::~CCreatureAnimation()
{
	delete [] FDef;
	if (RWEntries)
		delete [] RWEntries;
	if (RLEntries)
		delete [] RLEntries;
}

void CCreatureAnimation::putPixel(SDL_Surface * dest, const int & ftcp, const BMPPalette & color, const unsigned char & palc, const bool & yellowBorder) const
{
	if(palc!=0)
	{
		Uint8 * p = (Uint8*)dest->pixels + ftcp*3;
		if(palc > 7) //normal color
		{
			p[0] = color.B;
			p[1] = color.G;
			p[2] = color.R;
		}
		else if(yellowBorder && (palc == 6 || palc == 7)) //dark yellow border
		{
			p[0] = 0;
			p[1] = 0xff;
			p[2] = 0xff;
		}
		else if(yellowBorder && (palc == 5)) //yellow border
		{
			p[0] = color.B;
			p[1] = color.G;
			p[2] = color.R;
		}
		else if(palc < 5) //shadow
		{ 
			Uint16 alpha;
			switch(color.G)
			{
			case 0:
				alpha = 128;
				break;
			case 50:
				alpha = 50+32;
				break;
			case 100:
				alpha = 100+64;
				break;
			case 125:
				alpha = 125+64;
				break;
			case 128:
				alpha = 128+64;
				break;
			case 150:
				alpha = 150+64;
				break;
			default:
				alpha = 255;
				break;
			}
			//alpha counted
			p[0] = (p[0] * alpha)>>8;
			p[1] = (p[1] * alpha)>>8;
			p[2] = (p[2] * alpha)>>8;
		}
	}
}

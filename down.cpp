#include "stdio.h"
#include "conio.h"
#include "stdlib.h"
#include "time.h"

#include <SDL/SDL.h>
#include <string.h>

#define ulong unsigned long
#define uint  unsigned int
#define uchar unsigned char
#define ushort unsigned short

short the_map[6442][10] ;
int scroll = 0 ;
extern SDL_Surface * tiles ;
extern void DrawIMGpart(SDL_Surface *img, int x, int y, int s, int nb, int szy) ;

void drawMap(void)
{    int dy = -(scroll%32) ;
     int cptx=10, cpty=11, dx=0, cy, tile ;
     if(!dy) cpty=10 ;

     int mapy = scroll>>5 ;

     while(cptx--)
     {   for(cy=0;cy<cpty;cy++)
         {  tile = the_map[mapy+cy][9-cptx] ;
            if(tile) DrawIMGpart(tiles, dx, (cy<<5)+dy, 32, tile-1,0) ;
         }; dx+=32 ;
     }
}

void generate_map(void)
{
	unsigned int cptx=0, cpty=0 ;
	unsigned short skip=0, nb_stop_cases=0, value, tmp=0 ;
	
	for(cpty=0;cpty<10;cpty++)
	{
		for(cptx=1;cptx<9;cptx++)
			the_map[cpty][cptx] = 0 ;
	
		if(tmp)
		{
			the_map[cpty][0] = 3 ;
			the_map[cpty][9] = 3 ;
			tmp=0 ;
		}
		else
		{
			the_map[cpty][0] = 4 ;
			the_map[cpty][9] = 4 ;
			tmp=1 ;
		}		
	} ;
	
	tmp=0 ;  int y=0, count=0 ;
    char gladal[10] ;
    memset(gladal,0,10) ;

	for(cpty=10;cpty<6442;cpty++)
	{
		skip++ ;
		
		if(skip==4) // add line
		{	if(++tmp>=4) tmp=2 ;
            if(++count%42 == 1) tmp=4 ;
			skip=0 ;
			nb_stop_cases=0 ;

			for(cptx=0;cptx<10;cptx+=2)
			{
				value = rand()%3 ;

				if(nb_stop_cases < 8)
				{
					if(value)
					{
						the_map[cpty][cptx] = tmp ;
						nb_stop_cases++ ;
					} else the_map[cpty][cptx] = 0 ;
				}
				else { the_map[cpty][rand()%9] = 0 ;
                       if(cpty&15) the_map[cpty][rand()%9] = tmp ;
                       the_map[cpty][cptx] = 0 ;
                     }
			} ;
            y = nb_stop_cases ;
			for(cptx=1;cptx<10;cptx+=2)
			{
				value = rand()%3 ;
				
				if(nb_stop_cases < 8)
				{
					if(value)
					{
						the_map[cpty][cptx] = tmp ;
						nb_stop_cases++ ;
					} else the_map[cpty][cptx] = 0 ;
				}
				else { the_map[cpty][rand()%9] = 0 ;
                       if(cpty&15) the_map[cpty][rand()%9] = tmp ;
                       the_map[cpty][cptx] = 0 ;
                     }
			} ;
            y += nb_stop_cases ;
            if(nb_stop_cases<6) for(int c=0;c<10;c++)  if(!the_map[cpty][c]) if(++(gladal[c]) > 2) { the_map[cpty][c]=tmp ; gladal[c]=0 ; }
		}
		else	for(cptx=0;cptx<10;cptx++) if(y < 7 || ((skip!=3)&&(skip!=1)))
                                           {	if(!(rand()%(42*2))) the_map[cpty][cptx] = tmp ;
                                                     else        the_map[cpty][cptx] = 0 ;
                                           } else  the_map[cpty][cptx] = 0 ;
	} ;
}


/**** falldown !
        x86 release
            C 2k4 r043v ****/

/**** licence
   it's easy : free for non commercial use    **/

#include "stdio.h"
#include "conio.h"
#include "stdlib.h"
#include "time.h"

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <string.h>

#define ulong unsigned long
#define uint  unsigned int
#define uchar unsigned char
#define ushort unsigned short

#include "./unrarlib/unrarlib.h"
#include "fall.h" // game's data into a rar file

Uint8 *keyboard;

extern short the_map[6442][10] ;
extern void generate_map(void) ;
extern void drawMap(void) ;
SDL_Surface * tiles ;
extern int scroll ;
int hero_y = 42 ;
int hero_x = (320-32)/2 ;
int walk=0 ;
int difficult = 2 ;

int onScreen(void)
{ int d = hero_y+32-scroll ;
  if(d<0) return 0 ;
  if(d > 240) hero_y = scroll + 208 ;
  return 1 ;
}

int die=0 ;

#define clipLeft  3
#define clipRight 3
#define clipUp    8
#define clipDown  8

int downSpeed  = 3 ;
int scrollTime = 7 ;
int moveTime   = 4 ;

uint lastScroll =  0 ;
uint lastMove   =  0 ;
int lastMoveWay = -1 ;

int change=2 ;

void heroDown(void)
{ static uint lastDown=0 ;
  if(lastDown + downSpeed  > SDL_GetTicks()) return ;
  lastDown = SDL_GetTicks() ;
  int down=1 ;
  int mapy = (hero_y+35)>>5 ;
  int mapx = (hero_x+clipLeft+3)>>5 ;
  if(the_map[mapy][mapx]) down=0 ;
  mapx = (hero_x+35-clipRight-3)>>5 ;
  if(the_map[mapy][mapx]) down=0 ;
  walk = !down ;
  if(down) hero_y++ ;
  change=2 ;
}

void heroLeft(void)
{ if(lastMoveWay) if(lastMove + moveTime - walk > SDL_GetTicks()) return ;
  lastMoveWay = 1 ;  lastMove = SDL_GetTicks() ;
  if(!hero_x) return ;  int move=1 ;
  int mapy = (hero_y+35-clipDown)>>5 ;
  int mapx = (hero_x+clipLeft)>>5 ;
  if(the_map[mapy][mapx]) move=0 ;
  mapy = (hero_y+clipUp)>>5 ;
  if(the_map[mapy][mapx]) move=0 ;
  if(move) hero_x-- ;
  change=2 ;
}

void heroRight(void)
{ if(!lastMoveWay) if(lastMove + moveTime - walk > SDL_GetTicks()) return ;
  lastMoveWay = 0 ;  lastMove = SDL_GetTicks() ;
  if(hero_x > 320-34) return ;  int move=1 ;
  int mapy = (hero_y+35-clipDown)>>5 ;
  int mapx = (hero_x+35-clipRight)>>5 ;
  if(the_map[mapy][mapx]) move=0 ;
  mapy = (hero_y+clipUp)>>5 ;
  if(the_map[mapy][mapx]) move=0 ;
  if(move) hero_x++ ;
  change=2 ;
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{   if(x<0 || x>319 || y<0 || y>239) return ;
    int bpp = surface->format->BytesPerPixel;
    Uint16 * scr = (Uint16 *)surface->pixels ;
    *(scr + y * 320 + x) = pixel ;
}

void blitImg(SDL_Surface *surface, char * img, int sx, int sy, int px, int py, char trColor, int * pal)
{   int color ;
    for(int x=0;x<sx;x++)
      for(int y=0;y<sy;y++)
      { color = *(img + x*sy + (sy-y)) ;
        if(color != trColor)
          putpixel(surface, px+x, py+y, pal[color]) ;
      };
}

SDL_Surface *screen, *bg, *chfont, *ltfont ;

void DrawIMG(SDL_Surface *img, int x, int y)
{SDL_Rect dest;
 dest.x = x;
 dest.y = y;
 SDL_BlitSurface(img, NULL, screen, &dest);
}

void DrawIMG(SDL_Surface *img, int x, int y, int w, int h, int x2, int y2)
{SDL_Rect dest;
 dest.x = x;
 dest.y = y;
 SDL_Rect dest2;
 dest2.x = x2;
 dest2.y = y2;
 dest2.w = w;
 dest2.h = h;
 SDL_BlitSurface(img, &dest2, screen, &dest);
}

void DrawIMGpart(SDL_Surface *img, int x, int y, int s, int nb, int szy)
{SDL_Rect dest, src;
 if(!szy) szy=s ;
 src.x = nb*s ; src.y = 0 ;
 src.h = szy ; src.w = s ;
 dest.x = x;
 dest.y = y;
 SDL_BlitSurface(img, &src, screen, &dest);
}

void drawText(char * t, int x, int y)
{ int c=0 ;  while(*t && x+c*11 < 320){   if(*t < 'a') DrawIMGpart(chfont, x+c*11, y, 11, (*(uchar*)t)-'0', 16) ;
                                          else DrawIMGpart(ltfont, x+c*11, y, 11, (*(uchar*)t)-'a', 16) ;
                                          t++ ; c++ ;
                                      }
}

void drawTextR(char * t, int x, int y)
{ int c=0 ; while(t[c]) c++ ;
  drawText(t,x-(c*11),y) ;
}

void drawInt(int i, int x, int y)
{ char b[42] ; sprintf(b,"%i",i) ; char * t = b ; int c=0 ;  while(*t && x+c*9 < 320){  DrawIMGpart(chfont, x+c*9, y, 11, (*(uchar*)t)-48, 16) ; t++ ; c++ ; }  }

void drawLetter(char l, int x, int y) {  DrawIMGpart(ltfont, x, y, 11, l-'a', 16) ;  }

void DrawBg(int x, int y)
{SDL_Rect src;
 src.x = x ; src.y = y ;
 src.h = 240 ; src.w = 320 ;
 SDL_BlitSurface(bg, &src, screen, 0);
}

class anim
{ SDL_Surface *img ;
  int flipTime, sx, sy ;
  int nb ;
  Uint32 lastTime ;
  public :
  int way, curent, start ;
  anim(SDL_Surface *i, int x, int y, int n, int s, int t, uint tr, int a=0) ;
  void draw(int px, int py) ;
};

anim::anim(SDL_Surface *i, int x, int y, int n, int s, int t, uint tr, int a)
{ img=i ; sx=x ; sy=y ; nb=n ; flipTime=t ; curent=start=s ; lastTime = SDL_GetTicks() ;
  if(tr) SDL_SetColorKey(i, SDL_SRCCOLORKEY, tr); way=a ;
}

void anim::draw(int px, int py)
{  SDL_Rect dest, src;
   src.x = curent*sx ; src.y = 0 ;
   src.h = sx ; src.w = sy ;
   dest.x = px;
   dest.y = py;
   SDL_BlitSurface(img, &src, screen, &dest);
   if(lastTime + flipTime < SDL_GetTicks())
   { lastTime = SDL_GetTicks() ;    change=2 ;
     if(!way) { if(++curent >= start+nb) curent=start ; }
     else     { if(--curent <= start)    curent=start+nb-1 ; }
   }
}

anim *rhero, *whero ;

void drawHero(void)
{ if(!onScreen()) { die=1 ; return ; }
  if(walk) { whero->draw(hero_x,hero_y-scroll) ;  rhero->curent=rhero->start ; }
  else rhero->draw(hero_x,hero_y-scroll) ;
}

__attribute__((aligned(4))) char  saved_score[42+8] = { 't','r','i','p','a','w','a' } ; // Zz² !!

void loadScore(void)
{ char spath[64] ; sprintf(spath,"%s\\falldown.sav",getenv("windir")) ;
  int reset=1 ;  FILE *f = fopen(spath,"r") ;
  if(f) {  char * b = spath ; fread(b,54,1,f) ; int crc=0, _crc ;
           if(*(int*)b == 0x6c6c6166) // "fall"
           {    for(int c=0;c<50;c++){ saved_score[49-c] = 0xFF - *(b+4+c) ; crc+=saved_score[49-c] ; }
                fread(&_crc,4,1,f) ;
           }    else printf("bad check %s\n",b) ;
           if(*(int*)saved_score == 0x70697274) reset=0 ;
           else printf("uncript error %s\n",saved_score) ;
           if(crc != _crc) { printf("crc error !\n") ; reset=1 ; }
           fclose(f) ;
         } else printf("file not found %s\n",spath) ;
  if(reset)
  {  printf("reset score\n") ;
     int *s = (int*)(saved_score+8) ;
     char * mstr = (char*)"r043v\0" ;
     *s++ = 242 ; *s++ = 142 ; *s++ = 42 ;
     for(int c=0;c<3;c++) memcpy(saved_score+8+12+10*c,mstr,6) ;
  }
  int * s = (int*)(saved_score+8) ; char * str = saved_score+20 ;
  printf("%i %i %i by %s|%s|%s",*s++,*s++,*s++,str,str+10,str+20) ;
}

//__attribute__((aligned(4))) char  score_offset[8] = { 'p','A','w','a',0,0,0,0 } ;

void saveScore(void)
{  char spath[128] ; sprintf(spath,"%s\\falldown.sav",getenv("windir")) ;
   FILE *f = fopen(spath,"w") ;
   if(f) {  *(int*)spath = 0x6c6c6166 ; // "fall"
            int crc = 0 ;
            for(int c=0;c<50;c++){  *(spath+4+c) = 0xFF - saved_score[49-c] ; crc+=saved_score[c] ; }
            if(!fwrite(spath, 54, 1, f) || !fwrite(&crc,4,1,f)) printf("\nwrite error on save") ;
            fclose(f) ;
         }
   /*printf("check  : %s\n",score_offset) ;
   if((int)score_offset&3) printf("not aligned :/\n") ;
   int offset = *(int*)(score_offset+4) ;
   printf("offset : %i\n",offset) ;
   if(!offset) { printf("file not patched.\n") ; return ; }
   FILE * f = fopen("falldown.exe","r+") ;
   if(!f) printf("file open error.\n") ;
   else { if(fseek(f,*(int*)(score_offset+4),SEEK_SET)) printf("fseek error") ;
          if(!fwrite(saved_score,42+8,1,f)) printf("write error") ;
          fclose(f) ;
        }*/    //      windows tu suxx :/
}

int mpx, mpy, score=0, cnt=0 ;

void enterName(char pos)
{  char *st, *n = saved_score+8+12+10*pos ; st=n ;
   memset(n,0,10) ;
   int c=0 ; char ch = 'a' ;
   int draw=1 ; uint time ; uint stime=42 ;
   while(c<9){ DrawBg(mpx,mpy) ; drawMap() ;
               drawText("enter your name", 90, 62) ;
               drawText(st,95,80) ; if(draw) drawLetter(ch,95 + c*11,80) ;

               if(stime + 202 < SDL_GetTicks()) { draw^=1 ; stime = SDL_GetTicks() ; }

               SDL_PumpEvents();  keyboard = SDL_GetKeyState(NULL);
               if(keyboard[SDLK_SPACE])
               { n[c++] = ch ; time = SDL_GetTicks() ;
                 while(keyboard[SDLK_SPACE]) { SDL_PumpEvents();  keyboard = SDL_GetKeyState(NULL);
                                               if(time + 342 < SDL_GetTicks()) break ;
                                             }
               }

               if(keyboard[SDLK_LEFT] || keyboard[SDLK_DOWN])
               {  if(ch > 'a') ch-- ; time = SDL_GetTicks() ;
                  while(keyboard[SDLK_LEFT] || keyboard[SDLK_DOWN]) { SDL_PumpEvents();  keyboard = SDL_GetKeyState(NULL);
                                                                      if(time + 342 < SDL_GetTicks()) break ;
                                                                    } draw=1 ; stime = SDL_GetTicks() ;
               }

               if(keyboard[SDLK_RIGHT] || keyboard[SDLK_UP])
               { if(ch < 'z') ch++ ; time = SDL_GetTicks() ; draw=1 ;
                 while(keyboard[SDLK_RIGHT] || keyboard[SDLK_UP]) { SDL_PumpEvents();  keyboard = SDL_GetKeyState(NULL);
                                                                    if(time + 342 < SDL_GetTicks()) break ;
                                                                  } draw=1 ; stime = SDL_GetTicks() ;
               }

               if(keyboard[SDLK_RETURN] && c) break ;
               SDL_Flip(screen);
   };
}

int quit=0 ;

int main(int argc, char *argv[])
{    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) exit(1);
     screen = SDL_SetVideoMode(320, 240, 16, SDL_DOUBLEBUF|SDL_HWSURFACE);
     if(!screen) exit(1);
     Uint32 color = SDL_MapRGB(screen->format, 0xff, 0xff, 0x00);
     SDL_EnableKeyRepeat(100,20);
     SDL_WM_SetCaption("falldown :D                                       r043v / 2k4",0);
     memory_file * programData = loadMemoryFile(data,58250) ;
     char * _title ; unsigned long data_size ;
 	 urarlib_get(&_title, &data_size, (char*)"title.bmp", programData,0) ;
     SDL_RWops * z = SDL_RWFromMem(_title,data_size);
     SDL_Surface * title = SDL_LoadBMP_RW(z,1); free(_title) ;
     DrawIMG(title,0,0) ; SDL_Flip(screen); loadScore() ;

     Mix_Music *music; 	int volume=SDL_MIX_MAXVOLUME;
	 SDL_ShowCursor(SDL_DISABLE);
     Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,1024) ;
	 music=Mix_LoadMUS("./falldown.zik");
	 Mix_PlayMusic(music, -1) ;

   	 urarlib_get(&_title, &data_size, (char*)"chfont.bmp", programData,0) ;
     z = SDL_RWFromMem(_title,data_size);
     chfont = SDL_LoadBMP_RW(z,1); free(_title) ; SDL_SetColorKey(chfont, SDL_SRCCOLORKEY, 10);
   	 urarlib_get(&_title, &data_size, (char*)"ltfont.bmp", programData,0) ;
     z = SDL_RWFromMem(_title,data_size);
     ltfont = SDL_LoadBMP_RW(z,1); free(_title) ; SDL_SetColorKey(ltfont, SDL_SRCCOLORKEY, 10);
 	 urarlib_get(&_title, &data_size, (char*)"perso.bmp", programData,0) ;
     z = SDL_RWFromMem(_title,data_size);
     SDL_Surface * perso = SDL_LoadBMP_RW(z,1); free(_title) ;
 	 urarlib_get(&_title, &data_size, (char*)"tiles.bmp", programData,0) ;
     z = SDL_RWFromMem(_title,data_size);
     tiles = SDL_LoadBMP_RW(z,1); free(_title) ;
 	 urarlib_get(&_title, &data_size, (char*)"kial.bmp", programData,0) ;
     z = SDL_RWFromMem(_title,data_size);
     bg = SDL_LoadBMP_RW(z,1); free(_title) ;

     anim heror(perso, 35, 35, 15, 2, 100, SDL_MapRGB(perso->format, 255, 0, 255)) ;
     anim herow(perso, 35, 35,  2, 0, 100, SDL_MapRGB(perso->format, 255, 0, 255)) ;
     rhero = &heror ; whero = &herow ;
     int *s ; char *n ; char scoreStr[42] ; int *sc ;
  do{
     scroll=score=die=quit=walk=0 ; hero_y=42 ; hero_x=(320-32)/2 ; change=2 ;
     DrawIMG(title,0,0) ; generate_map() ;
     s = (int*)(saved_score+8) ;
     drawInt(*s++, 282, 5 ) ;
     drawInt(*s++, 282, 20) ;
     drawInt(*s, 282, 35) ;
     n = saved_score+20 ;
     drawTextR(n, 272, 5 ) ; n+=10 ;
     drawTextR(n, 272, 20) ; n+=10 ;
     drawTextR(n, 272, 35) ;
     SDL_Flip(screen);

     do{  SDL_PumpEvents();
          keyboard = SDL_GetKeyState(NULL);
     } while(keyboard[SDLK_RETURN]) ;

     while(1){  SDL_PumpEvents();
                keyboard = SDL_GetKeyState(NULL);
                if(keyboard[SDLK_ESCAPE]) { die=quit=1 ; break ; }
                if(keyboard[SDLK_RETURN]) break ;
     };

     while(!die) // launch game
     {  SDL_PumpEvents();
        keyboard = SDL_GetKeyState(NULL);
        if(keyboard[SDLK_ESCAPE]) die=quit=1 ;
        if(keyboard[SDLK_LEFT]){ heroLeft() ; heror.way = 0 ; }
        if(keyboard[SDLK_RIGHT]){ heroRight() ; heror.way = 1 ; }

       if(lastScroll + scrollTime < SDL_GetTicks())
       { lastScroll = SDL_GetTicks() ; scroll+=difficult ; change=2 ; }

       if(!walk)
        if(cnt++ > 42*5) { score++ ; cnt=0 ; sprintf(scoreStr,"%i",score) ; }

       heroDown() ;
       if(change) { mpx = hero_x>>2 ; if(mpx > 49) mpx = 49 ;
                    mpy = hero_y-scroll ; if(mpy < 0) mpy=0 ;
                    DrawBg(mpx,mpy) ; drawMap() ; drawHero() ;
                    drawInt(score, 282, 5) ;
                    SDL_Flip(screen); --change ;
                  }
     };

     sc = (int*)(saved_score+8) ;
     if(score >= sc[2]) // are in the 3 best score ?
     {   printf("\n you made a score !\n") ;
         if(score >= sc[1]) // are in the 2 first ?
         {   if(score >= *sc) // are first ?
             {        sc[2] = sc[1] ; memcpy(saved_score+40,saved_score+30,10) ;
                      sc[1] = sc[0] ; memcpy(saved_score+30,saved_score+20,10) ;
                      *sc = score ;   printf("premier, bravo !!") ;
                      enterName(0) ;
             } else { // 2eme
                      sc[2] = sc[1] ; memcpy(saved_score+40,saved_score+30,10) ;
                      sc[1] = score ; printf("tu est second !") ;
                      enterName(1) ;
             }
         } else { sc[2] = score ; printf("3eme :]") ; enterName(2) ; } // 3eme
         printf("\nwith %i",score) ;
     }

     saveScore() ;

  } while(!quit) ;
     SDL_Quit() ;
}


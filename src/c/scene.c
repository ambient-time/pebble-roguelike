// Copyright 2026 Luke Steuber. MIT License.
#include "scene.h"
Scene scene_at(int h,int m,int s,uint32_t day){
 h=((h%24)+24)%24;m=((m%60)+60)%60;s=((s%60)+60)%60;
 uint32_t seed=day*1440u+(uint32_t)h*60u+(uint32_t)m;
 Scene a={.hour=h%12?h%12:12,.minute=m,.second=s,.kind=(int)((seed+seed/6)%6),.seed=seed,.heroX=23,.bossAlive=s<49};
 a.phase=s<7?0:s<42?1:s<49?2:s<55?3:4;
 if(s<7)a.heroX=8+s*2;
 else if(s>=55)a.heroX=23+(s-55)*7;
 else if(s%6<3)a.heroX+=s%3*2;
 return a;
}

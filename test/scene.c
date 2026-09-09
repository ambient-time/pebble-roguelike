#include "../src/c/scene.h"
#include <assert.h>
#include <stdio.h>
int main(void){
 for(int t=0;t<86400;t++){
  Scene a=scene_at(t/3600,t/60%60,t%60,20000),b=scene_at(t/3600,t/60%60,0,20000);
  assert(a.hour==t/3600&&a.minute==t/60%60&&a.second==t%60);
  assert(a.kind>=0&&a.kind<6&&a.kind==b.kind&&a.seed==b.seed);
  assert(a.heroX>=8&&a.heroX<=51&&a.phase>=0&&a.phase<=4);
  assert(a.bossAlive==(t%60<49));
  if(t%60==59){Scene n=scene_at((t+1)/3600,(t+1)/60%60,0,20000+(t==86399));assert(n.seed!=a.seed);assert(n.phase==0&&n.bossAlive);}
 }
 assert(scene_at(0,0,0,0).hour==0&&scene_at(12,0,0,0).hour==12);
 assert(scene_at(17,29,0,0).hour==17&&scene_at(17,29,0,0).minute==29);
 puts("86400 states passed: time encoding, minute encounters, bounded actors, resets and phases");
}

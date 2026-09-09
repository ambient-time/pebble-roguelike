// Copyright 2026 Luke Steuber. MIT License.
#include <pebble.h>
#include "scene.h"
static Window *window;static Layer *canvas;static bool active;static int w,h,ox,oy;
#ifdef PBL_COLOR
#define STONE GColorOxfordBlue
#define SEAM GColorDarkGray
#define GOLD GColorChromeYellow
#define RUNE GColorElectricBlue
#define CLOAK GColorJazzberryJam
#else
#define STONE GColorBlack
#define SEAM GColorWhite
#define GOLD GColorWhite
#define RUNE GColorWhite
#define CLOAK GColorWhite
#endif
// Every scene pixel is exactly 2x2 device pixels, including Emery. Larger watches
// show more chamber around the same crisp sprites instead of stretched pixels.
static void px(GContext*c,int x,int y,int ww,int hh,GColor col){graphics_context_set_fill_color(c,col);graphics_fill_rect(c,GRect(ox+x*2,oy+y*2,ww*2,hh*2),0,GCornerNone);}
static const uint8_t digits[10][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
 {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
 {14,17,17,15,1,1,14}
};
static void glyph(GContext*c,int x,int y,const uint8_t rows[7],int scale,GColor color){
 for(int r=0;r<7;r++)for(int col=0;col<5;col++)if(rows[r]&(1<<(4-col)))px(c,x+col*scale,y+r*scale,scale,scale,color);
}
static void label(GContext*c,int x,int y,const char*text){
 static const uint8_t f[7]={31,16,16,30,16,16,16},l[7]={16,16,16,16,16,16,31},o[7]={14,17,17,17,17,17,14},r[7]={30,17,17,30,20,18,17},m[7]={17,27,21,21,17,17,17};
 for(;*text;text++,x+=6){const uint8_t*rows=*text=='F'?f:*text=='L'?l:*text=='O'?o:*text=='R'?r:m;glyph(c,x,y,rows,1,GOLD);}
}
static void room_number(GContext*c,int center,int value){
 glyph(c,center-11,13,digits[value/10],2,GColorWhite);
 glyph(c,center+1,13,digits[value%10],2,GColorWhite);
}
static void hero(GContext*c,int x,int y,int s){
 px(c,x-3,y-10,6,4,GColorWhite);px(c,x+1,y-9,1,1,GColorBlack);
 px(c,x-4,y-6,7,6,CLOAK);px(c,x-2,y-5,3,1,GColorBlack);
 px(c,x-3-s%2,y,2,2,GColorWhite);px(c,x+1+s%2,y,2,2,GColorWhite);
 px(c,x+5,y-10,1,8,GColorWhite);px(c,x+3,y-4,4,1,GOLD);
}
static void boss(GContext*c,int x,int y,int kind,int s){
 GColor color=GOLD;
#ifdef PBL_COLOR
 const GColor cols[6]={GColorSunsetOrange,GColorIslamicGreen,GColorLightGray,GColorVividViolet,GColorChromeYellow,GColorElectricBlue};color=cols[kind];
#endif
 if(kind==1){px(c,x-8,y-5-s%2,17,6+s%2,color);px(c,x-5,y-9,11,5,color);}
 else if(kind==4){px(c,x-9,y-7,18,8,color);px(c,x+5,y-11,7,6,color);for(int i=0;i<3;i++)px(c,x-7+i*6,y+1,3,2,color);}
 else{px(c,x-6,y-11,13,12,color);px(c,x-4,y-16,9,6,color);px(c,x-9,y-10,3,8,color);px(c,x+7,y-10,3,8,color);if(kind==0||kind==3){px(c,x-6,y-18,2,5,color);px(c,x+5,y-18,2,5,color);}if(kind==2)for(int i=0;i<3;i++)px(c,x-4,y-9+i*3,9,1,GColorBlack);}
 px(c,x-3,y-13,2,2,GColorBlack);px(c,x+2,y-13,2,2,GColorBlack);
 if(kind==1){px(c,x-4,y-6,2,2,GColorBlack);px(c,x+3,y-6,2,2,GColorBlack);}
}
static void draw(Layer*l,GContext*c){
 time_t epoch=time(NULL);struct tm t=*localtime(&epoch);Scene a=scene_at(t.tm_hour,t.tm_min,t.tm_sec,(uint32_t)(epoch/86400));
 graphics_context_set_antialiased(c,false);graphics_context_set_fill_color(c,GColorBlack);graphics_fill_rect(c,layer_get_bounds(l),0,GCornerNone);
 int inset=(w-72)/2,ground=h-29;px(c,1,1,w-2,ground-1,STONE);
 for(int y=2;y<ground;y+=7){px(c,1,y,w-2,1,SEAM);for(int x=2+(y%2)*5;x<w-1;x+=12)px(c,x,y,1,6,SEAM);}
 // The owner-requested floor and room numbers make local 24-hour time direct.
 px(c,inset+3,2,66,26,GColorBlack);
 label(c,inset+6,4,"FLOOR");label(c,inset+41,4,"ROOM");
 room_number(c,inset+20,a.hour);room_number(c,inset+52,a.minute);
 // Open stone arches at left and right, only the exit opens after loot.
 px(c,inset+3,ground-17,8,17,GColorBlack);px(c,inset+2,ground-19,10,2,SEAM);
 px(c,inset+60,ground-19,10,19,GColorBlack);px(c,inset+59,ground-21,12,2,SEAM);
 if(a.phase<4)for(int i=0;i<3;i++)px(c,inset+61+i*3,ground-18,1,18,SEAM);
 px(c,0,ground,w,1,SEAM);
 // Combat stays in a clean central stage: sprite shapes survive monochrome.
 px(c,inset+13,28,46,ground-28,GColorBlack);
 if(a.bossAlive)boss(c,inset+49,ground-3,a.kind,a.second);
 else{px(c,inset+43,ground-3,12,2,SEAM);}
 hero(c,inset+a.heroX,ground-3,a.second);
 if(a.phase==1||a.phase==2){int xx=inset+33+a.second%4*3;for(int k=0;k<4;k++)px(c,xx+k,ground-16+k,1,1,a.phase==2?GOLD:RUNE);}
 if(a.phase==3){px(c,inset+31,ground-12,9,6,GOLD);px(c,inset+32,ground-10,7,3,GColorBlack);px(c,inset+35,ground-10,1,2,GOLD);}
 // Sixty floor stones, six separated rows of ten, carry minute value only.
 int left=(w-56)/2;
 for(int i=0;i<60;i++){int x=left+(i%10)*6,y=ground+3+(i/10)*4;bool on=i<a.minute;
 if(on){px(c,x,y,2,2,RUNE);px(c,x+2,y+1,1,1,RUNE);}else px(c,x,y+1,1,1,SEAM);}
 APP_LOG(APP_LOG_LEVEL_INFO,"Dungeon time=%02d:%02d:%02d kind=%d phase=%d heap=%lu",t.tm_hour,t.tm_min,t.tm_sec,a.kind,a.phase,(unsigned long)heap_bytes_free());
}
static void tick(struct tm*t,TimeUnits u){if(active)layer_mark_dirty(canvas);(void)t;(void)u;}
static void focus(bool v){active=v;if(v&&canvas)layer_mark_dirty(canvas);}
static void load(Window*win){GRect b=layer_get_bounds(window_get_root_layer(win));w=b.size.w/2;h=b.size.h/2;ox=(b.size.w-w*2)/2;oy=(b.size.h-h*2)/2;canvas=layer_create(b);layer_set_update_proc(canvas,draw);layer_add_child(window_get_root_layer(win),canvas);}
static void unload(Window*win){layer_destroy(canvas);canvas=NULL;(void)win;}
int main(void){window=window_create();window_set_window_handlers(window,(WindowHandlers){.load=load,.unload=unload});window_stack_push(window,false);active=true;tick_timer_service_subscribe(SECOND_UNIT,tick);app_focus_service_subscribe(focus);app_event_loop();tick_timer_service_unsubscribe();app_focus_service_unsubscribe();window_destroy(window);}

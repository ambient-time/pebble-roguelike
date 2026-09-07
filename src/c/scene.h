// Copyright 2026 Luke Steuber. MIT License.
#pragma once
#include <stdint.h>
typedef struct { int hour,minute,second,kind,phase,heroX,bossAlive;uint32_t seed; } Scene;
Scene scene_at(int hour,int minute,int second,uint32_t day);

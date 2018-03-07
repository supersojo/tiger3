/* Coding: ANSI */
#ifndef TIGER_TYPE_H
#define TIGER_TYPE_H

/* type specification */
typedef          char s8;
typedef unsigned char u8;
typedef          short s16;
typedef unsigned short u16;
typedef          int s32;
typedef unsigned int u32;

typedef long long s64;
typedef unsigned long long u64;

/* type conversion macros */
#define CAST(t,p) dynamic_cast<t>(p)

#endif




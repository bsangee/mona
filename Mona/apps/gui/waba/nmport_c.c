/*
Copyright (C) 1998, 1999, 2000 Wabasoft

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later version. 

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details. 

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA. 
*/

//#ifdef COMMENT
/*

If you're looking here, you've probably already looked at (or worked on) the
nm<platform>_b.c file. If you've gotten the _a and _b files working, this is
the easy part. This file contains all the native functions that are part of
the waba core framework. When you've added all the native functions then you're
done porting the VM.

The native functions that need to be ported are all those listed in the waba.c
native function table. There are stubs included in this file to make things
a bit easier than starting from scratch.

Some rules:

As a rule for native functions, don't hold a pointer across any call
that could garbage collect. For example, this is bad:

ptr = WOBJ_arrayStart(array)
...
string = createString(..)
ptr[0]

since the createString() could GC, the ptr inside of array could be invalid
after the call since a GC would move memory around. Instead, use:

ptr = WOBJ_arrayStart(array)
...
string = createString(..)
...
ptr = WOBJ_arrayStart(array)
ptr[0]

to recompute the pointer after the possible GC. The main thing to know there is
that when you call VM functions, it might garbage collect, so you want to 
recompute any pointers again after that function returns since the memory
locations might have changed. When you don't call VM functions that garbage
collect, there isn't a problem, because the VM is all single threaded. Things
don't move around on you unless you call a function that can garbage collect.

Another thing to note is that if you subclass a class with an object
destroy function, you must explicity call your superclasses objects
destroy functions. This isn't done automatically. See one of the objects that does
that in nmwin32_c.c for reference.

Before jumping into writing native functions, we need to look at 'classHooks'.
A classHooks array contains 'class hooks' like this:

ClassHook classHooks[] =
	{
	{ "waba/fx/Graphics", NULL, 11},
	{ "waba/fx/Image", ImageDestroy, 1 },
	{ "waba/io/Catalog", CatalogDestroy, 7 },
	{ "waba/io/Socket", SocketDestroy, 2 }, 
	{ "waba/io/SerialPort", SerialPortDestroy, 2 }, 
	{ NULL, NULL }
	};

You will need to define a classHooks array. Its an array of triples:

- a class name
- a object destroy function
- a number of native variables

You need a class hook when an object needs to allocate some native system
resource which it needs to use later on and/or needs to be freed.

For example, a Socket object will probably need to keep around a reference
to a socket descriptor around. And when a socket object is garbage
collected, it should close the socket.

So, it needs a classHook. In the above, the destroy method for a socket
is SocketDestroy and it will get called when the object is garbage
collected. The classHook for the Socket class above has 2 native
variables associated with it. Each of these native variables
are 32 bit values. One could hold a socket descriptor and the
other could hold something else. You can see nmpalm_c.c or nmwin32_c.c
for more on how that works for sockets.

So, how do you access a class hook variable or a variable that is in
a waba object? You can access a waba object's variables directly like this:

#define WOBJ_RectX(o) (objectPtr(o))[1].intValue

The objectPtr(o) gets a pointer to the data in an object and the [1]
references the second value in the object. The first value is a pointer
to the class of the object so the WOBJ_RectX(o) above is a macro that
pulls out the first actual variable in the object which is its X value.

If you look in the Rect class, you'll see that it looks like:

class Rect
{
int x;
int y;
..

So, the first variable - (objectPtr(o))[1].intValue is the x value.

The hook variables get stuck in after all the normal variables. If you look in the
nmwin32_c.c or nmpalm_c.c code, you'll see things like this:

//
// Graphics
//
// var[0] = Class
// var[1] = Surface
// var[2] = hook var - 1 for window surface and 2 for image
// var[3] = hook var - rgb
// var[4] = hook var - has clip
// var[5] = hook var - clipX
// ...

and then:

#define WOBJ_GraphicsSurface(o) (objectPtr(o))[1].obj
#define WOBJ_GraphicsSurfType(o) (objectPtr(o))[2].intValue
#define WOBJ_GraphicsRGB(o) (objectPtr(o))[3].intValue
#define WOBJ_GraphicsHasClip(o) (objectPtr(o))[4].intValue
#define WOBJ_GraphicsClipX(o) (objectPtr(o))[5].intValue
#define WOBJ_GraphicsClipY(o) (objectPtr(o))[6].intValue
#define WOBJ_GraphicsClipWidth(o) (objectPtr(o))[7].intValue

See how the hook variables start right after the last variable in the
class. The above are macros that access the value of a waba object in C code.
Making macros like the above makes the code easier to read.

With the macros, we can inspect a graphic's object clipX with:

int32 x = WObjectGraphicClipX(object);

Of course, if you change the Graphics object and add a new variable before the
surface variable or if you add a new variable in a base class, you need to
recompute those mappings or everything will get messed up.

The best way to port the native functions is to start out by making something
that doesn't work but does compile and then take code from the other native
VM's implementations and hack it up one class at a time. It's probably best
to start with the window and drawing classes so you can see something when
you start out.

*/
//#endif COMMENT

// WHEN PORTING: You'll probably need classHooks for these things when you get
// things going. Here we've allocate no hook variables for each object but have
// assigned some object destructor functions so when you actually hook some
// data in there, you'll need to set the 0 values to something else

/* static */ void GraphicsDestroy(WObject obj);
/* static */ void ImageDestroy(WObject obj);
/* static */ void CatalogDestroy(WObject obj);
/* static */ void SocketDestroy(WObject obj);
/* static */ void SerialPortDestroy(WObject obj);

ClassHook classHooks[] =
	{
	{ "waba/fx/Graphics", NULL, 0 },
	{ "waba/fx/Image", ImageDestroy, 0 },
	{ "waba/io/Catalog", CatalogDestroy, 0 },
	{ "waba/io/Socket", SocketDestroy, 0 }, 
	{ "waba/io/SerialPort", SerialPortDestroy,  }, 
	{ NULL, NULL }
	};

//
// Rect
//
// var[0] = Class
// var[1] = int x
// var[2] = int y
// var[3] = int width
// var[4] = int height

#define WOBJ_RectX(o) (objectPtr(o))[1].intValue
#define WOBJ_RectY(o) (objectPtr(o))[2].intValue
#define WOBJ_RectWidth(o) (objectPtr(o))[3].intValue
#define WOBJ_RectHeight(o) (objectPtr(o))[4].intValue

//
// Control
//
// var[0] = Class
// var[1] = int x
// var[2] = int y
// var[3] = int width
// var[4] = int height

#define WOBJ_ControlX(o) (objectPtr(o))[1].intValue
#define WOBJ_ControlY(o) (objectPtr(o))[2].intValue
#define WOBJ_ControlWidth(o) (objectPtr(o))[3].intValue
#define WOBJ_ControlHeight(o) (objectPtr(o))[4].intValue

//
// Window
//

/* static */ Var WindowCreate(Var stack[])
	{
	Var v;
	WObject win;

#if defined(OSASK)
	// ウィンドウのオープン
	window = lib_openwindow(AUTO_MALLOC, 0x0210, g_mainWinWidth, g_mainWinHeight);

	// ウィンドウタイトル用のテキストボックスをオープン
	wintitle = lib_opentextbox(0x1000, AUTO_MALLOC, 0, 4, 1, 0, 0, window, 0x00c0, 0);

	// グラフィックボックスのオープン
	//gbox = lib_opengraphbox(1, AUTO_MALLOC, 1, 8, g_mainWinWidth, g_mainWinHeight, 0, 0, window);
	gbox = lib_opengraphbox(1, AUTO_MALLOC, 4, 0xC6C6C6, g_mainWinWidth, g_mainWinHeight, 0, 0, window);

	// 文字描画用グラフィックボックスのオープン
	//sbox = xmalloc(sizeof (struct LIB_GRAPHBOX) + g_mainWinWidth * g_mainWinHeight);

	// ウィンドウタイトルを表示
	lib_putstring_ASCII(0x0000, 0, 0, wintitle, 0, 0, "waba");

	// サウンドトラックのオープン
	lib_opensoundtrack(0x0250);

	// シグナルの定義
	// タイマーオープン
	lib_opentimer(0x01c0 /* SYSTEM_TIMER */);

	//キーシグナル定義
	//ESC〜DOWNを32〜79に設定
	lib_definesignal1p0(47, 0x0100, 0x80, window, 32);
	//ASCIIコードをに80〜173設定
	lib_definesignal1p0(0x7e - 0x20, 0x0100, 0x20, window, 48 + 0x20);

	// クリックとドラッグと自由移動(ウィンドウ内のみ)
	lib_definesignal1p0(0, 0x0200, 0x2010, window, COORDINATE);  /* 座標 */
	lib_definesignal1p0(0, 0x0200, 0x2011, window, OUT_WINDOW);  /* ウィンドウ外 */
	lib_definesignal1p0(0, 0x0200, 0x2020, window, LEFT_DOWN);   /* 左プレス */
	lib_definesignal1p0(0, 0x0200, 0x2021, window, CENTER_DOWN); /* 右プレス */
	lib_definesignal1p0(0, 0x0200, 0x2022, window, RIGHT_DOWN);  /* 中プレス */
	lib_definesignal1p0(0, 0x0200, 0x2030, window, LEFT_UP);     /* 左リリース */
	lib_definesignal1p0(0, 0x0200, 0x2031, window, CENTER_UP);   /* 右リリース */
	lib_definesignal1p0(0, 0x0200, 0x2032, window, RIGHT_UP);    /* 中リリース */

	// システムタイマー
	lib_definesignal1p0(0, 0x0010 /* timer */, 0x01c0 /* SYSTEM_TIMER */, 0, TIMER_SIGNAL);

	//割り当て終了通知
	lib_definesignal0p0(0, 0, 0, 0);
#endif

	// ウィンドウサイズの設定
	win = stack[0].obj;
	WOBJ_ControlX(win) = 0;
	WOBJ_ControlY(win) = 0;
	WOBJ_ControlWidth(win) = g_mainWinWidth;
	WOBJ_ControlHeight(win) = g_mainWinHeight;

	v.obj = 0;
	return v;
	}

//
// MainWindow
//

/* static */ Var MainWinCreate(Var stack[])
	{
	Var v;

	printf("MainWinCreate\n");
#if defined(MONA) || defined(MONAGUI)
	// 領域を灰色で塗りつぶす
	my_fillrect0(0, 0, g_mainWinWidth, g_mainWinHeight, getRGB16(200,200,200));
	// 枠線を描く
	//my_drawline0(-1, -1, g_mainWinWidth + 1, -1, getRGB16(0,0,0));
	//my_drawline0(-1, -1, -1, g_mainWinHeight + 1, getRGB16(0,0,0));
	//my_drawline0(g_mainWinWidth, -1, g_mainWinWidth, g_mainWinHeight, getRGB16(0,0,0));
	//my_drawline0(-1, g_mainWinHeight, g_mainWinWidth, g_mainWinHeight, getRGB16(0,0,0));
#endif
	v.obj = 0;
	return v;
	}

/* static */ Var MainWinExit(Var stack[])
	{
	Var v;
	int exitCode;

	v.obj = 0;
	exitCode = stack[1].intValue;
	my_exit(exitCode);
	return v;
	}

/* static */ Var MainWinSetTimerInterval(Var stack[])
	{
	Var v;
	int32 millis;

	v.obj = 0;
	millis = stack[1].intValue;
	g_mainWinTimerId = millis;
	if (millis > 0)
		setsystimer(millis);
	return v;
	}

//
// Surface
//

#define SURF_WINDOW 1
#define SURF_IMAGE 2

/* static */ WClass *mainWinClass = 0;
/* static */ WClass *imageClass = 0;

/* static */ int SurfaceGetType(WObject surface)
	{
	WClass *wclass;

	if (surface == 0)
		return 0;

	// cache class pointers for performance
	if (!mainWinClass)
		mainWinClass = getClass(createUtfString("waba/ui/Window"));
	if (!imageClass)
		imageClass = getClass(createUtfString("waba/fx/Image"));

	wclass = (WClass *)WOBJ_class(surface);
	if (compatible(wclass, mainWinClass))
		return SURF_WINDOW;
	if (compatible(wclass, imageClass))
		return SURF_IMAGE;
	return 0;
	}

//
// Font
//
// var[0] = Class
// var[1] = String name
// var[2] = int size
// var[3] = int style
//

#define WOBJ_FontName(o) (objectPtr(o))[1].obj
#define WOBJ_FontStyle(o) (objectPtr(o))[2].intValue
#define WOBJ_FontSize(o) (objectPtr(o))[3].intValue 
#define Font_PLAIN 0
#define Font_BOLD 1

//
// FontMetrics
//

#define WOBJ_FontMetricsFont(o) (objectPtr(o))[1].obj
#define WOBJ_FontMetricsSurface(o) (objectPtr(o))[2].obj
#define WOBJ_FontMetricsAscent(o) (objectPtr(o))[3].intValue
#define WOBJ_FontMetricsDescent(o) (objectPtr(o))[4].intValue
#define WOBJ_FontMetricsLeading(o) (objectPtr(o))[5].intValue

/* static */ Var FontMetricsCreate(Var stack[])
	{
	WObject fontMetrics;
	Var v;

	fontMetrics = stack[0].obj;
	WOBJ_FontMetricsAscent(fontMetrics) = 16;
	WOBJ_FontMetricsDescent(fontMetrics) = 0;
	WOBJ_FontMetricsLeading(fontMetrics) = 0;
	v.obj = 0;
	return v;
	}

#define FM_STRINGWIDTH 1
#define FM_CHARARRAYWIDTH 2
#define FM_CHARWIDTH 3

/* static */ Var FontMetricsGetWidth(int type, Var stack[])
	{
	Var v;
	int32 width;

	switch (type)
	{
	case FM_CHARWIDTH:
		width = 8;
		break;
	case FM_STRINGWIDTH:
	case FM_CHARARRAYWIDTH:
		{
		WObject string, charArray;
		int32 start, count;

		width = 0;
		if (type == FM_STRINGWIDTH)
			{
			string = stack[1].obj;
			if (string == 0)
				break;
			charArray = WOBJ_StringCharArrayObj(string);
			if (charArray == 0)
				break;
			width = 8 * WOBJ_arrayLen(charArray);
			}
		else // FM_CHARARRAYWIDTH
			{
			charArray = stack[1].obj;
			start = stack[2].intValue;
			count = stack[3].intValue;
			width = 8 * count;
			}
		break;
		}
	default:
		width = 0;
	}
	v.intValue = width;
	return v;
	}

/* static */ Var FontMetricsGetStringWidth(Var stack[])
	{
	return FontMetricsGetWidth(FM_STRINGWIDTH, stack);
	}

/* static */ Var FontMetricsGetCharArrayWidth(Var stack[])
	{
	return FontMetricsGetWidth(FM_CHARARRAYWIDTH, stack);
	}

/* static */ Var FontMetricsGetCharWidth(Var stack[])
	{
	return FontMetricsGetWidth(FM_CHARWIDTH, stack);
	}

//
// Image
//

/* static */ Var ImageFree(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var ImageLoad(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var ImageCreate(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ void ImageDestroy(WObject image)
	{
	Var v;

	v.obj = 0;
	return;// v;
	}

/* static */ Var ImageSetPixels(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ void ImageLoadBMP(WObject image, uchar *p)
	{
	Var v;

	v.obj = 0;
	return;// v;
	}

//
// Graphics
//

// var[0] = Class
// var[1] = Surface
// var[2] = hook var - 1 for window surface and 2 for image
// var[3] = hook var - rgb (24bit)
// var[4] = hook var - has clip
// var[5] = hook var - clipX
// var[6] = hook var - clipY
// var[7] = hook var - clipWidth
// var[8] = hook var - clipHeight
// var[9] = hook var - font style
// var[10] = hook var - drawing op
// var[11] = hook var - x translation
// var[12] = hook var - y translation

#define WOBJ_GraphicsSurface(o) (objectPtr(o))[1].obj
#define WOBJ_GraphicsSurfType(o) (objectPtr(o))[2].intValue
#define WOBJ_GraphicsRGB(o) (objectPtr(o))[3].intValue
#define WOBJ_GraphicsHasClip(o) (objectPtr(o))[4].intValue
#define WOBJ_GraphicsClipX(o) (objectPtr(o))[5].intValue
#define WOBJ_GraphicsClipY(o) (objectPtr(o))[6].intValue
#define WOBJ_GraphicsClipWidth(o) (objectPtr(o))[7].intValue
#define WOBJ_GraphicsClipHeight(o) (objectPtr(o))[8].intValue
#define WOBJ_GraphicsFontStyle(o) (objectPtr(o))[9].intValue
#define WOBJ_GraphicsDrawOp(o) (objectPtr(o))[10].intValue
#define WOBJ_GraphicsTransX(o) (objectPtr(o))[11].intValue
#define WOBJ_GraphicsTransY(o) (objectPtr(o))[12].intValue

#define GR_FILLRECT   0
#define GR_DRAWLINE   1
#define GR_FILLPOLY   2
#define GR_DRAWCHARS  3
#define GR_DRAWSTRING 4
#define GR_DOTS       5
#define GR_COPYRECT   6
#define GR_DRAWCURSOR 7

/* static */ Var GraphicsCreate(Var stack[])
	{
	Var v;
	WObject gr, surface;

	v.obj = 0;
	gr = stack[0].obj;
	//printf("GraphicsCreate gr %d\n", gr);
	surface = WOBJ_GraphicsSurface(gr);
	WOBJ_GraphicsSurfType(gr) = SurfaceGetType(surface);
	//WOBJ_GraphicsRGB(gr) = 0;
	//WOBJ_GraphicsHasClip(gr) = 0;
	//WOBJ_GraphicsClipX(gr) = 0;
	//WOBJ_GraphicsClipY(gr) = 0;
	//WOBJ_GraphicsClipWidth(gr) = g_mainWinWidth;
	//WOBJ_GraphicsClipHeight(gr) = g_mainWinHeight;
	//WOBJ_GraphicsFontStyle(gr) = Font_PLAIN;
	//WOBJ_GraphicsDrawOp(gr) = DRAW_OVER;
	//WOBJ_GraphicsTransX(gr) = 0;
	//WOBJ_GraphicsTransY(gr) = 0;
	GraphicsRGB16  = 0;
	GraphicsDrawOP = DRAW_OVER;
	GraphicsTransX = 0;
	GraphicsTransY = 0;
#if 0
	int32 surfaceType;
	surfaceType = WOBJ_GraphicsSurfType(gr);
	printf("GraphicsCreate %d,%d,%d,%d\n", surfaceType, GraphicsRGB16, GraphicsTransX, GraphicsTransY);
#endif
	return v;
	}

/* static */ void GraphicsDestroy(WObject obj){
}

#define GraphicsFree Return0Func

/* static */ Var GraphicsSetFont(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var GraphicsSetColor(Var stack[])
	{
	Var v;
	WObject gr;
	int32 r, g, b;

	v.obj = 0;
	gr = stack[0].obj;
	r = stack[1].intValue;
	g = stack[2].intValue;
	b = stack[3].intValue;
	//GraphicsRGB16 = 0xFF000000 | Color::rgb(r, g, b);
	GraphicsRGB16 = getRGB16(r,g,b);
	return v;
	}

/* static */ Var GraphicsSetDrawOp(Var stack[])
	{
	Var v;
	WObject gr;
	int32 op;

	gr = stack[0].obj;
	op = stack[1].intValue;
	//WOBJ_GraphicsDrawOp(gr) = op;
	GraphicsDrawOP = op;
	v.obj = 0;
	return v;
	}

/* static */ Var GraphicsSetClip(Var stack[])
	{
	Var v;
	WObject gr;
	//int32 GraphicsTransX, GraphicsTransY;

	gr = stack[0].obj;
	//printf("GraphicsSetClip gr %d\n", gr);
	//GraphicsTransX = WOBJ_GraphicsTransX(gr);
	//GraphicsTransY = WOBJ_GraphicsTransY(gr);
	//printf("%d,%d,%d,%d,%d,%d\n", 
	//	GraphicsTransX, GraphicsTransY, stack[1].intValue, stack[2].intValue, stack[3].intValue, stack[4].intValue);
	// clip X and Y are stored in absolute coordinates
	/*WOBJ_GraphicsHasClip(gr) = 1;
	WOBJ_GraphicsClipX(gr) = stack[1].intValue + GraphicsTransX;
	WOBJ_GraphicsClipY(gr) = stack[2].intValue + GraphicsTransY;
	WOBJ_GraphicsClipWidth(gr) = stack[3].intValue;
	WOBJ_GraphicsClipHeight(gr) = stack[4].intValue;*/

	v.obj = 0;
	return v;
	}

/* static */ Var GraphicsGetClip(Var stack[])
	{
	Var v;
	WObject gr;//, rect;

	gr = stack[0].obj;
	//printf("GraphicsGetClip gr %d\n", gr);
	/*rect = stack[1].obj;
	if (rect == 0 || WOBJ_GraphicsHasClip(gr) != 1)
		return v;
	WOBJ_RectX(rect) = WOBJ_GraphicsClipX(gr) - WOBJ_GraphicsTransX(gr);
	WOBJ_RectY(rect) = WOBJ_GraphicsClipY(gr) - WOBJ_GraphicsTransY(gr);
	WOBJ_RectWidth(rect) = WOBJ_GraphicsClipWidth(gr);
	WOBJ_RectHeight(rect) = WOBJ_GraphicsClipHeight(gr);*/
	v.obj = 0;
	return v;
	}

/* static */ Var GraphicsClearClip(Var stack[])
	{
	Var v;
	WObject gr;
	//int32 GraphicsTransX, GraphicsTransY;

	gr = stack[0].obj;
	//printf("GraphicsClearClip gr %d\n", gr);
	//GraphicsTransX = WOBJ_GraphicsTransX(gr);
	//GraphicsTransY = WOBJ_GraphicsTransY(gr);
	// clip X and Y are stored in absolute coordinates
	//WOBJ_GraphicsHasClip(gr) = 0;
	//WOBJ_GraphicsClipX(gr) = GraphicsTransX;
	//WOBJ_GraphicsClipY(gr) = GraphicsTransY;
	//WOBJ_GraphicsClipWidth(gr) = g_mainWinWidth;
	//WOBJ_GraphicsClipHeight(gr) = g_mainWinHeight;
	v.obj = 0;
	return v;
	}

/* static */ Var GraphicsTranslate(Var stack[])
	{
	WObject gr;
	Var v;

	gr = stack[0].obj;
	//printf("GraphicsTranslate gr %d\n", gr);
	/*WOBJ_GraphicsTransX(gr) += stack[1].intValue;
	WOBJ_GraphicsTransY(gr) += stack[2].intValue;*/
	GraphicsTransX += stack[1].intValue;
	GraphicsTransY += stack[2].intValue;
	v.obj = 0;
	return v;
	}

/* static */ Var GraphicsDraw(int type, Var stack[])
	{
	Var v;
	WObject gr, surface;
	int32 surfaceType;

	gr = stack[0].obj;
	//printf("GraphicsDraw gr %d\n", gr);
	surface = WOBJ_GraphicsSurface(gr);
	surfaceType = WOBJ_GraphicsSurfType(gr);
	if(surfaceType != SURF_WINDOW && surfaceType != SURF_IMAGE){
		surfaceType = SURF_WINDOW;
	}
	v.obj = 0;
	//printf("GraphicsDraw %d,%d,%d,%d\n", surfaceType, GraphicsRGB16, GraphicsTransX, GraphicsTransY);
	switch(type) {
		case GR_FILLRECT:
			{
			int x, y, w, h;
			
			x = stack[1].intValue + GraphicsTransX;
			y = stack[2].intValue + GraphicsTransY;
			w = stack[3].intValue;
			h = stack[4].intValue;
			//printf("GR_FILLRECT %d,%d,%d,%d\n", x, y, w, h);
			my_fillrect0(x, y, w, h, GraphicsRGB16);
			break;
			}
		case GR_DRAWLINE:
		case GR_DOTS:
			{
			int x0, y0, x1, y1;
			
			x0 = stack[1].intValue + GraphicsTransX;
			y0 = stack[2].intValue + GraphicsTransY;
			x1 = stack[3].intValue + GraphicsTransX;
			y1 = stack[4].intValue + GraphicsTransY;
			//printf("GR_DRAWLINE %d,%d,%d,%d\n", x0, y0, x1, y1);
			if (surfaceType == SURF_WINDOW) {
				my_drawline0(x0, y0, x1, y1, GraphicsRGB16);
			} else if (surfaceType == SURF_IMAGE) {
				my_drawline0(x0, y0, x1, y1, GraphicsRGB16);
			}
			break;
			}
		case GR_FILLPOLY:
			{
			WObject xArray, yArray;
			int32 count, *x, *y;
			int i, x0, y0, x1, y1;

			xArray = stack[1].obj;
			yArray = stack[2].obj;
			if (xArray == 0 || yArray == 0) break;
			x = (int32 *)WOBJ_arrayStart(xArray);
			y = (int32 *)WOBJ_arrayStart(yArray);
			count = stack[3].intValue;
			if (count < 3 || count > WOBJ_arrayLen(xArray) || count > WOBJ_arrayLen(yArray))
				break;
			for (i = 0; i < count; i++) {
				if(i == count - 1){
					x0 = x[0] + GraphicsTransX;
					y0 = y[0] + GraphicsTransY;
					x1 = x[i] + GraphicsTransX;
					y1 = y[i] + GraphicsTransY;
				}else{
					x0 = x[ i ] + GraphicsTransX;
					y0 = y[ i ] + GraphicsTransY;
					x1 = x[i+1] + GraphicsTransX;
					y1 = y[i+1] + GraphicsTransY;
				}
				//printf("GR_DRAWLINE %d,%d,%d,%d\n", x0, y0, x1, y1);
				if (surfaceType == SURF_WINDOW) {
					my_drawline0(x0, y0, x1, y1, GraphicsRGB16);
				} else if (surfaceType == SURF_IMAGE) {
					my_drawline0(x0, y0, x1, y1, GraphicsRGB16);
				}
			}
			break;
			}
		case GR_DRAWCHARS:
		case GR_DRAWSTRING:
			{
			WObject string, charArray;
			int32 start, count;
			int i, j, x, y;
			char buf[80];
			uint16* chars;
			
			if (type == GR_DRAWSTRING) {
				string = stack[1].obj;
				if (string == 0)
					break;
				x = stack[2].intValue;
				y = stack[3].intValue;
				charArray = WOBJ_StringCharArrayObj(string);
				if (charArray == 0)
					break;
				start = 0;
				count = WOBJ_arrayLen(charArray);
			} else {
				charArray = stack[1].obj;
				start = stack[2].intValue;
				count = stack[3].intValue;
				x = stack[4].intValue;
				y = stack[5].intValue;
				if (arrayRangeCheck(charArray, start, count) == 0)
					break; // array null or range invalid
			}
			if (count == 0 || x > g_mainWinWidth || y > g_mainWinHeight) return v;
			chars = (uint16 *)WOBJ_arrayStart(charArray);
			chars = &chars[start];
			for (i = start, j = 0; i < count; i++, j++) {
				buf[j] = (char)(chars[i] & 0xFF);
			}
			buf[j] = '\0';
			x += GraphicsTransX;
			y += GraphicsTransY;
			if (0 <= x && x < g_mainWinWidth && 0 <= y && y < g_mainWinHeight) {
				//printf("GR_DRAWSTRING %d,%d\n", x, y);
				my_drawstr0(x, y, buf, GraphicsRGB16);
			}
			break;
			}
		case GR_COPYRECT:
			{
			/*WObject surface0;
			int surfaceType0, x, y, w, h, dx, dy;
			int32 *ibox_ptr0;

			surface0 = stack[1].obj;
			surfaceType0 = SurfaceGetType(surface0);
			x = stack[2].intValue + GraphicsTransX;
			y = stack[3].intValue + GraphicsTransY;
			w = stack[4].intValue;
			h = stack[5].intValue;
			dx = stack[6].intValue + GraphicsTransX;
			dy = stack[7].intValue + GraphicsTransY;*/
			break;
			}
		case GR_DRAWCURSOR:
			{
			int x0, y0, x1, y1;
			x0 = stack[1].intValue + GraphicsTransX;
			y0 = stack[2].intValue + GraphicsTransY;
			x1 = stack[3].intValue + x0 - 1;
			y1 = stack[4].intValue + y0 - 1;
			//printf("GR_DRAWCURSOR %d,%d,%d,%d\n", x0, y0, x1, y1);
			my_drawline0(x0, y0, x1, y1, GraphicsRGB16);
			break;
			}
		}
	return v;
	}

/* static */ Var GraphicsFillRect(Var stack[])
	{
	return GraphicsDraw(GR_FILLRECT, stack);
	}

/* static */ Var GraphicsDrawLine(Var stack[])
	{
	return GraphicsDraw(GR_DRAWLINE, stack);
	}

/* static */ Var GraphicsFillPolygon(Var stack[])
	{
	return GraphicsDraw(GR_FILLPOLY, stack);
	}

/* static */ Var GraphicsDrawChars(Var stack[])
	{
	return GraphicsDraw(GR_DRAWCHARS, stack);
	}

/* static */ Var GraphicsDrawString(Var stack[])
	{
	return GraphicsDraw(GR_DRAWSTRING, stack);
	}

/* static */ Var GraphicsDrawDots(Var stack[])
	{
	return GraphicsDraw(GR_DOTS, stack);
	}

/* static */ Var GraphicsCopyRect(Var stack[])
	{
	return GraphicsDraw(GR_COPYRECT, stack);
	}

/* static */ Var GraphicsDrawCursor(Var stack[])
	{
	return GraphicsDraw(GR_DRAWCURSOR, stack);
	}

//
// File
//

// WHEN PORTING: Note, this was just another way of stubbing these functions out

#define FileGetLength Return0Func
#define FileCreateDir Return0Func
#define FileRead ReturnNeg1Func
#define FileCreate Return0Func
#define FileWrite ReturnNeg1Func
#define FileListDir Return0Func
#define FileIsDir Return0Func
#define FileClose Return0Func
#define FileDelete Return0Func
#define FileExists Return0Func
#define FileIsOpen Return0Func
#define FileSeek Return0Func
#define FileRename Return0Func

/* static */ Var Return0Func(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var ReturnNeg1Func(Var stack[])
	{
	Var v;

	v.intValue = -1;
	return v;
	}

//
// Socket
//

/* static */ Var SocketCreate(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ void SocketDestroy(WObject socket)
	{
	}

/* static */ Var SocketClose(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SocketIsOpen(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SocketSetReadTimeout(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SocketReadWriteBytes(Var stack[], int isRead)
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SocketRead(Var stack[])
	{
	return SocketReadWriteBytes(stack, 1);
	}

/* static */ Var SocketWrite(Var stack[])
	{
	return SocketReadWriteBytes(stack, 0);
	}

//
// Sound
//
// all functions static

/* static */ void NativeTone(int32 freq, int32 duration)
	{
#if defined(OSASK)
	/* NULLチェック */
	if (window == NULL) return;

	freq = 4294967290UL / freq /* 1/f */;

	lib_controlfreq(0x0250, freq);
	
	if(duration != 0)
		{
		wait(duration);
		lib_controlfreq(0x0250, 0);
		}
#endif
	}

/* static */ Var SoundTone(Var stack[])
	{
	Var v;
	int32 freq, duration;

	freq = stack[0].intValue;
	duration = stack[1].intValue;
	NativeTone(freq, duration);
	v.obj = 0;
	return v;
	}

/* static */ Var SoundBeep(Var stack[])
	{
	Var v;
	int32 freq, duration;

	freq = (int32)659.26/* E5 */;
	duration = 500;
	NativeTone(freq, duration);
	v.obj = 0;
	return v;
	}

//
// SoundClip
//

#define SoundClipPlay Return0Func

//
// Convert
//

/* static */ Var ConvertFloatToIntBitwise(Var stack[])
	{
	return stack[0];
	}

/* static */ Var ConvertIntToFloatBitwise(Var stack[])
	{
	return stack[0];
	}

/* static */ Var ConvertStringToInt(Var stack[])
	{
	WObject string, charArray;
	int32 i, isNeg, len, value;
	uint16 *chars;
	Var v;

	// NOTE: We do it all here instead of calling atoi() since it looks
	// like various versions of CE don't support atoi(). It's also faster
	// this way since we don't have to convert to a byte array.
	v.intValue = 0;
	string = stack[0].obj;
	if (string == 0)
		return v;
	charArray = WOBJ_StringCharArrayObj(string);
	if (charArray == 0)
		return v;
	chars = (uint16 *)WOBJ_arrayStart(charArray);
	len = WOBJ_arrayLen(charArray);
	isNeg = 0;
	if (len > 0 && chars[0] == '-')
		isNeg = 1;
	value = 0;
	for (i = isNeg; i < len; i++)
		{
		if (chars[i] < (uint16)'0' || chars[i] > (uint16)'9')
			return v;
		value = (value * 10) + ((int32)chars[i] - (int32)'0');
		}
	if (isNeg)
		value = -(value);
	v.intValue = value;
	return v;
	}

/* static */ Var ConvertIntToString(Var stack[])
	{
	Var v;
	char buf[20];

	sprintf(buf, "%d", stack[0].intValue);
	v.obj = createString(buf);
	return v;
	}

/* static */ Var ConvertFloatToString(Var stack[])
	{
	Var v;
	char buf[40];

	sprintf(buf, "%f", stack[0].floatValue);
	v.obj = createString(buf);
	return v;
	}

/* static */ Var ConvertCharToString(Var stack[])
	{
	Var v;
	char buf[2];

	buf[0] = (char)stack[0].intValue;
	buf[1] = 0;
	v.obj = createString(buf);
	return v;
	}

/* static */ Var ConvertBooleanToString(Var stack[])
	{
	Var v;
	char *s;

	if (stack[0].intValue == 0)
		s = "false";
	else
		s = "true";
	v.obj = createString(s);
	return v;
	}

//
// Catalog
//

/* static */ Var CatalogCreate(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogIsOpen(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ void CatalogDestroy(WObject cat)
	{
	}

/* static */ Var CatalogClose(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogDelete(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogListCatalogs(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogGetRecordSize(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogGetRecordCount(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogDeleteRecord(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogResizeRecord(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogAddRecord(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogSetRecordPos(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogReadWriteBytes(Var stack[], int isRead)
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogRead(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogWrite(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var CatalogSkipBytes(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

//
// Time
//

#define WOBJ_TimeYear(o) (objectPtr(o))[1].intValue
#define WOBJ_TimeMonth(o) (objectPtr(o))[2].intValue
#define WOBJ_TimeDay(o) (objectPtr(o))[3].intValue
#define WOBJ_TimeHour(o) (objectPtr(o))[4].intValue
#define WOBJ_TimeMinute(o) (objectPtr(o))[5].intValue
#define WOBJ_TimeSecond(o) (objectPtr(o))[6].intValue
#define WOBJ_TimeMillis(o) (objectPtr(o))[7].intValue

/* static */ Var TimeCreate(Var stack[])
	{
	Var v;
	WObject time;
	int year, month, day, hour, min ,sec;

	time = stack[0].obj;
	// 2004/01/01 12:34:56
	year = 2004;
	month = 1;
	day = 1;
	hour = 12;
	min = 34;
	sec = 56;
	WOBJ_TimeYear(time) = year;
	WOBJ_TimeMonth(time) = month;
	WOBJ_TimeDay(time) = day;
	WOBJ_TimeHour(time) = hour;
	WOBJ_TimeMinute(time) = min;
	WOBJ_TimeSecond(time) = sec;
	WOBJ_TimeMillis(time) = 0;
	v.obj = 0;
	return v;
	}

//
// SerialPort
//

/* static */ Var SerialPortCreate(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ void SerialPortDestroy(WObject port)
	{
	}

/* static */ Var SerialPortIsOpen(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SerialPortSetReadTimeout(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SerialPortReadCheck(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SerialPortSetFlowControl(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SerialPortReadWriteBytes(Var stack[], int isRead)
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var SerialPortRead(Var stack[])
	{
	return SerialPortReadWriteBytes(stack, 1);
	}

/* static */ Var SerialPortWrite(Var stack[])
	{
	return SerialPortReadWriteBytes(stack, 0);
	}

/* static */ Var SerialPortClose(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

//
// Vm
//

/* static */ Var VmIsColor()
	{
	Var v;

	//v.obj = 0;
	v.intValue = 1;
	return v;
	}

/* static */ Var VmGetTimeStamp(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var VmExec(Var stack[])
	{
	Var v;

	v.obj = 0;
	return v;
	}

/* static */ Var VmSleep(Var stack[])
	{
	Var v;

	v.obj = 0;
	wait(stack[0].intValue);
	return v;
	}

/* static */ Var VmGetPlatform(Var stack[])
	{
	Var v;

	v.obj = createString(VM_OS);
	return v;
	}

/* static */ Var VmSetDeviceAutoOff(Var stack[])
	{
	Var v;

	v.obj = 0;
	my_exit(1);
	return v;
	}

/* static */ Var VmGetUserName(Var stack[])
	{
	Var v;

	v.obj = createString(VM_USER);
	return v;
	}

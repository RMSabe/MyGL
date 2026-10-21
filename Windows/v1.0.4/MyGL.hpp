/*
	MyGL: Win32 OpenGL auxiliary resource
	Version 1.0.4

	This code uses my custom WinLib version 4.0.2
	GitHub Repository: https://github.com/RMSabe/WinLib/tree/main/v4.0.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

/*
	This resource uses the global variable "p_processheap".

	Make sure "p_processheap" is properly initialized by calling "p_processheap = GetProcessHeap();" prior to calling "MyGL::initialize()".
*/

#ifndef MYGL_HPP
#define MYGL_HPP

#include "globldef.h"
#include "strdef.hpp"

#include <GL/gl.h>

struct _mygl_coord {
	__declspec(align(4)) FLOAT x;
	__declspec(align(4)) FLOAT y;
};

typedef struct _mygl_coord mygl_coord_t;

__EXTERNC__ VOID WINAPI mygl_setcolor(COLORREF color);

class MyGL {
	public:
		MyGL(VOID);
		MyGL(HWND p_wnd);
		~MyGL(VOID);

		BOOL WINAPI initialize(VOID);
		VOID WINAPI deinitialize(VOID);

		BOOL WINAPI clearAll(VOID);
		LONG_PTR WINAPI getEntryCount(VOID);
		LONG_PTR WINAPI getEntryCount(ULONG_PTR *p_datasizebytes);

		BOOL WINAPI removeEntry(ULONG_PTR index);

		BOOL WINAPI addColorEntry(COLORREF color, BOOL updateGL);
		BOOL WINAPI addPointSizeEntry(FLOAT pointSize, BOOL updateGL);
		BOOL WINAPI addLineWidthEntry(FLOAT lineWidth, BOOL updateGL);

		BOOL WINAPI addPointObjectEntry(FLOAT x, FLOAT y, BOOL updateGL);
		BOOL WINAPI addPointObjectEntry(const mygl_coord_t *p_coord, BOOL updateGL);

		BOOL WINAPI addLineObjectEntry(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, BOOL updateGL);
		BOOL WINAPI addLineObjectEntry(const mygl_coord_t *p_coords, BOOL updateGL);

		BOOL WINAPI addTriangleObjectEntry(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, BOOL updateGL);
		BOOL WINAPI addTriangleObjectEntry(const mygl_coord_t *p_coords, BOOL updateGL);

		BOOL WINAPI addQuadObjectEntry(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, BOOL updateGL);
		BOOL WINAPI addQuadObjectEntry(const mygl_coord_t *p_coords, BOOL updateGL);

		BOOL WINAPI addPolygonObjectEntry(const mygl_coord_t *p_coords, UINT32 n_coords, BOOL updateGL);

		BOOL WINAPI paintBufferGL(VOID);

		BOOL WINAPI makeCurrentGL(VOID);
		BOOL WINAPI setColorGL(COLORREF color);

		BOOL WINAPI updateWndCtx(VOID);
		HDC WINAPI getWndDC(VOID);
		HGLRC WINAPI getWndRC(VOID);

		INT WINAPI getStatus(VOID);
		__string WINAPI getLastErrorMessage(VOID);

		__declspec(align(PTR_SIZE_BYTES)) HWND p_wnd = NULL;

		enum Status {
			STATUS_ERROR_MEMORY = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_OK = 1
		};

	private:
		static constexpr ULONG_PTR BUFFER_INIT_SIZE_BYTES = 4096u;

		enum EntryID {
			ENTRY_ID_NULL = -1,
			ENTRY_ID_COLOR = 0,
			ENTRY_ID_GLPOINTSIZE = 1,
			ENTRY_ID_GLLINEWIDTH = 2,
			ENTRY_ID_OBJECT = 3,
		};

		enum ObjectID {
			OBJECT_ID_NULL = -1,
			OBJECT_ID_POINT = GL_POINTS,
			OBJECT_ID_LINE = GL_LINES,
			OBJECT_ID_TRIANGLE = GL_TRIANGLES,
			OBJECT_ID_QUAD = GL_QUADS,
			OBJECT_ID_POLYGON = GL_POLYGON
		};

		__declspec(align(PTR_SIZE_BYTES)) VOID *p_buffer = NULL;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR buffer_size = 0u;

		__declspec(align(PTR_SIZE_BYTES)) HDC p_wnddc = NULL;
		__declspec(align(PTR_SIZE_BYTES)) HGLRC p_wndrc = NULL;

		__declspec(align(PTR_SIZE_BYTES)) __string err_msg = TEXT("");
		__declspec(align(4)) INT status = MyGL::STATUS_UNINITIALIZED;

		BOOL WINAPI buffer_alloc(VOID);
		BOOL WINAPI buffer_free(VOID);

		BOOL WINAPI buffer_resize(ULONG_PTR new_size_bytes);
		BOOL WINAPI buffer_resize_default(VOID);

		BOOL WINAPI wndctx_create(VOID);
		BOOL WINAPI wndctx_destroy(VOID);
		BOOL WINAPI wndctx_validate(VOID);

		BOOL WINAPI add_entry(const VOID *p_entry);

		LONG_PTR WINAPI buffer_get_byteindex_from_entryindex(ULONG_PTR index);
};

#endif /*MYGL_HPP*/

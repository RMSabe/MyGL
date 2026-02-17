/*
	MyGL: Win32 OpenGL auxiliary resource
	Version 1.0.2

	This code uses my custom WinLib version 5.0
	GitHub Repository: https://github.com/RMSabe/WinLib/tree/main/v5.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "MyGL.hpp"

struct _mygl_entry_base {
	INT32 entry_id;
	UINT32 size;
};

typedef struct _mygl_entry_base mygl_entry_base_t;

constexpr ULONG_PTR __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET = 0U;
constexpr ULONG_PTR __MYGL_ENTRYBASE_SIZE_BYTEOFFSET = 4U;

struct _mygl_entry_color {
	mygl_entry_base_t base;
	UINT32 colorref;
};

typedef struct _mygl_entry_color mygl_entry_color_t;

constexpr ULONG_PTR __MYGL_ENTRYCOLOR_COLORREF_BYTEOFFSET = sizeof(mygl_entry_base_t);

struct _mygl_entry_pixelsize {
	mygl_entry_base_t base;
	FLOAT pixelsize;
};

typedef struct _mygl_entry_pixelsize mygl_entry_pixelsize_t;

constexpr ULONG_PTR __MYGL_ENTRYPIXELSIZE_PIXELSIZE_BYTEOFFSET = sizeof(mygl_entry_base_t);

struct _mygl_entry_baseobj {
	mygl_entry_base_t base;
	INT32 object_id;
};

typedef struct _mygl_entry_baseobj mygl_entry_baseobj_t;

constexpr ULONG_PTR __MYGL_ENTRYBASEOBJ_OBJECTID_BYTEOFFSET = sizeof(mygl_entry_base_t);

struct _mygl_entry_pointobj {
	mygl_entry_baseobj_t baseobj;
	mygl_coord_t coord;
};

typedef struct _mygl_entry_pointobj mygl_entry_pointobj_t;

constexpr ULONG_PTR __MYGL_ENTRYPOINTOBJ_COORD_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

struct _mygl_entry_lineobj {
	mygl_entry_baseobj_t baseobj;
	mygl_coord_t coords[2];
};

typedef struct _mygl_entry_lineobj mygl_entry_lineobj_t;

constexpr ULONG_PTR __MYGL_ENTRYLINEOBJ_COORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

struct _mygl_entry_triangleobj {
	mygl_entry_baseobj_t baseobj;
	mygl_coord_t coords[3];
};

typedef struct _mygl_entry_triangleobj mygl_entry_triangleobj_t;

constexpr ULONG_PTR __MYGL_ENTRYTRIANGLEOBJ_COORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

struct _mygl_entry_quadobj {
	mygl_entry_baseobj_t baseobj;
	mygl_coord_t coords[4];
};

typedef struct _mygl_entry_quadobj mygl_entry_quadobj_t;

constexpr ULONG_PTR __MYGL_ENTRYQUADOBJ_COORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

/*
	mygl_entry_polygonobj !!theoretical!! {
		mygl_entry_baseobj_t; (byte index = 0)
		UINT32 n_coords; (byte index = 12)
		mygl_coord_t[]; (byte index = 16)
	};
*/

constexpr ULONG_PTR __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);
constexpr ULONG_PTR __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET = __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET + 4U;

VOID WINAPI mygl_setcolor(COLORREF color)
{
	FLOAT fred;
	FLOAT fgreen;
	FLOAT fblue;

	fred = ((FLOAT) *((UINT8*) &color))/255.0f;
	fgreen = ((FLOAT) *((UINT8*) (((ULONG_PTR) &color) + 1u)))/255.0f;
	fblue = ((FLOAT) *((UINT8*) (((ULONG_PTR) &color) + 2u)))/255.0f;

	glColor3f(fred, fgreen, fblue);
	return;
}

MyGL::MyGL(VOID)
{
}

MyGL::MyGL(HWND p_wnd) : MyGL()
{
	this->p_wnd = p_wnd;
}

MyGL::~MyGL(VOID)
{
	this->deinitialize();
}

BOOL WINAPI MyGL::initialize(VOID)
{
	if(this->status > 0) return TRUE;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->buffer_alloc())
	{
		this->err_msg = TEXT("MyGL::initialize: Error: memory allocate failed.");
		this->status = this->STATUS_ERROR_MEMALLOC;
		return FALSE;
	}

	this->status = this->STATUS_OK;
	return TRUE;
}

BOOL WINAPI MyGL::clearAll(VOID)
{
	if(this->status < 1) return FALSE;

	FillMemory(this->p_buffer, this->buffer_size, 0xff);

	return TRUE;
}

LONG_PTR WINAPI MyGL::getEntryCount(VOID)
{
	return this->getEntryCount(NULL);
}

LONG_PTR WINAPI MyGL::getEntryCount(ULONG_PTR *p_datasizebytes)
{
	ULONG_PTR byte_index;
	ULONG_PTR entry_size;
	ULONG_PTR n_entries;
	INT32 entry_id;

	if(this->status < 1) return -1;

	byte_index = 0u;
	n_entries = 0u;

	while(TRUE)
	{
		if(byte_index >= this->buffer_size) break;

		entry_id = *((INT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET));
		if(entry_id == this->ENTRY_ID_NULL) break;

		entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));
		byte_index += entry_size;
		n_entries++;
	}

	if(p_datasizebytes != NULL) *p_datasizebytes = byte_index;

	return (LONG_PTR) n_entries;
}

BOOL WINAPI MyGL::removeObject(ULONG_PTR index)
{
	HANDLE p_processheap = NULL;
	ULONG_PTR *p_entry_byte_indexes = NULL;
	ULONG_PTR byte_index;
	ULONG_PTR entry_index;
	ULONG_PTR entry_size;
	ULONG_PTR n_entries;
	ULONG_PTR n_byte;
	LONG_PTR n_tocopy_entries;

	if(this->status < 1) return FALSE;

	n_entries = (ULONG_PTR) this->getEntryCount(NULL);

	if(((LONG_PTR) n_entries) < 0) return FALSE;

	if(index >= n_entries)
	{
		this->err_msg = TEXT("MyGL::removeObject: Error: given object index is out of bounds");
		return FALSE;
	}

	byte_index = (ULONG_PTR) this->buffer_get_byteindex_from_entryindex(index);
	if(((LONG_PTR) byte_index) < 0) return FALSE;

	entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));

	n_tocopy_entries = (LONG_PTR) (n_entries - index - 1u);

	if(!n_tocopy_entries)
	{
		FillMemory((VOID*) (((ULONG_PTR) this->p_buffer) + byte_index), entry_size, 0xff);
		return TRUE;
	}

	/*If there are other valid objects after the one to be removed, move/append them to the buffer byte index after the last valid object*/

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("MyGL::removeObject: Error: failed to retrieve process heap.");
		return FALSE;
	}

	p_entry_byte_indexes = (ULONG_PTR*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, ((ULONG_PTR) n_tocopy_entries)*PTR_SIZE_BYTES);
	if(p_entry_byte_indexes == NULL)
	{
		this->err_msg = TEXT("MyGL::removeObject: Error: memory allocate failed");
		return FALSE;
	}

	entry_index = index + 1u;
	while(entry_index < n_entries)
	{
		p_entry_byte_indexes[entry_index] = (ULONG_PTR) this->buffer_get_byteindex_from_entryindex(entry_index);
		if(((LONG_PTR) p_entry_byte_indexes[entry_index]) < 0)
		{
			HeapFree(p_processheap, 0u, p_entry_byte_indexes);
			return FALSE;
		}

		entry_index++;
	}

	entry_index = 0u;
	while(n_tocopy_entries > 0)
	{
		entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) this->p_buffer) + p_entry_byte_indexes[entry_index] + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));

		/*
			Technically, we could use CopyMemory() here.
			The problem is, because were moving data within the buffer (copying from buffer to a different position within the same buffer),
			the order of bytes being copied must be very specific.
			In this case, the order of bytes being copied must be from the first byte (index 0) to last byte (index entry_size - 1).

			Attempting to copy the data chunk in any other byte order might cause runtime errors.
		*/

		n_byte = 0u;
		while(n_byte < entry_size)
		{
			*((UINT8*) (((ULONG_PTR) this->p_buffer) + byte_index + n_byte)) = *((UINT8*) (((ULONG_PTR) this->p_buffer) + p_entry_byte_indexes[entry_index] + n_byte));
			n_byte++;
		}

		byte_index += entry_size;
		entry_index++;
		n_tocopy_entries--;
	}

	FillMemory((VOID*) (((ULONG_PTR) this->p_buffer) + byte_index), (this->buffer_size - byte_index), 0xff);

	HeapFree(p_processheap, 0u, p_entry_byte_indexes);

	return TRUE;
}

BOOL WINAPI MyGL::addColor(COLORREF color, BOOL updateGL)
{
	mygl_entry_color_t _color;

	if(this->status < 1) return FALSE;

	_color.base.entry_id = this->ENTRY_ID_COLOR;
	_color.base.size = sizeof(mygl_entry_color_t);
	_color.colorref = color;

	if(!this->add_entry(&_color)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();
	mygl_setcolor(color);

	return TRUE;
}

BOOL WINAPI MyGL::addPointSize(FLOAT pointSize, BOOL updateGL)
{
	mygl_entry_pixelsize_t _pixelsize;

	if(this->status < 1) return FALSE;

	_pixelsize.base.entry_id = this->ENTRY_ID_GLPOINTSIZE;
	_pixelsize.base.size = sizeof(mygl_entry_pixelsize_t);
	_pixelsize.pixelsize = pointSize;

	if(!this->add_entry(&_pixelsize)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();
	glPointSize(pointSize);

	return TRUE;
}

BOOL WINAPI MyGL::addLineWidth(FLOAT lineWidth, BOOL updateGL)
{
	mygl_entry_pixelsize_t _pixelsize;

	if(this->status < 1) return FALSE;

	_pixelsize.base.entry_id = this->ENTRY_ID_GLLINEWIDTH;
	_pixelsize.base.size = sizeof(mygl_entry_pixelsize_t);
	_pixelsize.pixelsize = lineWidth;

	if(!this->add_entry(&_pixelsize)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();
	glLineWidth(lineWidth);

	return TRUE;
}

BOOL WINAPI MyGL::addPointObject(FLOAT x, FLOAT y, BOOL updateGL)
{
	mygl_coord_t coord;

	if(this->status < 1) return FALSE;

	coord.x = x;
	coord.y = y;

	return this->addPointObject(&coord, updateGL);
}

BOOL WINAPI MyGL::addPointObject(const mygl_coord_t *p_coord, BOOL updateGL)
{
	mygl_entry_pointobj_t _point;

	if(this->status < 1) return FALSE;
	if(p_coord == NULL)
	{
		this->err_msg = TEXT("MyGL::addPointObject: Error: invalid pcoord parameter.");
		return FALSE;
	}

	_point.baseobj.base.entry_id = this->ENTRY_ID_OBJECT;
	_point.baseobj.base.size = sizeof(mygl_entry_pointobj_t);
	_point.baseobj.object_id = this->OBJECT_ID_POINT;
	_point.coord.x = p_coord->x;
	_point.coord.y = p_coord->y;

	if(!this->add_entry(&_point)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_POINTS);
	glVertex2f(p_coord->x, p_coord->y);
	glEnd();

	return TRUE;
}

BOOL WINAPI MyGL::addLineObject(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, BOOL updateGL)
{
	mygl_coord_t coords[2];

	if(this->status < 1) return FALSE;

	coords[0].x = x0;
	coords[0].y = y0;
	coords[1].x = x1;
	coords[1].y = y1;

	return this->addLineObject(coords, updateGL);
}

BOOL WINAPI MyGL::addLineObject(const mygl_coord_t *p_coords, BOOL updateGL)
{
	ULONG_PTR n_coord;
	mygl_entry_lineobj_t _line;

	if(this->status < 1) return FALSE;
	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addLineObject: Error: invalid pcoords parameter.");
		return FALSE;
	}

	_line.baseobj.base.entry_id = this->ENTRY_ID_OBJECT;
	_line.baseobj.base.size = sizeof(mygl_entry_lineobj_t);
	_line.baseobj.object_id = this->OBJECT_ID_LINE;

	CopyMemory(&(_line.coords), p_coords, 2u*sizeof(mygl_coord_t));

	if(!this->add_entry(&_line)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_LINES);

	for(n_coord = 0u; n_coord < 2u; n_coord++) glVertex2f(p_coords[n_coord].x, p_coords[n_coord].y);

	glEnd();

	return TRUE;
}

BOOL WINAPI MyGL::addTriangleObject(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, BOOL updateGL)
{
	mygl_coord_t coords[3];

	if(this->status < 1) return FALSE;

	coords[0].x = x0;
	coords[0].y = y0;
	coords[1].x = x1;
	coords[1].y = y1;
	coords[2].x = x2;
	coords[2].y = y2;

	return this->addTriangleObject(coords, updateGL);
}

BOOL WINAPI MyGL::addTriangleObject(const mygl_coord_t *p_coords, BOOL updateGL)
{
	ULONG_PTR n_coord;
	mygl_entry_triangleobj_t _triangle;

	if(this->status < 1) return FALSE;
	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addTriangleObject: Error: invalid pcoords parameter.");
		return FALSE;
	}

	_triangle.baseobj.base.entry_id = this->ENTRY_ID_OBJECT;
	_triangle.baseobj.base.size = sizeof(mygl_entry_triangleobj_t);
	_triangle.baseobj.object_id = this->OBJECT_ID_TRIANGLE;

	CopyMemory(&(_triangle.coords), p_coords, 3u*sizeof(mygl_coord_t));

	if(!this->add_entry(&_triangle)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_TRIANGLES);
	for(n_coord = 0u; n_coord < 3u; n_coord++) glVertex2f(p_coords[n_coord].x, p_coords[n_coord].y);
	glEnd();

	return TRUE;
}

BOOL WINAPI MyGL::addQuadObject(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, BOOL updateGL)
{
	mygl_coord_t coords[4];

	if(this->status < 1) return FALSE;

	coords[0].x = x0;
	coords[0].y = y0;
	coords[1].x = x1;
	coords[1].y = y1;
	coords[2].x = x2;
	coords[2].y = y2;
	coords[3].x = x3;
	coords[3].y = y3;

	return this->addQuadObject(coords, updateGL);
}

BOOL WINAPI MyGL::addQuadObject(const mygl_coord_t *p_coords, BOOL updateGL)
{
	ULONG_PTR n_coord;
	mygl_entry_quadobj_t _quad;

	if(this->status < 1) return FALSE;
	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addQuadObject: Error: invalid pcoords parameter.");
		return FALSE;
	}

	_quad.baseobj.base.entry_id = this->ENTRY_ID_OBJECT;
	_quad.baseobj.base.size = sizeof(mygl_entry_quadobj_t);
	_quad.baseobj.object_id = this->OBJECT_ID_QUAD;

	CopyMemory(&(_quad.coords), p_coords, 4u*sizeof(mygl_coord_t));

	if(!this->add_entry(&_quad)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_QUADS);
	for(n_coord = 0u; n_coord < 4u; n_coord++) glVertex2f(p_coords[n_coord].x, p_coords[n_coord].y);
	glEnd();

	return TRUE;
}

BOOL WINAPI MyGL::addPolygonObject(const mygl_coord_t *p_coords, UINT32 n_coords, BOOL updateGL)
{
	HANDLE p_processheap = NULL;
	VOID *p_entry = NULL;
	ULONG_PTR entry_size;
	ULONG_PTR n_coord;
	BOOL b_ret;

	if(this->status < 1) return FALSE;

	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addPolygonObject: Error: invalid pcoords parameter.");
		return FALSE;
	}

	if(!n_coords)
	{
		this->err_msg = TEXT("MyGL::addPolygonObject: Error: invalid ncoords parameter.");
		return FALSE;
	}

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("MyGL::addPolygonObject: Error: failed to retrieve process heap.");
		return FALSE;
	}

	entry_size = __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET + ((ULONG_PTR) n_coords)*sizeof(mygl_coord_t);

	p_entry = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, entry_size);
	if(p_entry == NULL)
	{
		this->err_msg = TEXT("MyGL::addPolygonObject: Error: memory allocate failed.");
		return FALSE;
	}

	*((INT32*) (((ULONG_PTR) p_entry) + __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET)) = this->ENTRY_ID_OBJECT;
	*((UINT32*) (((ULONG_PTR) p_entry) + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET)) = (UINT32) entry_size;
	*((INT32*) (((ULONG_PTR) p_entry) + __MYGL_ENTRYBASEOBJ_OBJECTID_BYTEOFFSET)) = this->OBJECT_ID_POLYGON;
	*((UINT32*) (((ULONG_PTR) p_entry) + __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET)) = n_coords;

	CopyMemory((VOID*) (((ULONG_PTR) p_entry) + __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET), p_coords, ((ULONG_PTR) n_coords)*sizeof(mygl_coord_t));

	b_ret = this->add_entry(p_entry);

	HeapFree(p_processheap, 0u, p_entry);
	p_entry = NULL;

	if(!b_ret) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_POLYGON);
	for(n_coord = 0u; n_coord < ((ULONG_PTR) n_coords); n_coord++) glVertex2f(p_coords[n_coord].x, p_coords[n_coord].y);
	glEnd();

	return TRUE;	
}

BOOL WINAPI MyGL::paintBufferGL(VOID)
{
	mygl_coord_t *p_coord = NULL;

	ULONG_PTR n_entries;
	ULONG_PTR entry_index;
	ULONG_PTR entry_size;
	ULONG_PTR byte_index;
	ULONG_PTR n_coords;
	ULONG_PTR n_coord;
	INT32 entry_id;
	INT32 object_id;
	FLOAT pixelsize;
	COLORREF color;
	BOOL obj;

	if(this->status < 1) return FALSE;

	n_entries = (ULONG_PTR) this->getEntryCount(NULL);
	if(((LONG_PTR) n_entries) < 0) return FALSE;

	if(!n_entries) return TRUE;

	this->makeCurrentGL();

	byte_index = 0u;
	for(entry_index = 0u; entry_index < n_entries; entry_index++)
	{
		entry_id = *((INT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET));
		entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));

		switch(entry_id)
		{
			case this->ENTRY_ID_COLOR:
				obj = FALSE;
				color = (COLORREF) *((UINT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYCOLOR_COLORREF_BYTEOFFSET));
				mygl_setcolor(color);
				break;

			case this->ENTRY_ID_GLPOINTSIZE:
				obj = FALSE;
				pixelsize = *((FLOAT*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYPIXELSIZE_PIXELSIZE_BYTEOFFSET));
				glPointSize(pixelsize);
				break;

			case this->ENTRY_ID_GLLINEWIDTH:
				obj = FALSE;
				pixelsize = *((FLOAT*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYPIXELSIZE_PIXELSIZE_BYTEOFFSET));
				glLineWidth(pixelsize);
				break;

			case this->ENTRY_ID_OBJECT:
				obj = TRUE;
				break;

			default:
				this->err_msg = TEXT("MyGL::paintBufferGL: Error: buffer data corrupted.");
				return FALSE;
		}

		if(!obj)
		{
			byte_index += entry_size;
			continue;
		}

		object_id = *((INT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYBASEOBJ_OBJECTID_BYTEOFFSET));

		switch(object_id)
		{
			case this->OBJECT_ID_POINT:
				n_coords = 1u;
				p_coord = (mygl_coord_t*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYPOINTOBJ_COORD_BYTEOFFSET);
				break;

			case this->OBJECT_ID_LINE:
				n_coords = 2u;
				p_coord = (mygl_coord_t*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYLINEOBJ_COORDS_BYTEOFFSET);
				break;

			case this->OBJECT_ID_TRIANGLE:
				n_coords = 3u;
				p_coord = (mygl_coord_t*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYTRIANGLEOBJ_COORDS_BYTEOFFSET);
				break;

			case this->OBJECT_ID_QUAD:
				n_coords = 4u;
				p_coord = (mygl_coord_t*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYQUADOBJ_COORDS_BYTEOFFSET);
				break;

			case this->OBJECT_ID_POLYGON:
				n_coords = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET));
				p_coord = (mygl_coord_t*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET);
				break;

			default:
				this->err_msg = TEXT("MyGL::paintBufferGL: Error: buffer data corrupted.");
				return FALSE;
		}

		glBegin(object_id);

		n_coord = 0u;
		while(n_coord < n_coords)
		{
			glVertex2f(p_coord[n_coord].x, p_coord[n_coord].y);
			n_coord++;
		}

		glEnd();

		byte_index += entry_size;
	}

	return TRUE;
}

BOOL WINAPI MyGL::makeCurrentGL(VOID)
{
	if(!this->wndctx_validate())
		if(!this->wndctx_create())
			return FALSE;

	if(!wglMakeCurrent(this->p_wnddc, this->p_wndrc))
	{
		this->err_msg = TEXT("MyGL::makeCurrentGL: Error: wglMakeCurrent failed.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI MyGL::setColorGL(COLORREF color)
{
	if(!this->makeCurrentGL()) return FALSE;

	mygl_setcolor(color);
	return TRUE;
}

BOOL WINAPI MyGL::updateWndCtx(VOID)
{
	return this->wndctx_create();
}

HDC WINAPI MyGL::getWndDC(VOID)
{
	return this->p_wnddc;
}

HGLRC WINAPI MyGL::getWndRC(VOID)
{
	return this->p_wndrc;
}

__string WINAPI MyGL::getLastErrorMessage(VOID)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return TEXT("Error: MyGL object not initialized\r\nExtended error message: ") + this->err_msg;

	return this->err_msg;
}

INT WINAPI MyGL::getStatus(VOID)
{
	return this->status;
}

VOID WINAPI MyGL::deinitialize(VOID)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->buffer_free();
	this->wndctx_destroy();
	return;
}

BOOL WINAPI MyGL::buffer_alloc(VOID)
{
	HANDLE p_processheap = NULL;

	this->buffer_free();

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL) return FALSE;

	this->p_buffer = HeapAlloc(p_processheap, 0u, this->BUFFER_INIT_SIZE_BYTES);
	if(this->p_buffer == NULL) return FALSE;

	FillMemory(this->p_buffer, this->BUFFER_INIT_SIZE_BYTES, 0xff);

	this->buffer_size = this->BUFFER_INIT_SIZE_BYTES;
	return TRUE;
}

VOID WINAPI MyGL::buffer_free(VOID)
{
	HANDLE p_processheap = NULL;

	p_processheap = GetProcessHeap();

	if(this->p_buffer != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_buffer);
		this->p_buffer = NULL;
	}

	this->buffer_size = 0u;
	return;
}

BOOL WINAPI MyGL::buffer_resize(ULONG_PTR new_size_bytes)
{
	HANDLE p_processheap = NULL;
	VOID *p_newbuffer = NULL;

	if(!new_size_bytes)
	{
		this->err_msg = TEXT("MyGL::buffer_resize: Error: invalid parameter.");
		return FALSE;
	}

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("MyGL::buffer_resize: Error: failed to retrieve process heap.");
		return FALSE;
	}

	p_newbuffer = HeapAlloc(p_processheap, 0u, new_size_bytes);
	if(p_newbuffer == NULL)
	{
		this->err_msg = TEXT("MyGL::buffer_resize: Error: memory allocate failed.");
		return FALSE;
	}

	FillMemory(p_newbuffer, new_size_bytes, 0xff);
	CopyMemory(p_newbuffer, this->p_buffer, this->buffer_size);

	HeapFree(p_processheap, 0u, this->p_buffer);

	this->p_buffer = p_newbuffer;
	this->buffer_size = new_size_bytes;

	return TRUE;
}

BOOL WINAPI MyGL::buffer_resize_default(VOID)
{
	return this->buffer_resize(2u*(this->buffer_size));
}

BOOL WINAPI MyGL::wndctx_create(VOID)
{
	if(p_wnd == NULL) return FALSE;

	this->wndctx_destroy();

	this->p_wnddc = GetDC(this->p_wnd);
	if(this->p_wnddc == NULL) return FALSE;

	this->p_wndrc = wglCreateContext(this->p_wnddc);

	if(this->p_wndrc == NULL)
	{
		this->wndctx_destroy();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI MyGL::wndctx_destroy(VOID)
{
	wglMakeCurrent(NULL, NULL);

	if(this->p_wndrc != NULL)
	{
		wglDeleteContext(this->p_wndrc);
		this->p_wndrc = NULL;
	}

	if(this->p_wnddc != NULL)
	{
		if(this->p_wnd == NULL) return FALSE;

		ReleaseDC(this->p_wnd, this->p_wnddc);
		this->p_wnddc = NULL;
	}

	return TRUE;
}

BOOL WINAPI MyGL::wndctx_validate(VOID)
{
	if(this->p_wnddc == NULL) return FALSE;
	if(this->p_wndrc == NULL) return FALSE;

	return TRUE;
}

BOOL WINAPI MyGL::add_entry(const VOID *p_entry)
{
	ULONG_PTR byteindex_next;
	ULONG_PTR entry_size;

	if(p_entry == NULL)
	{
		this->err_msg = TEXT("MyGL::add_entry: Error: invalid object parameter given.");
		return FALSE;
	}

	if(this->getEntryCount(&byteindex_next) < 0) return FALSE;

	entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) p_entry) + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));
	if(entry_size < sizeof(mygl_entry_base_t))
	{
		this->err_msg = TEXT("MyGL::add_entry: Error: invalid entry size parameter.");
		return FALSE;
	}

	if((byteindex_next + entry_size) >= this->buffer_size)
		if(!this->buffer_resize_default())
			return FALSE;

	CopyMemory((VOID*) (((ULONG_PTR) this->p_buffer) + byteindex_next), p_entry, entry_size);

	return TRUE;
}

LONG_PTR WINAPI MyGL::buffer_get_byteindex_from_entryindex(ULONG_PTR index)
{
	ULONG_PTR current_index;
	ULONG_PTR byte_index;
	ULONG_PTR entry_size;
	LONG_PTR n_entries;

	n_entries = this->getEntryCount(NULL);
	if(n_entries < 0) return -1;

	if(index >= ((ULONG_PTR) n_entries))
	{
		this->err_msg = TEXT("MyGL::buffer_get_byteindex_from_objectindex: Error: given object index is out of bounds.");
		return -1;
	}

	byte_index = 0u;
	current_index = 0u;

	while(current_index < index)
	{
		entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) this->p_buffer) + byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));
		byte_index += entry_size;
		current_index++;
	}

	return (LONG_PTR) byte_index;
}

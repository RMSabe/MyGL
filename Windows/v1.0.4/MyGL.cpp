/*
	MyGL: Win32 OpenGL auxiliary resource
	Version 1.0.4

	This code uses my custom WinLib version 4.0.2
	GitHub Repository: https://github.com/RMSabe/WinLib/tree/main/v4.0.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "MyGL.hpp"

struct _mygl_entry_base {
	__declspec(align(4)) INT32 entry_id;
	__declspec(align(4)) UINT32 size;
};

typedef struct _mygl_entry_base mygl_entry_base_t;

static constexpr ULONG_PTR __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET = 0U;
static constexpr ULONG_PTR __MYGL_ENTRYBASE_SIZE_BYTEOFFSET = 4U;

struct _mygl_entry_color {
	__declspec(align(4)) mygl_entry_base_t base;
	__declspec(align(4)) UINT32 colorref;
};

typedef struct _mygl_entry_color mygl_entry_color_t;

static constexpr ULONG_PTR __MYGL_ENTRYCOLOR_COLORREF_BYTEOFFSET = sizeof(mygl_entry_base_t);

struct _mygl_entry_pixelsize {
	__declspec(align(4)) mygl_entry_base_t base;
	__declspec(align(4)) FLOAT pixelsize;
};

typedef struct _mygl_entry_pixelsize mygl_entry_pixelsize_t;

static constexpr ULONG_PTR __MYGL_ENTRYPIXELSIZE_PIXELSIZE_BYTEOFFSET = sizeof(mygl_entry_base_t);

struct _mygl_entry_baseobj {
	__declspec(align(4)) mygl_entry_base_t base;
	__declspec(align(4)) INT32 object_id;
};

typedef struct _mygl_entry_baseobj mygl_entry_baseobj_t;

static constexpr ULONG_PTR __MYGL_ENTRYBASEOBJ_OBJECTID_BYTEOFFSET = sizeof(mygl_entry_base_t);

struct _mygl_entry_pointobj {
	__declspec(align(4)) mygl_entry_baseobj_t baseobj;
	__declspec(align(4)) mygl_coord_t coord;
};

typedef struct _mygl_entry_pointobj mygl_entry_pointobj_t;

static constexpr ULONG_PTR __MYGL_ENTRYPOINTOBJ_COORD_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

struct _mygl_entry_lineobj {
	__declspec(align(4)) mygl_entry_baseobj_t baseobj;
	__declspec(align(4)) mygl_coord_t coords[2];
};

typedef struct _mygl_entry_lineobj mygl_entry_lineobj_t;

static constexpr ULONG_PTR __MYGL_ENTRYLINEOBJ_COORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

struct _mygl_entry_triangleobj {
	__declspec(align(4)) mygl_entry_baseobj_t baseobj;
	__declspec(align(4)) mygl_coord_t coords[3];
};

typedef struct _mygl_entry_triangleobj mygl_entry_triangleobj_t;

static constexpr ULONG_PTR __MYGL_ENTRYTRIANGLEOBJ_COORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

struct _mygl_entry_quadobj {
	__declspec(align(4)) mygl_entry_baseobj_t baseobj;
	__declspec(align(4)) mygl_coord_t coords[4];
};

typedef struct _mygl_entry_quadobj mygl_entry_quadobj_t;

static constexpr ULONG_PTR __MYGL_ENTRYQUADOBJ_COORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);

/*
	mygl_entry_polygonobj !!theoretical!! {
		mygl_entry_baseobj_t; (byte index = 0)
		UINT32 n_coords; (byte index = 12)
		mygl_coord_t[]; (byte index = 16)
	};
*/

static constexpr ULONG_PTR __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET = sizeof(mygl_entry_baseobj_t);
static constexpr ULONG_PTR __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET = __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET + 4U;

VOID WINAPI mygl_setcolor(COLORREF color)
{
	FLOAT _fred;
	FLOAT _fgreen;
	FLOAT _fblue;

	_fred = ((FLOAT) *((UINT8*) &color))/255.0f;
	_fgreen = ((FLOAT) *((UINT8*) (((ULONG_PTR) &color) + 1u)))/255.0f;
	_fblue = ((FLOAT) *((UINT8*) (((ULONG_PTR) &color) + 2u)))/255.0f;

	glColor3f(_fred, _fgreen, _fblue);
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
	if(this->status > MyGL::STATUS_UNINITIALIZED) return TRUE;

	this->status = MyGL::STATUS_UNINITIALIZED;

	if(!this->buffer_alloc()) return FALSE;

	this->status = MyGL::STATUS_OK;
	return TRUE;
}

VOID WINAPI MyGL::deinitialize(VOID)
{
	this->status = MyGL::STATUS_UNINITIALIZED;
	this->buffer_free();
	this->wndctx_destroy();
	return;
}

BOOL WINAPI MyGL::clearAll(VOID)
{
	if(this->status < MyGL::STATUS_OK) return FALSE;

	FillMemory(this->p_buffer, this->buffer_size, 0xff);

	return TRUE;
}

LONG_PTR WINAPI MyGL::getEntryCount(VOID)
{
	return this->getEntryCount(NULL);
}

LONG_PTR WINAPI MyGL::getEntryCount(ULONG_PTR *p_datasizebytes)
{
	ULONG_PTR _byte_index;
	ULONG_PTR _entry_size;
	ULONG_PTR _n_entries;
	INT32 _entry_id;

	if(this->status < MyGL::STATUS_OK) return -1;

	_byte_index = 0u;
	_n_entries = 0u;

	while(TRUE)
	{
		if(_byte_index >= this->buffer_size) break;

		_entry_id = *((INT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET));
		if(_entry_id == MyGL::ENTRY_ID_NULL) break;

		_entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));
		_byte_index += _entry_size;
		_n_entries++;
	}

	if(p_datasizebytes != NULL) *p_datasizebytes = _byte_index;

	return (LONG_PTR) _n_entries;
}

BOOL WINAPI MyGL::removeEntry(ULONG_PTR index)
{
	ULONG_PTR *_p_entry_byte_indexes = NULL;
	ULONG_PTR _byte_index;
	ULONG_PTR _entry_index;
	ULONG_PTR _entry_size;
	ULONG_PTR _n_entries;
	ULONG_PTR _n_byte;
	LONG_PTR _n_tocopy_entries;

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_n_entries = (ULONG_PTR) this->getEntryCount(NULL);

	if(((LONG_PTR) _n_entries) < 0) return FALSE;

	if(index >= _n_entries)
	{
		this->err_msg = TEXT("MyGL::removeEntry: Error: given object index is out of bounds.");
		return FALSE;
	}

	_byte_index = (ULONG_PTR) this->buffer_get_byteindex_from_entryindex(index);
	if(((LONG_PTR) _byte_index) < 0) return FALSE;

	_entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));

	_n_tocopy_entries = (LONG_PTR) (_n_entries - index - 1u);

	if(!_n_tocopy_entries)
	{
		FillMemory((VOID*) (((ULONG_PTR) (this->p_buffer)) + _byte_index), _entry_size, 0xff);
		return TRUE;
	}

	/*If there are other valid objects after the one to be removed, move/append them to the buffer byte index after the last valid object*/

	if(p_processheap == NULL)
	{
		this->status = MyGL::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("MyGL::removeEntry: Error: p_processheap is NULL.");
		return FALSE;
	}

	_p_entry_byte_indexes = (ULONG_PTR*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, ((ULONG_PTR) _n_tocopy_entries)*PTR_SIZE_BYTES);
	if(_p_entry_byte_indexes == NULL)
	{
		/*this->status = MyGL::STATUS_ERROR_MEMORY;*/
		this->err_msg = TEXT("MyGL::removeEntry: Error: failed to allocate heap memory.");
		return FALSE;
	}

	_entry_index = index + 1u;
	while(_entry_index < _n_entries)
	{
		_p_entry_byte_indexes[_entry_index] = (ULONG_PTR) this->buffer_get_byteindex_from_entryindex(_entry_index);
		if(((LONG_PTR) _p_entry_byte_indexes[_entry_index]) < 0)
		{
			HeapFree(p_processheap, 0u, _p_entry_byte_indexes);
			return FALSE;
		}

		_entry_index++;
	}

	_entry_index = 0u;
	while(_n_tocopy_entries > 0)
	{
		_entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) (this->p_buffer)) + _p_entry_byte_indexes[_entry_index] + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));

		/*
			Technically, we could use CopyMemory() here.
			The problem is, because were moving data within the buffer (copying from buffer to a different position within the same buffer),
			the order of bytes being copied must be very specific.
			In this case, the order of bytes being copied must be from the first byte (index 0) to last byte (index entry_size - 1).

			Attempting to copy the data chunk in any other byte order might cause runtime errors.
		*/

		_n_byte = 0u;
		while(_n_byte < _entry_size)
		{
			*((UINT8*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + _n_byte)) = *((UINT8*) (((ULONG_PTR) (this->p_buffer)) + _p_entry_byte_indexes[_entry_index] + _n_byte));
			_n_byte++;
		}

		_byte_index += _entry_size;
		_entry_index++;
		_n_tocopy_entries--;
	}

	FillMemory((VOID*) (((ULONG_PTR) (this->p_buffer)) + _byte_index), (this->buffer_size - _byte_index), 0xff);

	HeapFree(p_processheap, 0u, _p_entry_byte_indexes);

	return TRUE;
}

BOOL WINAPI MyGL::addColorEntry(COLORREF color, BOOL updateGL)
{
	mygl_entry_color_t _color;

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_color.base.entry_id = MyGL::ENTRY_ID_COLOR;
	_color.base.size = sizeof(mygl_entry_color_t);
	_color.colorref = color;

	if(!this->add_entry(&_color)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();
	mygl_setcolor(color);

	return TRUE;
}

BOOL WINAPI MyGL::addPointSizeEntry(FLOAT pointSize, BOOL updateGL)
{
	mygl_entry_pixelsize_t _pixelsize;

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_pixelsize.base.entry_id = MyGL::ENTRY_ID_GLPOINTSIZE;
	_pixelsize.base.size = sizeof(mygl_entry_pixelsize_t);
	_pixelsize.pixelsize = pointSize;

	if(!this->add_entry(&_pixelsize)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();
	glPointSize(pointSize);

	return TRUE;
}

BOOL WINAPI MyGL::addLineWidthEntry(FLOAT lineWidth, BOOL updateGL)
{
	mygl_entry_pixelsize_t _pixelsize;

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_pixelsize.base.entry_id = MyGL::ENTRY_ID_GLLINEWIDTH;
	_pixelsize.base.size = sizeof(mygl_entry_pixelsize_t);
	_pixelsize.pixelsize = lineWidth;

	if(!this->add_entry(&_pixelsize)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();
	glLineWidth(lineWidth);

	return TRUE;
}

BOOL WINAPI MyGL::addPointObjectEntry(FLOAT x, FLOAT y, BOOL updateGL)
{
	mygl_coord_t _coord;

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_coord.x = x;
	_coord.y = y;

	return this->addPointObjectEntry(&_coord, updateGL);
}

BOOL WINAPI MyGL::addPointObjectEntry(const mygl_coord_t *p_coord, BOOL updateGL)
{
	mygl_entry_pointobj_t _point;

	if(this->status < MyGL::STATUS_OK) return FALSE;
	if(p_coord == NULL)
	{
		this->err_msg = TEXT("MyGL::addPointObjectEntry: Error: invalid pcoord parameter.");
		return FALSE;
	}

	_point.baseobj.base.entry_id = MyGL::ENTRY_ID_OBJECT;
	_point.baseobj.base.size = sizeof(mygl_entry_pointobj_t);
	_point.baseobj.object_id = MyGL::OBJECT_ID_POINT;
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

BOOL WINAPI MyGL::addLineObjectEntry(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, BOOL updateGL)
{
	mygl_coord_t _coords[2];

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_coords[0].x = x0;
	_coords[0].y = y0;
	_coords[1].x = x1;
	_coords[1].y = y1;

	return this->addLineObjectEntry(_coords, updateGL);
}

BOOL WINAPI MyGL::addLineObjectEntry(const mygl_coord_t *p_coords, BOOL updateGL)
{
	ULONG_PTR _n_coord;
	mygl_entry_lineobj_t _line;

	if(this->status < MyGL::STATUS_OK) return FALSE;
	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addLineObjectEntry: Error: invalid pcoords parameter.");
		return FALSE;
	}

	_line.baseobj.base.entry_id = MyGL::ENTRY_ID_OBJECT;
	_line.baseobj.base.size = sizeof(mygl_entry_lineobj_t);
	_line.baseobj.object_id = MyGL::OBJECT_ID_LINE;

	CopyMemory(&(_line.coords), p_coords, 2u*sizeof(mygl_coord_t));

	if(!this->add_entry(&_line)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_LINES);
	for(_n_coord = 0u; _n_coord < 2u; _n_coord++) glVertex2f(p_coords[_n_coord].x, p_coords[_n_coord].y);
	glEnd();

	return TRUE;
}

BOOL WINAPI MyGL::addTriangleObjectEntry(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, BOOL updateGL)
{
	mygl_coord_t _coords[3];

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_coords[0].x = x0;
	_coords[0].y = y0;
	_coords[1].x = x1;
	_coords[1].y = y1;
	_coords[2].x = x2;
	_coords[2].y = y2;

	return this->addTriangleObjectEntry(_coords, updateGL);
}

BOOL WINAPI MyGL::addTriangleObjectEntry(const mygl_coord_t *p_coords, BOOL updateGL)
{
	ULONG_PTR _n_coord;
	mygl_entry_triangleobj_t _triangle;

	if(this->status < MyGL::STATUS_OK) return FALSE;
	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addTriangleObjectEntry: Error: invalid pcoords parameter.");
		return FALSE;
	}

	_triangle.baseobj.base.entry_id = MyGL::ENTRY_ID_OBJECT;
	_triangle.baseobj.base.size = sizeof(mygl_entry_triangleobj_t);
	_triangle.baseobj.object_id = MyGL::OBJECT_ID_TRIANGLE;

	CopyMemory(&(_triangle.coords), p_coords, 3u*sizeof(mygl_coord_t));

	if(!this->add_entry(&_triangle)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_TRIANGLES);
	for(_n_coord = 0u; _n_coord < 3u; _n_coord++) glVertex2f(p_coords[_n_coord].x, p_coords[_n_coord].y);
	glEnd();

	return TRUE;
}

BOOL WINAPI MyGL::addQuadObjectEntry(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, BOOL updateGL)
{
	mygl_coord_t _coords[4];

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_coords[0].x = x0;
	_coords[0].y = y0;
	_coords[1].x = x1;
	_coords[1].y = y1;
	_coords[2].x = x2;
	_coords[2].y = y2;
	_coords[3].x = x3;
	_coords[3].y = y3;

	return this->addQuadObjectEntry(_coords, updateGL);
}

BOOL WINAPI MyGL::addQuadObjectEntry(const mygl_coord_t *p_coords, BOOL updateGL)
{
	ULONG_PTR _n_coord;
	mygl_entry_quadobj_t _quad;

	if(this->status < MyGL::STATUS_OK) return FALSE;
	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addQuadObjectEntry: Error: invalid pcoords parameter.");
		return FALSE;
	}

	_quad.baseobj.base.entry_id = MyGL::ENTRY_ID_OBJECT;
	_quad.baseobj.base.size = sizeof(mygl_entry_quadobj_t);
	_quad.baseobj.object_id = MyGL::OBJECT_ID_QUAD;

	CopyMemory(&(_quad.coords), p_coords, 4u*sizeof(mygl_coord_t));

	if(!this->add_entry(&_quad)) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_QUADS);
	for(_n_coord = 0u; _n_coord < 4u; _n_coord++) glVertex2f(p_coords[_n_coord].x, p_coords[_n_coord].y);
	glEnd();

	return TRUE;
}

BOOL WINAPI MyGL::addPolygonObjectEntry(const mygl_coord_t *p_coords, UINT32 n_coords, BOOL updateGL)
{
	VOID *_p_entry = NULL;
	ULONG_PTR _entry_size;
	ULONG_PTR _n_coord;
	BOOL _b_ret;

	if(this->status < MyGL::STATUS_OK) return FALSE;

	if(p_coords == NULL)
	{
		this->err_msg = TEXT("MyGL::addPolygonObjectEntry: Error: invalid pcoords parameter.");
		return FALSE;
	}

	if(!n_coords)
	{
		this->err_msg = TEXT("MyGL::addPolygonObjectEntry: Error: invalid ncoords parameter.");
		return FALSE;
	}

	if(p_processheap == NULL)
	{
		this->status = MyGL::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("MyGL::addPolygonObjectEntry: Error: p_processheap is NULL.");
		return FALSE;
	}

	_entry_size = __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET + ((ULONG_PTR) n_coords)*sizeof(mygl_coord_t);

	_p_entry = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, _entry_size);
	if(_p_entry == NULL)
	{
		/*this->status = MyGL::STATUS_ERROR_MEMORY;*/
		this->err_msg = TEXT("MyGL::addPolygonObjectEntry: Error: failed to allocate heap memory.");
		return FALSE;
	}

	*((INT32*) (((ULONG_PTR) _p_entry) + __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET)) = MyGL::ENTRY_ID_OBJECT;
	*((UINT32*) (((ULONG_PTR) _p_entry) + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET)) = (UINT32) _entry_size;
	*((INT32*) (((ULONG_PTR) _p_entry) + __MYGL_ENTRYBASEOBJ_OBJECTID_BYTEOFFSET)) = MyGL::OBJECT_ID_POLYGON;
	*((UINT32*) (((ULONG_PTR) _p_entry) + __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET)) = n_coords;

	CopyMemory((VOID*) (((ULONG_PTR) _p_entry) + __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET), p_coords, ((ULONG_PTR) n_coords)*sizeof(mygl_coord_t));

	_b_ret = this->add_entry(_p_entry);

	HeapFree(p_processheap, 0u, _p_entry);
	_p_entry = NULL;

	if(!_b_ret) return FALSE;

	if(!updateGL) return TRUE;

	this->makeCurrentGL();

	glBegin(GL_POLYGON);
	for(_n_coord = 0u; _n_coord < ((ULONG_PTR) n_coords); _n_coord++) glVertex2f(p_coords[_n_coord].x, p_coords[_n_coord].y);
	glEnd();

	return TRUE;	
}

BOOL WINAPI MyGL::paintBufferGL(VOID)
{
	mygl_coord_t *_p_coord = NULL;

	ULONG_PTR _n_entries;
	ULONG_PTR _entry_index;
	ULONG_PTR _entry_size;
	ULONG_PTR _byte_index;
	ULONG_PTR _n_coords;
	ULONG_PTR _n_coord;
	INT32 _entry_id;
	INT32 _object_id;
	FLOAT _pixelsize;
	COLORREF _color;
	BOOL _is_obj;

	if(this->status < MyGL::STATUS_OK) return FALSE;

	_n_entries = (ULONG_PTR) this->getEntryCount(NULL);
	if(((LONG_PTR) _n_entries) < 0) return FALSE;

	if(!_n_entries) return TRUE;

	this->makeCurrentGL();

	_byte_index = 0u;
	for(_entry_index = 0u; _entry_index < _n_entries; _entry_index++)
	{
		_entry_id = *((INT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYBASE_ENTRYID_BYTEOFFSET));
		_entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));

		switch(_entry_id)
		{
			case MyGL::ENTRY_ID_COLOR:
				_is_obj = FALSE;
				_color = (COLORREF) *((UINT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYCOLOR_COLORREF_BYTEOFFSET));
				mygl_setcolor(_color);
				break;

			case MyGL::ENTRY_ID_GLPOINTSIZE:
				_is_obj = FALSE;
				_pixelsize = *((FLOAT*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYPIXELSIZE_PIXELSIZE_BYTEOFFSET));
				glPointSize(_pixelsize);
				break;

			case MyGL::ENTRY_ID_GLLINEWIDTH:
				_is_obj = FALSE;
				_pixelsize = *((FLOAT*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYPIXELSIZE_PIXELSIZE_BYTEOFFSET));
				glLineWidth(_pixelsize);
				break;

			case MyGL::ENTRY_ID_OBJECT:
				_is_obj = TRUE;
				break;

			default:
				this->err_msg = TEXT("MyGL::paintBufferGL: Error: buffer data corrupted.");
				return FALSE;
		}

		if(!_is_obj)
		{
			_byte_index += _entry_size;
			continue;
		}

		_object_id = *((INT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYBASEOBJ_OBJECTID_BYTEOFFSET));

		switch(_object_id)
		{
			case MyGL::OBJECT_ID_POINT:
				_n_coords = 1u;
				_p_coord = (mygl_coord_t*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYPOINTOBJ_COORD_BYTEOFFSET);
				break;

			case MyGL::OBJECT_ID_LINE:
				_n_coords = 2u;
				_p_coord = (mygl_coord_t*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYLINEOBJ_COORDS_BYTEOFFSET);
				break;

			case MyGL::OBJECT_ID_TRIANGLE:
				_n_coords = 3u;
				_p_coord = (mygl_coord_t*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYTRIANGLEOBJ_COORDS_BYTEOFFSET);
				break;

			case MyGL::OBJECT_ID_QUAD:
				_n_coords = 4u;
				_p_coord = (mygl_coord_t*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYQUADOBJ_COORDS_BYTEOFFSET);
				break;

			case MyGL::OBJECT_ID_POLYGON:
				_n_coords = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYPOLYGONOBJ_NCOORDS_BYTEOFFSET));
				_p_coord = (mygl_coord_t*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYPOLYGONOBJ_COORDS_BYTEOFFSET);
				break;

			default:
				this->err_msg = TEXT("MyGL::paintBufferGL: Error: buffer data corrupted.");
				return FALSE;
		}

		glBegin(_object_id);

		_n_coord = 0u;
		while(_n_coord < _n_coords)
		{
			glVertex2f(_p_coord[_n_coord].x, _p_coord[_n_coord].y);
			_n_coord++;
		}

		glEnd();

		_byte_index += _entry_size;
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

INT WINAPI MyGL::getStatus(VOID)
{
	return this->status;
}

__string WINAPI MyGL::getLastErrorMessage(VOID)
{
	if(this->status == MyGL::STATUS_UNINITIALIZED)
		return (TEXT("Error: MyGL object not initialized\r\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

BOOL WINAPI MyGL::buffer_alloc(VOID)
{
	if(!this->buffer_free()) return FALSE;

	if(p_processheap == NULL)
	{
		this->status = MyGL::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("MyGL::buffer_alloc: Error: p_processheap is NULL.");
		return FALSE;
	}

	this->p_buffer = HeapAlloc(p_processheap, 0u, MyGL::BUFFER_INIT_SIZE_BYTES);
	if(this->p_buffer == NULL)
	{
		this->status = MyGL::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("MyGL::buffer_alloc: Error: failed to allocate heap memory.");
		return FALSE;
	}

	FillMemory(this->p_buffer, MyGL::BUFFER_INIT_SIZE_BYTES, 0xff);

	this->buffer_size = MyGL::BUFFER_INIT_SIZE_BYTES;
	return TRUE;
}

BOOL WINAPI MyGL::buffer_free(VOID)
{
	if(p_processheap == NULL)
	{
		this->status = MyGL::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("MyGL::buffer_free: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_buffer != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_buffer))
		{
			this->status = MyGL::STATUS_ERROR_MEMORY;
			this->err_msg = TEXT("MyGL::buffer_free: Error: failed to release heap memory.");
			return FALSE;
		}

		this->p_buffer = NULL;
	}

	this->buffer_size = 0u;
	return TRUE;
}

BOOL WINAPI MyGL::buffer_resize(ULONG_PTR new_size_bytes)
{
	VOID *_p_newbuffer = NULL;
	ULONG_PTR _memcpy_size_bytes;

	if(!new_size_bytes)
	{
		this->err_msg = TEXT("MyGL::buffer_resize: Error: invalid parameter.");
		return FALSE;
	}

	if(p_processheap == NULL)
	{
		this->status = MyGL::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("MyGL::buffer_resize: Error: p_processheap is NULL.");
		return FALSE;
	}

	_p_newbuffer = HeapAlloc(p_processheap, 0u, new_size_bytes);
	if(_p_newbuffer == NULL)
	{
		this->status = MyGL::STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("MyGL::buffer_resize: Error: failed to allocate heap memory.");
		return FALSE;
	}

	if(this->buffer_size < new_size_bytes) _memcpy_size_bytes = this->buffer_size;
	else _memcpy_size_bytes = new_size_bytes;

	FillMemory(_p_newbuffer, new_size_bytes, 0xff);
	CopyMemory(_p_newbuffer, this->p_buffer, _memcpy_size_bytes);

	HeapFree(p_processheap, 0u, this->p_buffer);

	this->p_buffer = _p_newbuffer;
	this->buffer_size = new_size_bytes;

	return TRUE;
}

BOOL WINAPI MyGL::buffer_resize_default(VOID)
{
	return this->buffer_resize(2u*(this->buffer_size));
}

BOOL WINAPI MyGL::wndctx_create(VOID)
{
	if(p_wnd == NULL)
	{
		this->err_msg = TEXT("MyGL::wndctx_create: Error: p_wnd object is NULL.");
		return FALSE;
	}

	this->wndctx_destroy();

	this->p_wnddc = GetDC(this->p_wnd);
	if(this->p_wnddc == NULL)
	{
		this->err_msg = TEXT("MyGL::wndctx_create: Error: GetDC failed.");
		return FALSE;
	}

	this->p_wndrc = wglCreateContext(this->p_wnddc);

	if(this->p_wndrc == NULL)
	{
		this->wndctx_destroy();
		this->err_msg = TEXT("MyGL::wndctx_create: Error: wglCreateContext failed.");
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
		if(this->p_wnd == NULL)
		{
			this->err_msg = TEXT("MyGL::wndctx_destroy: Error: p_wnd object is NULL.");
			return FALSE;
		}

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
	ULONG_PTR _byteindex_next;
	ULONG_PTR _entry_size;

	if(p_entry == NULL)
	{
		this->err_msg = TEXT("MyGL::add_entry: Error: invalid p_entry parameter given.");
		return FALSE;
	}

	if(this->getEntryCount(&_byteindex_next) < 0) return FALSE;

	_entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) p_entry) + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));
	if(_entry_size < sizeof(mygl_entry_base_t))
	{
		this->err_msg = TEXT("MyGL::add_entry: Error: invalid entry size parameter.");
		return FALSE;
	}

	if((_byteindex_next + _entry_size) >= this->buffer_size)
		if(!this->buffer_resize_default())
			return FALSE;

	CopyMemory((VOID*) (((ULONG_PTR) (this->p_buffer)) + _byteindex_next), p_entry, _entry_size);

	return TRUE;
}

LONG_PTR WINAPI MyGL::buffer_get_byteindex_from_entryindex(ULONG_PTR index)
{
	ULONG_PTR _current_index;
	ULONG_PTR _byte_index;
	ULONG_PTR _entry_size;
	LONG_PTR _n_entries;

	_n_entries = this->getEntryCount(NULL);
	if(_n_entries < 0) return -1;

	if(index >= ((ULONG_PTR) _n_entries))
	{
		this->err_msg = TEXT("MyGL::buffer_get_byteindex_from_objectindex: Error: given object index is out of bounds.");
		return -1;
	}

	_byte_index = 0u;
	_current_index = 0u;

	while(_current_index < index)
	{
		_entry_size = (ULONG_PTR) *((UINT32*) (((ULONG_PTR) (this->p_buffer)) + _byte_index + __MYGL_ENTRYBASE_SIZE_BYTEOFFSET));
		_byte_index += _entry_size;
		_current_index++;
	}

	return (LONG_PTR) _byte_index;
}

#ifdef _MSC_VER
#pragma warning(disable:4101)		// "unreferenced local variable"
#endif

#undef _CONCAT2
#undef _CONCAT3
#undef _CONCAT4
#undef _TOKEN2
#undef _TOKEN3
#undef _TOKEN4

#undef P
#undef MUR
#undef MDR
#undef MDL
#undef MUL
#undef I1
#undef I2
#undef I3

#undef PIXEL_UINT
#undef PIXEL_INDEX
#undef PIXEL_COLOR
#undef PIXEL_PACK
#undef PIXEL_UNPACK
#undef PIXEL_COPY
#undef PIXEL_SRC
#undef PIXEL_DIFF

#undef INTERP_0
#undef INTERP_

#define _CONCAT2(a, b)					a ## b
#define _CONCAT3(a, b, c)				a ## b ## c
#define _CONCAT4(a, b, c, d)			a ## b ## c ## d
#define _TOKEN2(a, b)					_CONCAT2(a, b)
#define _TOKEN3(a, b, c)				_CONCAT3(a, b, c)
#define _TOKEN4(a, b, c, d)				_CONCAT4(a, b, c, d)

#define PIXEL_UINT						_TOKEN2(UINT, PIXEL_BITS)
#define UINT15							UINT16

#ifdef INTERP_MMX

	#if PIXEL_BITS == 16
		#define PIXEL_COLOR				PIXEL_UINT
		#define PIXEL_PACK(v)			(v)
		#define PIXEL_UNPACK(v)			(v)
		#define PIXEL_COPY(p0, p1)		c[p0] = c[p1]
		#define PIXEL_SRC(p, v)			c[p] = v
		
		#ifdef PIXEL_LQ_FILTER
			#define PIXEL_DIFF(p0, p1)	(c[p0] != c[p1])
		#else
			#define PIXEL_DIFF(p0, p1)	interp_diff(c[p0], c[p1])
		#endif
		
		#define INTERP_0(p)				c[p]
		#define INTERP_(id)				_TOKEN2(interp_16_, id)	
	#else
		#define PIXEL_COLOR				__m64
		#define PIXEL_PACK(v)			_TOKEN3(interp_, PIXEL_BITS, _pack)(v)
		#define PIXEL_UNPACK(v)			_TOKEN3(interp_, PIXEL_BITS, _unpack)(v)
		#define PIXEL_COPY(p0, p1)		c[p0] = c[p1], s[p0] = s[p1]

		#ifdef PIXEL_LQ_FILTER
			#define PIXEL_INDEX			PIXEL_UINT
			#define PIXEL_SRC(p, v)		c[p] = PIXEL_UNPACK(v), s[p] = v
			#define PIXEL_DIFF(p0, p1)	(s[p0] != s[p1])
			#define INTERP_0(p)			s[p]
		#else
			#define PIXEL_INDEX			UINT16
			#define PIXEL_DIFF(p0, p1)	interp_diff(s[p0], s[p1])
			#define PIXEL_SRC(p, v)		c[p] = PIXEL_UNPACK(v), s[p] = interp_32_to_16(v)
			#define INTERP_0(p)			PIXEL_PACK(c[p])
		#endif

		#define INTERP_(id)				_TOKEN2(interp_, id)	
	#endif
	
#else

	#define PIXEL_COLOR					PIXEL_UINT
	#define PIXEL_PACK(v)				v
	#define PIXEL_UNPACK(v)				v
	#define PIXEL_SRC(p, v)				c[p] = v
	#define PIXEL_COPY(p0, p1)			c[p0] = c[p1]
	
	#ifdef PIXEL_LQ_FILTER
		#define PIXEL_DIFF(p0, p1)		(c[p0] != c[p1])
	#else
		#define PIXEL_DIFF(p0, p1)		_TOKEN4(interp_, PIXEL_BITS, _, diff)(c[p0], c[p1])
	#endif
	
	#define INTERP_0(p)					c[p]
	#define INTERP_(id)					_TOKEN4(interp_, PIXEL_BITS, _, id)
		
#endif

#define P(a, b) 						_TOKEN2(dst, b)[a]

#define MUR								PIXEL_DIFF(1, 5)
#define MDR								PIXEL_DIFF(5, 7)
#define MDL								PIXEL_DIFF(7, 3)
#define MUL								PIXEL_DIFF(3, 1)

#define I1(p0) 							INTERP_0(p0)
#define I2(i0, i1, p0, p1) 				PIXEL_PACK(INTERP_(_TOKEN2(i0, i1))(c[p0], c[p1]))
#define I3(i0, i1, i2, p0, p1, p2) 		PIXEL_PACK(INTERP_(_TOKEN3(i0, i1, i2))(c[p0], c[p1], c[p2]))

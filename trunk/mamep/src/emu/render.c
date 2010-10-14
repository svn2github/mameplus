/***************************************************************************

    render.c

    Core rendering system.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************

    Windows-specific to-do:
        * no fallback if we run out of video memory

    Longer-term to do: (once old renderer is gone)
        * make vector updates asynchronous

****************************************************************************

    Overview of objects:

        render_target -- This represents a final rendering target. It
            is specified using integer width/height values, can have
            non-square pixels, and you can specify its rotation. It is
            what really determines the final rendering details. The OSD
            layer creates one or more of these to encapsulate the
            rendering process. Each render_target holds a list of
            layout_files that it can use for drawing. When rendering, it
            makes use of both layout_files and render_containers.

        render_container -- Containers are the top of a hierarchy that is
            not directly related to the objects above. Containers hold
            high level primitives that are generated at runtime by the
            video system. They are used currently for each screen and
            the user interface. These high-level primitives are broken down
            into low-level primitives at render time.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "rendfont.h"
#include "rendlay.h"
#include "rendutil.h"
#include "config.h"
#include "xmlfile.h"
#include <ctype.h>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define INTERNAL_FLAG_CHAR		0x00000001

enum
{
	COMPONENT_TYPE_IMAGE = 0,
	COMPONENT_TYPE_RECT,
	COMPONENT_TYPE_DISK,
	COMPONENT_TYPE_MAX
};


enum
{
	CONTAINER_ITEM_LINE = 0,
	CONTAINER_ITEM_QUAD,
	CONTAINER_ITEM_MAX
};



/***************************************************************************
    MACROS
***************************************************************************/

#define ISWAP(var1, var2) do { int temp = var1; var1 = var2; var2 = temp; } while (0)
#define FSWAP(var1, var2) do { float temp = var1; var1 = var2; var2 = temp; } while (0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* typedef struct _render_container render_container; -- defined in render.h */
typedef struct _object_transform object_transform;
typedef struct _scaled_texture scaled_texture;
typedef struct _container_item container_item;


/* a render_ref is an abstract reference to an internal object of some sort */
struct _render_ref
{
	render_ref *		next;				/* link to the next reference */
	void *				refptr;				/* reference pointer */
};


/* an object_transform is used to track transformations when building an object list */
struct _object_transform
{
	float				xoffs, yoffs;		/* offset transforms */
	float				xscale, yscale;		/* scale transforms */
	render_color		color;				/* color transform */
	int					orientation;		/* orientation transform */
	int					no_center;			/* center the container? */
};


/* a scaled_texture contains a single scaled entry for a texture */
struct _scaled_texture
{
	bitmap_t *			bitmap;				/* final bitmap */
	UINT32				seqid;				/* sequence number */
};


/* a render_texture is used to track transformations when building an object list */
struct render_texture
{
	render_texture *	next;				/* next texture (for free list) */
	render_texture *	base;				/* pointer to base of texture group */
	bitmap_t *			bitmap;				/* pointer to the original bitmap */
	rectangle			sbounds;			/* source bounds within the bitmap */
	palette_t *			palette;			/* palette associated with the texture */
	int					format;				/* format of the texture data */
	texture_scaler_func	scaler;				/* scaling callback */
	void *				param;				/* scaling callback parameter */
	UINT32				curseq;				/* current sequence number */
	scaled_texture		scaled[MAX_TEXTURE_SCALES];	/* array of scaled variants of this texture */
	rgb_t *				bcglookup;			/* dynamically allocated B/C/G lookup table */
	UINT32				bcglookup_entries;	/* number of B/C/G lookup entries allocated */
};


/* a render_target describes a surface that is being rendered to */
class render_target
{
public:
	render_target *		next;				/* keep a linked list of targets */
	running_machine *	machine;			/* pointer to the machine we are connected with */
	layout_view *		curview;			/* current view */
	layout_file *		filelist;			/* list of layout files */
	UINT32				flags;				/* creation flags */
	render_primitive_list primlist[NUM_PRIMLISTS];/* list of primitives */
	int					listindex;			/* index of next primlist to use */
	INT32				width;				/* width in pixels */
	INT32				height;				/* height in pixels */
	render_bounds		bounds;				/* bounds of the target */
	float				pixel_aspect;		/* aspect ratio of individual pixels */
	float				max_refresh;		/* maximum refresh rate, 0 or if none */
	int					orientation;		/* orientation */
	int					layerconfig;		/* layer configuration */
	layout_view *		base_view;			/* the view at the time of first frame */
	int					base_orientation;	/* the orientation at the time of first frame */
	int					base_layerconfig;	/* the layer configuration at the time of first frame */
	int					maxtexwidth;		/* maximum width of a texture */
	int					maxtexheight;		/* maximum height of a texture */
	render_container *	debug_containers;
};


/* a container_item describes a high level primitive that is added to a container */
struct _container_item
{
	container_item *	next;				/* pointer to the next element in the list */
	UINT8				type;				/* type of element */
	render_bounds		bounds;				/* bounds of the element */
	render_color		color;				/* RGBA factors */
	UINT32				flags;				/* option flags */
	UINT32				internal;			/* internal flags */
	float				width;				/* width of the line (lines only) */
	render_texture *	texture;			/* pointer to the source texture (quads only) */
};


/* a render_container holds a list of items and an orientation for the entire collection */
struct _render_container
{
	render_container *	next;				/* the next container in the list */
	container_item *	itemlist;			/* head of the item list */
	container_item **	nextitem;			/* pointer to the next item to add */
	screen_device *screen;			/* the screen device */
	int					orientation;		/* orientation of the container */
	float				brightness;			/* brightness of the container */
	float				contrast;			/* contrast of the container */
	float				gamma;				/* gamma of the container */
	float				xscale;				/* X scale factor of the container */
	float				yscale;				/* Y scale factor of the container */
	float				xoffset;			/* X offset of the container */
	float				yoffset;			/* Y offset of the container */
	bitmap_t *			overlaybitmap;		/* overlay bitmap */
	render_texture *	overlaytexture;		/* overlay texture */
	palette_client *	palclient;			/* client to the system palette */
	rgb_t				bcglookup256[0x400];/* lookup table for brightness/contrast/gamma */
	rgb_t				bcglookup32[0x80];	/* lookup table for brightness/contrast/gamma */
	rgb_t				bcglookup[0x10000];	/* full palette lookup with bcg adjustements */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* array of live targets */
static render_target *targetlist;
static render_target *ui_target;

/* free lists */
static render_primitive *render_primitive_free_list;
static container_item *container_item_free_list;
static render_ref *render_ref_free_list;
static render_texture *render_texture_free_list;

/* containers for the UI and for screens */
static render_container *ui_container;
static render_container *screen_container_list;
static bitmap_t *screen_overlay;

/* variables for tracking extents to clear */
static INT32 clear_extents[MAX_CLEAR_EXTENTS];
static INT32 clear_extent_count;

/* precomputed UV coordinates for various orientations */
static const render_quad_texuv oriented_texcoords[8] =
{
	{ { 0,0 }, { 1,0 }, { 0,1 }, { 1,1 } },		/* 0 */
	{ { 1,0 }, { 0,0 }, { 1,1 }, { 0,1 } },		/* ORIENTATION_FLIP_X */
	{ { 0,1 }, { 1,1 }, { 0,0 }, { 1,0 } },		/* ORIENTATION_FLIP_Y */
	{ { 1,1 }, { 0,1 }, { 1,0 }, { 0,0 } },		/* ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y */
	{ { 0,0 }, { 0,1 }, { 1,0 }, { 1,1 } },		/* ORIENTATION_SWAP_XY */
	{ { 0,1 }, { 0,0 }, { 1,1 }, { 1,0 } },		/* ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X */
	{ { 1,0 }, { 1,1 }, { 0,0 }, { 0,1 } },		/* ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y */
	{ { 1,1 }, { 1,0 }, { 0,1 }, { 0,0 } }		/* ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y */
};

/* layer orders */
static const int layer_order_standard[] = { ITEM_LAYER_SCREEN, ITEM_LAYER_OVERLAY, ITEM_LAYER_BACKDROP, ITEM_LAYER_BEZEL };
static const int layer_order_alternate[] = { ITEM_LAYER_BACKDROP, ITEM_LAYER_SCREEN, ITEM_LAYER_OVERLAY, ITEM_LAYER_BEZEL };



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core system */
static void render_exit(running_machine &machine);
static void render_load(running_machine *machine, int config_type, xml_data_node *parentnode);
static void render_save(running_machine *machine, int config_type, xml_data_node *parentnode);

/* render targets */
static void release_render_list(render_primitive_list *list);
static int load_layout_files(render_target *target, const char *layoutfile, int singlefile);
static void add_container_primitives(render_target *target, render_primitive_list *list, const object_transform *xform, render_container *container, int blendmode);
static void add_element_primitives(render_target *target, render_primitive_list *list, const object_transform *xform, const layout_element *element, int state, int blendmode);
static void add_clear_and_optimize_primitive_list(render_target *target, render_primitive_list *list);

/* render references */
static void invalidate_all_render_ref(void *refptr);

/* render textures */
static int texture_get_scaled(render_texture *texture, UINT32 dwidth, UINT32 dheight, render_texinfo *texinfo, render_ref **reflist);
static const rgb_t *texture_get_adjusted_palette(render_texture *texture, render_container *container);

/* render containers */
static render_container *render_container_alloc(running_machine *machine);
static void render_container_free(render_container *container);
static container_item *render_container_item_add_generic(render_container *container, UINT8 type, float x0, float y0, float x1, float y1, rgb_t argb);
static void render_container_overlay_scale(bitmap_t *dest, const bitmap_t *source, const rectangle *sbounds, void *param);
static void render_container_recompute_lookups(render_container *container);
static void render_container_update_palette(render_container *container);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    apply_orientation - apply orientation to a
    set of bounds
-------------------------------------------------*/

INLINE void apply_orientation(render_bounds *bounds, int orientation)
{
	/* swap first */
	if (orientation & ORIENTATION_SWAP_XY)
	{
		FSWAP(bounds->x0, bounds->y0);
		FSWAP(bounds->x1, bounds->y1);
	}

	/* apply X flip */
	if (orientation & ORIENTATION_FLIP_X)
	{
		bounds->x0 = 1.0f - bounds->x0;
		bounds->x1 = 1.0f - bounds->x1;
	}

	/* apply Y flip */
	if (orientation & ORIENTATION_FLIP_Y)
	{
		bounds->y0 = 1.0f - bounds->y0;
		bounds->y1 = 1.0f - bounds->y1;
	}
}


/*-------------------------------------------------
    normalize_bounds - normalize bounds so that
    x0/y0 are less than x1/y1
-------------------------------------------------*/

INLINE void normalize_bounds(render_bounds *bounds)
{
	if (bounds->x0 > bounds->x1)
		FSWAP(bounds->x0, bounds->x1);
	if (bounds->y0 > bounds->y1)
		FSWAP(bounds->y0, bounds->y1);
}


/*-------------------------------------------------
    alloc_container_item - allocate a new
    container item object
--------------------------------------------------*/

INLINE container_item *alloc_container_item(void)
{
	container_item *result = container_item_free_list;

	/* allocate from the free list if we can; otherwise, malloc a new item */
	if (result != NULL)
		container_item_free_list = result->next;
	else
		result = global_alloc(container_item);

	memset(result, 0, sizeof(*result));
	return result;
}


/*-------------------------------------------------
    container_item_free - free a previously
    allocated render element object
-------------------------------------------------*/

INLINE void free_container_item(container_item *item)
{
	item->next = container_item_free_list;
	container_item_free_list = item;
}


/*-------------------------------------------------
    alloc_render_primitive - allocate a new empty
    element object
-------------------------------------------------*/

INLINE render_primitive *alloc_render_primitive(int type)
{
	render_primitive *result = render_primitive_free_list;

	/* allocate from the free list if we can; otherwise, malloc a new item */
	if (result != NULL)
		render_primitive_free_list = result->next;
	else
		result = global_alloc(render_primitive);

	/* clear to 0 */
	memset(result, 0, sizeof(*result));
	result->type = type;
	return result;
}


/*-------------------------------------------------
    append_render_primitive - append a primitive
    to the end of the list
-------------------------------------------------*/

INLINE void append_render_primitive(render_primitive_list *list, render_primitive *prim)
{
	*list->nextptr = prim;
	list->nextptr = &prim->next;
}


/*-------------------------------------------------
    free_render_primitive - free a previously
    allocated render element object
-------------------------------------------------*/

INLINE void free_render_primitive(render_primitive *element)
{
	element->next = render_primitive_free_list;
	render_primitive_free_list = element;
}


/*-------------------------------------------------
    add_render_ref - add a new reference
-------------------------------------------------*/

INLINE void add_render_ref(render_ref **list, void *refptr)
{
	render_ref *ref;

	/* skip if we already have one */
	for (ref = *list; ref != NULL; ref = ref->next)
		if (ref->refptr == refptr)
			return;

	/* allocate from the free list if we can; otherwise, malloc a new item */
	ref = render_ref_free_list;
	if (ref != NULL)
		render_ref_free_list = ref->next;
	else
		ref = global_alloc(render_ref);

	/* set the refptr and link us into the list */
	ref->refptr = refptr;
	ref->next = *list;
	*list = ref;
}


/*-------------------------------------------------
    has_render_ref - find a refptr in a reference
    list
-------------------------------------------------*/

INLINE int has_render_ref(render_ref *list, void *refptr)
{
	render_ref *ref;

	/* skip if we already have one */
	for (ref = list; ref != NULL; ref = ref->next)
		if (ref->refptr == refptr)
			return TRUE;
	return FALSE;
}


/*-------------------------------------------------
    free_render_ref - free a previously
    allocated render reference
-------------------------------------------------*/

INLINE void free_render_ref(render_ref *ref)
{
	ref->next = render_ref_free_list;
	render_ref_free_list = ref;
}


/*-------------------------------------------------
    get_layer_and_blendmode - return the
    appropriate layer index and blendmode
-------------------------------------------------*/

INLINE int get_layer_and_blendmode(const layout_view *view, int index, int *blendmode)
{
    const int *layer_order = layer_order_standard;
    int layer;

	/*
        if we have multiple backdrop pieces and no overlays, render:
            backdrop (add) + screens (add) + bezels (alpha)
        else render:
            screens (add) + overlays (RGB multiply) + backdrop (add) + bezels (alpha)
    */
	if (view->itemlist[ITEM_LAYER_BACKDROP] != NULL && view->itemlist[ITEM_LAYER_BACKDROP]->next != NULL && view->itemlist[ITEM_LAYER_OVERLAY] == NULL)
		layer_order = layer_order_alternate;

	/* select the layer */
	layer = layer_order[index];

	/* if we want the blendmode as well, compute it */
	if (blendmode != NULL)
	{
		/* pick a blendmode */
		if (layer == ITEM_LAYER_SCREEN && layer_order == layer_order_standard)
			*blendmode = -1;
		else if (layer == ITEM_LAYER_SCREEN || (layer == ITEM_LAYER_BACKDROP && layer_order == layer_order_standard))
			*blendmode = BLENDMODE_ADD;
		else if (layer == ITEM_LAYER_OVERLAY)
			*blendmode = BLENDMODE_RGB_MULTIPLY;
		else
			*blendmode = BLENDMODE_ALPHA;
	}
	return layer;
}


/*-------------------------------------------------
    get_screen_container_by_index - get the
    screen container for this screen index
-------------------------------------------------*/

INLINE render_container *get_screen_container_by_index(int index)
{
	render_container *container;

	assert(index >= 0);

	/* get the container for the screen index */
	for (container = screen_container_list; container != NULL; container = container->next)
	{
		if (index == 0)
			break;
		index--;
	}

	assert(index == 0);

	return container;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    render_init - allocate base structures for
    the rendering system
-------------------------------------------------*/

void render_init(running_machine *machine)
{
	render_container **current_container_ptr = &screen_container_list;

	/* make sure we clean up after ourselves */
	machine->add_notifier(MACHINE_NOTIFY_EXIT, render_exit);

	/* set up the list of render targets */
	targetlist = NULL;

	/* zap the free lists */
	render_primitive_free_list = NULL;
	container_item_free_list = NULL;

	/* zap more variables */
	ui_target = NULL;

	/* create a UI container */
	ui_container = render_container_alloc(machine);

	/* create a container for each screen and determine its orientation */
	for (screen_device *screendev = screen_first(*machine); screendev != NULL; screendev = screen_next(screendev))
	{
		render_container *screen_container = render_container_alloc(machine);
		render_container **temp = &screen_container->next;
		render_container_user_settings settings;

		/* set the initial orientation and brightness/contrast/gamma */
		render_container_get_user_settings(screen_container, &settings);
		settings.orientation = machine->gamedrv->flags & ORIENTATION_MASK;
		settings.brightness = options_get_float(machine->options(), OPTION_BRIGHTNESS);
		settings.contrast = options_get_float(machine->options(), OPTION_CONTRAST);
		settings.gamma = options_get_float(machine->options(), OPTION_GAMMA);
		render_container_set_user_settings(screen_container, &settings);

		screen_container->screen = screendev;

		/* link it up */
		*current_container_ptr = screen_container;
		current_container_ptr = temp;
	}

	/* terminate list */
	*current_container_ptr = NULL;

	/* register callbacks */
	config_register(machine, "video", render_load, render_save);
}


/*-------------------------------------------------
    render_exit - free all rendering data
-------------------------------------------------*/

static void render_exit(running_machine &machine)
{
	render_texture **texture_ptr;
	render_container *container;

	/* free the UI container */
	if (ui_container != NULL)
		render_container_free(ui_container);

	/* free the screen container */
	for (container = screen_container_list; container != NULL; )
	{
		render_container *temp = container;
		container = temp->next;
		render_container_free(temp);
	}

	/* remove all non-head entries from the texture free list */
	for (texture_ptr = &render_texture_free_list; *texture_ptr != NULL; texture_ptr = &(*texture_ptr)->next)
		while (*texture_ptr != NULL && (*texture_ptr)->base != *texture_ptr)
			*texture_ptr = (*texture_ptr)->next;

	/* free the targets; this must be done before freeing the texture groups
       as that will forcefully free everything, and if it goes first, we may
       end up double-freeing textures of the render targets */
	while (targetlist != NULL)
		render_target_free(targetlist);

	/* free the screen overlay; similarly, do this before any of the following
       calls to avoid double-frees */
	global_free(screen_overlay);
	screen_overlay = NULL;

	/* free the texture groups */
	while (render_texture_free_list != NULL)
	{
		render_texture *temp = render_texture_free_list;
		render_texture_free_list = temp->next;
		global_free(temp);
	}

	/* free the render primitives */
	while (render_primitive_free_list != NULL)
	{
		render_primitive *temp = render_primitive_free_list;
		render_primitive_free_list = temp->next;
		global_free(temp);
	}

	/* free the render refs */
	while (render_ref_free_list != NULL)
	{
		render_ref *temp = render_ref_free_list;
		render_ref_free_list = temp->next;
		global_free(temp);
	}

	/* free the container items */
	while (container_item_free_list != NULL)
	{
		container_item *temp = container_item_free_list;
		container_item_free_list = temp->next;
		global_free(temp);
	}
}


/*-------------------------------------------------
    render_load - read and apply data from the
    configuration file
-------------------------------------------------*/

static void render_load(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	xml_data_node *targetnode;
	xml_data_node *screennode;
	xml_data_node *uinode;
	int tmpint;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == NULL)
		return;

	/* check the UI target */
	uinode = xml_get_sibling(parentnode->child, "interface");
	if (uinode != NULL)
	{
		render_target *target = render_target_get_indexed(xml_get_attribute_int(uinode, "target", 0));
		if (target != NULL)
			render_set_ui_target(target);
	}

	/* iterate over target nodes */
	for (targetnode = xml_get_sibling(parentnode->child, "target"); targetnode; targetnode = xml_get_sibling(targetnode->next, "target"))
	{
		render_target *target = render_target_get_indexed(xml_get_attribute_int(targetnode, "index", -1));
		if (target != NULL)
		{
			const char *viewname = xml_get_attribute_string(targetnode, "view", NULL);
			int viewnum;

			/* find the view */
			if (viewname != NULL)
				for (viewnum = 0; viewnum < 1000; viewnum++)
				{
					const char *testname = render_target_get_view_name(target, viewnum);
					if (testname == NULL)
						break;
					if (!strcmp(viewname, testname))
					{
						render_target_set_view(target, viewnum);
						break;
					}
				}

			/* modify the artwork config */
			tmpint = xml_get_attribute_int(targetnode, "backdrops", -1);
			if (tmpint == 0)
				render_target_set_layer_config(target, target->layerconfig & ~LAYER_CONFIG_ENABLE_BACKDROP);
			else if (tmpint == 1)
				render_target_set_layer_config(target, target->layerconfig | LAYER_CONFIG_ENABLE_BACKDROP);

			tmpint = xml_get_attribute_int(targetnode, "overlays", -1);
			if (tmpint == 0)
				render_target_set_layer_config(target, target->layerconfig & ~LAYER_CONFIG_ENABLE_OVERLAY);
			else if (tmpint == 1)
				render_target_set_layer_config(target, target->layerconfig | LAYER_CONFIG_ENABLE_OVERLAY);

			tmpint = xml_get_attribute_int(targetnode, "bezels", -1);
			if (tmpint == 0)
				render_target_set_layer_config(target, target->layerconfig & ~LAYER_CONFIG_ENABLE_BEZEL);
			else if (tmpint == 1)
				render_target_set_layer_config(target, target->layerconfig | LAYER_CONFIG_ENABLE_BEZEL);

			tmpint = xml_get_attribute_int(targetnode, "zoom", -1);
			if (tmpint == 0)
				render_target_set_layer_config(target, target->layerconfig & ~LAYER_CONFIG_ZOOM_TO_SCREEN);
			else if (tmpint == 1)
				render_target_set_layer_config(target, target->layerconfig | LAYER_CONFIG_ZOOM_TO_SCREEN);

			/* apply orientation */
			tmpint = xml_get_attribute_int(targetnode, "rotate", -1);
			if (tmpint != -1)
			{
				if (tmpint == 90)
					tmpint = ROT90;
				else if (tmpint == 180)
					tmpint = ROT180;
				else if (tmpint == 270)
					tmpint = ROT270;
				else
					tmpint = ROT0;
				render_target_set_orientation(target, orientation_add(tmpint, target->orientation));

				/* apply the opposite orientation to the UI */
				if (target == render_get_ui_target())
				{
					render_container_user_settings settings;

					render_container_get_user_settings(ui_container, &settings);
					settings.orientation = orientation_add(orientation_reverse(tmpint), settings.orientation);
					render_container_set_user_settings(ui_container, &settings);
				}
			}
		}
	}

	/* iterate over screen nodes */
	for (screennode = xml_get_sibling(parentnode->child, "screen"); screennode; screennode = xml_get_sibling(screennode->next, "screen"))
	{
		int index = xml_get_attribute_int(screennode, "index", -1);
		render_container *container = get_screen_container_by_index(index);
		render_container_user_settings settings;

		/* fetch current settings */
		render_container_get_user_settings(container, &settings);

		/* fetch color controls */
		settings.brightness = xml_get_attribute_float(screennode, "brightness", settings.brightness);
		settings.contrast = xml_get_attribute_float(screennode, "contrast", settings.contrast);
		settings.gamma = xml_get_attribute_float(screennode, "gamma", settings.gamma);

		/* fetch positioning controls */
		settings.xoffset = xml_get_attribute_float(screennode, "hoffset", settings.xoffset);
		settings.xscale = xml_get_attribute_float(screennode, "hstretch", settings.xscale);
		settings.yoffset = xml_get_attribute_float(screennode, "voffset", settings.yoffset);
		settings.yscale = xml_get_attribute_float(screennode, "vstretch", settings.yscale);

		/* set the new values */
		render_container_set_user_settings(container, &settings);
	}
}


/*-------------------------------------------------
    render_save - save data to the configuration
    file
-------------------------------------------------*/

static void render_save(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	render_target *target;
	render_container *container;
	int scrnum;
	int targetnum = 0;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* write out the interface target */
	target = render_get_ui_target();
	if (target != render_target_get_indexed(0))
	{
		xml_data_node *uinode;

		/* find the target index */
		for (targetnum = 0; ; targetnum++)
			if (render_target_get_indexed(targetnum) == target)
				break;

		/* create a node for it */
		uinode = xml_add_child(parentnode, "interface", NULL);
		if (uinode != NULL)
			xml_set_attribute_int(uinode, "target", targetnum);
	}

	/* iterate over targets */
	for (targetnum = 0; targetnum < 1000; targetnum++)
	{
		xml_data_node *targetnode;

		/* get this target and break when we fail */
		target = render_target_get_indexed(targetnum);
		if (target == NULL)
			break;

		/* create a node */
		targetnode = xml_add_child(parentnode, "target", NULL);
		if (targetnode != NULL)
		{
			int changed = FALSE;

			/* output the basics */
			xml_set_attribute_int(targetnode, "index", targetnum);

			/* output the view */
			if (target->curview != target->base_view)
			{
				xml_set_attribute(targetnode, "view", target->curview->name);
				changed = TRUE;
			}

			/* output the layer config */
			if (target->layerconfig != target->base_layerconfig)
			{
				xml_set_attribute_int(targetnode, "backdrops", (target->layerconfig & LAYER_CONFIG_ENABLE_BACKDROP) != 0);
				xml_set_attribute_int(targetnode, "overlays", (target->layerconfig & LAYER_CONFIG_ENABLE_OVERLAY) != 0);
				xml_set_attribute_int(targetnode, "bezels", (target->layerconfig & LAYER_CONFIG_ENABLE_BEZEL) != 0);
				xml_set_attribute_int(targetnode, "zoom", (target->layerconfig & LAYER_CONFIG_ZOOM_TO_SCREEN) != 0);
				changed = TRUE;
			}

			/* output rotation */
			if (target->orientation != target->base_orientation)
			{
				int rotate = 0;
				if (orientation_add(ROT90, target->base_orientation) == target->orientation)
					rotate = 90;
				else if (orientation_add(ROT180, target->base_orientation) == target->orientation)
					rotate = 180;
				else if (orientation_add(ROT270, target->base_orientation) == target->orientation)
					rotate = 270;
				assert(rotate != 0);
				xml_set_attribute_int(targetnode, "rotate", rotate);
				changed = TRUE;
			}

			/* if nothing changed, kill the node */
			if (!changed)
				xml_delete_node(targetnode);
		}
	}

	/* iterate over screen containers */
	for (container = screen_container_list, scrnum = 0; container != NULL; container = container->next, scrnum++)
	{
		xml_data_node *screennode;

		/* create a node */
		screennode = xml_add_child(parentnode, "screen", NULL);

		if (screennode != NULL)
		{
			int changed = FALSE;

			/* output the basics */
			xml_set_attribute_int(screennode, "index", scrnum);

			/* output the color controls */
			if (container->brightness != options_get_float(machine->options(), OPTION_BRIGHTNESS))
			{
				xml_set_attribute_float(screennode, "brightness", container->brightness);
				changed = TRUE;
			}

			if (container->contrast != options_get_float(machine->options(), OPTION_CONTRAST))
			{
				xml_set_attribute_float(screennode, "contrast", container->contrast);
				changed = TRUE;
			}

			if (container->gamma != options_get_float(machine->options(), OPTION_GAMMA))
			{
				xml_set_attribute_float(screennode, "gamma", container->gamma);
				changed = TRUE;
			}

			/* output the positioning controls */
			if (container->xoffset != 0.0f)
			{
				xml_set_attribute_float(screennode, "hoffset", container->xoffset);
				changed = TRUE;
			}

			if (container->xscale != 1.0f)
			{
				xml_set_attribute_float(screennode, "hstretch", container->xscale);
				changed = TRUE;
			}

			if (container->yoffset != 0.0f)
			{
				xml_set_attribute_float(screennode, "voffset", container->yoffset);
				changed = TRUE;
			}

			if (container->yscale != 1.0f)
			{
				xml_set_attribute_float(screennode, "vstretch", container->yscale);
				changed = TRUE;
			}

			/* if nothing changed, kill the node */
			if (!changed)
				xml_delete_node(screennode);
		}
	}
}


/*-------------------------------------------------
    render_is_live_screen - return if the screen
    is 'live'
-------------------------------------------------*/

int render_is_live_screen(device_t *screen)
{
	render_target *target;
	int screen_index;
	UINT32 bitmask = 0;

	assert(screen != NULL);
	assert(screen->machine != NULL);
	assert(screen->machine->config != NULL);

	screen_index = screen->machine->m_devicelist.index(SCREEN, screen->tag());

	assert(screen_index != -1);

	/* iterate over all live targets and or together their screen masks */
	for (target = targetlist; target != NULL; target = target->next)
		bitmask |= target->curview->screens;

	return (bitmask & (1 << screen_index)) ? TRUE : FALSE;
}


/*-------------------------------------------------
    render_get_max_update_rate - return the
    smallest maximum update rate across all targets
-------------------------------------------------*/

float render_get_max_update_rate(void)
{
	render_target *target;
	float minimum = 0;

	/* iterate over all live targets and or together their screen masks */
	for (target = targetlist; target != NULL; target = target->next)
		if (target->max_refresh != 0)
		{
			if (minimum == 0)
				minimum = target->max_refresh;
			else
				minimum = MIN(target->max_refresh, minimum);
		}

	return minimum;
}


/*-------------------------------------------------
    render_set_ui_target - select the UI target
-------------------------------------------------*/

void render_set_ui_target(render_target *target)
{
	assert(target != NULL);
	ui_target = target;
}


/*-------------------------------------------------
    render_get_ui_target - return the UI target
-------------------------------------------------*/

render_target *render_get_ui_target(void)
{
	assert(ui_target != NULL);
	return ui_target;
}


/*-------------------------------------------------
    render_get_ui_aspect - return the aspect
    ratio for UI fonts
-------------------------------------------------*/

float render_get_ui_aspect(void)
{
	render_target *target = render_get_ui_target();
	if (target != NULL)
	{
		int orient = orientation_add(target->orientation, ui_container->orientation);
		float aspect;

		/* based on the orientation of the target, compute height/width or width/height */
		if (!(orient & ORIENTATION_SWAP_XY))
			aspect = (float)target->height / (float)target->width;
		else
			aspect = (float)target->width / (float)target->height;

		/* if we have a valid pixel aspect, apply that and return */
		if (target->pixel_aspect != 0.0f)
			return aspect / target->pixel_aspect;

		/* if not, clamp for extreme proportions */
		if (aspect < 0.66f)
			aspect = 0.66f;
		if (aspect > 1.5f)
			aspect = 1.5f;
		return aspect;
	}

	return 1.0f;
}



/***************************************************************************
    RENDER TARGETS
***************************************************************************/

/*-------------------------------------------------
    render_target_alloc - allocate a new render
    target
-------------------------------------------------*/

render_target *render_target_alloc(running_machine *machine, const char *layoutfile, UINT32 flags)
{
	render_target *target;
	render_target **nextptr;
	int listnum;

	/* allocate memory for the target */
	target = global_alloc_clear(render_target);

	/* add it to the end of the list */
	for (nextptr = &targetlist; *nextptr != NULL; nextptr = &(*nextptr)->next) ;
	*nextptr = target;

	/* fill in the basics with reasonable defaults */
	target->machine = machine;
	target->flags = flags;
	target->width = 640;
	target->height = 480;
	target->pixel_aspect = 0.0f;
	target->orientation = ROT0;
	target->layerconfig = LAYER_CONFIG_DEFAULT;
	target->maxtexwidth = 65536;
	target->maxtexheight = 65536;

	/* determine the base layer configuration based on options */
	target->base_layerconfig = LAYER_CONFIG_DEFAULT;
	if (!options_get_bool(machine->options(), OPTION_USE_BACKDROPS)) target->base_layerconfig &= ~LAYER_CONFIG_ENABLE_BACKDROP;
	if (!options_get_bool(machine->options(), OPTION_USE_OVERLAYS)) target->base_layerconfig &= ~LAYER_CONFIG_ENABLE_OVERLAY;
	if (!options_get_bool(machine->options(), OPTION_USE_BEZELS)) target->base_layerconfig &= ~LAYER_CONFIG_ENABLE_BEZEL;
	if (options_get_bool(machine->options(), OPTION_ARTWORK_CROP)) target->base_layerconfig |= LAYER_CONFIG_ZOOM_TO_SCREEN;

	/* determine the base orientation based on options */
	target->orientation = ROT0;
	if (!options_get_bool(machine->options(), OPTION_ROTATE))
		target->base_orientation = orientation_reverse(machine->gamedrv->flags & ORIENTATION_MASK);

	/* rotate left/right */
	if (options_get_bool(machine->options(), OPTION_ROR) || (options_get_bool(machine->options(), OPTION_AUTOROR) && (machine->gamedrv->flags & ORIENTATION_SWAP_XY)))
		target->base_orientation = orientation_add(ROT90, target->base_orientation);
	if (options_get_bool(machine->options(), OPTION_ROL) || (options_get_bool(machine->options(), OPTION_AUTOROL) && (machine->gamedrv->flags & ORIENTATION_SWAP_XY)))
		target->base_orientation = orientation_add(ROT270, target->base_orientation);

	/* flip X/Y */
	if (options_get_bool(machine->options(), OPTION_FLIPX))
		target->base_orientation ^= ORIENTATION_FLIP_X;
	if (options_get_bool(machine->options(), OPTION_FLIPY))
		target->base_orientation ^= ORIENTATION_FLIP_Y;

	/* set the orientation and layerconfig equal to the base */
	target->orientation = target->base_orientation;
	target->layerconfig = target->base_layerconfig;

	/* allocate a lock for the primitive list */
	for (listnum = 0; listnum < ARRAY_LENGTH(target->primlist); listnum++)
		target->primlist[listnum].lock = osd_lock_alloc();

	/* load the layout files */
	if (load_layout_files(target, layoutfile, flags & RENDER_CREATE_SINGLE_FILE))
	{
		render_target_free(target);
		return NULL;
	}

	/* set the current view to the first one */
	render_target_set_view(target, 0);

	/* make us the UI target if there is none */
	if (ui_target == NULL && !(flags & RENDER_CREATE_HIDDEN))
		render_set_ui_target(target);
	return target;
}


/*-------------------------------------------------
    render_target_free - free memory for a render
    target
-------------------------------------------------*/

void render_target_free(render_target *target)
{
	render_target **curr;
	int listnum;

	/* remove us from the list */
	for (curr = &targetlist; *curr != target; curr = &(*curr)->next) ;
	*curr = target->next;

	/* free any primitives */
	for (listnum = 0; listnum < ARRAY_LENGTH(target->primlist); listnum++)
	{
		release_render_list(&target->primlist[listnum]);
		osd_lock_free(target->primlist[listnum].lock);
	}

	/* free the layout files */
	while (target->filelist != NULL)
	{
		layout_file *temp = target->filelist;
		target->filelist = temp->next;
		layout_file_free(temp);
	}

	/* free the target itself */
	global_free(target);
}


/*-------------------------------------------------
    render_target_get_indexed - get a render_target
    by index
-------------------------------------------------*/

render_target *render_target_get_indexed(int index)
{
	render_target *target;

	/* count up the targets until we hit the requested index */
	for (target = targetlist; target != NULL; target = target->next)
		if (!(target->flags & RENDER_CREATE_HIDDEN))
			if (index-- == 0)
				return target;
	return NULL;
}


/*-------------------------------------------------
    render_target_get_view_name - return the
    name of the indexed view, or NULL if it
    doesn't exist
-------------------------------------------------*/

const char *render_target_get_view_name(render_target *target, int viewindex)
{
	layout_file *file;
	layout_view *view;

	/* return the name from the indexed view */
	for (file = target->filelist; file != NULL; file = file->next)
		for (view = file->viewlist; view != NULL; view = view->next)
			if (!(target->flags & RENDER_CREATE_NO_ART) || !layout_view_has_art(view))
				if (viewindex-- == 0)
					return view->name;

	return NULL;
}


/*-------------------------------------------------
    render_target_get_translated_view_name - return the
    localized name of the indexed view
-------------------------------------------------*/

const char *render_target_get_translated_view_name(render_target *target, int viewindex)
{
	const char *s = render_target_get_view_name(target, viewindex);
	const char *idx[8];
	const char **pp;
	astring temp, res;

	if (!s)
		return NULL;

	pp = idx;
	while (*s)
	{
		if (isdigit(*s))
		{
			*pp++ = s;

			temp.cat("%");
			temp.cat("d");

			for (s++; *s; s++)
				if (!isdigit(*s))
					break;
		}
		else
			temp.cat(s++, 1);
	}

	s = _(temp);

	pp = idx;
	while (*s)
	{
		if (s[0] == '%' && s[1] == 'd')
		{
			s += 2;

			while (isdigit(**pp))
				res.cat((*pp)++, 1);
			pp++;
		}
		else
			res.cat(s++, 1);
	}

	s = mame_strdup(res);

	return s;
}


/*-------------------------------------------------
    render_target_get_view_screens - return a
    bitmask of which screens are visible on a
    given view
-------------------------------------------------*/

UINT32 render_target_get_view_screens(render_target *target, int viewindex)
{
	layout_file *file;
	layout_view *view;

	/* return the name from the indexed view */
	for (file = target->filelist; file != NULL; file = file->next)
		for (view = file->viewlist; view != NULL; view = view->next)
			if (!(target->flags & RENDER_CREATE_NO_ART) || !layout_view_has_art(view))
				if (viewindex-- == 0)
					return view->screens;

	return 0;
}


/*-------------------------------------------------
    render_target_get_bounds - get the bounds and
    pixel aspect of a target
-------------------------------------------------*/

void render_target_get_bounds(render_target *target, INT32 *width, INT32 *height, float *pixel_aspect)
{
	if (width != NULL)
		*width = target->width;
	if (height != NULL)
		*height = target->height;
	if (pixel_aspect != NULL)
		*pixel_aspect = target->pixel_aspect;
}


/*-------------------------------------------------
    render_target_set_bounds - set the bounds and
    pixel aspect of a target
-------------------------------------------------*/

void render_target_set_bounds(render_target *target, INT32 width, INT32 height, float pixel_aspect)
{
	target->width = width;
	target->height = height;
	target->bounds.x0 = target->bounds.y0 = 0;
	target->bounds.x1 = (float)width;
	target->bounds.y1 = (float)height;
	target->pixel_aspect = pixel_aspect;
}


/*-------------------------------------------------
    render_target_get_max_update_rate - get the
    maximum update rate (refresh rate) of a target,
    or 0 if no maximum
-------------------------------------------------*/

float render_target_get_max_update_rate(render_target *target)
{
	return target->max_refresh;
}


/*-------------------------------------------------
    render_target_set_max_update_rate - set the
    maximum update rate (refresh rate) of a target,
    or 0 if no maximum
-------------------------------------------------*/

void render_target_set_max_update_rate(render_target *target, float updates_per_second)
{
	target->max_refresh = updates_per_second;
}


/*-------------------------------------------------
    render_target_get_orientation - get the
    orientation of a target
-------------------------------------------------*/

int render_target_get_orientation(render_target *target)
{
	return target->orientation;
}


/*-------------------------------------------------
    render_target_set_orientation - set the
    orientation of a target
-------------------------------------------------*/

void render_target_set_orientation(render_target *target, int orientation)
{
	target->orientation = orientation;
}


/*-------------------------------------------------
    render_target_get_layer_config - get the
    layer config of a target
-------------------------------------------------*/

int render_target_get_layer_config(render_target *target)
{
	return target->layerconfig;
}


/*-------------------------------------------------
    render_target_set_layer_config - set the
    layer config of a target
-------------------------------------------------*/

void render_target_set_layer_config(render_target *target, int layerconfig)
{
	target->layerconfig = layerconfig;
	layout_view_recompute(target->curview, layerconfig);
}


/*-------------------------------------------------
    render_target_get_view - return the currently
    selected view index
-------------------------------------------------*/

int render_target_get_view(render_target *target)
{
	layout_file *file;
	layout_view *view;
	int index = 0;

	/* find the first named match */
	for (file = target->filelist; file != NULL; file = file->next)
		for (view = file->viewlist; view != NULL; view = view->next)
			if (!(target->flags & RENDER_CREATE_NO_ART) || !layout_view_has_art(view))
			{
				if (target->curview == view)
					return index;
				index++;
			}
	return 0;
}


/*-------------------------------------------------
    render_target_set_view - dynamically change
    the view for a target
-------------------------------------------------*/

void render_target_set_view(render_target *target, int viewindex)
{
	layout_file *file;
	layout_view *view;

	/* find the first named match */
	for (file = target->filelist; file != NULL; file = file->next)
		for (view = file->viewlist; view != NULL; view = view->next)
			if (!(target->flags & RENDER_CREATE_NO_ART) || !layout_view_has_art(view))
				if (viewindex-- == 0)
				{
					target->curview = view;
					layout_view_recompute(view, target->layerconfig);
					break;
				}
}


/*-------------------------------------------------
    render_target_set_max_texture_size - set the
    upper bound on the texture size
-------------------------------------------------*/

void render_target_set_max_texture_size(render_target *target, int maxwidth, int maxheight)
{
	target->maxtexwidth = maxwidth;
	target->maxtexheight = maxheight;
}


/*-------------------------------------------------
    render_target_compute_visible_area - compute
    the visible area for the given target with
    the current layout and proposed new parameters
-------------------------------------------------*/

void render_target_compute_visible_area(render_target *target, INT32 target_width, INT32 target_height, float target_pixel_aspect, int target_orientation, INT32 *visible_width, INT32 *visible_height)
{
	float width, height;
	float scale;

	/* constrained case */
	if (target_pixel_aspect != 0.0f)
	{
		/* start with the aspect ratio of the square pixel layout */
		width = ((target->layerconfig & LAYER_CONFIG_ZOOM_TO_SCREEN) && target->curview->screens > 0) ? target->curview->scraspect : target->curview->aspect;
		height = 1.0f;

		/* first apply target orientation */
		if (target_orientation & ORIENTATION_SWAP_XY)
			FSWAP(width, height);

		/* apply the target pixel aspect ratio */
		height *= target_pixel_aspect;

		/* based on the height/width ratio of the source and target, compute the scale factor */
		if (width / height > (float)target_width / (float)target_height)
			scale = (float)target_width / width;
		else
			scale = (float)target_height / height;
	}

	/* stretch-to-fit case */
	else
	{
		width = (float)target_width;
		height = (float)target_height;
		scale = 1.0f;
	}

	// set the final width/height
	visible_width = render_round_nearest(width * scale);
	visible_height = render_round_nearest(height * scale);
}


//-------------------------------------------------
//  compute_minimum_size - compute the "minimum"
//  size of a target, which is the smallest bounds
//  that will ensure at least 1 target pixel per
//  source pixel for all included screens
//-------------------------------------------------

void render_target::compute_minimum_size(INT32 &minwidth, INT32 &minheight)
{
	float maxxscale = 1.0f, maxyscale = 1.0f;
	int screens_considered = 0;

	// scan the current view for all screens
	for (int layer = 0; layer < ITEM_LAYER_MAX; layer++)

		// iterate over items in the layer
		for (view_item *item = m_curview->itemlist[layer]; item != NULL; item = item->next)
			if (item->element == NULL)
			{
				const screen_device_config *scrconfig = downcast<const screen_device_config *>(m_manager.machine().config->m_devicelist.find(SCREEN, item->index));
				screen_device *screendev = m_manager.machine().device<screen_device>(scrconfig->tag());

				// we may be called very early, before machine->visible_area is initialized; handle that case
				const rectangle vectorvis = { 0, 639, 0, 479 };
				const rectangle *visarea = NULL;
				if (scrconfig->screen_type() == SCREEN_TYPE_VECTOR)
					visarea = &vectorvis;
				else if (screendev != NULL && screendev->started())
					visarea = &screendev->visible_area();
				else
					visarea = &scrconfig->visible_area();

				// apply target orientation to the bounds
				render_bounds bounds = item->bounds;
				apply_orientation(bounds, m_orientation);
				normalize_bounds(bounds);

				// based on the orientation of the screen container, check the bitmap
				render_container *container = m_manager.m_screen_container_list.find(item->index);
				float xscale, yscale;
				if (!(orientation_add(m_orientation, container->orientation()) & ORIENTATION_SWAP_XY))
				{
					xscale = (float)(visarea->max_x + 1 - visarea->min_x) / (bounds.x1 - bounds.x0);
					yscale = (float)(visarea->max_y + 1 - visarea->min_y) / (bounds.y1 - bounds.y0);
				}
				else
				{
					xscale = (float)(visarea->max_y + 1 - visarea->min_y) / (bounds.x1 - bounds.x0);
					yscale = (float)(visarea->max_x + 1 - visarea->min_x) / (bounds.y1 - bounds.y0);
				}

				// pick the greater
				maxxscale = MAX(xscale, maxxscale);
				maxyscale = MAX(yscale, maxyscale);
				screens_considered++;
			}

	// if there were no screens considered, pick a nominal default
	if (screens_considered == 0)
	{
		maxxscale = 640.0f;
		maxyscale = 480.0f;
	}

	// round up
	minwidth = render_round_nearest(maxxscale);
	minheight = render_round_nearest(maxyscale);
}


//-------------------------------------------------
//  get_primitives - return a list of primitives
//  for a given render target
//-------------------------------------------------

render_primitive_list &render_target::get_primitives()
{
	// remember the base values if this is the first frame
	if (m_base_view == NULL)
		m_base_view = m_curview;

	// switch to the next primitive list
	render_primitive_list &list = m_primlist[m_listindex];
	m_listindex = (m_listindex + 1) % ARRAY_LENGTH(m_primlist);
	list.acquire_lock();

	// free any previous primitives
	list.release_all();

	// compute the visible width/height
	INT32 viswidth, visheight;
	compute_visible_area(m_width, m_height, m_pixel_aspect, m_orientation, viswidth, visheight);

	// create a root transform for the target
	object_transform root_xform;
	root_xform.xoffs = (float)(m_width - viswidth) / 2;
	root_xform.yoffs = (float)(m_height - visheight) / 2;
	root_xform.xscale = (float)viswidth;
	root_xform.yscale = (float)visheight;
	root_xform.color.r = root_xform.color.g = root_xform.color.b = root_xform.color.a = 1.0f;
	root_xform.orientation = m_orientation;
    root_xform.no_center = false;

	// iterate over layers back-to-front, but only if we're running
	if (m_manager.machine().phase() >= MACHINE_PHASE_RESET)
		for (int layernum = 0; layernum < ITEM_LAYER_MAX; layernum++)
		{
			int blendmode;
			int layer = get_layer_and_blendmode(*m_curview, layernum, blendmode);
			if (m_curview->layenabled[layer])
			{
				// iterate over items in the layer
				for (view_item *item = m_curview->itemlist[layer]; item != NULL; item = item->next)
				{
					// first apply orientation to the bounds
					render_bounds bounds = item->bounds;
					apply_orientation(bounds, root_xform.orientation);
					normalize_bounds(bounds);

					// apply the transform to the item
					object_transform item_xform;
					item_xform.xoffs = root_xform.xoffs + bounds.x0 * root_xform.xscale;
					item_xform.yoffs = root_xform.yoffs + bounds.y0 * root_xform.yscale;
					item_xform.xscale = (bounds.x1 - bounds.x0) * root_xform.xscale;
					item_xform.yscale = (bounds.y1 - bounds.y0) * root_xform.yscale;
					item_xform.color.r = item->color.r * root_xform.color.r;
					item_xform.color.g = item->color.g * root_xform.color.g;
					item_xform.color.b = item->color.b * root_xform.color.b;
					item_xform.color.a = item->color.a * root_xform.color.a;
					item_xform.orientation = orientation_add(item->orientation, root_xform.orientation);
                    item_xform.no_center = false;

					// if there is no associated element, it must be a screen element
					if (item->element != NULL)
					{
						int state = 0;
						if (item->output_name[0] != 0)
							state = output_get_value(item->output_name);
						else if (item->input_tag[0] != 0)
						{
							const input_field_config *field = input_field_by_tag_and_mask(m_manager.machine().m_portlist, item->input_tag, item->input_mask);
							if (field != NULL)
								state = ((input_port_read_safe(&m_manager.machine(), item->input_tag, 0) ^ field->defvalue) & item->input_mask) ? 1 : 0;
						}
						add_element_primitives(list, item_xform, *item->element, state, blendmode);
					}
					else
					{
						render_container *container = m_manager.m_screen_container_list.find(item->index);
						add_container_primitives(list, item_xform, *container, blendmode);
					}
				}
			}
		}

	// if we are not in the running stage, draw an outer box
	else
	{
		render_primitive *prim = list.alloc(render_primitive::QUAD);
		set_render_bounds_xy(&prim->bounds, 0.0f, 0.0f, (float)m_width, (float)m_height);
		set_render_color(&prim->color, 1.0f, 1.0f, 1.0f, 1.0f);
		prim->texture.base = NULL;
		prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
		list.append(*prim);

		if (m_width > 1 && m_height > 1)
		{
			prim = list.alloc(render_primitive::QUAD);
			set_render_bounds_xy(&prim->bounds, 1.0f, 1.0f, (float)(m_width - 1), (float)(m_height - 1));
			set_render_color(&prim->color, 1.0f, 0.0f, 0.0f, 0.0f);
			prim->texture.base = NULL;
			prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
			list.append(*prim);
		}
	}

	// process the debug containers
	for (render_container *debug = m_debug_containers.first(); debug != NULL; debug = debug->next())
	{
		object_transform ui_xform;
		ui_xform.xoffs = 0;
		ui_xform.yoffs = 0;
		ui_xform.xscale = (float)m_width;
		ui_xform.yscale = (float)m_height;
		ui_xform.color.r = ui_xform.color.g = ui_xform.color.b = ui_xform.color.a = 1.0f;
		ui_xform.color.a = 0.9f;
		ui_xform.orientation = m_orientation;
		ui_xform.no_center = true;

		// add UI elements
		add_container_primitives(list, ui_xform, *debug, BLENDMODE_ALPHA);
	}

	// process the UI if we are the UI target
	if (is_ui_target())
	{
		// compute the transform for the UI
		object_transform ui_xform;
		ui_xform.xoffs = 0;
		ui_xform.yoffs = 0;
		ui_xform.xscale = (float) m_width;
		ui_xform.yscale = (float) m_height;
		ui_xform.color.r = ui_xform.color.g = ui_xform.color.b = ui_xform.color.a = 1.0f;
		ui_xform.orientation = m_orientation;
        ui_xform.no_center = false;

		// add UI elements
		add_container_primitives(list, ui_xform, m_manager.ui_container(), BLENDMODE_ALPHA);
	}

	// optimize the list before handing it off
	add_clear_and_optimize_primitive_list(list);
	list.release_lock();
	return list;
}


//-------------------------------------------------
//  map_point_container - attempts to map a point
//  on the specified render_target to the
//  specified container, if possible
//-------------------------------------------------

bool render_target::map_point_container(INT32 target_x, INT32 target_y, render_container &container, float &container_x, float &container_y)
{
	view_item *item;
	return map_point_internal(target_x, target_y, &container, container_x, container_y, item);
}


//-------------------------------------------------
//  map_point_input - attempts to map a point on
//  the specified render_target to the specified
//  container, if possible
//-------------------------------------------------

bool render_target::map_point_input(INT32 target_x, INT32 target_y, const char *&input_tag, UINT32 &input_mask, float &input_x, float &input_y)
{
	view_item *item = NULL;

	bool result = map_point_internal(target_x, target_y, NULL, input_x, input_y, item);
	if (result && item != NULL)
	{
		input_tag = item->input_tag;
		input_mask = item->input_mask;
	}
	return result;
}


//-------------------------------------------------
//  invalidate_all - if any of our primitive lists
//  contain a reference to the given pointer,
//  clear them
//-------------------------------------------------

void render_target::invalidate_all(void *refptr)
{
	// iterate through all our primitive lists
	for (int listnum = 0; listnum < ARRAY_LENGTH(m_primlist); listnum++)
	{
		render_primitive_list &list = m_primlist[listnum];

		// if we have a reference to this object, release our list
		list.acquire_lock();
		if (list.has_reference(refptr))
			list.release_all();
		list.release_lock();
	}
}


//-------------------------------------------------
//  debug_alloc - allocate a container for a debug
//  view
//-------------------------------------------------

render_container *render_target::debug_alloc()
{
	return &m_debug_containers.append(*m_manager.container_alloc());
}


//-------------------------------------------------
//  debug_free - free a container for a debug view
//-------------------------------------------------

void render_target::debug_free(render_container &container)
{
	m_debug_containers.remove(container);
}


//-------------------------------------------------
//  debug_top - move a debug view container to
//  the top of the list
//-------------------------------------------------

void render_target::debug_top(render_container &container)
{
	m_debug_containers.prepend(m_debug_containers.detach(container));
}


//-------------------------------------------------
//  load_layout_files - load layout files for a
//  given render target
//-------------------------------------------------

void render_target::load_layout_files(const char *layoutfile, bool singlefile)
{
	layout_file **nextfile = &m_filelist;

	// if there's an explicit file, load that first
	const char *basename = m_manager.machine().basename();
	if (layoutfile != NULL)
	{
		*nextfile = layout_file_load(m_manager.machine(), basename, layoutfile);
		if (*nextfile != NULL)
			nextfile = &(*nextfile)->next;
	}

	// if we're only loading this file, we know our final result
	if (singlefile)
		return;

	// try to load a file based on the driver name
	const game_driver *gamedrv = m_manager.machine().gamedrv;
	*nextfile = layout_file_load(m_manager.machine(), basename, gamedrv->name);
	if (*nextfile == NULL)
		*nextfile = layout_file_load(m_manager.machine(), basename, "default");
	if (*nextfile != NULL)
		nextfile = &(*nextfile)->next;

	// if a default view has been specified, use that as a fallback
	if (gamedrv->default_layout != NULL)
	{
		*nextfile = layout_file_load(m_manager.machine(), NULL, gamedrv->default_layout);
		if (*nextfile != NULL)
			nextfile = &(*nextfile)->next;
	}
	if (m_manager.machine().m_config.m_default_layout != NULL)
	{
		*nextfile = layout_file_load(m_manager.machine(), NULL, m_manager.machine().m_config.m_default_layout);
		if (*nextfile != NULL)
			nextfile = &(*nextfile)->next;
	}

	// try to load another file based on the parent driver name
	const game_driver *cloneof = driver_get_clone(gamedrv);
	if (cloneof != NULL)
	{
		*nextfile = layout_file_load(m_manager.machine(), cloneof->name, cloneof->name);
		if (*nextfile == NULL)
			*nextfile = layout_file_load(m_manager.machine(), cloneof->name, "default");
		if (*nextfile != NULL)
			nextfile = &(*nextfile)->next;
	}

	// now do the built-in layouts for single-screen games
	if (screen_count(m_manager.machine().m_config) == 1)
	{
		if (gamedrv->flags & ORIENTATION_SWAP_XY)
			*nextfile = layout_file_load(m_manager.machine(), NULL, layout_vertical);
		else
			*nextfile = layout_file_load(m_manager.machine(), NULL, layout_horizont);
		assert_always(*nextfile != NULL, "Couldn't parse default layout??");
		nextfile = &(*nextfile)->next;
	}
}


//-------------------------------------------------
//  add_container_primitives - add primitives
//  based on the container
//-------------------------------------------------

void render_target::add_container_primitives(render_primitive_list &list, const object_transform &xform, render_container &container, int blendmode)
{
	// first update the palette for the container, if it is dirty
	container.update_palette();

	// compute the clip rect
	render_bounds cliprect;
	cliprect.x0 = xform.xoffs;
	cliprect.y0 = xform.yoffs;
	cliprect.x1 = xform.xoffs + xform.xscale;
	cliprect.y1 = xform.yoffs + xform.yscale;
	sect_render_bounds(&cliprect, &m_bounds);

	// compute the container transform
	object_transform container_xform;
	container_xform.orientation = orientation_add(container.orientation(), xform.orientation);
	{
		float xscale = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.yscale() : container.xscale();
		float yscale = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.xscale() : container.yscale();
		float xoffs = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.yoffset() : container.xoffset();
		float yoffs = (container_xform.orientation & ORIENTATION_SWAP_XY) ? container.xoffset() : container.yoffset();
		if (container_xform.orientation & ORIENTATION_FLIP_X) xoffs = -xoffs;
		if (container_xform.orientation & ORIENTATION_FLIP_Y) yoffs = -yoffs;
		container_xform.xscale = xform.xscale * xscale;
		container_xform.yscale = xform.yscale * yscale;
		if (xform.no_center)
		{
			container_xform.xoffs = xform.xscale * (xoffs) + xform.xoffs;
			container_xform.yoffs = xform.yscale * (yoffs) + xform.yoffs;
		}
		else
		{
			container_xform.xoffs = xform.xscale * (0.5f - 0.5f * xscale + xoffs) + xform.xoffs;
			container_xform.yoffs = xform.yscale * (0.5f - 0.5f * yscale + yoffs) + xform.yoffs;
		}
		container_xform.color = xform.color;
	}

	// iterate over elements
	for (render_container::item *curitem = container.first_item(); curitem != NULL; curitem = curitem->next())
	{
		// compute the oriented bounds
		render_bounds bounds = curitem->bounds();
		apply_orientation(bounds, container_xform.orientation);

		// allocate the primitive and set the transformed bounds/color data
		render_primitive *prim = list.alloc(render_primitive::INVALID);
		prim->bounds.x0 = render_round_nearest(container_xform.xoffs + bounds.x0 * container_xform.xscale);
		prim->bounds.y0 = render_round_nearest(container_xform.yoffs + bounds.y0 * container_xform.yscale);
		if (curitem->internal() & INTERNAL_FLAG_CHAR)
		{
			prim->bounds.x1 = prim->bounds.x0 + render_round_nearest((bounds.x1 - bounds.x0) * container_xform.xscale);
			prim->bounds.y1 = prim->bounds.y0 + render_round_nearest((bounds.y1 - bounds.y0) * container_xform.yscale);
		}
		else
		{
			prim->bounds.x1 = render_round_nearest(container_xform.xoffs + bounds.x1 * container_xform.xscale);
			prim->bounds.y1 = render_round_nearest(container_xform.yoffs + bounds.y1 * container_xform.yscale);
		}

		// compute the color of the primitive
		prim->color.r = container_xform.color.r * curitem->color().r;
		prim->color.g = container_xform.color.g * curitem->color().g;
		prim->color.b = container_xform.color.b * curitem->color().b;
		prim->color.a = container_xform.color.a * curitem->color().a;

		// now switch off the type
		bool clipped = true;
		switch (curitem->type())
		{
			case CONTAINER_ITEM_LINE:
				// adjust the color for brightness/contrast/gamma
				prim->color.a = container.apply_brightness_contrast_gamma_fp(prim->color.a);
				prim->color.r = container.apply_brightness_contrast_gamma_fp(prim->color.r);
				prim->color.g = container.apply_brightness_contrast_gamma_fp(prim->color.g);
				prim->color.b = container.apply_brightness_contrast_gamma_fp(prim->color.b);

				// set the line type
				prim->type = render_primitive::LINE;

				// scale the width by the minimum of X/Y scale factors
				prim->width = curitem->width() * MIN(container_xform.xscale, container_xform.yscale);
				prim->flags = curitem->flags();

				// clip the primitive
				clipped = render_clip_line(&prim->bounds, &cliprect);
				break;

			case CONTAINER_ITEM_QUAD:
				// set the quad type
				prim->type = render_primitive::QUAD;

				// normalize the bounds
				normalize_bounds(prim->bounds);

				// get the scaled bitmap and set the resulting palette
				if (curitem->texture() != NULL)
				{
					// determine the final orientation
					int finalorient = orientation_add(PRIMFLAG_GET_TEXORIENT(curitem->flags()), container_xform.orientation);

					// based on the swap values, get the scaled final texture
					int width = (finalorient & ORIENTATION_SWAP_XY) ? (prim->bounds.y1 - prim->bounds.y0) : (prim->bounds.x1 - prim->bounds.x0);
					int height = (finalorient & ORIENTATION_SWAP_XY) ? (prim->bounds.x1 - prim->bounds.x0) : (prim->bounds.y1 - prim->bounds.y0);
					width = MIN(width, m_maxtexwidth);
					height = MIN(height, m_maxtexheight);
					if (curitem->texture()->get_scaled(width, height, prim->texture, list))
					{
						// set the palette
						prim->texture.palette = curitem->texture()->get_adjusted_palette(container);

						// determine UV coordinates and apply clipping
						prim->texcoords = oriented_texcoords[finalorient];
						clipped = render_clip_quad(&prim->bounds, &cliprect, &prim->texcoords);

						// apply the final orientation from the quad flags and then build up the final flags
						prim->flags = (curitem->flags() & ~(PRIMFLAG_TEXORIENT_MASK | PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) |
										PRIMFLAG_TEXORIENT(finalorient) |
										PRIMFLAG_TEXFORMAT(curitem->texture()->format());
						if (blendmode != -1)
							prim->flags |= PRIMFLAG_BLENDMODE(blendmode);
						else
							prim->flags |= PRIMFLAG_BLENDMODE(PRIMFLAG_GET_BLENDMODE(curitem->flags()));
					}
				}
				else
				{
					// adjust the color for brightness/contrast/gamma
					prim->color.r = container.apply_brightness_contrast_gamma_fp(prim->color.r);
					prim->color.g = container.apply_brightness_contrast_gamma_fp(prim->color.g);
					prim->color.b = container.apply_brightness_contrast_gamma_fp(prim->color.b);

					// no texture -- set the basic flags
					prim->texture.base = NULL;
					prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);

					// apply clipping
					clipped = render_clip_quad(&prim->bounds, &cliprect, NULL);
				}
				break;
		}

		// add to the list or free if we're clipped out
		list.append_or_return(*prim, clipped);
	}

	// add the overlay if it exists
	if (container.overlay() != NULL && screen_overlay_enabled())
	{
		INT32 width, height;

		// allocate a primitive
		render_primitive *prim = list.alloc(render_primitive::QUAD);
		set_render_bounds_wh(&prim->bounds, xform.xoffs, xform.yoffs, xform.xscale, xform.yscale);
		prim->color = container_xform.color;
		width = render_round_nearest(prim->bounds.x1) - render_round_nearest(prim->bounds.x0);
		height = render_round_nearest(prim->bounds.y1) - render_round_nearest(prim->bounds.y0);

		bool got_scaled = container.overlay()->get_scaled(
				(container_xform.orientation & ORIENTATION_SWAP_XY) ? height : width,
				(container_xform.orientation & ORIENTATION_SWAP_XY) ? width : height, prim->texture, list);
		if (got_scaled)
		{
			// determine UV coordinates
			prim->texcoords = oriented_texcoords[container_xform.orientation];

			// set the flags and add it to the list
			prim->flags = PRIMFLAG_TEXORIENT(container_xform.orientation) |
							PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY) |
							PRIMFLAG_TEXFORMAT(container.overlay()->format());
		}
		list.append_or_return(*prim, !got_scaled);
	}
}


//-------------------------------------------------
//  add_element_primitives - add the primitive
//  for an element in the current state
//-------------------------------------------------

void render_target::add_element_primitives(render_primitive_list &list, const object_transform &xform, const layout_element &element, int state, int blendmode)
{
	// if we're out of range, bail
	if (state > element.maxstate)
		return;
	if (state < 0)
		state = 0;

	// get a pointer to the relevant texture
	render_texture *texture = element.elemtex[state].texture;
	if (texture != NULL)
	{
		render_primitive *prim = list.alloc(render_primitive::QUAD);

		// configure the basics
		prim->color = xform.color;
		prim->flags = PRIMFLAG_TEXORIENT(xform.orientation) | PRIMFLAG_BLENDMODE(blendmode) | PRIMFLAG_TEXFORMAT(texture->format());

		// compute the bounds
		INT32 width = render_round_nearest(xform.xscale);
		INT32 height = render_round_nearest(xform.yscale);
		set_render_bounds_wh(&prim->bounds, render_round_nearest(xform.xoffs), render_round_nearest(xform.yoffs), (float) width, (float) height);
		if (xform.orientation & ORIENTATION_SWAP_XY)
			ISWAP(width, height);
		width = MIN(width, m_maxtexwidth);
		height = MIN(height, m_maxtexheight);

		// get the scaled texture and append it
		bool clipped = true;
		if (texture->get_scaled(width, height, prim->texture, list))
		{
			// compute the clip rect
			render_bounds cliprect;
			cliprect.x0 = render_round_nearest(xform.xoffs);
			cliprect.y0 = render_round_nearest(xform.yoffs);
			cliprect.x1 = render_round_nearest(xform.xoffs + xform.xscale);
			cliprect.y1 = render_round_nearest(xform.yoffs + xform.yscale);
			sect_render_bounds(&cliprect, &m_bounds);

			// determine UV coordinates and apply clipping
			prim->texcoords = oriented_texcoords[xform.orientation];
			clipped = render_clip_quad(&prim->bounds, &cliprect, &prim->texcoords);
		}

		// add to the list or free if we're clipped out
		list.append_or_return(*prim, clipped);
	}
}


//-------------------------------------------------
//  map_point_internal - internal logic for
//  mapping points
//-------------------------------------------------

bool render_target::map_point_internal(INT32 target_x, INT32 target_y, render_container *container, float &mapped_x, float &mapped_y, view_item *&mapped_item)
{
	// default to point not mapped
	mapped_x = -1.0;
	mapped_y = -1.0;

	// convert target coordinates to float
	float target_fx = (float)target_x / m_width;
	float target_fy = (float)target_y / m_height;

	// explicitly check for the UI container
	if (container != NULL && container == &m_manager.ui_container())
	{
		// this hit test went against the UI container
		if (target_fx >= 0.0 && target_fx < 1.0 && target_fy >= 0.0 && target_fy < 1.0)
		{
			// this point was successfully mapped
			mapped_x = target_fx;
			mapped_y = target_fy;
			mapped_item = NULL;
			return true;
		}
		return false;
	}

	// loop through each layer
	for (int layernum = 0; layernum < ITEM_LAYER_MAX; layernum++)
	{
		int blendmode;
		int layer = get_layer_and_blendmode(*m_curview, layernum, blendmode);
		if (m_curview->layenabled[layer])
		{
			// iterate over items in the layer
			for (view_item *item = m_curview->itemlist[layer]; item != NULL; item = item->next)
			{
				bool checkit;

				// if we're looking for a particular container, verify that we have the right one
				if (container != NULL)
					checkit = (item->element == NULL && container == m_manager.m_screen_container_list.find(item->index));

				// otherwise, assume we're looking for an input
				else
					checkit = (item->input_tag[0] != 0);

				// this target is worth looking at; now check the point
				if (checkit && target_fx >= item->bounds.x0 && target_fx < item->bounds.x1 && target_fy >= item->bounds.y0 && target_fy < item->bounds.y1)
				{
					// point successfully mapped
					mapped_x = (target_fx - item->bounds.x0) / (item->bounds.x1 - item->bounds.x0);
					mapped_y = (target_fy - item->bounds.y0) / (item->bounds.y1 - item->bounds.y0);
					mapped_item = item;
					return true;
				}
			}
		}
	}
	return false;
}


//-------------------------------------------------
//  view_name - return the name of the indexed
//  view, or NULL if it doesn't exist
//-------------------------------------------------

layout_view *render_target::view_by_index(int index) const
{
	// scan the list of views within each layout, skipping those that don't apply
	for (layout_file *file = m_filelist; file != NULL; file = file->next)
		for (layout_view *view = file->viewlist; view != NULL; view = view->next)
			if (!(m_flags & RENDER_CREATE_NO_ART) || !layout_view_has_art(view))
				if (index-- == 0)
					return view;
	return NULL;
}


//-------------------------------------------------
//  view_index - return the index of the given
//  view
//-------------------------------------------------

int render_target::view_index(layout_view &targetview) const
{
	// find the first named match
	int index = 0;

	// scan the list of views within each layout, skipping those that don't apply
	for (layout_file *file = m_filelist; file != NULL; file = file->next)
		for (layout_view *view = file->viewlist; view != NULL; view = view->next)
			if (!(m_flags & RENDER_CREATE_NO_ART) || !layout_view_has_art(view))
			{
				if (&targetview == view)
					return index;
				index++;
			}
	return 0;
}


//-------------------------------------------------
//  config_load - process config information
//-------------------------------------------------

void render_target::config_load(xml_data_node &targetnode)
{
	// find the view
	const char *viewname = xml_get_attribute_string(&targetnode, "view", NULL);
	if (viewname != NULL)
		for (int viewnum = 0; viewnum < 1000; viewnum++)
		{
			const char *testname = view_name(viewnum);
			if (testname == NULL)
				break;
			if (!strcmp(viewname, testname))
			{
				set_view(viewnum);
				break;
			}
		}

	// modify the artwork config
	int tmpint = xml_get_attribute_int(&targetnode, "backdrops", -1);
	if (tmpint == 0 || tmpint == 1)
		set_backdrops_enabled(tmpint);

	tmpint = xml_get_attribute_int(&targetnode, "overlays", -1);
	if (tmpint == 0 || tmpint == 1)
		set_overlays_enabled(tmpint);

	tmpint = xml_get_attribute_int(&targetnode, "bezels", -1);
	if (tmpint == 0 || tmpint == 1)
		set_bezels_enabled(tmpint);

	tmpint = xml_get_attribute_int(&targetnode, "zoom", -1);
	if (tmpint == 0 || tmpint == 1)
		set_zoom_to_screen(tmpint);

	// apply orientation
	tmpint = xml_get_attribute_int(&targetnode, "rotate", -1);
	if (tmpint != -1)
	{
		if (tmpint == 90)
			tmpint = ROT90;
		else if (tmpint == 180)
			tmpint = ROT180;
		else if (tmpint == 270)
			tmpint = ROT270;
		else
			tmpint = ROT0;
		set_orientation(orientation_add(tmpint, orientation()));

		// apply the opposite orientation to the UI
		if (is_ui_target())
		{
			render_container::user_settings settings;
			render_container &ui_container = m_manager.ui_container();

			ui_container.get_user_settings(settings);
			settings.m_orientation = orientation_add(orientation_reverse(tmpint), settings.m_orientation);
			ui_container.set_user_settings(settings);
		}
	}
}


//-------------------------------------------------
//  config_save - save our configuration, or
//  return false if we are the same as the default
//-------------------------------------------------

bool render_target::config_save(xml_data_node &targetnode)
{
	bool changed = false;

	// output the basics
	xml_set_attribute_int(&targetnode, "index", index());

	// output the view
	if (m_curview != m_base_view)
	{
		xml_set_attribute(&targetnode, "view", m_curview->name);
		changed = true;
	}

	// output the layer config
	if (m_layerconfig != m_base_layerconfig)
	{
		xml_set_attribute_int(&targetnode, "backdrops", backdrops_enabled());
		xml_set_attribute_int(&targetnode, "overlays", overlays_enabled());
		xml_set_attribute_int(&targetnode, "bezels", bezels_enabled());
		xml_set_attribute_int(&targetnode, "zoom", zoom_to_screen());
		changed = true;
	}

	// output rotation
	if (m_orientation != m_base_orientation)
	{
		int rotate = 0;
		if (orientation_add(ROT90, m_base_orientation) == m_orientation)
			rotate = 90;
		else if (orientation_add(ROT180, m_base_orientation) == m_orientation)
			rotate = 180;
		else if (orientation_add(ROT270, m_base_orientation) == m_orientation)
			rotate = 270;
		assert(rotate != 0);
		xml_set_attribute_int(&targetnode, "rotate", rotate);
		changed = true;
	}

	return changed;
}


//-------------------------------------------------
//  init_clear_extents - reset the extents list
//-------------------------------------------------

void render_target::init_clear_extents()
{
	m_clear_extents[0] = -m_height;
	m_clear_extents[1] = 1;
	m_clear_extents[2] = m_width;
	m_clear_extent_count = 3;
}


//-------------------------------------------------
//  remove_clear_extent - remove a quad from the
//  list of stuff to clear, unless it overlaps
//  a previous quad
//-------------------------------------------------

bool render_target::remove_clear_extent(const render_bounds &bounds)
{
	INT32 *max = &m_clear_extents[MAX_CLEAR_EXTENTS];
	INT32 *last = &m_clear_extents[m_clear_extent_count];
	INT32 *ext = &m_clear_extents[0];
	INT32 boundsx0 = ceil(bounds.x0);
	INT32 boundsx1 = floor(bounds.x1);
	INT32 boundsy0 = ceil(bounds.y0);
	INT32 boundsy1 = floor(bounds.y1);
	INT32 y0, y1 = 0;

	// loop over Y extents
	while (ext < last)
	{
		INT32 *linelast;

		// first entry of each line should always be negative
		assert(ext[0] < 0.0f);
		y0 = y1;
		y1 = y0 - ext[0];

		// do we intersect this extent?
		if (boundsy0 < y1 && boundsy1 > y0)
		{
			INT32 *xext;
			INT32 x0, x1 = 0;

			// split the top
			if (y0 < boundsy0)
			{
				int diff = boundsy0 - y0;

				// make a copy of this extent
				memmove(&ext[ext[1] + 2], &ext[0], (last - ext) * sizeof(*ext));
				last += ext[1] + 2;
				assert_always(last < max, "Ran out of clear extents!\n");

				// split the extent between pieces
				ext[ext[1] + 2] = -(-ext[0] - diff);
				ext[0] = -diff;

				// advance to the new extent
				y0 -= ext[0];
				ext += ext[1] + 2;
				y1 = y0 - ext[0];
			}

			// split the bottom
			if (y1 > boundsy1)
			{
				int diff = y1 - boundsy1;

				// make a copy of this extent
				memmove(&ext[ext[1] + 2], &ext[0], (last - ext) * sizeof(*ext));
				last += ext[1] + 2;
				assert_always(last < max, "Ran out of clear extents!\n");

				// split the extent between pieces
				ext[ext[1] + 2] = -diff;
				ext[0] = -(-ext[0] - diff);

				// recompute y1
				y1 = y0 - ext[0];
			}

			// now remove the X extent
			linelast = &ext[ext[1] + 2];
			xext = &ext[2];
			while (xext < linelast)
			{
				x0 = x1;
				x1 = x0 + xext[0];

				// do we fully intersect this extent?
				if (boundsx0 >= x0 && boundsx1 <= x1)
				{
					// yes; split it
					memmove(&xext[2], &xext[0], (last - xext) * sizeof(*xext));
					last += 2;
					linelast += 2;
					assert_always(last < max, "Ran out of clear extents!\n");

					// split this extent into three parts
					xext[0] = boundsx0 - x0;
					xext[1] = boundsx1 - boundsx0;
					xext[2] = x1 - boundsx1;

					// recompute x1
					x1 = boundsx1;
					xext += 2;
				}

				// do we partially intersect this extent?
				else if (boundsx0 < x1 && boundsx1 > x0)
					goto abort;

				// advance
				xext++;

				// do we partially intersect the next extent (which is a non-clear extent)?
				if (xext < linelast)
				{
					x0 = x1;
					x1 = x0 + xext[0];
					if (boundsx0 < x1 && boundsx1 > x0)
						goto abort;
					xext++;
				}
			}

			// update the count
			ext[1] = linelast - &ext[2];
		}

		// advance to the next row
		ext += 2 + ext[1];
	}

	// update the total count
	m_clear_extent_count = last - &m_clear_extents[0];
	return true;

abort:
	// update the total count even on a failure as we may have split extents
	m_clear_extent_count = last - &m_clear_extents[0];
	return false;
}


//-------------------------------------------------
//  add_clear_extents - add the accumulated
//  extents as a series of quads to clear
//-------------------------------------------------

void render_target::add_clear_extents(render_primitive_list &list)
{
	simple_list<render_primitive> clearlist;
	INT32 *last = &m_clear_extents[m_clear_extent_count];
	INT32 *ext = &m_clear_extents[0];
	INT32 y0, y1 = 0;

	// loop over all extents
	while (ext < last)
	{
		INT32 *linelast = &ext[ext[1] + 2];
		INT32 *xext = &ext[2];
		INT32 x0, x1 = 0;

		// first entry should always be negative
		assert(ext[0] < 0);
		y0 = y1;
		y1 = y0 - ext[0];

		// now remove the X extent
		while (xext < linelast)
		{
			x0 = x1;
			x1 = x0 + *xext++;

			// only add entries for non-zero widths
			if (x1 - x0 > 0)
			{
				render_primitive *prim = list.alloc(render_primitive::QUAD);
				set_render_bounds_xy(&prim->bounds, (float)x0, (float)y0, (float)x1, (float)y1);
				set_render_color(&prim->color, 1.0f, 0.0f, 0.0f, 0.0f);
				prim->texture.base = NULL;
				prim->flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA);
				clearlist.append(*prim);
			}

			// skip the non-clearing extent
			x0 = x1;
			x1 = x0 + *xext++;
		}

		// advance to the next part
		ext += 2 + ext[1];
	}

	// we know that the first primitive in the list will be the global clip
	// so we insert the clears immediately after
	list.m_primlist.prepend_list(clearlist);
}


//-------------------------------------------------
//  add_clear_and_optimize_primitive_list -
//  optimize the primitive list
//-------------------------------------------------

void render_target::add_clear_and_optimize_primitive_list(render_primitive_list &list)
{
	// start with the assumption that we need to clear the whole screen
	init_clear_extents();

	// scan the list until we hit an intersection quad or a line
	for (render_primitive *prim = list.first(); prim != NULL; prim = prim->next())
	{
		// switch off the type
		switch (prim->type)
		{
			case render_primitive::LINE:
				goto done;

			case render_primitive::QUAD:
			{
				// stop when we hit an alpha texture
				if (PRIMFLAG_GET_TEXFORMAT(prim->flags) == TEXFORMAT_ARGB32 || PRIMFLAG_GET_TEXFORMAT(prim->flags) == TEXFORMAT_PALETTEA16)
					goto done;

				// if this quad can't be cleanly removed from the extents list, we're done
				if (!remove_clear_extent(prim->bounds))
					goto done;

				// change the blendmode on the first primitive to be NONE
				if (PRIMFLAG_GET_BLENDMODE(prim->flags) == BLENDMODE_RGB_MULTIPLY)
				{
					// RGB multiply will multiply against 0, leaving nothing
					set_render_color(&prim->color, 1.0f, 0.0f, 0.0f, 0.0f);
					prim->texture.base = NULL;
					prim->flags = (prim->flags & ~PRIMFLAG_BLENDMODE_MASK) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE);
				}
				else
				{
					// for alpha or add modes, we will blend against 0 or add to 0; treat it like none
					prim->flags = (prim->flags & ~PRIMFLAG_BLENDMODE_MASK) | PRIMFLAG_BLENDMODE(BLENDMODE_NONE);
				}

				// since alpha is disabled, premultiply the RGB values and reset the alpha to 1.0
				prim->color.r *= prim->color.a;
				prim->color.g *= prim->color.a;
				prim->color.b *= prim->color.a;
				prim->color.a = 1.0f;
				break;
			}

			default:
				throw emu_fatalerror("Unexpected primitive type");
		}
	}

done:
	// now add the extents to the clear list
	add_clear_extents(list);
}



//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  render_manager - constructor
//-------------------------------------------------

render_manager::render_manager(running_machine &machine)
	: m_machine(machine),
	  m_targetlist(machine.m_respool),
	  m_ui_target(NULL),
	  m_live_textures(0),
	  m_texture_allocator(machine.m_respool),
	  m_ui_container(auto_alloc(&machine, render_container(*this))),
	  m_screen_container_list(machine.m_respool)
{
	// register callbacks
	config_register(&machine, "video", config_load_static, config_save_static);

	// create one container per screen
	for (screen_device *screen = screen_first(machine); screen != NULL; screen = screen_next(screen))
		container_alloc(screen);
}


//-------------------------------------------------
//  ~render_manager - destructor
//-------------------------------------------------

render_manager::~render_manager()
{
	// better not be any outstanding textures when we die
	assert(m_live_textures == 0);

	// free the UI container
	container_free(m_ui_container);
}


//-------------------------------------------------
//  is_live - return if the screen is 'live'
//-------------------------------------------------

bool render_manager::is_live(screen_device &screen) const
{
	int screen_index = screen.machine->m_devicelist.index(SCREEN, screen.tag());
	assert(screen_index != -1);

	// iterate over all live targets and or together their screen masks
	UINT32 bitmask = 0;
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		bitmask |= target->view_screens(target->view());

	return (bitmask & (1 << screen_index)) ? true : false;
}


//-------------------------------------------------
//  max_update_rate - return the smallest maximum
//  update rate across all targets
//-------------------------------------------------

float render_manager::max_update_rate() const
{
	// iterate over all live targets and or together their screen masks
	float minimum = 0;
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		if (target->max_update_rate() != 0)
		{
			if (minimum == 0)
				minimum = target->max_update_rate();
			else
				minimum = MIN(target->max_update_rate(), minimum);
		}

	return minimum;
}


//-------------------------------------------------
//  target_alloc - allocate a new target
//-------------------------------------------------

render_target *render_manager::target_alloc(const char *layoutfile, UINT32 flags)
{
	return &m_targetlist.append(*auto_alloc(&m_machine, render_target(*this, layoutfile, flags)));
}


//-------------------------------------------------
//  target_free - free a target
//-------------------------------------------------

void render_manager::target_free(render_target *target)
{
	if (target != NULL)
		m_targetlist.remove(*target);
}


//-------------------------------------------------
//  target_by_index - get a render_target by index
//-------------------------------------------------

render_target *render_manager::target_by_index(int index) const
{
	// count up the targets until we hit the requested index
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		if (!target->hidden())
			if (index-- == 0)
				return target;
	return NULL;
}


//-------------------------------------------------
//  ui_aspect - return the aspect ratio for UI
//  fonts
//-------------------------------------------------

float render_manager::ui_aspect()
{
	int orient = orientation_add(m_ui_target->orientation(), m_ui_container->orientation());

	// based on the orientation of the target, compute height/width or width/height
	float aspect;
	if (!(orient & ORIENTATION_SWAP_XY))
		aspect = (float)m_ui_target->height() / (float)m_ui_target->width();
	else
		aspect = (float)m_ui_target->width() / (float)m_ui_target->height();

	// if we have a valid pixel aspect, apply that and return
	if (m_ui_target->pixel_aspect() != 0.0f)
		return aspect / m_ui_target->pixel_aspect();

	// if not, clamp for extreme proportions
	if (aspect < 0.66f)
		aspect = 0.66f;
	if (aspect > 1.5f)
		aspect = 1.5f;
	return aspect;
}


//-------------------------------------------------
//  container_for_screen - return the container
//  allocated for the given screen device
//-------------------------------------------------

render_container *render_manager::container_for_screen(screen_device *screen)
{
	// scan the list of screen containers for a match
	for (render_container *container = m_screen_container_list.first(); container != NULL; container = container->next())
		if (container->screen() == screen)
			return container;
	return NULL;
}


//-------------------------------------------------
//  texture_alloc - allocate a new texture
//-------------------------------------------------

render_texture *render_manager::texture_alloc(texture_scaler_func scaler, void *param)
{
	// allocate a new texture and reset it
	render_texture *tex = m_texture_allocator.alloc();
	tex->reset(*this, scaler, param);
	m_live_textures++;
	return tex;
}


//-------------------------------------------------
//  texture_free - release a texture
//-------------------------------------------------

void render_manager::texture_free(render_texture *texture)
{
	if (texture != NULL)
		m_live_textures--;
	m_texture_allocator.reclaim(texture);
}


//-------------------------------------------------
//  invalidate_all - remove all refs to a
//  particular reference pointer
//-------------------------------------------------

void render_manager::invalidate_all(void *refptr)
{
	// permit NULL
	if (refptr == NULL)
		return;

	// loop over targets
	for (render_target *target = m_targetlist.first(); target != NULL; target = target->next())
		target->invalidate_all(refptr);
}


//-------------------------------------------------
//  container_alloc - allocate a new container
//-------------------------------------------------

render_container *render_manager::container_alloc(screen_device *screen)
{
	render_container *container = auto_alloc(&m_machine, render_container(*this, screen));
	if (screen != NULL)
		m_screen_container_list.append(*container);
	return container;
}


//-------------------------------------------------
//  container_free - release a container
//-------------------------------------------------

void render_manager::container_free(render_container *container)
{
	m_screen_container_list.detach(*container);
	auto_free(&m_machine, container);
}


//-------------------------------------------------
//  config_load - read and apply data from the
//  configuration file
//-------------------------------------------------

void render_manager::config_load_static(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	machine->render().config_load(config_type, parentnode);
}

void render_manager::config_load(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// might not have any data
	if (parentnode == NULL)
		return;

	// check the UI target
	xml_data_node *uinode = xml_get_sibling(parentnode->child, "interface");
	if (uinode != NULL)
	{
		render_target *target = target_by_index(xml_get_attribute_int(uinode, "target", 0));
		if (target != NULL)
			set_ui_target(*target);
	}

	// iterate over target nodes
	for (xml_data_node *targetnode = xml_get_sibling(parentnode->child, "target"); targetnode; targetnode = xml_get_sibling(targetnode->next, "target"))
	{
		render_target *target = target_by_index(xml_get_attribute_int(targetnode, "index", -1));
		if (target != NULL)
			target->config_load(*targetnode);
	}

	// iterate over screen nodes
	for (xml_data_node *screennode = xml_get_sibling(parentnode->child, "screen"); screennode; screennode = xml_get_sibling(screennode->next, "screen"))
	{
		int index = xml_get_attribute_int(screennode, "index", -1);
		render_container *container = m_screen_container_list.find(index);
		render_container::user_settings settings;

		// fetch current settings
		container->get_user_settings(settings);

		// fetch color controls
		settings.m_brightness = xml_get_attribute_float(screennode, "brightness", settings.m_brightness);
		settings.m_contrast = xml_get_attribute_float(screennode, "contrast", settings.m_contrast);
		settings.m_gamma = xml_get_attribute_float(screennode, "gamma", settings.m_gamma);

		// fetch positioning controls
		settings.m_xoffset = xml_get_attribute_float(screennode, "hoffset", settings.m_xoffset);
		settings.m_xscale = xml_get_attribute_float(screennode, "hstretch", settings.m_xscale);
		settings.m_yoffset = xml_get_attribute_float(screennode, "voffset", settings.m_yoffset);
		settings.m_yscale = xml_get_attribute_float(screennode, "vstretch", settings.m_yscale);

		// set the new values
		container->set_user_settings(settings);
	}
}


//-------------------------------------------------
//  config_save - save data to the configuration
//  file
//-------------------------------------------------

void render_manager::config_save_static(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	machine->render().config_save(config_type, parentnode);
}

void render_manager::config_save(int config_type, xml_data_node *parentnode)
{
	// we only care about game files
	if (config_type != CONFIG_TYPE_GAME)
		return;

	// write out the interface target
	if (m_ui_target->index() != 0)
	{
		// create a node for it
		xml_data_node *uinode = xml_add_child(parentnode, "interface", NULL);
		if (uinode != NULL)
			xml_set_attribute_int(uinode, "target", m_ui_target->index());
	}

	// iterate over targets
	for (int targetnum = 0; targetnum < 1000; targetnum++)
	{
		// get this target and break when we fail
		render_target *target = target_by_index(targetnum);
		if (target == NULL)
			break;

		// create a node
		xml_data_node *targetnode = xml_add_child(parentnode, "target", NULL);
		if (targetnode != NULL && !target->config_save(*targetnode))
			xml_delete_node(targetnode);
	}

	// iterate over screen containers
	int scrnum = 0;
	for (render_container *container = m_screen_container_list.first(); container != NULL; container = container->next(), scrnum++)
	{
		// create a node
		xml_data_node *screennode = xml_add_child(parentnode, "screen", NULL);
		if (screennode != NULL)
		{
			bool changed = false;

			// output the basics
			xml_set_attribute_int(screennode, "index", scrnum);

			render_container::user_settings settings;
			container->get_user_settings(settings);

			// output the color controls
			if (settings.m_brightness != options_get_float(m_machine.options(), OPTION_BRIGHTNESS))
			{
				xml_set_attribute_float(screennode, "brightness", settings.m_brightness);
				changed = true;
			}

			if (settings.m_contrast != options_get_float(m_machine.options(), OPTION_CONTRAST))
			{
				xml_set_attribute_float(screennode, "contrast", settings.m_contrast);
				changed = true;
			}

			if (settings.m_gamma != options_get_float(m_machine.options(), OPTION_GAMMA))
			{
				xml_set_attribute_float(screennode, "gamma", settings.m_gamma);
				changed = true;
			}

			// output the positioning controls
			if (settings.m_xoffset != 0.0f)
			{
				xml_set_attribute_float(screennode, "hoffset", settings.m_xoffset);
				changed = true;
			}

			if (settings.m_xscale != 1.0f)
			{
				xml_set_attribute_float(screennode, "hstretch", settings.m_xscale);
				changed = true;
			}

			if (settings.m_yoffset != 0.0f)
			{
				xml_set_attribute_float(screennode, "voffset", settings.m_yoffset);
				changed = true;
			}

			if (settings.m_yscale != 1.0f)
			{
				xml_set_attribute_float(screennode, "vstretch", settings.m_yscale);
				changed = true;
			}

			// if nothing changed, kill the node
			if (!changed)
				xml_delete_node(screennode);
		}
	}
}

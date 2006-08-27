/* Sega Master System */

/* Still vastly incomplete
 -- add support for TMS modes
 -- add proper I/O
 -- add proper region detection (currently locked to Japan)
 -- fix off by 1 raster effects
 -- emulate SMS1 VDP bugs
 -- add other games, clean up list of titles, most of the 'UE' games in GoodSMS are really
    just PAL / Brazil / Euro games, what a mess.
 -- hook up megaplay / megatech systems using new rendering
*/

#include "driver.h"
#include "sound/sn76496.h"

UINT8* sms_rom;
UINT8* sms_mainram;
UINT8* smsgg_backupram;
static void sms_scanline_timer_callback(void* param);
struct sms_vdp *vdp2;
struct sms_vdp *vdp1;

/* All Accesses to VRAM go through here for safety */
#define SMS_VDP_VRAM(address) chip->vram[(address)&0x3fff]

static ADDRESS_MAP_START( sms_readmem, ADDRESS_SPACE_PROGRAM, 8 )
//	AM_RANGE(0x0000 , 0xbfff) AM_READ(MRA8_ROM)
//	AM_RANGE(0xc000 , 0xdfff) AM_READ(MRA8_RAM) AM_MIRROR(0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_writemem, ADDRESS_SPACE_PROGRAM, 8 )
//	AM_RANGE(0x0000 , 0xbfff) AM_WRITE(MWA8_ROM)
//	AM_RANGE(0xc000 , 0xdfff) AM_WRITE(MWA8_RAM) AM_MIRROR(0x2000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
ADDRESS_MAP_END

/* Precalculated tables for H/V counters.  Note the position the counter 'jumps' is marked with with
   an empty comment */
static UINT8 hc_256[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,    0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,    0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,    0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,    0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,    0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,    0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,    0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,    0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,    0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,    0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,    0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,/**/0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
    0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3,    0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
    0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
    0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3,    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3,    0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3,    0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};


static UINT8 vc_ntsc_192[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,/**/0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4,    0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static UINT8 vc_ntsc_224[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,/**/0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static UINT8 vc_ntsc_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};



static UINT8 vc_pal_192[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,/**/0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6,
    0xc7, 0xc8, 0xc9,    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};


static UINT8 vc_pal_224[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02,/**/0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static UINT8 vc_pal_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,    0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa,    0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,/**/0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,    0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,    0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static struct
{
	unsigned char sms2_name[40];
	int sms2_valid;
	int sms2_height;
	int sms2_tilemap_height;
	UINT8* sms_vcounter_table;
	UINT8* sms_hcounter_table;

} sms_mode_table[] =
{
	/* NTSC Modes */
	{ "Graphic 1 (NTSC)",         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Text (NTSC)",              0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Graphic 2 (NTSC)",         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+2 (NTSC)" ,         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Multicolor (NTSC)",        0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+3 (NTSC)",          0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 2+3 (NTSC)",          0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+2+3 (NTSC)",        0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Invalid Text (NTSC)",      0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (224-line) (NTSC)", 1, 224, 256, vc_ntsc_224, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Invalid Text (NTSC)",      0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (240-line) (NTSC)", 1, 240, 256, vc_ntsc_240, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 244, vc_ntsc_192, hc_256 },

	/* Pal Modes (different Vcounter) */
	{ "Graphic 1 (PAL)",         0, 192, 224, vc_pal_192, hc_256 },
	{ "Text (PAL)",              0, 192, 224, vc_pal_192, hc_256 },
	{ "Graphic 2 (PAL)",         0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+2 (PAL)" ,         0, 192, 224, vc_pal_192, hc_256 },
	{ "Multicolor (PAL)",        0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+3 (PAL)",          0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 2+3 (PAL)",          0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+2+3 (PAL)",        0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Invalid Text (PAL)",      0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (224-line) (PAL)", 1, 224, 256, vc_pal_224, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Invalid Text (PAL)",      0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (240-line) (PAL)", 1, 240, 256, vc_pal_240, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 244, vc_pal_192, hc_256 }
};

enum
{
	SMS_VDP = 0,  // SMS1 VDP
	SMS2_VDP = 1, // SMS2 VDP, or Game Gear VDP running in SMS2 Mode
	GG_VDP = 2,   // Game Gear VDP running in Game Gear Mode
	GEN_VDP = 3   // Genesis VDP running in SMS2 Mode
};

int sms_vdp_null_irq_callback(int status)
{
	return -1;
}

int sms_vdp_cpu0_irq_callback(int status)
{
	if (status==1)
	{
		cpunum_set_input_line(0,0,HOLD_LINE);
	}
	else
	{
		cpunum_set_input_line(0,0,CLEAR_LINE);
	}

	return 0;
}




struct sms_vdp
{
	UINT8 chip_id;

	UINT8  cmd_pend;
	UINT8  cmd_part1;
	UINT8  cmd_part2;
	UINT16 addr_reg;
	UINT8  cmd_reg;
	UINT8  regs[0x10];
	UINT8  readbuf;
	UINT8* vram;
	UINT8* cram;
	UINT8  writemode;
	mame_bitmap* r_bitmap;
	UINT8* tile_renderline;
	UINT8* sprite_renderline;

	UINT8 sprite_collision;
	UINT8 sprite_overflow;

	UINT8  yscroll;
	UINT8  hint_counter;

	UINT8 frame_irq_pending;
	UINT8 line_irq_pending;

	UINT8 vdp_type;

	UINT8 gg_cram_latch; // gamegear specific.

	/* below are MAME specific, to make things easier */
	UINT8 screen_mode;
	UINT8 is_pal;
	int sms_scanline_counter;
	int sms_total_scanlines;
	int sms_framerate;
	mame_timer* sms_scanline_timer;
	UINT16* cram_mamecolours; // for use on RGB_DIRECT screen
	int	 (*set_irq)(int state);

};



void *start_vdp(int type)
{
	struct sms_vdp *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	chip->vdp_type = type;

	chip->set_irq = sms_vdp_null_irq_callback;

	chip->cmd_pend = 0;
	chip->cmd_part1 = 0;
	chip->cmd_part2 = 0;
	chip->addr_reg = 0;
	chip->cmd_reg = 0;

	chip->regs[0x0] = 0x06; // mode 4
	chip->regs[0x1] = 0x18; // mode 4
	chip->regs[0x2] = 0;
	chip->regs[0x3] = 0;
	chip->regs[0x4] = 0;
	chip->regs[0x5] = 0;
	chip->regs[0x6] = 0;
	chip->regs[0x7] = 0;
	chip->regs[0x8] = 0;
	chip->regs[0x9] = 0;
	chip->regs[0xa] = 0;
	/* b-f don't matter */
	chip->readbuf = 0;
	chip->vram = auto_malloc(0x4000);
	memset(chip->vram,0x00,0x4000);

	//printf("%d\n", (*chip->set_irq)(200));

	if (chip->vdp_type==GG_VDP)
	{
		chip->cram = auto_malloc(0x0040);
		memset(chip->cram,0x00,0x0040);
		chip->cram_mamecolours = auto_malloc(0x0080);
		memset(chip->cram_mamecolours,0x00,0x0080);
		chip->gg_cram_latch = 0;
	}
	else
	{
		chip->cram = auto_malloc(0x0020);
		memset(chip->cram,0x00,0x0020);
		chip->cram_mamecolours = auto_malloc(0x0040);
		memset(chip->cram_mamecolours,0x00,0x0040);
	}

	chip->tile_renderline = auto_malloc(256+8);
	memset(chip->tile_renderline,0x00,256+8);

	chip->sprite_renderline = auto_malloc(256+32);
	memset(chip->sprite_renderline,0x00,256+32);

	chip->writemode = 0;
	chip->r_bitmap = auto_bitmap_alloc(Machine->screen[0].width,Machine->screen[0].height);

	chip->sms_scanline_timer = timer_alloc_ptr(sms_scanline_timer_callback, chip);

	return chip;
}



WRITE8_HANDLER( codemasters_rom_bank_0000_w )
{
	int bank = data&0x1f;
	memcpy(sms_rom+0x0000, memory_region(REGION_USER1)+bank*0x4000, 0x4000);
}

WRITE8_HANDLER( codemasters_rom_bank_4000_w )
{
	int bank = data&0x1f;
	memcpy(sms_rom+0x4000, memory_region(REGION_USER1)+bank*0x4000, 0x4000);
}

WRITE8_HANDLER( codemasters_rom_bank_8000_w )
{
	int bank = data&0x1f;
	memcpy(sms_rom+0x8000, memory_region(REGION_USER1)+bank*0x4000, 0x4000);
}

static READ8_HANDLER( z80_unmapped_r )
{
	printf("unmapped z80 read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_w )
{
	printf("unmapped z80 write %04x\n",offset);
}

void sn76496_write(UINT8 data)
{
	SN76496_0_w(0, data & 0xff);
}

/***** The Inputs should be more complex than this.. closer to megadriv.c! *****/

INPUT_PORTS_START( sms )
	PORT_START /* Joypad 1 (2 button) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START /* Joypad 2 (2 button) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START /* Buttons on SMS Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_IMPULSE(1) // pause, triggers an NMI
INPUT_PORTS_END


#define B_BUTTON(player)     ( (readinputport(player) & 0x20) >> 5 )
#define A_BUTTON(player)     ( (readinputport(player) & 0x10) >> 4 )
#define RIGHT_BUTTON(player) ( (readinputport(player) & 0x08) >> 3 )
#define LEFT_BUTTON(player)  ( (readinputport(player) & 0x04) >> 2 )
#define DOWN_BUTTON(player)  ( (readinputport(player) & 0x02) >> 1 )
#define UP_BUTTON(player)    ( (readinputport(player) & 0x01) >> 0 )
#define SMS_PAUSE_BUTTON     ( (readinputport(2)      & 0x01) >> 0 )


UINT8 ioport_dc_r(void)
{
	return (DOWN_BUTTON(1)  << 7) |
		   (UP_BUTTON(1)    << 6) |
		   (B_BUTTON(0)     << 5) | // TR-A
		   (A_BUTTON(0)     << 4) | // TL-A
		   (RIGHT_BUTTON(0) << 3) |
		   (LEFT_BUTTON(0)  << 2) |
		   (DOWN_BUTTON(0)  << 1) |
		   (UP_BUTTON(0)    << 0);
}

UINT8 ioport_dd_r(void)
{
	return (0               << 7) | // TH-B
		   (0               << 6) | // TH-A
		   (0               << 5) | // unused
		   (1               << 4) | // RESET button
		   (B_BUTTON(1)     << 3) | // TR-B
		   (A_BUTTON(1)     << 2) | // TL-B
		   (RIGHT_BUTTON(1) << 1) |
		   (LEFT_BUTTON(1)  << 0);
}

INPUT_PORTS_START( gg )
	PORT_START /* Joypad 1 (2 button) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // Start button


	PORT_START /* Joypad 2 (2 button) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START /* Buttons on SMS Console */
INPUT_PORTS_END

#define GG_START_BUTTON     ( (readinputport(0)      & 0x40) >> 6 )

UINT8 ioport_gg00_r(void)
{
	return (GG_START_BUTTON << 7) |
		   (0               << 6) |
		   (0               << 5) |
		   (0               << 4) |
		   (0               << 3) |
		   (0               << 2) |
		   (0               << 1) |
		   (0               << 0);
}


UINT8 vcounter_r(struct sms_vdp *chip)
{
//	return vc_pal_224[sms_scanline_counter%(sizeof vc_pal_224)];
	UINT8 retvalue;
	int scanline = chip->sms_scanline_counter%chip->sms_total_scanlines;

	retvalue = sms_mode_table[chip->screen_mode].sms_vcounter_table[scanline];

	return retvalue;
	//printf("retvalue %d\n";
}


UINT8 vdp_data_r(struct sms_vdp *chip)
{
	UINT8 retdata = chip->readbuf;
	chip->readbuf = SMS_VDP_VRAM(chip->addr_reg);
	chip->addr_reg++; chip->addr_reg&=0x3fff;
	return retdata;
}

void vdp_data_w(UINT8 data, struct sms_vdp* chip)
{
	/* data writes clear the pending flag */
	chip->cmd_pend = 0;

	if (chip->writemode==0)
	{ /* Write to VRAM */
		SMS_VDP_VRAM(chip->addr_reg)=data;
		chip->addr_reg++; chip->addr_reg&=0x3fff;
		chip->readbuf = data; // quirk of the VDP
	}
	else if (chip->writemode==1)
	{
		if (chip->vdp_type==GG_VDP)
		{
			if (!(chip->addr_reg&1))
			{ /* Even address, value latched */
				chip->gg_cram_latch = data;
			}
			else
			{
				chip->cram[(chip->addr_reg&0x3e)+1]=data;
				chip->cram[(chip->addr_reg&0x3e)+0]=chip->gg_cram_latch;

				/* Set Colour */
				{
					UINT16 palword;
					UINT8 r,g,b;

					palword = ((chip->cram[(chip->addr_reg&0x3e)+1])<<8)|(chip->cram[(chip->addr_reg&0x3e)+0]);

					//printf("addr %04x palword %04x\n", chip->addr_reg&0x3f, palword);

					r = (palword & 0x000f)>>0;
					g = (palword & 0x00f0)>>4;
					b = (palword & 0x0f00)>>8;
					palette_set_color((chip->addr_reg&0x3e)/2, r<<4, g<<4, b<<4);
					chip->cram_mamecolours[(chip->addr_reg&0x3e)/2]=(b<<1)|(g<<6)|(r<<11);
				}
			}
		}
		else
		{
			chip->cram[chip->addr_reg&0x1f]=data;

			/* Set Colour */
			{
				UINT8 r,g,b;
				r = (data & 0x03)>>0;
				g = (data & 0x0c)>>2;
				b = (data & 0x30)>>4;
				palette_set_color(chip->addr_reg&0x1f, r<<6, g<<6, b<<6);
				chip->cram_mamecolours[chip->addr_reg&0x1f]=(b<<3)|(g<<8)|(r<<13);
			}

		}

		chip->addr_reg++; chip->addr_reg&=0x3fff;
		chip->readbuf = data; // quirk of the VDP

	}

}

UINT8 vdp_ctrl_r(struct sms_vdp *chip)
{
	UINT8 retvalue;

	retvalue = (chip->frame_irq_pending<<7) |
	   	       (chip->sprite_overflow<<6) |
	           (chip->sprite_collision<<5);

	chip->cmd_pend = 0;
	chip->frame_irq_pending = 0;
	chip->line_irq_pending = 0;
	chip->sprite_collision = 0;
	chip->sprite_overflow = 0;

	(chip->set_irq)(0); // clear IRQ;


	return retvalue;
}

/* check me */
void vdp_update_code_addr_regs(struct sms_vdp *chip)
{
	chip->addr_reg = ((chip->cmd_part2&0x3f)<<8) | chip->cmd_part1;
	chip->cmd_reg = (chip->cmd_part2&0xc0)>>6;
}

void vdp_set_register(struct sms_vdp *chip)
{
	UINT8 reg = chip->cmd_part2&0x0f;
	chip->regs[reg] = chip->cmd_part1;

	//if(reg==0) printf("setting reg 0 to %02x\n",chip->cmd_part1);

	//if (reg>0xa) printf("Invalid register write to register %01x\n",reg);

	if(reg==1)
	{
		if ((chip->regs[0x1]&0x20) && chip->frame_irq_pending)
		{
			(chip->set_irq)(1); // set IRQ;
		}
		else
		{
			(chip->set_irq)(0); // clear IRQ;
		}
	}

	if(reg==0)
	{
		if ((chip->regs[0x0]&0x10) && chip->line_irq_pending)
		{
			(chip->set_irq)(1); // set IRQ;
		}
		else
		{
			(chip->set_irq)(0); // clear IRQ;
		}
	}


//	printf("VDP: setting register %01x to %02x\n",reg, chip->cmd_part1);
}

void vdp_ctrl_w(UINT8 data, struct sms_vdp *chip)
{
	if (chip->cmd_pend)
	{ /* Part 2 of a command word write */
		chip->cmd_pend = 0;
		chip->cmd_part2 = data;
		vdp_update_code_addr_regs(chip);

		switch (chip->cmd_reg)
		{
			case 0x0: /* VRAM read mode */
				chip->readbuf = SMS_VDP_VRAM(chip->addr_reg);
				chip->addr_reg++; chip->addr_reg&=0x3fff;
				chip->writemode = 0;
				break;

			case 0x1: /* VRAM write mode */
				chip->writemode = 0;
				break;

			case 0x2: /* REG setting */
				vdp_set_register(chip);
				chip->writemode = 0;
				break;

			case 0x3: /* CRAM write mode */
				chip->writemode = 1;
				break;
		}
	}
	else
	{ /* Part 1 of a command word write */
		chip->cmd_pend = 1;
		chip->cmd_part1 = data;
		vdp_update_code_addr_regs(chip);
	}
}





/* Read / Write Handlers - call other functions */

READ8_HANDLER( sms_ioport_dc_r )
{
	return ioport_dc_r();
}

READ8_HANDLER( sms_ioport_dd_r )
{
	return ioport_dd_r();
}

READ8_HANDLER( sms_ioport_gg00_r )
{
	return ioport_gg00_r();
}



READ8_HANDLER( sms_vcounter_r )
{
	return vcounter_r(vdp1);
}

READ8_HANDLER( sms_vdp_data_r )
{
	return vdp_data_r(vdp1);
}

WRITE8_HANDLER( sms_vdp_data_w )
{
	vdp_data_w(data, vdp1);
}

READ8_HANDLER( sms_vdp_ctrl_r )
{
	return vdp_ctrl_r(vdp1);
}

WRITE8_HANDLER( sms_vdp_ctrl_w )
{
	vdp_ctrl_w(data, vdp1);
}

WRITE8_HANDLER( sms_sn76496_w )
{
	sn76496_write(data);
}

void init_ports_standard(void)
{
	/* INIT THE PORTS *********************************************************************************************/

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0x7e, 0x7e, 0, 0, sms_vcounter_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7e, 0x7e, 0, 0, sms_sn76496_w);

	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7f, 0x7f, 0, 0, sms_sn76496_w);

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbe, 0xbe, 0, 0, sms_vdp_data_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbe, 0xbe, 0, 0, sms_vdp_data_w);

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbf, 0xbf, 0, 0, sms_vdp_ctrl_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbf, 0xbf, 0, 0, sms_vdp_ctrl_w);

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0x10, 0x10, 0, 0, sms_ioport_dd_r); // super tetris

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xdc, 0xdc, 0, 0, sms_ioport_dc_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xdd, 0xdd, 0, 0, sms_ioport_dd_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xdf, 0xdf, 0, 0, sms_ioport_dd_r); // adams family
}

void init_extra_gg_ports(void)
{
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0x00, 0x00, 0, 0, sms_ioport_gg00_r);
}

void init_codemasters_map(void)
{
	/* INIT THE MEMMAP / BANKING *********************************************************************************/

	/* catch any addresses that don't get mapped */
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_w);

	/* fixed rom bank area */
	sms_rom = auto_malloc(0xc000);
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MRA8_BANK1);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MWA8_ROM);
	memory_set_bankptr( 1, sms_rom );

	memcpy(sms_rom, memory_region(REGION_USER1), 0xc000);

	/* main ram area */
	sms_mainram = auto_malloc(0x2000); // 8kb of main ram
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xdfff, 0, 0, MRA8_BANK2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xdfff, 0, 0, MWA8_BANK2);
	memory_set_bankptr( 2, sms_mainram );
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0xe000, 0xffff, 0, 0, MRA8_BANK3);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xe000, 0xffff, 0, 0, MWA8_BANK3);
	memory_set_bankptr( 3, sms_mainram );
	memset(sms_mainram,0x00,0x2000);


	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x0000, 0, 0, codemasters_rom_bank_0000_w);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x4000, 0x4000, 0, 0, codemasters_rom_bank_4000_w);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0x8000, 0, 0, codemasters_rom_bank_8000_w);

	init_ports_standard();
	smsgg_backupram = NULL;
}

READ8_HANDLER( smsgg_backupram_r )
{
	return smsgg_backupram[offset];
}

WRITE8_HANDLER( smsgg_backupram_w )
{
	smsgg_backupram[offset] = data;
}

WRITE8_HANDLER( standard_rom_bank_w )
{
	int bank = data&0x1f;

	//logerror("bank w %02x %02x\n", offset, data);

	sms_mainram[0x1ffc+offset] = data;
	switch (offset)
	{
		case 0:
			logerror("bank w %02x %02x\n", offset, data);
			if ((data & 0x08) && smsgg_backupram)
			{
				memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x8000, 0x9fff, 0, 0, smsgg_backupram_r);
				memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0x9fff, 0, 0, smsgg_backupram_w);
			}
			else
			{
				memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MRA8_BANK1);
				memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MWA8_ROM);
			}

			//printf("bank ram??\n");
			break;
		case 1:
			memcpy(sms_rom+0x0000, memory_region(REGION_USER1)+bank*0x4000, 0x4000);
			break;
		case 2:
			memcpy(sms_rom+0x4000, memory_region(REGION_USER1)+bank*0x4000, 0x4000);
			break;
		case 3:
			memcpy(sms_rom+0x8000, memory_region(REGION_USER1)+bank*0x4000, 0x4000);
			break;

	}
}

void init_standard_map(void)
{
	/* INIT THE MEMMAP / BANKING *********************************************************************************/

	/* catch any addresses that don't get mapped */
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_w);

	/* fixed rom bank area */
	sms_rom = auto_malloc(0xc000);
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MRA8_BANK1);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MWA8_ROM);
	memory_set_bankptr( 1, sms_rom );

	memcpy(sms_rom, memory_region(REGION_USER1), 0xc000);

	/* main ram area */
	sms_mainram = auto_malloc(0x2000); // 8kb of main ram
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xdfff, 0, 0, MRA8_BANK2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xdfff, 0, 0, MWA8_BANK2);
	memory_set_bankptr( 2, sms_mainram );
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0xe000, 0xffff, 0, 0, MRA8_BANK3);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xe000, 0xffff, 0, 0, MWA8_BANK3);
	memory_set_bankptr( 3, sms_mainram );
	memset(sms_mainram,0x00,0x2000);

	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xfffc, 0xffff, 0, 0, standard_rom_bank_w);

	init_ports_standard();
	smsgg_backupram = NULL;

}

WRITE8_HANDLER( megatech_bios_6000_w )
{
	static int count = 0;

	printf("6000_w %d/9 %02x\n", count%9, data);
	count++;
}

READ8_HANDLER( megatech_bios_6800_r )
{
	return rand();
}

READ8_HANDLER( megatech_bios_6801_r )
{
	return rand();
}

void init_megatech_map(void)
{
	/* catch any addresses that don't get mapped */
//	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_r);
//	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_w);

	/* fixed rom bank areas */
	sms_rom = auto_malloc(0x8000);
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x2fff, 0, 0, MRA8_BANK1);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x2fff, 0, 0, MWA8_ROM);
	memory_set_bankptr( 1, sms_rom );
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x7000, 0x7fff, 0, 0, MRA8_BANK3);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x7000, 0x7fff, 0, 0, MWA8_ROM);
	memory_set_bankptr( 3, sms_rom+0x7000 );
	memcpy(sms_rom, memory_region(REGION_USER1), 0x8000);

	/* main ram area */
	sms_mainram = auto_malloc(0x3000); // 8kb of main ram
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x3000, 0x5fff, 0, 0, MRA8_BANK2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x3000, 0x5fff, 0, 0, MWA8_BANK2);
	memory_set_bankptr( 2, sms_mainram );
	memset(sms_mainram,0x00,0x2000);

	init_ports_standard();

	memory_install_write8_handler (0, ADDRESS_SPACE_PROGRAM, 0x6000, 0x6000, 0, 0, megatech_bios_6000_w );

	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x6800, 0x6800, 0, 0, megatech_bios_6800_r );
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x6801, 0x6801, 0, 0, megatech_bios_6801_r );

	smsgg_backupram = NULL;
}

void draw_tile_line(int drawxpos, int tileline, UINT16 tiledata, UINT8* linebuf, struct sms_vdp* chip)
{
	int xx;
	UINT32 gfxdata;
	UINT16 gfx_base = (tiledata & 0x01ff)<<5;
	UINT8  flipx = (tiledata & 0x0200)>>9;
	UINT8  flipy = (tiledata & 0x0400)>>10;
	UINT8  pal   = (tiledata & 0x0800)>>11;
	UINT8  pri   = (tiledata & 0x1000)>>12;

	if (flipy)
	{
		gfx_base+=(7-tileline)*4;
	}
	else
	{
		gfx_base+=tileline*4;
	}

	gfxdata = (SMS_VDP_VRAM(gfx_base)<<24)|(SMS_VDP_VRAM(gfx_base+1)<<16)|(SMS_VDP_VRAM(gfx_base+2)<<8)|(SMS_VDP_VRAM(gfx_base+3)<<0);

	for (xx=0;xx<8;xx++)
	{
		UINT8 pixel;

		if (flipx)
		{
			pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
		            (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
		            (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
		            (( (gfxdata>>(24+xx) ) &0x00000001)<<0);
		}
		else
		{
			pixel = (( (gfxdata>>(7-xx)  ) &0x00000001)<<3)|
			        (( (gfxdata>>(15-xx) ) &0x00000001)<<2)|
			        (( (gfxdata>>(23-xx) ) &0x00000001)<<1)|
			        (( (gfxdata>>(31-xx) ) &0x00000001)<<0);
		}

		pixel += pal*0x10;

		if (!pri) linebuf[drawxpos+xx] = pixel;
		else
		{
			if (pixel&0xf)
				linebuf[drawxpos+xx] = pixel|0x80;
			else
				linebuf[drawxpos+xx] = pixel;

		}
	}
}

void sms_render_spriteline(int scanline, struct sms_vdp* chip)
{
	int spritenum;
	int height = 8;
	int width = 8;
	int max_sprites = 8;
	int visible_line = 0;

	UINT16 table_base = (chip->regs[0x5]&0x7e) << 7;
	UINT8 pattern_bit = (chip->regs[0x6]&0x04) >> 2; // high bit of the tile # (because spriteram can only contain an 8-bit tile #)


	memset(chip->sprite_renderline, 0, 256+32);

	for (spritenum = 0;spritenum<64;spritenum++)
	{
		int xpos,ypos,num;
		/*
		00: yyyyyyyyyyyyyyyy
		10: yyyyyyyyyyyyyyyy
		20: yyyyyyyyyyyyyyyy
		30: yyyyyyyyyyyyyyyy
		40: ????????????????
		50: ????????????????
		60: ????????????????
		70: ????????????????
		80: xnxnxnxnxnxnxnxn
		90: xnxnxnxnxnxnxnxn
		A0: xnxnxnxnxnxnxnxn
		B0: xnxnxnxnxnxnxnxn
		C0: xnxnxnxnxnxnxnxn
		D0: xnxnxnxnxnxnxnxn
		E0: xnxnxnxnxnxnxnxn
		F0: xnxnxnxnxnxnxnxn
		*/

		ypos = SMS_VDP_VRAM(table_base+spritenum);
		xpos = SMS_VDP_VRAM(table_base+0x80+spritenum*2+0);
		num  = SMS_VDP_VRAM(table_base+0x80+spritenum*2+1)|(pattern_bit<<8);

		if (chip->regs[0x1]&0x2)
		{
			num &=0x1fe;
			height=16;
		}
		else height = 8;


		xpos+=16; // allow room either side for clipping (avoids xdrawpos of -8 if bit below is set)

		if (chip->regs[0x0]&0x08) xpos-=8;

		if ((sms_mode_table[chip->screen_mode].sms2_height)==192)
		{
			if (ypos == 0xd0)
				return;
		}

		ypos++;

		num <<= 5;
		//num+=((scanline-ypos)&0x7)*4;

		visible_line = 0;

		if (ypos<=scanline && ypos+height>scanline)
		{
			visible_line = 1;
			num+=((scanline-ypos)&(height-1))*4;

		}
		else if (ypos+height>0x100)
		{
			if (scanline< ypos+height-0x100)
			{
				visible_line = 1;
				num+=((scanline-ypos)&(height-1))*4;

			}
		}

		if (visible_line)
		{
			int xx;
			UINT32 gfxdata;

			gfxdata = (SMS_VDP_VRAM(num&0x3fff)<<24)|(SMS_VDP_VRAM((num+1)&0x3fff)<<16)|(SMS_VDP_VRAM((num+2)&0x3fff)<<8)|(SMS_VDP_VRAM((num+3)&0x3fff)<<0);


			for (xx=0;xx<8;xx++)
			{
				UINT8 pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
				              (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
				              (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
				              (( (gfxdata>>(24+xx) ) &0x00000001)<<0);

				if (pixel)
				{
					if (!chip->sprite_renderline[xpos+((width-1)-xx)])
					{
						chip->sprite_renderline[xpos+((width-1)-xx)] = pixel;
					}
					else
					{
						chip->sprite_collision = 1;
					}
				}
			}

			max_sprites--;

			if (max_sprites==0)
			{
				chip->sprite_overflow = 1;
				return;
			}

		}
	}
}

void sms_render_tileline(int scanline, struct sms_vdp* chip)
{
	int column = 0;
	int count = 32;
	int drawxpos;

	UINT8 xscroll = chip->regs[0x8];
	UINT8 yscroll = chip->yscroll;
	UINT16 table_base = (chip->regs[0x2]&0x0e)<<10;
	UINT16 our_base;
	UINT8  our_line = (scanline+yscroll) & 0x7;

	/* In 224 and 240 line modes the table base is different */
	if ((sms_mode_table[chip->screen_mode].sms2_height)!=192)
	{
		table_base &=0x3700; table_base|=0x700;
	}

	if ((chip->regs[0x0]&0x40) && scanline < 16)
	{
		xscroll = 0;
	}

//	xscroll = 0;

//	table_base -= 0x0100;

	our_base = table_base+(((scanline+yscroll)%sms_mode_table[chip->screen_mode].sms2_tilemap_height)>>3)*64;// + (yscroll>>3)*32;

	our_base &=0x3fff;

	memset(chip->tile_renderline, (chip->regs[0x7]&0x0f)+0x10, 256+8);

	drawxpos = xscroll&0x7;
	column = 32-(xscroll>>3);

	do
	{
		UINT16 tiledata = (SMS_VDP_VRAM((our_base+(column&0x1f)*2+1)&0x3fff) << 8) |
		                  (SMS_VDP_VRAM((our_base+(column&0x1f)*2+0)&0x3fff) << 0);

//		UINT16 pattern = ((column+((scanline>>3)*32)) & 0x01ff)<<5;

		draw_tile_line(drawxpos, our_line, tiledata, chip->tile_renderline, chip);

		drawxpos+=8;
		column++;
		column&=0x1f;
		count--;
	} while (count);

}

void sms_copy_to_renderbuffer(int scanline, struct sms_vdp* chip)
{
	int x;
	UINT16* lineptr = chip->r_bitmap->line[scanline];

	for (x=0;x<256;x++)
	{
		UINT8 dat = chip->tile_renderline[x];
		UINT8 col;


		col = (chip->regs[0x7]&0x0f)+0x10;
		lineptr[x] = chip->cram_mamecolours[col];


		if ((x<8 && (chip->regs[0x0]&0x20)) || !(chip->regs[0x1]&0x40))
			continue;


		if (!(dat & 0x80))
		{
			lineptr[x] = chip->cram_mamecolours[dat&0x1f];
			if ((dat&0xf)==0x0) lineptr[x]|=0x8000;

		}

		if (chip->sprite_renderline[x+16])
		{
			lineptr[x] =  chip->cram_mamecolours[chip->sprite_renderline[x+16]+0x10];
		}

		if (dat & 0x80)
		{
			lineptr[x] = chip->cram_mamecolours[dat&0x1f];
			if ((dat&0xf)==0x0) lineptr[x]|=0x8000;
		}

	}

}

void sms_draw_scanline(int scanline, struct sms_vdp* chip)
{

	if (scanline>=0 && scanline<sms_mode_table[chip->screen_mode].sms2_height)
	{
		sms_render_spriteline(scanline, chip);
		sms_render_tileline(scanline, chip);
		sms_copy_to_renderbuffer(scanline, chip);

	}
}


static void sms_scanline_timer_callback(void* param)
{
	/* This function is called at the very start of every scanline starting at the very
	   top-left of the screen.  The first scanline is scanline 0 (we set scanline to -1 in
	   VIDEO_EOF) */

	/* Compensate for some rounding errors

	   When the counter reaches 314 (or whatever the max lines is) we should have reached the
	   end of the frame, however due to rounding errors in the timer calculation we're not quite
	   there.  Let's assume we are still in the previous scanline for now.

	   The position to get the H position also has to compensate for a few errors
	*/
//	printf("num %d\n",num );
	struct sms_vdp *chip = param;

	if (chip->sms_scanline_counter<(chip->sms_total_scanlines-1))
	{
		chip->sms_scanline_counter++;
		timer_adjust_ptr(chip->sms_scanline_timer,  SUBSECONDS_TO_DOUBLE(1000000000000000000LL/chip->sms_framerate/chip->sms_total_scanlines), 0);


		if (chip->sms_scanline_counter>sms_mode_table[chip->screen_mode].sms2_height)
		{
			chip->hint_counter=chip->regs[0xa];
		}

		if (chip->sms_scanline_counter==0)
		{
			chip->hint_counter=chip->regs[0xa];
		}

		if (chip->sms_scanline_counter<=192)
		{
			chip->hint_counter--;

			if (chip->hint_counter==0xff)
			{
				//if (chip->chip_id==2) printf("irq triggerd on scanline %d %d\n", vdp1->sms_scanline_counter, vdp2->sms_scanline_counter);

				chip->line_irq_pending = 1;
				chip->hint_counter=chip->regs[0xa];
				if (chip->regs[0x0]&0x10)
				{
					(chip->set_irq)(1); // set IRQ;
				}
				else
				{
					(chip->set_irq)(0); // clear IRQ;
				}
			}

		}


		sms_draw_scanline(chip->sms_scanline_counter, chip);

		//if(sms_scanline_counter==0) chip->sprite_collision = 0;

		if (chip->sms_scanline_counter==sms_mode_table[chip->screen_mode].sms2_height+1 )
		{
			chip->frame_irq_pending = 1;
			if (chip->regs[0x1]&0x20)
			{
				(chip->set_irq)(1); // set IRQ;
			}
			else
			{
				(chip->set_irq)(0); // clear IRQ;
			}
		}
	}
	else
	{	/* if we're called passed the total number of scanlines then assume we're still on the last one.
	       this can happen due to rounding errors */
		chip->sms_scanline_counter = chip->sms_total_scanlines-1;
	}
}

void show_tiles(struct sms_vdp* chip)
{
	int x,y,xx,yy;
	UINT16 count = 0;

	for (y=0;y<16;y++)
	{
		for (x=0;x<32;x++)
		{
			for (yy=0;yy<8;yy++)
			{
				int drawypos = y*8+yy;
				UINT16* lineptr = chip->r_bitmap->line[drawypos];

				UINT32 gfxdata = (SMS_VDP_VRAM(count)<<24)|(SMS_VDP_VRAM(count+1)<<16)|(SMS_VDP_VRAM(count+2)<<8)|(SMS_VDP_VRAM(count+3)<<0);

				for (xx=0;xx<8;xx++)
				{
					int drawxpos = x*8+xx;

					UINT8 pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
					              (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
					              (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
					              (( (gfxdata>>(24+xx) ) &0x00000001)<<0);

					lineptr[drawxpos] = chip->cram_mamecolours[pixel+16];

				}


				count+=4;count&=0x3fff;
			}
		}
	}
}

/*
 Register $00 - Mode Control No. 1

 D7 - 1= Disable vertical scrolling for columns 24-31
 D6 - 1= Disable horizontal scrolling for rows 0-1
 D5 - 1= Mask column 0 with overscan color from register #7
 D4 - (IE1) 1= Line interrupt enable
 D3 - (EC) 1= Shift sprites left by 8 pixels
 D2 - (M4) 1= Use Mode 4, 0= Use TMS9918 modes (selected with M1, M2, M3)
 D1 - (M2) Must be 1 for M1/M3 to change screen height in Mode 4.
      Otherwise has no effect.
 D0 - 1= No sync, display is monochrome, 0= Normal display

 Bits 0 and 5 have no effect on the GameGear in either mode, while bit 6
 has no effect in GG mode but works normally in SMS mode.
 */

 /*
  Register $01 - Mode Control No. 2

  D7 - No effect
  D6 - (BLK) 1= Display visible, 0= display blanked.
  D5 - (IE0) 1= Frame interrupt enable.
  D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
  D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
  D2 - No effect
  D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
  D0 - Sprite pixels are doubled in size.

 Even though some games set bit 7, it does nothing.
 */

void end_of_frame(struct sms_vdp *chip)
{
	UINT8 m1 = (chip->regs[0x1]&0x10)>>4;
	UINT8 m2 = (chip->regs[0x0]&0x02)>>1;
	UINT8 m3 = (chip->regs[0x1]&0x08)>>3;
	UINT8 m4 = (chip->regs[0x0]&0x04)>>2;
	UINT8 m5 = chip->is_pal;
	chip->screen_mode = m1|(m2<<1)|(m3<<2)|(m4<<3)|(m5<<4);

	if (chip->vdp_type!=GG_VDP) /* In GG mode the Game Gear resolution is fixed */
	{
		rectangle visarea;

		visarea.min_x = 0;
		visarea.max_x = 256-1;
		visarea.min_y = 0;
		visarea.max_y = sms_mode_table[chip->screen_mode].sms2_height-1;

		video_screen_configure(0, 256, sms_mode_table[chip->screen_mode].sms2_height, &visarea, chip->sms_framerate);
	}
	else /* 160x144 */
	{
		rectangle visarea;
		visarea.min_x = (256-160)/2;
		visarea.max_x = (256-160)/2+160-1;
		visarea.min_y = (192-144)/2;
		visarea.max_y = (192-144)/2+144-1;

		video_screen_configure(0, 256, sms_mode_table[chip->screen_mode].sms2_height, &visarea, chip->sms_framerate);

	}



//	printf("Mode: %s is ok\n", sms_mode_table[chip->screen_mode].sms2_name);

	chip->sms_scanline_counter = -1;
	chip->yscroll = chip->regs[0x9]; // this can't change mid-frame
	timer_adjust_ptr(chip->sms_scanline_timer,  TIME_NOW, 0);
}

VIDEO_EOF(sms)
{
	end_of_frame(vdp1);
	if (SMS_PAUSE_BUTTON) cpunum_set_input_line(0,INPUT_LINE_NMI,PULSE_LINE);
}

VIDEO_START(sms)
{

//	vdp->is_pal = 1;
//	vdp->sms_total_scanlines = 313;
//	vdp->sms_framerate = 50;



	return 0;
}

VIDEO_UPDATE(sms)
{
//	show_tiles();

//	copybitmap(bitmap, vdp1->r_bitmap, 0, 0, 0, 0, cliprect, TRANSPARENCY_NONE, 0);
	int x,y;

	for (y=0;y<224;y++)
	{
		UINT16* lineptr = bitmap->line[y];
		UINT16* srcptr =  vdp1->r_bitmap->line[y];

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}

	}
	return 0;
}

MACHINE_RESET(sms)
{
	timer_adjust_ptr(vdp1->sms_scanline_timer,  TIME_NOW, 0);
}

static NVRAM_HANDLER( smsgg )
{
	if (smsgg_backupram!=NULL)
	{
		if (read_or_write)
			mame_fwrite(file, smsgg_backupram, 0x2000);
		else
		{
			if (file)
			{
				mame_fread(file, smsgg_backupram, 0x2000);
			}
		}
	}
}

MACHINE_DRIVER_START( sms )
	MDRV_CPU_ADD_TAG("z80", Z80, 3579540)
	MDRV_CPU_PROGRAM_MAP(sms_readmem,sms_writemem)
	MDRV_CPU_IO_MAP(sms_readport,sms_writeport)

	/* IRQ handled via the timers */

	MDRV_FRAMES_PER_SECOND(50)
	MDRV_VBLANK_DURATION(0) // Vblank handled manually.
	MDRV_MACHINE_RESET(sms)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER|VIDEO_RGB_DIRECT)

	MDRV_SCREEN_SIZE(256, 256)
	MDRV_VISIBLE_AREA(0, 255, 0, 223)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(sms)
	MDRV_VIDEO_UPDATE(sms) /* Copies a bitmap */
	MDRV_VIDEO_EOF(sms) /* Used to Sync the timing */

	MDRV_NVRAM_HANDLER( smsgg )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SN76496, 3579540)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( s_fantdz )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fantdz.bin", 0x000000, 0x040000, CRC(b9664ae1) SHA1(4202ce26832046c7ca8209240f097a8a0a84d981) )
ROM_END

ROM_START( s_dinob )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dinob.bin", 0x000000, 0x040000, CRC(ea5c3a6f) SHA1(05b4f23e33ada08e0a8b1fc6feccd8a97c690a21) )
ROM_END

ROM_START( s_cosmic )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cosmic.bin", 0x000000, 0x040000, CRC(29822980) SHA1(f46f716dd34a1a5013a2d8a59769f6ef7536a567) )
ROM_END

ROM_START( s_micro )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_micro.bin", 0x000000, 0x040000, CRC(a577ce46) SHA1(425621f350d011fd021850238c6de9999625fd69) )
ROM_END


ROM_START( gg_exldz )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_exldz.bin", 0x000000, 0x080000,  CRC(aa140c9c) SHA1(db3c0686715373242911f8471a1e91673811d62a) )
ROM_END

ROM_START( s_landil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_landil.bin", 0x000000, 0x080000, CRC(24e97200) SHA1(a120f29c6bf2e733775d5b984bd3a156682699c6) )
ROM_END

ROM_START( s_tazman )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tazman.bin", 0x000000, 0x040000, CRC(1b312e04) SHA1(274641b3b05245504f763a5e4bc359183d16a092) )
ROM_END

ROM_START( s_bubbob )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bubbob.bin", 0x000000, 0x040000, CRC(e843ba7e) SHA1(44876b44089b4174858e54202e567e02efa76859) )
ROM_END

ROM_START( s_chuck )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chuck.bin", 0x000000, 0x080000, CRC(dd0e2927) SHA1(0199c62afb5c09f09999a4815079875b480129f3) )
ROM_END

ROM_START( s_chuck2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chuck2.bin", 0x000000, 0x080000,  CRC(c30e690a) SHA1(46c326d7eb73b0393de7fc40bf2ee094ebab482d) )
ROM_END


ROM_START( s_adams )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_adams.bin", 0x000000, 0x040000,  CRC(72420f38) SHA1(3fc6ccc556a1e4eb376f77eef8f16b1ff76a17d0) )
ROM_END
ROM_START( s_aburn )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aburn.bin", 0x000000, 0x080000,  CRC(1c951f8e) SHA1(51531df038783c84640a0cab93122e0b59e3b69a) )
ROM_END
ROM_START( s_aladin )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aladin.bin", 0x000000, 0x080000,  CRC(c8718d40) SHA1(585967f400473e289cda611e7686ae98ae43172e) )
ROM_END
ROM_START( s_alexmi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexmi.bin", 0x000000, 0x020000,  CRC(aed9aac4) SHA1(6d052e0cca3f2712434efd856f733c03011be41c) )
ROM_END
ROM_START( s_alsynd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alsynd.bin", 0x000000, 0x040000,  CRC(5cbfe997) SHA1(0da0b9755b6a6ef145ec3b95e651d2a384b3f7f9) )
ROM_END
ROM_START( s_alstor )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alstor.bin", 0x000000, 0x040000,  CRC(7f30f793) SHA1(aece64ecbfbe08b199b29df9bc75743773ea3d34) )
ROM_END
ROM_START( s_actfgh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_actfgh.bin", 0x000000, 0x020000,  CRC(3658f3e0) SHA1(b462246fed3cbb9dc3909a2d5befaec65d7a0014) )
ROM_END
ROM_START( s_column )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_column.bin", 0x000000, 0x020000,  CRC(665fda92) SHA1(3d16b0954b5419b071de270b44d38fc6570a8439) )
ROM_END
ROM_START( s_bean )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bean.bin", 0x000000, 0x040000,  CRC(6c696221) SHA1(89df035da8de68517f82bdf176d3b3f2edcd9e31) )
ROM_END
ROM_START( s_fzone )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fzone.bin", 0x000000, 0x020000,  CRC(65d7e4e0) SHA1(0278cd120dc3a7707eda9314c46c7f27f9e8fdda) )
ROM_END
ROM_START( s_fzone2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fzone2.bin", 0x000000, 0x040000,  CRC(b8b141f9) SHA1(b84831378c7c19798b6fd560647e2941842bb80a) )
ROM_END
ROM_START( s_fzone3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fzone3.bin", 0x000000, 0x020000,  CRC(d29889ad) SHA1(a2cd356826c8116178394d8aba444cb636ef784e) )
ROM_END
ROM_START( s_flint )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_flint.bin", 0x000000, 0x040000,  CRC(ca5c78a5) SHA1(a0aa76d89f6831999c328877057a99e72c6b533b) )
ROM_END


ROM_START( s_wb3dt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wb3dt.bin", 0x000000, 0x040000,  CRC(679e1676) SHA1(99e73de2ffe5ea5d40998faec16504c226f4c1ba) )
ROM_END
ROM_START( s_woody )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_woody.bin", 0x000000, 0x008000,  CRC(315917d4) SHA1(b74078c4a3e6d20d21ca81e88c0cb3381b0c84a4) )
ROM_END
ROM_START( s_zool )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zool.bin", 0x000000, 0x040000,  CRC(9d9d0a5f) SHA1(aed98f2fc885c9a6e121982108f843388eb46304) )
ROM_END
ROM_START( s_smgpa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smgpa.bin", 0x000000, 0x040000,  CRC(55bf81a0) SHA1(0f11b9d7d6d16b09f7be0dace3b6c7d3524da725) )
ROM_END
ROM_START( s_sor )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sor.bin", 0x000000, 0x080000,  CRC(6f2cf48a) SHA1(ea4214198a53dbcc0e4df5f1c34530bff5bea1f5) )
ROM_END
ROM_START( s_lucky )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lucky.bin", 0x000000, 0x040000,  CRC(a1710f13) SHA1(a08815d27e431f0bee70b4ebfb870a3196c2a732) )
ROM_END
ROM_START( s_lionk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lionk.bin", 0x000000, 0x080000,   CRC(c352c7eb) SHA1(3a64657e3523a5da1b99db9d89b1a48ed4ccc5ed) )
ROM_END
ROM_START( s_lemm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lemm.bin", 0x000000, 0x040000,  CRC(f369b2d8) SHA1(f3a853cce1249a0848bfc0344f3ee2db6efa4c01) )
ROM_END
ROM_START( s_jp2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_jp2.bin", 0x000000, 0x080000,   CRC(102d5fea) SHA1(df4f55f7ff9236f65aee737743e7500c4d96cf12) )
ROM_END
ROM_START( s_gpride )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gpride.bin", 0x000000, 0x080000,  CRC(ec2da554) SHA1(3083069782c7cfcf2cc1229aca38f8f2971cf284) )
ROM_END
ROM_START( s_jungbk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_jungbk.bin", 0x000000, 0x040000,  CRC(695a9a15) SHA1(f65b5957b4b21bd16b4aa8a6e93fed944dd7d9ac) )
ROM_END

ROM_START( s_gaunt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gaunt.bin", 0x000000, 0x020000,  CRC(d9190956) SHA1(4e583ce9b95e20ecddc6c1dac9941c28a3e80808) )
ROM_END
ROM_START( s_gng )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gng.bin", 0x000000, 0x040000, CRC(7a92eba6) SHA1(b193e624795b2beb741249981d621cb650c658db) )
ROM_END


ROM_START( s_castil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_castil.bin", 0x000000, 0x040000, CRC(953f42e1) SHA1(c200b5e585d59f8bfcbb40fd6d4314de8abcfae3) )
ROM_END
ROM_START( s_sonic )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sonic.bin", 0x000000, 0x040000, CRC(b519e833) SHA1(6b9677e4a9abb37765d6db4658f4324251807e07) )
ROM_END
ROM_START( s_sonic2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sonic2.bin", 0x000000, 0x080000,  CRC(5b3b922c) SHA1(acdb0b5e8bf9c1e9c9d1a8ac6d282cb8017d091c) )
ROM_END
ROM_START( s_spyspy )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spyspy.bin", 0x000000, 0x008000, CRC(78d7faab) SHA1(feab16dd8b807b88395e91c67e9ff52f8f7aa7e4) )
ROM_END
ROM_START( s_suptet )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_suptet.bin", 0x000000, 0x010000, CRC(bd1cc7df) SHA1(3772e9dc07b15de62326ee78981ec0b6f4387590) )
ROM_END
ROM_START( s_supko )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supko.bin", 0x000000, 0x040000, CRC(406aa0c2) SHA1(4116e4a742e3209ca5db9887cd92219d15cf3c9a) )
ROM_END
ROM_START( s_strid )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_strid.bin", 0x000000, 0x080000, CRC(9802ed31) SHA1(051e72c8ffec7606c04409ef38244cfdd592252f) )
ROM_END
ROM_START( s_ssi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ssi.bin", 0x000000, 0x040000, CRC(1d6244ee) SHA1(b28d2a9c0fe597892e21fb2611798765f5435885) )
ROM_END
ROM_START( s_rrsh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rrsh.bin", 0x000000, 0x080000, CRC(b876fc74) SHA1(7976e717125757b1900a540a68e0ef3083839f85) )
ROM_END
ROM_START( s_psycho )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_psycho.bin", 0x000000, 0x040000, CRC(97993479) SHA1(278cc3853905626138e83b6cfa39c26ba8e4f632) )
ROM_END
ROM_START( s_tnzs )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tnzs.bin", 0x000000, 0x040000, CRC(c660ff34) SHA1(e433b21b505ed5428d1b2f07255e49c6db2edc6c) )
ROM_END


ROM_START( s_20em1 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_20em1.bin", 0x000000, 0x040000, CRC(f0f35c22) SHA1(2012763dc08dedc68af8538698bd66618212785d) )
ROM_END
ROM_START( s_aceace )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aceace.bin", 0x000000, 0x040000, CRC(887d9f6b) SHA1(08a79905767b8e5af8a9c9c232342e6c47588093) )
ROM_END
ROM_START( s_actfgj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_actfgj.bin", 0x000000, 0x020000, CRC(d91b340d) SHA1(5dbcfb75958f4cfa1b61d9ea114bab67787b113e) )
ROM_END
ROM_START( s_aerial )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aerial.bin", 0x000000, 0x040000, CRC(ecf491cf) SHA1(2a9090ed365e7425ca7a59f87b942c16b376f0a3) )
ROM_END
ROM_START( s_airesc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_airesc.bin", 0x000000, 0x040000, CRC(8b43d21d) SHA1(4edd1b62abbbf2220961a06eb139db1838fb700b) )
ROM_END
ROM_START( s_aleste )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aleste.bin", 0x000000, 0x020000, CRC(d8c4165b) SHA1(cba75b0d54df3c9a8e399851a05c194c7a05fbfe) )
ROM_END
ROM_START( s_alexls )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexls.bin", 0x000000, 0x040000, CRC(c13896d5) SHA1(6dbf2684c3dfea7442d0b40a9ff7c8b8fc9b1b98) )
ROM_END
ROM_START( s_alexbm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexbm.bin", 0x000000, 0x020000, CRC(f9dbb533) SHA1(77cc767bfae01e9cc81612c780c939ed954a6312) )
ROM_END
ROM_START( s_alexht )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexht.bin", 0x000000, 0x020000,  CRC(013c0a94) SHA1(2d0a581da1c787b1407fb1cfefd0571e37899978) )
ROM_END
ROM_START( s_alf )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alf.bin", 0x000000, 0x020000, CRC(82038ad4) SHA1(7706485b735f5d7f7a59c7d626b13b23e8696087) )
ROM_END


ROM_START( s_alien3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alien3.bin", 0x000000, 0x040000, CRC(b618b144) SHA1(be40ffc72ee19620a8bac89d5d96bbafcefc74e7) )
ROM_END
ROM_START( s_altbea )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_altbea.bin", 0x000000, 0x040000, CRC(bba2fe98) SHA1(413986f174799f094a8dd776d91dcf018ee17290) )
ROM_END
ROM_START( s_ash )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ash.bin", 0x000000, 0x040000, CRC(e4163163) SHA1(44ed3aeaa4c8a627b88c099b184ca99710fac0ad) )
ROM_END
ROM_START( s_astrx )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astrx.bin", 0x000000, 0x080000, CRC(147e02fa) SHA1(70e311421467acd01e55f1908eae653bf20de175) )
ROM_END
ROM_START( s_astrxa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astrxa.bin", 0x000000, 0x080000, CRC(8c9d5be8) SHA1(ae56c708bc197f462b68c3e5ff9f0379d841c8b0) )
ROM_END
ROM_START( s_astgr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astgr.bin", 0x000000, 0x080000, CRC(f9b7d26b) SHA1(5e409247f6a611437380b7a9f0e3cccab5dd1987) )
ROM_END
ROM_START( s_astsm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astsm.bin", 0x000000, 0x080000, CRC(def9b14e) SHA1(de6a32a548551553a4b3ae332f4bf98ed22d8ab5) )
ROM_END
ROM_START( s_bttf2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bttf2.bin", 0x000000, 0x040000, CRC(e5ff50d8) SHA1(31af58e655e12728b01e7da64b46934979b82ecf) )
ROM_END
ROM_START( s_bttf3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bttf3.bin", 0x000000, 0x040000,  CRC(2d48c1d3) SHA1(7d67dd38fea5dba4224a119cc4840f6fb8e023b9) )
ROM_END
ROM_START( s_baku )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_baku.bin", 0x000000, 0x040000, CRC(35d84dc2) SHA1(07d8f300b3a3542734fcd24fa8312fe99fbfef0e) )
ROM_END

ROM_START( s_bartsm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bartsm.bin", 0x000000, 0x040000, CRC(d1cc08ee) SHA1(4fa839db6de21fd589f6a91791fff25ca2ab88f4) )
ROM_END
ROM_START( s_boutr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_boutr.bin", 0x000000, 0x040000, CRC(c19430ce) SHA1(cfa8721d4fc71b1f14e9a06f2db715a6f88be7dd) )
ROM_END
ROM_START( s_calig )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_calig.bin", 0x000000, 0x040000,  CRC(ac6009a7) SHA1(d0f8298bb2a30a3569c65372a959612df3b608db) )
ROM_END
ROM_START( s_calig2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_calig2.bin", 0x000000, 0x040000, CRC(c0e25d62) SHA1(5c7d99ba54caf9a674328df787a89e0ab4730de8) )
ROM_END
ROM_START( s_coolsp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_coolsp.bin", 0x000000, 0x040000, CRC(13ac9023) SHA1(cf36c1900d1c658cbfd974464761d145af3467c8) )
ROM_END
ROM_START( s_ddux )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ddux.bin", 0x000000, 0x040000, CRC(0e1cc1e0) SHA1(2a513aef0f0bdcdf4aaa71e7b26a15ce686db765) )
ROM_END
ROM_START( s_legnil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_legnil.bin", 0x000000, 0x080000, CRC(6350e649) SHA1(62adbd8e5a41d08c4745e9fbb1c51f0091c9dea6) )
ROM_END
ROM_START( s_mspac )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mspac.bin", 0x000000, 0x020000, CRC(3cd816c6) SHA1(4dd7cb303793792edd9a4c717a727f46db857dae) )
ROM_END
ROM_START( s_pmania )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pmania.bin", 0x000000, 0x020000, CRC(be57a9a5) SHA1(c0a11248bbb556b643accd3c76737be35cbada54) )
ROM_END
ROM_START( s_rtype )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rtype.bin", 0x000000, 0x080000, CRC(bb54b6b0) SHA1(08ec70a2cd98fcb2645f59857f845d41b0045115) )
ROM_END

ROM_START( s_sensi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sensi.bin", 0x000000, 0x020000, CRC(f8176918) SHA1(08b4f5b8096bc811bc9a7750deb21def67711a9f) )
ROM_END
ROM_START( s_smgp2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smgp2.bin", 0x000000, 0x080000, CRC(e890331d) SHA1(b6819b014168aaa03b65dae97ba6cd5fa0d7f0d9) )
ROM_END
ROM_START( s_supoff )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supoff.bin", 0x000000, 0x020000, CRC(54f68c2a) SHA1(804cd74bbcf452613f6c3a722be1c94338499813) )
ROM_END
ROM_START( s_zill )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zill.bin", 0x000000, 0x020000,  CRC(5718762c) SHA1(3aab3e95dac0fa93612da20cf525dba4dc4ca6ba) )
ROM_END
ROM_START( s_zill2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zill2.bin", 0x000000, 0x020000, CRC(2f2e3bc9) SHA1(1e08be828ecf4cf5db787704ab8779f4b5a444b5) )
ROM_END









ROM_START( gg_bust )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_bust.bin", 0x000000, 0x040000, CRC(c90f29ef) SHA1(e6bb5f72cffb11c8dd44ac3e378088b04cec1297) )
ROM_END

ROM_START( gg_puzlo )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_puzlo.bin", 0x000000, 0x040000, CRC(d173a06f) SHA1(8ea2e623858221c5d39eb1e0f6532a0b23b00305) )
ROM_END

ROM_START( gg_puyo2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_puyo2.bin", 0x000000, 0x080000, CRC(3ab2393b) SHA1(04ea9ff57faeeccd1bbb17490d22313ae94c86d6) )
ROM_END

ROM_START( gg_tempj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_tempj.bin", 0x000000, 0x080000,  CRC(de466796) SHA1(a2d800d2836a03f81dd8f3dda23cd2d8bfff18a5) )
ROM_END

ROM_START( gg_tess )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_tess.bin", 0x000000, 0x020000, CRC(ca0e11cc) SHA1(2b2652c7e03218b212e4d6a6246bd70e925e7ee1) )
ROM_END

ROM_START( gg_popil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_popil.bin", 0x000000, 0x020000, CRC(cf6d7bc5) SHA1(fb939f0810d0763b9abaeec1a2bfbabacaad5441) )
ROM_END

ROM_START( gg_nazo )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_nazo.bin", 0x000000, 0x020000, CRC(bcce5fd4) SHA1(6ad654e11067ed12437022fcf22b9fea9be7ac46) )
ROM_END
ROM_START( gg_nazo2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_nazo2.bin", 0x000000, 0x040000, CRC(73939de4) SHA1(dbd511aff622c618eac0c21de36965b869362dbd) )
ROM_END
ROM_START( gg_gear )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_gear.bin", 0x000000, 0x020000, CRC(e9a2efb4) SHA1(dc71c749c4a78e28922e10df645d23cc8289531f) )
ROM_END
ROM_START( gg_bean )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_bean.bin", 0x000000, 0x040000, CRC(3c2d4f48) SHA1(c36bda8a86994d430506b196922ec1365622e560 ) )
ROM_END
ROM_START( gg_cols )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_cols.bin", 0x000000, 0x008000, CRC(83fa26d9) SHA1(05b73f39a90fd59e01262cbd3f4e7a21575d468a) )
ROM_END
ROM_START( gg_baku )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_baku.bin", 0x000000, 0x040000, CRC(10ac9374) SHA1(5145967d0a3632cf6bb1a9d58b2ef04faecd40db) )
ROM_END




ROM_START( megatech )
	ROM_REGION( 0x20000, REGION_USER1, 0 )
	/* Bios */
	ROM_LOAD( "epr12664.20",  0x00000, 0x8000, CRC(f71e9526) SHA1(1c7887541d02c41426992d17f8e3db9e03975953) )
	/* Instruction Rom (read at 0x8000)*/
//	ROM_LOAD( "12368-13.ic1", 0x10000, 0x8000, CRC(4038cbd1) SHA1(696bc1efce45d9f0052b2cf0332a232687c8d6ab) )

	ROM_REGION( 0x400000, REGION_USER3, 0 ) /* z80 Code */
//	ROM_LOAD( "ep13817.ic2", 0x000000, 0x020000, CRC(299cbb74) SHA1(901697a3535ad70190647f34ad5b30b695d54542) )
ROM_END

ROM_START( mt_astro )
	ROM_REGION( 0x20000, REGION_USER1, 0 )
	/* Bios */
	ROM_LOAD( "epr12664.20",  0x00000, 0x8000, CRC(f71e9526) SHA1(1c7887541d02c41426992d17f8e3db9e03975953) )
	/* Instruction Rom (read at 0x8000)*/
	ROM_LOAD( "12368-13.ic1", 0x10000, 0x8000, CRC(4038cbd1) SHA1(696bc1efce45d9f0052b2cf0332a232687c8d6ab) )

	ROM_REGION( 0x400000, REGION_USER3, 0 ) /* z80 Code */
//	ROM_LOAD( "ep13817.ic2", 0x000000, 0x020000, CRC(299cbb74) SHA1(901697a3535ad70190647f34ad5b30b695d54542) )
ROM_END

static DRIVER_INIT( codemast )
{
	init_codemasters_map();

	vdp1 = start_vdp(SMS2_VDP);
	vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 1;
	vdp1->sms_total_scanlines = 313;
	vdp1->sms_framerate = 50;
}

static DRIVER_INIT( standard )
{
	init_standard_map();

	vdp1 = start_vdp(SMS2_VDP);
	vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;

}

static DRIVER_INIT( standpal )
{
	init_standard_map();

	vdp1 = start_vdp(SMS2_VDP);
	vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 1;
	vdp1->sms_total_scanlines = 313;
	vdp1->sms_framerate = 50;
}

static DRIVER_INIT( mtc )
{
	init_megatech_map();

	vdp1 = start_vdp(SMS2_VDP);
	vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;
}


static DRIVER_INIT( codemagg )
{
	init_codemasters_map();
	init_extra_gg_ports();

	vdp1 = start_vdp(SMS_VDP);
	/* the gamegear is meant to be NTSC? but the codemsasters games expect
	   it to be PAL? (the game timings are clearly designed for it..) */
	vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 1;
	vdp1->sms_total_scanlines = 313;
	vdp1->sms_framerate = 50;
}

static DRIVER_INIT( standagg )
{
	init_standard_map();
	init_extra_gg_ports();

	vdp1 = start_vdp(GG_VDP);
	vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;
}

static DRIVER_INIT( gg_popil )
{
	init_standagg();
	smsgg_backupram = auto_malloc(0x2000);
	memset(smsgg_backupram, 0xff, 0x2000);
}

GAME( 199?, s_fantdz,        0,        sms,    sms,    codemast, ROT0,   "Codemasters", "Fantastic Dizzy (Master System)", 0 )
GAME( 199?, s_micro,         0,        sms,    sms,    codemast, ROT0,   "Codemasters", "Micro Machines (Master System)", 0 )
GAME( 199?, s_cosmic,        0,        sms,    sms,    codemast, ROT0,   "Codemasters", "Cosmic Spacehead (Master System)", 0 )
GAME( 199?, s_dinob,         0,        sms,    sms,    codemast, ROT0,   "Codemasters", "Dinobasher (proto) (Master System)", 0 ) // pause crashes it (!)

GAME( 199?, s_landil,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Mickey Mouse - Land of Illusion  [!] (Master System)", 0 )
GAME( 199?, s_tazman,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Taz-Mania  [!] (Master System)", 0 )
GAME( 199?, s_bubbob,        0,        sms,    sms,    standard, ROT0,   "Sega", "Bubble Bobble  [!] (Master System)", 0 )
GAME( 199?, s_chuck,         0,        sms,    sms,    standard, ROT0,   "Sega", "Chuck Rock  [!] (Master System)", 0 )
GAME( 199?, s_chuck2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Chuck Rock 2  [!] (Master System)", 0 )

GAME( 199?, s_adams,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Addams Family, The  [!] (Master System)", GAME_NOT_WORKING )
GAME( 199?, s_aburn,         0,        sms,    sms,    standard, ROT0,   "Sega", "After Burner  [!] (Master System)", 0 )
GAME( 199?, s_aladin,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Aladdin  [!] (Master System)", 0 )
GAME( 199?, s_alexmi,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Alex Kidd in Miracle World  [!] (Master System)", 0 )
GAME( 199?, s_alsynd,        0,        sms,    sms,    standard, ROT0,   "Sega", "Alien Syndrome  [!] (Master System)", 0 )
GAME( 199?, s_alstor,        0,        sms,    sms,    standard, ROT0,   "Sega", "Alien Storm  [!] (Master System)", 0 )
GAME( 199?, s_actfgh,        0,        sms,    sms,    standard, ROT0,   "Sega", "Action Fighter  [!] (Master System)", 0 )
GAME( 199?, s_column,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Columns  [!] (Master System)", 0 )
GAME( 199?, s_bean,          0,        sms,    sms,    standpal, ROT0,   "Sega", "Dr. Robotnik's Mean Bean Machine  [!] (Master System)", 0 )
GAME( 199?, s_fzone,         0,        sms,    sms,    standard, ROT0,   "Sega", "Fantasy Zone (Master System)", 0 )
GAME( 199?, s_fzone2,        0,        sms,    sms,    standard, ROT0,   "Sega", "Fantasy Zone 2 - The Tears of Opa-Opa  [!] (Master System)", 0 )
GAME( 199?, s_fzone3,        0,        sms,    sms,    standard, ROT0,   "Sega", "Fantasy Zone 3 - The Maze  [!] (Master System)", 0 )
GAME( 199?, s_flint,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Flintstones, The  [!] (Master System)", 0 )

GAME( 199?, s_wb3dt,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Wonder Boy 3 - The Dragon's Trap  [!] (Master System)", 0 )
GAME( 199?, s_woody,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Woody Pop (J) [!] (Master System)", GAME_NOT_WORKING ) // input
GAME( 199?, s_zool,          0,        sms,    sms,    standpal, ROT0,   "Sega", "Zool  [!] (Master System)", GAME_NOT_WORKING ) // locks up..
GAME( 199?, s_smgpa,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Super Monaco GP  [a1][!] (Master System)", 0 )
GAME( 199?, s_sor,           0,        sms,    sms,    standpal, ROT0,   "Sega", "Streets of Rage  [!] (Master System)", 0 )
GAME( 199?, s_lucky,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Lucky Dime Caper Starring Donald Duck, The  [!] (Master System)", 0 )
GAME( 199?, s_lionk,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Lion King, The  [!] (Master System)", 0 )
GAME( 199?, s_lemm,          0,        sms,    sms,    standpal, ROT0,   "Sega", "Lemmings  [!] (Master System)", 0 )
GAME( 199?, s_jp2,           0,        sms,    sms,    standpal, ROT0,   "Sega", "James Pond 2 - Codename Robocod  [!] (Master System)", GAME_NOT_WORKING ) // used to work..
GAME( 199?, s_gpride,        0,        sms,    sms,    standpal, ROT0,   "Sega", "GP Rider  [!] (Master System)", 0 )
GAME( 199?, s_jungbk,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Jungle Book, The  [!] (Master System)", 0 )
GAME( 199?, s_gaunt,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Gauntlet  [!] (Master System)", 0 )
GAME( 199?, s_gng,           0,        sms,    sms,    standpal, ROT0,   "Sega", "Ghouls 'n Ghosts  [!] (Master System)", 0 )

GAME( 199?, s_castil,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Mickey Mouse - Castle of Illusion  [!] (Master System)", 0 )
GAME( 199?, s_sonic,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Sonic the Hedgehog  [!] (Master System)", 0 )
GAME( 199?, s_sonic2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Sonic the Hedgehog 2  (V1.0) [!] (Master System)", 0 )
GAME( 199?, s_spyspy,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Spy vs. Spy  [!] (Master System)", 0 )
GAME( 199?, s_suptet,        0,        sms,    sms,    standard, ROT0,   "Sega", "Super Tetris (Korea) (Master System)", 0 )
GAME( 199?, s_supko,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Super Kick Off  [!] (Master System)", 0 )
GAME( 199?, s_strid,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Strider  [!] (Master System)", 0 )
GAME( 199?, s_ssi,           0,        sms,    sms,    standpal, ROT0,   "Sega", "Super Space Invaders  [!] (Master System)", 0 )
GAME( 199?, s_rrsh,          0,        sms,    sms,    standpal, ROT0,   "Sega", "Road Rash  [!] (Master System)", 0 )
GAME( 199?, s_psycho,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Psycho Fox  [!] (Master System)", 0 )
GAME( 199?, s_tnzs,          0,        sms,    sms,    standpal, ROT0,   "Sega", "New Zealand Story, The  [!] (Master System)", 0 )

GAME( 199?, s_20em1,         0,        sms,    sms,    standpal, ROT0,   "Sega", "20-em-1 (Brazil) [!] (Master System)", 0 )
GAME( 199?, s_aceace,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Ace of Aces  [!] (Master System)", GAME_NOT_WORKING ) // doesn't boot
GAME( 199?, s_actfgj,        0,        sms,    sms,    standard, ROT0,   "Sega", "Action Fighter (J) [!] (Master System)", 0 )
GAME( 199?, s_aerial,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Aerial Assault  [!] (Master System)", 0 )
GAME( 199?, s_airesc,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Air Rescue  [!] (Master System)", 0 )
GAME( 199?, s_aleste,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Aleste [!] (Master System)", 0 )
GAME( 199?, s_alexls,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Alex Kidd - The Lost Stars  [!] (Master System)", 0 )
GAME( 199?, s_alexbm,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Alex Kidd BMX Trial  [!] (Master System)", 0 )
GAME( 199?, s_alexht,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Alex Kidd in High Tech World  [!] (Master System)", 0 )
GAME( 199?, s_alf,           0,        sms,    sms,    standpal, ROT0,   "Sega", "ALF  [!] (Master System)", 0 )


GAME( 199?, s_alien3,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Alien 3  [!] (Master System)", 0 )
GAME( 199?, s_altbea,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Altered Beast  [!] (Master System)", 0 )
GAME( 199?, s_ash,           0,        sms,    sms,    standpal, ROT0,   "Sega", "Arcade Smash Hits  [!] (Master System)", 0 )
GAME( 199?, s_astrx,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Asterix  [!] (Master System)", 0 )
GAME( 199?, s_astrxa,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Asterix  [a1] (Master System)", 0 )
GAME( 199?, s_astgr,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Asterix and the Great Rescue  [!] (Master System)", 0 )
GAME( 199?, s_astsm,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Asterix and the Secret Mission  [!] (Master System)", 0 )
GAME( 199?, s_bttf2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Back to the Future 2  [!] (Master System)", 0 )
GAME( 199?, s_bttf3,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Back to the Future 3  [!] (Master System)", 0 )
GAME( 199?, s_baku,          0,        sms,    sms,    standpal, ROT0,   "Sega", "Baku Baku Animals (Brazil) [!] (Master System)", 0 )


GAME( 199?, s_bartsm,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Bart vs. the Space Mutants  [!] (Master System)", 0 )
GAME( 199?, s_boutr,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Battle Out Run  [!] (Master System)", 0 )
GAME( 199?, s_calig,         0,        sms,    sms,    standpal, ROT0,   "Sega", "California Games  [!] (Master System)", 0 )
GAME( 199?, s_calig2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "California Games 2  [!] (Master System)", GAME_NOT_WORKING ) // doesn't boot
GAME( 199?, s_coolsp,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Cool Spot  [!] (Master System)", 0 )
GAME( 199?, s_ddux,          0,        sms,    sms,    standpal, ROT0,   "Sega", "Dynamite Dux  [!] (Master System)", 0 )
GAME( 199?, s_legnil,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Mickey Mouse - Legend of Illusion (Brazil) [!] (Master System)", 0 )
GAME( 199?, s_mspac,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Ms. Pac-man  [!] (Master System)", 0 )
GAME( 199?, s_pmania,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Pac Mania  [!] (Master System)", 0 )
GAME( 199?, s_rtype,         0,        sms,    sms,    standpal, ROT0,   "Sega", "R-Type  [!] (Master System)", 0 )


GAME( 199?, s_sensi,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Sensible Soccer  [!] (Master System)", 0 )
GAME( 199?, s_smgp2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Super Monaco GP 2  [!] (Master System)", 0 )
GAME( 199?, s_supoff,        0,        sms,    sms,    standpal, ROT0,   "Sega", "Super Off-Road  [!] (Master System)", 0 )
GAME( 199?, s_zill,          0,        sms,    sms,    standpal, ROT0,   "Sega", "Zillion  [!] (Master System)", 0 )
GAME( 199?, s_zill2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "Zillion II - The Tri Formation  [!] (Master System)", 0 )







GAME( 199?, gg_exldz,        0,        sms,    gg,    codemagg, ROT0,   "Codemasters", "Excellent Dizzy Collection (Game Gear, SMS2 Mode)", 0 ) // no start button
GAME( 1995, gg_bust,         0,        sms,    gg,    standagg, ROT0,   "Taito", "Bust-A-Move  [!] (Game Gear)", 0 )
GAME( 1993, gg_puzlo,        0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "Puzlow Kids (J) (Game Gear)", 0 )
GAME( 1994, gg_puyo2,        0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "Puyo Puyo 2 (J) [!] (Game Gear)", 0 )

GAME( 1995, gg_tempj,        0,        sms,    gg,    standagg, ROT0,   "Sega", "Tempo Jr. (JUE) [!] (Game Gear)", 0 )
GAME( 1993, gg_tess,         0,        sms,    gg,    standagg, ROT0,   "Gametek", "Tesserae  [!] (Game Gear)", 0 )
GAME( 1993, gg_popil,        0,        sms,    gg,    gg_popil, ROT0,   "Tengen", "Magical Puzzle Popils (J) (Game Gear)", 0 )

GAME( 1993, gg_nazo,         0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "Nazo Puyo (J) [!] (Game Gear)", 0 )
GAME( 1993, gg_nazo2,        0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "Nazo Puyo 2 (J) [!] (Game Gear)", 0 )
GAME( 1993, gg_gear,         0,        sms,    gg,    standagg, ROT0,   "Sony", "Gear Works  [!] (Game Gear)", 0 )
GAME( 1993, gg_bean,         0,        sms,    gg,    standagg, ROT0,   "Sega", "Dr. Robotnik's Mean Bean Machine  [!] (Game Gear)", 0 )
GAME( 1990, gg_cols,         0,        sms,    gg,    standagg, ROT0,   "Sega", "Columns  [!] (Game Gear)", 0 )
GAME( 1996, gg_baku,         0,        sms,    gg,    standagg, ROT0,   "Sega", "Baku Baku Animals  [!] (Game Gear)", 0 )




GAME( 199?, megatech,        0,        sms,    sms,    mtc, ROT0,   "Sega", "(Megatech) (Bios Test)", NOT_A_DRIVER )
GAME( 199?, mt_astro,        megatech,        sms,    sms,    mtc, ROT0,   "Sega", "Astro Warrior (Megatech) (Bios Test)", 0 )


/* Sega System E */

/*******************************************************************************
 Input Ports
********************************************************************************
 mostly unknown for the time being
*******************************************************************************/

	/* The Coinage is similar to Sega System 1 and C2, but
    it seems that Free Play is not used in all games
    (in fact, the only playable game that use it is
    Riddle of Pythagoras) */

#define SEGA_COIN_A \
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )

#define SEGA_COIN_B \
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" ) \
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit, 4/3" ) \
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit, 5/6" ) \
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit, 4/5" ) \
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit, 2/3" ) \
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) ) \
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )

INPUT_PORTS_START( dummy ) /* Used by the Totally Non-Working Games */
INPUT_PORTS_END

INPUT_PORTS_START( transfrm ) /* Used By Transformer */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x00, "1 Player Only" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x20, "10k, 30k, 50k and 70k" )
	PORT_DIPSETTING(    0x30, "20k, 60k, 100k and 140k"  )
	PORT_DIPSETTING(    0x10, "30k, 80k, 130k and 180k" )
	PORT_DIPSETTING(    0x00, "50k, 150k and 250k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( tetrisse ) /* Used By Transformer */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( hangonjr ) /* Used By Hang On Jr */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	SEGA_COIN_B

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Enemies" )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  // These three dips seems to be unused
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN1")	/* Read from Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN2")	/* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START_TAG("IN3")  /* Read from Port 0xf8 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)
INPUT_PORTS_END

INPUT_PORTS_START( ridleofp ) /* Used By Riddle Of Pythagoras */
	PORT_START_TAG("DSW0")	/* Read from Port 0xf2 */
	SEGA_COIN_A
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	SEGA_COIN_B
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_START_TAG("DSW1")	/* Read from Port 0xf3 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "98 (Cheat)")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  // Unknown
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Difficulty?" )	// To be tested ! I don't see what else it could do
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  // Unknown
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x60, "50K 100K 200K 500K 1M 2M 5M 10M" )
	PORT_DIPSETTING(    0x40, "100K 200K 500K 1M 2M 5M 10M" )
	PORT_DIPSETTING(    0x20, "200K 500K 1M 2M 5M 10M" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  // Unknown
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("IN0")	/* Read from Port 0xe0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // Would Be IPT_START2 but the code doesn't use it

	PORT_START_TAG("IN1")	/* Port 0xe1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN2")	/* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 )	/* is this used in the game? */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN3")	/* Read from Port 0xf8 */
	PORT_BIT( 0x0fff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(125) PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END



ROM_START( hangonjr )
	ROM_REGION( 0x30000, REGION_USER1, 0 )
	ROM_LOAD( "rom5.ic7",	0x00000, 0x08000, CRC(d63925a7) SHA1(699f222d9712fa42651c753fe75d7b60e016d3ad) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "rom4.ic5",	0x10000, 0x08000, CRC(ee3caab3) SHA1(f583cf92c579d1ca235e8b300e256ba58a04dc90) )
	ROM_LOAD( "rom3.ic4",	0x18000, 0x08000, CRC(d2ba9bc9) SHA1(85cf2a801883bf69f78134fc4d5075134f47dc03) )
	ROM_LOAD( "rom2.ic3",	0x20000, 0x08000, CRC(e14da070) SHA1(f8781f65be5246a23c1f492905409775bbf82ea8) )
	ROM_LOAD( "rom1.ic2",	0x28000, 0x08000, CRC(3810cbf5) SHA1(c8d5032522c0c903ab3d138f62406a66e14a5c69) )
ROM_END

ROM_START( ridleofp )
	ROM_REGION( 0x30000, REGION_USER1, 0 )
	ROM_LOAD( "epr10426.bin",	0x00000, 0x08000, CRC(4404c7e7) SHA1(555f44786976a009d96a6395c9173929ad6138a7) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr10425.bin",	0x10000, 0x08000, CRC(35964109) SHA1(a7bc64a87b23139b0edb9c3512f47dcf73feb854) )
	ROM_LOAD( "epr10424.bin",	0x18000, 0x08000, CRC(fcda1dfa) SHA1(b8497b04de28fc0d6b7cb0206ad50948cff07840) )
	ROM_LOAD( "epr10423.bin",	0x20000, 0x08000, CRC(0b87244f) SHA1(c88041614735a9b6cba1edde0a11ed413e115361) )
	ROM_LOAD( "epr10422.bin",	0x28000, 0x08000, CRC(14781e56) SHA1(f15d9d89e1ebff36c3867cfc8f0bdf7f6b3c96bc) )
ROM_END

ROM_START( transfrm )
	ROM_REGION( 0x30000, REGION_USER1, 0 )
	ROM_LOAD( "ic7.top",	0x00000, 0x08000, CRC(ccf1d123) SHA1(5ade9b00e2a36d034fafdf1902d47a9a00e96fc4) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr-7347.ic5",	0x10000, 0x08000, CRC(df0f639f) SHA1(a09a9841b66de246a585be63d911b9a42a323503) )
	ROM_LOAD( "epr-7348.ic4",	0x18000, 0x08000, CRC(0f38ea96) SHA1(d4d421c5d93832e2bc1f22f39dffb6b80f2750bd) )
	ROM_LOAD( "ic3.top",		0x20000, 0x08000, CRC(9d485df6) SHA1(b25f04803c8f7188021f3039aa13aac80d480823) )
	ROM_LOAD( "epr-7350.ic2",	0x28000, 0x08000, CRC(0052165d) SHA1(cf4b5dffa54238e513515b3fc90faa7ce0b65d34) )
ROM_END

ROM_START( tetrisse )
	ROM_REGION( 0x30000, REGION_USER1, 0 )
	ROM_LOAD( "epr12213.7",	0x00000, 0x08000, CRC(ef3c7a38) SHA1(cbb91aef330ab1a37d3e21ecf1d008143d0dd7ec) ) /* Fixed Code */

	/* The following are 8 0x4000 banks that get mapped to reads from 0x8000 - 0xbfff */
	ROM_LOAD( "epr12212.5",	0x10000, 0x08000, CRC(28b550bf) SHA1(445922a62e8a7360335c754ad70dabbe0208207b) )
	ROM_LOAD( "epr12211.4",	0x18000, 0x08000, CRC(5aa114e9) SHA1(f9fc7fe4d0444a264185e74d2abc8475f0976534) )
	/* ic3 unpopulated */
	/* ic2 unpopulated */
ROM_END

UINT8* vdp2_vram_bank0;
UINT8* vdp2_vram_bank1;

UINT8* vdp1_vram_bank0;
UINT8* vdp1_vram_bank1;
UINT8 f7_bank_value;

MACHINE_RESET(systeme)
{
	timer_adjust_ptr(vdp1->sms_scanline_timer,  TIME_NOW, 0);
	timer_adjust_ptr(vdp2->sms_scanline_timer,  TIME_NOW, 0);
}

VIDEO_EOF(systeme)
{
	end_of_frame(vdp1);
	end_of_frame(vdp2);
}

VIDEO_UPDATE(systeme)
{
//	show_tiles();
	int x,y;

	for (y=0;y<192;y++)
	{
		UINT16* lineptr = bitmap->line[y];
		UINT16* srcptr =  vdp1->r_bitmap->line[y];

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}

	}

	for (y=0;y<192;y++)
	{
		UINT16* lineptr = bitmap->line[y];
		UINT16* srcptr =  vdp2->r_bitmap->line[y];

		for (x=0;x<256;x++)
		{
			if(!(srcptr[x]&0x8000)) lineptr[x]=srcptr[x]&0x7fff;
		}
	}


	return 0;
}


MACHINE_DRIVER_START( systeme )
	MDRV_CPU_ADD_TAG("z80", Z80, 10738600/2) /* correct for hangonjr, and astroflash/transformer at least  */
	MDRV_CPU_PROGRAM_MAP(sms_readmem,sms_writemem)
	MDRV_CPU_IO_MAP(sms_readport,sms_writeport)

	/* IRQ handled via the timers */

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(0) // Vblank handled manually.
	MDRV_MACHINE_RESET(systeme)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER|VIDEO_RGB_DIRECT)

	MDRV_SCREEN_SIZE(256, 256)
	MDRV_VISIBLE_AREA(0, 255, 0, 223)


	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(sms)
	MDRV_VIDEO_UPDATE(systeme) /* Copies a bitmap */
	MDRV_VIDEO_EOF(systeme) /* Used to Sync the timing */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SN76496, 3579540)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(SN76496, 3579540)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


READ8_HANDLER( sms_vdp_2_data_r )
{
	return vdp_data_r(vdp2);
}

WRITE8_HANDLER( sms_vdp_2_data_w )
{
	vdp_data_w(data, vdp2);
}

READ8_HANDLER( sms_vdp_2_ctrl_r )
{
	return vdp_ctrl_r(vdp2);
}

WRITE8_HANDLER( sms_vdp_2_ctrl_w )
{
	vdp_ctrl_w(data, vdp2);
}

WRITE8_HANDLER( segasyse_videoram_w )
{
	if (f7_bank_value & 0x20)
	{ // to vdp1 vram
		if (f7_bank_value & 0x80)
		{
			vdp1_vram_bank0[offset] = data;
		}
		else
		{
			vdp1_vram_bank1[offset] = data;
		}
	}
	else
	{ // to vdp2 vram
		if (f7_bank_value & 0x40)
		{
			vdp2_vram_bank0[offset] = data;
		}
		else
		{
			vdp2_vram_bank1[offset] = data;
		}
	}

}

WRITE8_HANDLER( systeme_bank_w )
{
	int rombank;
	f7_bank_value = data;

	rombank = data & 0x07;

	if (f7_bank_value&0x80)
	{
		vdp1->vram = vdp1_vram_bank1;
	}
	else
	{
		vdp1->vram = vdp1_vram_bank0;
	}

	if (f7_bank_value&0x40)
	{
		vdp2->vram = vdp2_vram_bank1;
	}
	else
	{
		vdp2->vram = vdp2_vram_bank0;
	}

	memcpy(sms_rom+0x8000, memory_region(REGION_USER1)+0x10000+rombank*0x4000, 0x4000);

}

void sn76496_2_write(UINT8 data)
{
	SN76496_1_w(0, data & 0xff);
}

WRITE8_HANDLER( sms_sn76496_2_w )
{
	sn76496_2_write(data);
}

void init_ports_systeme(void)
{
	/* INIT THE PORTS *********************************************************************************************/

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0x7e, 0x7e, 0, 0, sms_vcounter_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7e, 0x7e, 0, 0, sms_sn76496_w);


	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7b, 0x7b, 0, 0, sms_sn76496_2_w);

	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0x7f, 0x7f, 0, 0, sms_sn76496_w);

	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xba, 0xba, 0, 0, sms_vdp_data_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xba, 0xba, 0, 0, sms_vdp_data_w);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbb, 0xbb, 0, 0, sms_vdp_ctrl_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbb, 0xbb, 0, 0, sms_vdp_ctrl_w);


	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbe, 0xbe, 0, 0, sms_vdp_2_data_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbe, 0xbe, 0, 0, sms_vdp_2_data_w);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xbf, 0xbf, 0, 0, sms_vdp_2_ctrl_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xbf, 0xbf, 0, 0, sms_vdp_2_ctrl_w);


	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xe0, 0xe0, 0, 0, input_port_2_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xe1, 0xe1, 0, 0, input_port_3_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xf2, 0xf2, 0, 0, input_port_0_r);
	memory_install_read8_handler (0, ADDRESS_SPACE_IO, 0xf3, 0xf3, 0, 0, input_port_1_r);


	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xf7, 0xf7, 0, 0, systeme_bank_w );

}



void init_systeme_map(void)
{
	/* INIT THE MEMMAP / BANKING *********************************************************************************/

	/* catch any addresses that don't get mapped */
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xffff, 0, 0, z80_unmapped_w);

	/* fixed rom bank area */
	sms_rom = auto_malloc(0xc000);
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0x0000, 0xbfff, 0, 0, MRA8_BANK1);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x7fff, 0, 0, MWA8_ROM);
	memory_set_bankptr( 1, sms_rom );

	/* alternate way of accessing video ram */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0x8000, 0xbfff, 0, 0, segasyse_videoram_w);


	memcpy(sms_rom, memory_region(REGION_USER1), 0x8000);

	/* main ram area */
	sms_mainram = auto_malloc(0x4000);
	memory_install_read8_handler (0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xffff, 0, 0, MRA8_BANK2);
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xc000, 0xffff, 0, 0, MWA8_BANK2);
	memory_set_bankptr( 2, sms_mainram );
	memset(sms_mainram,0x00,0x4000);

	init_ports_systeme();
}


static DRIVER_INIT( segasyse )
{
	init_systeme_map();

	vdp1 = start_vdp(SMS2_VDP);
//	vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;
	vdp1->chip_id = 1;

	vdp1_vram_bank0 = vdp1->vram;
	vdp1_vram_bank1 = auto_malloc(0x4000);


	vdp2 = start_vdp(SMS2_VDP);
	vdp2->set_irq = sms_vdp_cpu0_irq_callback;
	vdp2->is_pal = 0;
	vdp2->sms_total_scanlines = 262;
	vdp2->sms_framerate = 60;
	vdp2->chip_id = 2;

	vdp2_vram_bank0 = vdp2->vram;
	vdp2_vram_bank1 = auto_malloc(0x4000);
}

/*- Hang On Jr. Specific -*/
static UINT8 port_fa_last;		/* Last thing written to port 0xfa (control related) */

static READ8_HANDLER (segae_hangonjr_port_f8_r)
{
	UINT8 temp;

	temp = 0;

	if (port_fa_last == 0x08)  /* 0000 1000 */ /* Angle */
		temp = readinputport(4);

	if (port_fa_last == 0x09)  /* 0000 1001 */ /* Accel */
		temp = readinputport(5);

	return temp;
}

static WRITE8_HANDLER (segae_hangonjr_port_fa_w)
{
	/* Seems to write the same pattern again and again bits ---- xx-x used */
	port_fa_last = data;
}

/*- Riddle of Pythagoras Specific -*/

static int port_to_read,last1,last2,diff1,diff2;

static READ8_HANDLER (segae_ridleofp_port_f8_r)
{
	switch (port_to_read)
	{
		default:
		case 0:	return diff1 & 0xff;
		case 1:	return diff1 >> 8;
		case 2:	return diff2 & 0xff;
		case 3:	return diff2 >> 8;
	}
}

static WRITE8_HANDLER (segae_ridleofp_port_fa_w)
{
	/* 0x10 is written before reading the dial (hold counters?) */
	/* 0x03 is written after reading the dial (reset counters?) */

	port_to_read = (data & 0x0c) >> 2;

	if (data & 1)
	{
		int curr = readinputport(4);
		diff1 = ((curr - last1) & 0x0fff) | (curr & 0xf000);
		last1 = curr;
	}
	if (data & 2)
	{
		int curr = readinputport(5) & 0x0fff;
		diff2 = ((curr - last2) & 0x0fff) | (curr & 0xf000);
		last2 = curr;
	}
}

static DRIVER_INIT( ridleofp )
{
	init_segasyse();

	memory_install_read8_handler(0, ADDRESS_SPACE_IO, 0xf8, 0xf8, 0, 0, segae_ridleofp_port_f8_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xfa, 0xfa, 0, 0, segae_ridleofp_port_fa_w);
}


static DRIVER_INIT( hangonjr )
{
	init_segasyse();

	memory_install_read8_handler(0, ADDRESS_SPACE_IO, 0xf8, 0xf8, 0, 0, segae_hangonjr_port_f8_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xfa, 0xfa, 0, 0, segae_hangonjr_port_fa_w);
}

GAME( 1985, hangonjr, 0,        systeme, hangonjr, hangonjr, ROT0,  "Sega", "Hang-On Jr. (Arcade)", 0 )
GAME( 1986, transfrm, 0,        systeme, transfrm, segasyse, ROT0,  "Sega", "Transformer (Arcade)", 0 )
GAME( 1986, ridleofp, 0,        systeme, ridleofp, ridleofp, ROT90, "Sega / Nasco", "Riddle of Pythagoras (Japan) (Arcade)", 0 )
GAME( 1988, tetrisse, 0,        systeme, tetrisse, segasyse, ROT0,  "Sega", "Tetris (Japan, System E) (Arcade)", 0 )

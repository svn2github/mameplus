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
	chip->r_bitmap = auto_bitmap_alloc(Machine->screen[0].width,Machine->screen[0].height,Machine->screen[0].format);

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
					palette_set_color(Machine,(chip->addr_reg&0x3e)/2, r<<4, g<<4, b<<4);
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
				palette_set_color(Machine,chip->addr_reg&0x1f, r<<6, g<<6, b<<6);
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
	UINT16* lineptr = BITMAP_ADDR16(chip->r_bitmap, scanline, 0);

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
				UINT16* lineptr = BITMAP_ADDR16(chip->r_bitmap, drawypos, 0);

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
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp1->r_bitmap, y, 0);

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

	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(0) // Vblank handled manually.
	MDRV_MACHINE_RESET(sms)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)

	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 223)

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
	ROM_LOAD( "s_fantdz.sms", 0x000000, 0x040000, CRC(b9664ae1) SHA1(4202ce26832046c7ca8209240f097a8a0a84d981) )
ROM_END

ROM_START( s_dinob )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dinob.sms", 0x000000, 0x040000, CRC(ea5c3a6f) SHA1(05b4f23e33ada08e0a8b1fc6feccd8a97c690a21) )
ROM_END

ROM_START( s_cosmic )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cosmic.sms", 0x000000, 0x040000, CRC(29822980) SHA1(f46f716dd34a1a5013a2d8a59769f6ef7536a567) )
ROM_END

ROM_START( s_micro )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_micro.sms", 0x000000, 0x040000, CRC(a577ce46) SHA1(425621f350d011fd021850238c6de9999625fd69) )
ROM_END


ROM_START( gg_exldz )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_exldz.sms", 0x000000, 0x080000,  CRC(aa140c9c) SHA1(db3c0686715373242911f8471a1e91673811d62a) )
ROM_END

ROM_START( s_landil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_landil.sms", 0x000000, 0x080000, CRC(24e97200) SHA1(a120f29c6bf2e733775d5b984bd3a156682699c6) )
ROM_END

ROM_START( s_tazman )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tazman.sms", 0x000000, 0x040000, CRC(1b312e04) SHA1(274641b3b05245504f763a5e4bc359183d16a092) )
ROM_END

ROM_START( s_bubbob )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bubbob.sms", 0x000000, 0x040000, CRC(e843ba7e) SHA1(44876b44089b4174858e54202e567e02efa76859) )
ROM_END

ROM_START( s_chuck )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chuck.sms", 0x000000, 0x080000, CRC(dd0e2927) SHA1(0199c62afb5c09f09999a4815079875b480129f3) )
ROM_END

ROM_START( s_chuck2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chuck2.sms", 0x000000, 0x080000,  CRC(c30e690a) SHA1(46c326d7eb73b0393de7fc40bf2ee094ebab482d) )
ROM_END


ROM_START( s_adams )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_adams.sms", 0x000000, 0x040000,  CRC(72420f38) SHA1(3fc6ccc556a1e4eb376f77eef8f16b1ff76a17d0) )
ROM_END
ROM_START( s_aburn )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aburn.sms", 0x000000, 0x080000,  CRC(1c951f8e) SHA1(51531df038783c84640a0cab93122e0b59e3b69a) )
ROM_END
ROM_START( s_aladin )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aladin.sms", 0x000000, 0x080000,  CRC(c8718d40) SHA1(585967f400473e289cda611e7686ae98ae43172e) )
ROM_END
ROM_START( s_alexmi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexmi.sms", 0x000000, 0x020000,  CRC(aed9aac4) SHA1(6d052e0cca3f2712434efd856f733c03011be41c) )
ROM_END
ROM_START( s_alsynd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alsynd.sms", 0x000000, 0x040000,  CRC(5cbfe997) SHA1(0da0b9755b6a6ef145ec3b95e651d2a384b3f7f9) )
ROM_END
ROM_START( s_alstor )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alstor.sms", 0x000000, 0x040000,  CRC(7f30f793) SHA1(aece64ecbfbe08b199b29df9bc75743773ea3d34) )
ROM_END
ROM_START( s_actfgh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_actfgh.sms", 0x000000, 0x020000,  CRC(3658f3e0) SHA1(b462246fed3cbb9dc3909a2d5befaec65d7a0014) )
ROM_END
ROM_START( s_column )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_column.sms", 0x000000, 0x020000,  CRC(665fda92) SHA1(3d16b0954b5419b071de270b44d38fc6570a8439) )
ROM_END
ROM_START( s_bean )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bean.sms", 0x000000, 0x040000,  CRC(6c696221) SHA1(89df035da8de68517f82bdf176d3b3f2edcd9e31) )
ROM_END
ROM_START( s_fzone )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fzone.sms", 0x000000, 0x020000,  CRC(65d7e4e0) SHA1(0278cd120dc3a7707eda9314c46c7f27f9e8fdda) )
ROM_END
ROM_START( s_fzone2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fzone2.sms", 0x000000, 0x040000,  CRC(b8b141f9) SHA1(b84831378c7c19798b6fd560647e2941842bb80a) )
ROM_END
ROM_START( s_fzone3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fzone3.sms", 0x000000, 0x020000,  CRC(d29889ad) SHA1(a2cd356826c8116178394d8aba444cb636ef784e) )
ROM_END
ROM_START( s_flint )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_flint.sms", 0x000000, 0x040000,  CRC(ca5c78a5) SHA1(a0aa76d89f6831999c328877057a99e72c6b533b) )
ROM_END


ROM_START( s_wb3dt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wb3dt.sms", 0x000000, 0x040000,  CRC(679e1676) SHA1(99e73de2ffe5ea5d40998faec16504c226f4c1ba) )
ROM_END
ROM_START( s_woody )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_woody.sms", 0x000000, 0x008000,  CRC(315917d4) SHA1(b74078c4a3e6d20d21ca81e88c0cb3381b0c84a4) )
ROM_END
ROM_START( s_zool )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zool.sms", 0x000000, 0x040000,  CRC(9d9d0a5f) SHA1(aed98f2fc885c9a6e121982108f843388eb46304) )
ROM_END
ROM_START( s_smgpa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smgpa.sms", 0x000000, 0x040000,  CRC(55bf81a0) SHA1(0f11b9d7d6d16b09f7be0dace3b6c7d3524da725) )
ROM_END
ROM_START( s_sor )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sor.sms", 0x000000, 0x080000,  CRC(6f2cf48a) SHA1(ea4214198a53dbcc0e4df5f1c34530bff5bea1f5) )
ROM_END
ROM_START( s_lucky )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lucky.sms", 0x000000, 0x040000,  CRC(a1710f13) SHA1(a08815d27e431f0bee70b4ebfb870a3196c2a732) )
ROM_END
ROM_START( s_lionk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lionk.sms", 0x000000, 0x080000,   CRC(c352c7eb) SHA1(3a64657e3523a5da1b99db9d89b1a48ed4ccc5ed) )
ROM_END
ROM_START( s_lemm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lemm.sms", 0x000000, 0x040000,  CRC(f369b2d8) SHA1(f3a853cce1249a0848bfc0344f3ee2db6efa4c01) )
ROM_END
ROM_START( s_jp2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_jp2.sms", 0x000000, 0x080000,   CRC(102d5fea) SHA1(df4f55f7ff9236f65aee737743e7500c4d96cf12) )
ROM_END
ROM_START( s_gpride )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gpride.sms", 0x000000, 0x080000,  CRC(ec2da554) SHA1(3083069782c7cfcf2cc1229aca38f8f2971cf284) )
ROM_END
ROM_START( s_jungbk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_jungbk.sms", 0x000000, 0x040000,  CRC(695a9a15) SHA1(f65b5957b4b21bd16b4aa8a6e93fed944dd7d9ac) )
ROM_END

ROM_START( s_gaunt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gaunt.sms", 0x000000, 0x020000,  CRC(d9190956) SHA1(4e583ce9b95e20ecddc6c1dac9941c28a3e80808) )
ROM_END
ROM_START( s_gng )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gng.sms", 0x000000, 0x040000, CRC(7a92eba6) SHA1(b193e624795b2beb741249981d621cb650c658db) )
ROM_END


ROM_START( s_castil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_castil.sms", 0x000000, 0x040000, CRC(953f42e1) SHA1(c200b5e585d59f8bfcbb40fd6d4314de8abcfae3) )
ROM_END
ROM_START( s_sonic )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sonic.sms", 0x000000, 0x040000, CRC(b519e833) SHA1(6b9677e4a9abb37765d6db4658f4324251807e07) )
ROM_END
ROM_START( s_sonic2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sonic2.sms", 0x000000, 0x080000,  CRC(5b3b922c) SHA1(acdb0b5e8bf9c1e9c9d1a8ac6d282cb8017d091c) )
ROM_END
ROM_START( s_spyspy )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spyspy.sms", 0x000000, 0x008000, CRC(78d7faab) SHA1(feab16dd8b807b88395e91c67e9ff52f8f7aa7e4) )
ROM_END
ROM_START( s_suptet )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_suptet.sms", 0x000000, 0x010000, CRC(bd1cc7df) SHA1(3772e9dc07b15de62326ee78981ec0b6f4387590) )
ROM_END
ROM_START( s_supko )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supko.sms", 0x000000, 0x040000, CRC(406aa0c2) SHA1(4116e4a742e3209ca5db9887cd92219d15cf3c9a) )
ROM_END
ROM_START( s_strid )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_strid.sms", 0x000000, 0x080000, CRC(9802ed31) SHA1(051e72c8ffec7606c04409ef38244cfdd592252f) )
ROM_END
ROM_START( s_ssi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ssi.sms", 0x000000, 0x040000, CRC(1d6244ee) SHA1(b28d2a9c0fe597892e21fb2611798765f5435885) )
ROM_END
ROM_START( s_rrsh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rrsh.sms", 0x000000, 0x080000, CRC(b876fc74) SHA1(7976e717125757b1900a540a68e0ef3083839f85) )
ROM_END
ROM_START( s_psycho )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_psycho.sms", 0x000000, 0x040000, CRC(97993479) SHA1(278cc3853905626138e83b6cfa39c26ba8e4f632) )
ROM_END
ROM_START( s_tnzs )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tnzs.sms", 0x000000, 0x040000, CRC(c660ff34) SHA1(e433b21b505ed5428d1b2f07255e49c6db2edc6c) )
ROM_END


ROM_START( s_20em1 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_20em1.sms", 0x000000, 0x040000, CRC(f0f35c22) SHA1(2012763dc08dedc68af8538698bd66618212785d) )
ROM_END
ROM_START( s_aceace )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aceace.sms", 0x000000, 0x040000, CRC(887d9f6b) SHA1(08a79905767b8e5af8a9c9c232342e6c47588093) )
ROM_END
ROM_START( s_actfgj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_actfgj.sms", 0x000000, 0x020000, CRC(d91b340d) SHA1(5dbcfb75958f4cfa1b61d9ea114bab67787b113e) )
ROM_END
ROM_START( s_aerial )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aerial.sms", 0x000000, 0x040000, CRC(ecf491cf) SHA1(2a9090ed365e7425ca7a59f87b942c16b376f0a3) )
ROM_END
ROM_START( s_airesc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_airesc.sms", 0x000000, 0x040000, CRC(8b43d21d) SHA1(4edd1b62abbbf2220961a06eb139db1838fb700b) )
ROM_END
ROM_START( s_aleste )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aleste.sms", 0x000000, 0x020000, CRC(d8c4165b) SHA1(cba75b0d54df3c9a8e399851a05c194c7a05fbfe) )
ROM_END
ROM_START( s_alexls )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexls.sms", 0x000000, 0x040000, CRC(c13896d5) SHA1(6dbf2684c3dfea7442d0b40a9ff7c8b8fc9b1b98) )
ROM_END
ROM_START( s_alexbm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexbm.sms", 0x000000, 0x020000, CRC(f9dbb533) SHA1(77cc767bfae01e9cc81612c780c939ed954a6312) )
ROM_END
ROM_START( s_alexht )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexht.sms", 0x000000, 0x020000,  CRC(013c0a94) SHA1(2d0a581da1c787b1407fb1cfefd0571e37899978) )
ROM_END
ROM_START( s_alf )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alf.sms", 0x000000, 0x020000, CRC(82038ad4) SHA1(7706485b735f5d7f7a59c7d626b13b23e8696087) )
ROM_END


ROM_START( s_alien3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alien3.sms", 0x000000, 0x040000, CRC(b618b144) SHA1(be40ffc72ee19620a8bac89d5d96bbafcefc74e7) )
ROM_END
ROM_START( s_altbea )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_altbea.sms", 0x000000, 0x040000, CRC(bba2fe98) SHA1(413986f174799f094a8dd776d91dcf018ee17290) )
ROM_END
ROM_START( s_ash )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ash.sms", 0x000000, 0x040000, CRC(e4163163) SHA1(44ed3aeaa4c8a627b88c099b184ca99710fac0ad) )
ROM_END
ROM_START( s_astrx )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astrx.sms", 0x000000, 0x080000, CRC(147e02fa) SHA1(70e311421467acd01e55f1908eae653bf20de175) )
ROM_END
ROM_START( s_astrxa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astrxa.sms", 0x000000, 0x080000, CRC(8c9d5be8) SHA1(ae56c708bc197f462b68c3e5ff9f0379d841c8b0) )
ROM_END
ROM_START( s_astgr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astgr.sms", 0x000000, 0x080000, CRC(f9b7d26b) SHA1(5e409247f6a611437380b7a9f0e3cccab5dd1987) )
ROM_END
ROM_START( s_astsm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astsm.sms", 0x000000, 0x080000, CRC(def9b14e) SHA1(de6a32a548551553a4b3ae332f4bf98ed22d8ab5) )
ROM_END
ROM_START( s_bttf2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bttf2.sms", 0x000000, 0x040000, CRC(e5ff50d8) SHA1(31af58e655e12728b01e7da64b46934979b82ecf) )
ROM_END
ROM_START( s_bttf3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bttf3.sms", 0x000000, 0x040000,  CRC(2d48c1d3) SHA1(7d67dd38fea5dba4224a119cc4840f6fb8e023b9) )
ROM_END
ROM_START( s_baku )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_baku.sms", 0x000000, 0x040000, CRC(35d84dc2) SHA1(07d8f300b3a3542734fcd24fa8312fe99fbfef0e) )
ROM_END

ROM_START( s_bartsm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bartsm.sms", 0x000000, 0x040000, CRC(d1cc08ee) SHA1(4fa839db6de21fd589f6a91791fff25ca2ab88f4) )
ROM_END
ROM_START( s_boutr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_boutr.sms", 0x000000, 0x040000, CRC(c19430ce) SHA1(cfa8721d4fc71b1f14e9a06f2db715a6f88be7dd) )
ROM_END
ROM_START( s_calig )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_calig.sms", 0x000000, 0x040000,  CRC(ac6009a7) SHA1(d0f8298bb2a30a3569c65372a959612df3b608db) )
ROM_END
ROM_START( s_calig2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_calig2.sms", 0x000000, 0x040000, CRC(c0e25d62) SHA1(5c7d99ba54caf9a674328df787a89e0ab4730de8) )
ROM_END
ROM_START( s_coolsp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_coolsp.sms", 0x000000, 0x040000, CRC(13ac9023) SHA1(cf36c1900d1c658cbfd974464761d145af3467c8) )
ROM_END
ROM_START( s_ddux )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ddux.sms", 0x000000, 0x040000, CRC(0e1cc1e0) SHA1(2a513aef0f0bdcdf4aaa71e7b26a15ce686db765) )
ROM_END
ROM_START( s_legnil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_legnil.sms", 0x000000, 0x080000, CRC(6350e649) SHA1(62adbd8e5a41d08c4745e9fbb1c51f0091c9dea6) )
ROM_END
ROM_START( s_mspac )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mspac.sms", 0x000000, 0x020000, CRC(3cd816c6) SHA1(4dd7cb303793792edd9a4c717a727f46db857dae) )
ROM_END
ROM_START( s_pmania )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pmania.sms", 0x000000, 0x020000, CRC(be57a9a5) SHA1(c0a11248bbb556b643accd3c76737be35cbada54) )
ROM_END
ROM_START( s_rtype )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rtype.sms", 0x000000, 0x080000, CRC(bb54b6b0) SHA1(08ec70a2cd98fcb2645f59857f845d41b0045115) )
ROM_END

ROM_START( s_sensi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sensi.sms", 0x000000, 0x020000, CRC(f8176918) SHA1(08b4f5b8096bc811bc9a7750deb21def67711a9f) )
ROM_END
ROM_START( s_smgp2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smgp2.sms", 0x000000, 0x080000, CRC(e890331d) SHA1(b6819b014168aaa03b65dae97ba6cd5fa0d7f0d9) )
ROM_END
ROM_START( s_supoff )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supoff.sms", 0x000000, 0x020000, CRC(54f68c2a) SHA1(804cd74bbcf452613f6c3a722be1c94338499813) )
ROM_END
ROM_START( s_zill )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zill.sms", 0x000000, 0x020000,  CRC(5718762c) SHA1(3aab3e95dac0fa93612da20cf525dba4dc4ca6ba) )
ROM_END
ROM_START( s_zill2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zill2.sms", 0x000000, 0x020000, CRC(2f2e3bc9) SHA1(1e08be828ecf4cf5db787704ab8779f4b5a444b5) )
ROM_END

/* No-Intro */

ROM_START( s_alexmj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexmj.sms", 0x000000, 0x020000, CRC(08c9ec91) SHA1(62fdc25501e17b87c355378562c3b1966e5f9008) )
ROM_END
ROM_START( s_alexm1 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexm1.sms", 0x000000, 0x020000, CRC(17a40e29) SHA1(8cecf8ed0f765163b2657be1b0a3ce2a9cb767f4) )
ROM_END
ROM_START( s_alexsh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alexsh.sms", 0x000000, 0x040000, CRC(d2417ed7) SHA1(e7c7c24e208afb986ab389883f98a1b5a8249fea) )
ROM_END
ROM_START( s_alsynj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_alsynj.sms", 0x000000, 0x040000, CRC(4cc11df9) SHA1(5d786476b275de34efb95f576dd556cf4b335a83) )
ROM_END
ROM_START( s_ambase )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ambase.sms", 0x000000, 0x040000, CRC(7b27202f) SHA1(fff4006fe47de8138db246af8863e28b81718abe) )
ROM_END
ROM_START( s_amprof )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_amprof.sms", 0x000000, 0x040000, CRC(3727d8b2) SHA1(986b860465fc9748c6be1815c0e4c0ea94473040) )
ROM_END
ROM_START( s_anci )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_anci.sms", 0x000000, 0x040000,  CRC(b33e2827) SHA1(e73e836c353543e9f48315410b0d72278899ff59) )
ROM_END
ROM_START( s_ancij )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ancij.sms", 0x000000, 0x040000, CRC(32759751) SHA1(614b589080b732e17cc0d253e17216a72a268955) )
ROM_END
ROM_START( s_aate )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aate.sms", 0x000000, 0x040000, CRC(f499034d) SHA1(81dbacad4739b98281c750d9af21606275398fc8) )
ROM_END
ROM_START( s_anmit )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_anmit.sms", 0x000000, 0x020000, CRC(fff63b0b) SHA1(74057d3b7f384c91871a2db2fbc86d3e700c45e5) )
ROM_END


ROM_START( s_argos )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_argos.sms", 0x000000, 0x020000, CRC(bae75805) SHA1(a4b63ed417380f8170091e66c0417123799a731f) )
ROM_END
ROM_START( s_ariel )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ariel.sms", 0x000000, 0x040000, CRC(f4b3a7bd) SHA1(77a35c0b622786183d6703a5d7546728db44b68d) )
ROM_END
ROM_START( s_ashura )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ashura.sms", 0x000000, 0x020000, CRC(ae705699) SHA1(e9f90d63320295e4bd9a87e6078186c5efb7e84e) )
ROM_END
ROM_START( s_assaul )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_assaul.sms", 0x000000, 0x040000, CRC(0bd8da96) SHA1(ea46350ce4827b282b73600a7f4feadbec7c0ed4) )
ROM_END
ROM_START( s_assau1 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_assau1.sms", 0x000000, 0x040000, CRC(861b6e79) SHA1(835217550ecb92422d887a3353ff43890c71566b) )
ROM_END
ROM_START( s_astrof )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astrof.sms", 0x000000, 0x008000, CRC(c795182d) SHA1(8370957b1192349d0a610437cd5d91ea4e3892c4) )
ROM_END
ROM_START( s_awpp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_awpp.sms", 0x000000, 0x020000, CRC(69efd483) SHA1(2b3a9da256f2918b859ebcb6ffa1b36a09e7595d) )
ROM_END
ROM_START( s_astrow )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_astrow.sms", 0x000000, 0x020000, CRC(299cbb74) SHA1(901697a3535ad70190647f34ad5b30b695d54542) )
ROM_END
ROM_START( s_aztec )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_aztec.sms", 0x000000, 0x020000, CRC(ff614eb3) SHA1(317775a17867530a8fe3a5b17b681d5ada351432) )
ROM_END
ROM_START( s_bankpa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bankpa.sms", 0x000000, 0x008000, CRC(655fb1f4) SHA1(661bbe20f01b7afb242936d409fdd30420c6de5f) )
ROM_END

ROM_START( s_bballn )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bballn.sms", 0x000000, 0x040000, CRC(0df8597f) SHA1(0fa1156931c83763bc6906efce75045327cdd7aa) )
ROM_END
ROM_START( s_batmr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_batmr.sms", 0x000000, 0x040000, CRC(b154ec38) SHA1(0ccc0e2d91a345c39a7406e148da147a2edce979) )
ROM_END
ROM_START( s_batlmn )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_batlmn.sms", 0x000000, 0x040000, CRC(1cbb7bf1) SHA1(0854e36d3bb011e712a06633f188c0d64cd65893) )
ROM_END
ROM_START( s_bbelt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bbelt.sms", 0x000000, 0x020000, CRC(da3a2f57) SHA1(7c5524cff2de9b694e925297e8e74c7c8d292e46) )
ROM_END
ROM_START( s_bladee )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bladee.sms", 0x000000, 0x040000, CRC(8ecd201c) SHA1(b786d15b26b914c24cd1c36a8fca41b215c0a4e7) )
ROM_END
ROM_START( s_bomber )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bomber.sms", 0x000000, 0x040000, CRC(3084cf11) SHA1(d754ef2b6e05c76502c02c71dbfcf6150ee12f6f) )
ROM_END
ROM_START( s_bnzabr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bnzabr.sms", 0x000000, 0x040000, CRC(caea8002) SHA1(bbaedefa0bb489ece4bbd965f09a417be4b76cc7) )
ROM_END
ROM_START( s_bonker )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_bonker.sms", 0x000000, 0x080000, CRC(b3768a7a) SHA1(e1f8da3897f0756c8764ece6605f940ce79e81ca) )
ROM_END
ROM_START( s_drac )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_drac.sms", 0x000000, 0x040000, CRC(1b10a951) SHA1(6c9f52cdae96541020eaaa543ca6f729763f3ada) )
ROM_END
ROM_START( s_buggy )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_buggy.sms", 0x000000, 0x080000, CRC(b0fc4577) SHA1(4b1975190ac9d6281325de0925980283fdce51ca) )
ROM_END

ROM_START( s_capsil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_capsil.sms", 0x000000, 0x040000, CRC(a4852757) SHA1(88402392e93b220632a61e0c07731a7ed087cbef) )
ROM_END
ROM_START( s_capsiu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_capsiu.sms", 0x000000, 0x020000, CRC(b81f6fa5) SHA1(7c2b23f4a806c89a533f27e190499243e7311c47) )
ROM_END
ROM_START( s_casino )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_casino.sms", 0x000000, 0x040000, CRC(3cff6e80) SHA1(8353b86965a87c724b95bb768d00dc84eeadce96) )
ROM_END
ROM_START( s_castel )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_castel.sms", 0x000000, 0x080000, CRC(31ffd7c3) SHA1(4e40155720957a0ca7cf67d7c99bbc178e2f0fd4) )
ROM_END
ROM_START( s_cillu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cillu.sms", 0x000000, 0x040000, CRC(b9db4282) SHA1(c31d80429801e8c927cb0536d66a16d51788ff4f) )
ROM_END
ROM_START( s_champe )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_champe.sms", 0x000000, 0x040000, CRC(23163a12) SHA1(e1d0b1b25b7d9bb423dadfe792bd177e01bc2ca2) )
ROM_END
ROM_START( s_champh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_champh.sms", 0x000000, 0x040000, CRC(7e5839a0) SHA1(d11eefe122de42a73d221d9efde1086d4a8ce147) )
ROM_END
ROM_START( s_chapol )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chapol.sms", 0x000000, 0x020000, CRC(492c7c6e) SHA1(6590080f5db87afab1286f11ce77c60c3167b2b7) )
ROM_END
ROM_START( s_cheese )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cheese.sms", 0x000000, 0x080000, CRC(46340c41) SHA1(09edc943fc6da8657231b09f75d5e5c6bbbac24d) )
ROM_END
ROM_START( s_chopjp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chopjp.sms", 0x000000, 0x020000,  CRC(16ec3575) SHA1(c8e87b309bbae6af7cf05602ffbd28f9495c83d8) )
ROM_END


ROM_START( s_chopl )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chopl.sms", 0x000000, 0x020000,  CRC(4a21c15f) SHA1(856e741eec9692fcc3b22c5c5642f54482e6e00b) )
ROM_END
ROM_START( s_chouon )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chouon.sms", 0x000000, 0x020000, CRC(e421e466) SHA1(9f987e022090a40506b78d89523e9f88b3bb0c0b) )
ROM_END
ROM_START( s_chck2b )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chck2b.sms", 0x000000, 0x080000, CRC(87783c04) SHA1(b966120d9eacea683bc136c405c50a81763ecab8) )
ROM_END
ROM_START( s_circui )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_circui.sms", 0x000000, 0x020000, CRC(8fb75994) SHA1(34307a745d3d52a4b814e9831b7041f25e8052d1) )
ROM_END
ROM_START( s_cloudm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cloudm.sms", 0x000000, 0x040000, CRC(e7f62e6d) SHA1(b936276b272d8361bca8d7b05d1ebc59f1f639bc) )
ROM_END
ROM_START( s_cmj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cmj.sms", 0x000000, 0x008000, CRC(9d549e08) SHA1(33c21d164fd3cdf7aa9e7e0fe1a3ae5a491bd9f5) )
ROM_END
ROM_START( s_cybers )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cybers.sms", 0x000000, 0x040000, CRC(1350e4f8) SHA1(991803feb42a6f0f93ac0e97854132736def2933) )
ROM_END
ROM_START( s_cyborg )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_cyborg.sms", 0x000000, 0x020000,  CRC(908e7524) SHA1(b6131585cb944d7fae69ad609802a1b5d51b442f) )
ROM_END
ROM_START( s_daffy )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_daffy.sms", 0x000000, 0x080000, CRC(71abef27) SHA1(50ab645971b7d9c25a0a93080f70f3a9c6910c59) )
ROM_END

ROM_START( s_danan )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_danan.sms", 0x000000, 0x040000, CRC(ae4a28d7) SHA1(c99f2562117a2bf7100a3992608e9a2bcb50df35) )
ROM_END
ROM_START( s_deadan )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_deadan.sms", 0x000000, 0x040000, CRC(e2f7193e) SHA1(0236c5239c924b425a99367260b9ebfa8b8e0bca) )
ROM_END
ROM_START( s_deepd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_deepd.sms", 0x000000, 0x080000, CRC(42fc3a6e) SHA1(26ec82b96650a7329b66bf90b54b869c1ec12f6b) )
ROM_END
ROM_START( s_dspeed )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dspeed.sms", 0x000000, 0x040000, CRC(b137007a) SHA1(60e2b6ec69d73dd73c1ef846634942c81800655b) )
ROM_END
ROM_START( s_dstrik )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dstrik.sms", 0x000000, 0x080000, CRC(6c1433f9) SHA1(e6181baef80ecc88b3eb82a46cf93793f06e01f1) )
ROM_END
ROM_START( s_dicktr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dicktr.sms", 0x000000, 0x040000, CRC(f6fab48d) SHA1(b788f0394aafbc213e6fa6dcfae40ebb7659f533) )
ROM_END
ROM_START( s_plandj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_plandj.sms", 0x000000, 0x020000, CRC(2bcdb8fa) SHA1(c01cf44eee335d509dc20a165add8514e7fbb7c4) )
ROM_END
ROM_START( s_ddr )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ddr.sms", 0x000000, 0x040000, CRC(a55d89f3) SHA1(cad5532af94ed75c0ada8820a83fa04d22f7bef5) )
ROM_END
ROM_START( s_dhawk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dhawk.sms", 0x000000, 0x040000, CRC(8370f6cd) SHA1(d2428baf22da8a70a08ff35389d59030ce764372) )
ROM_END
ROM_START( s_dbltar )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dbltar.sms", 0x000000, 0x020000, CRC(52b83072) SHA1(51f9ce05e383983ce1fe930ec178406b277db69c) )
ROM_END

ROM_START( s_blee )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_blee.sms", 0x000000, 0x040000, CRC(c88a5064) SHA1(3d41a4e3b9ffc3e2ba87bd89baf13271f8560775) )
ROM_END
ROM_START( s_dcrys )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dcrys.sms", 0x000000, 0x020000, CRC(9549fce4) SHA1(021c6983fdab4b0215ca324734deef0d32c29562) )
ROM_END
ROM_START( s_dduke )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dduke.sms", 0x000000, 0x040000, CRC(07306947) SHA1(258901a74176fc78f9c669cd7d716da0c872ca67) )
ROM_END
ROM_START( s_dhead )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dhead.sms", 0x000000, 0x080000, CRC(7db5b0fa) SHA1(12877c1c9bfba6462606717cf0a94f700ac970e4) )
ROM_END
ROM_START( s_ejim )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ejim.sms", 0x000000, 0x080000, CRC(c4d5efc5) SHA1(a1966c2d8e75ea17df461a46c4a1a8b0b5fecd4e) )
ROM_END
ROM_START( s_ecco2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ecco2.sms", 0x000000, 0x080000,  CRC(7c28703a) SHA1(61cef405e5bc71f1a603881c025bc245a8d14be4) )
ROM_END
ROM_START( s_ecco )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ecco.sms", 0x000000, 0x080000, CRC(6687fab9) SHA1(3bfdef48f27f2e53e2c31cb9626606a1541889dd) )
ROM_END
ROM_START( s_enduro )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_enduro.sms", 0x000000, 0x020000, CRC(00e73541) SHA1(10dc132166c1aa47ca7b89fbb1f0a4e56b428359) )
ROM_END
ROM_START( s_endurj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_endurj.sms", 0x000000, 0x040000,  CRC(5d5c50b3) SHA1(7a0216af8d4a5aeda1d42e2703f140d08b3f92f6) )
ROM_END
ROM_START( s_eswat )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_eswat.sms", 0x000000, 0x040000, CRC(c4bb1676) SHA1(075297d2f3a8ec4c399eaeab6b60e246e11b41fe) )
ROM_END
ROM_START( s_eswat1 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_eswat1.sms", 0x000000, 0x040000,  CRC(c10fce39) SHA1(c481b4e5ca136fbb4105ae465259125392faffd3) )
ROM_END
ROM_START( s_exdzp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_exdzp.sms", 0x000000, 0x040000, CRC(8813514b) SHA1(09a2acf3ed90101be2629384c5c702f6a5408035) )
ROM_END

ROM_START( s_f1 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_f1.sms", 0x000000, 0x040000, CRC(ec788661) SHA1(247fd74485073695a88f6813482f67516860b3a0) )
ROM_END
ROM_START( s_f16 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_f16.sms", 0x000000, 0x008000, CRC(eaebf323) SHA1(de63ca2f59adb94fac87623fe68de75940397449) )
ROM_END
ROM_START( s_f16fj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_f16fj.sms", 0x000000, 0x008000, CRC(7ce06fce) SHA1(0f60f545ce99366860a94fbb115ce7d8531ab7ba) )
ROM_END
ROM_START( s_f16fu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_f16fu.sms", 0x000000, 0x008000, CRC(184c23b7) SHA1(a8f083b3db721b672b5b023d673a64577cb48ef3) )
ROM_END
ROM_START( s_fzon2j )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fzon2j.sms", 0x000000, 0x040000, CRC(c722fb42) SHA1(60d8135e8f15fe48f504d0ec69010a4b886dcda8) )
ROM_END
ROM_START( s_ferias )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ferias.sms", 0x000000, 0x080000, CRC(bf6c2e37) SHA1(81dd4d7f1376e639cabebecdc821e9b5a635952b) )
ROM_END
ROM_START( s_fifa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fifa.sms", 0x000000, 0x080000, CRC(9bb3b5f9) SHA1(360073cb28e87a05b7f3a5922a24601981330046) )
ROM_END
ROM_START( s_fbubbo )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fbubbo.sms", 0x000000, 0x040000, CRC(3ebb7457) SHA1(ab585612fddb90b5285e2804f63fd7fb7ba02900) )
ROM_END
ROM_START( s_fire )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fire.sms", 0x000000, 0x040000, CRC(f6ad7b1d) SHA1(f934b35d27330cc2737d6a2d590dcef56004b983) )
ROM_END
ROM_START( s_fireic )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_fireic.sms", 0x000000, 0x040000, CRC(8b24a640) SHA1(0e12ce919cda400b8681e18bdad31ba74f07a92b) )
ROM_END
ROM_START( s_flash )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_flash.sms", 0x000000, 0x040000, CRC(be31d63f) SHA1(1732b4c13fd00dd5efc5bf1ccb1ab6ed3889c8ba) )
ROM_END

ROM_START( s_forgot )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_forgot.sms", 0x000000, 0x040000, CRC(38c53916) SHA1(3f034429b23b6976c961595c1bcbd68826cb760d) )
ROM_END
ROM_START( s_pitpoj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pitpoj.sms", 0x000000, 0x008000, CRC(e6795c53) SHA1(b1afa682b2f70bfc4ab2020d7c3047aabbaf9a24) )
ROM_END
ROM_START( s_gain )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gain.sms", 0x000000, 0x040000, CRC(3ec5e627) SHA1(62c0ca61ad8f679f90f253ab6bbffd0c7737a8c0) )
ROM_END
ROM_START( s_galac )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_galac.sms", 0x000000, 0x020000, CRC(a6fa42d0) SHA1(e8bcc3621e30ee445a74e6ddbe0842d0a6753f36) )
ROM_END
ROM_START( s_gforc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gforc.sms", 0x000000, 0x080000, CRC(a4ac35d8) SHA1(54fe2f686fb9ec3e34b41d58778118b11f920440) )
ROM_END
ROM_START( s_gforcu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gforcu.sms", 0x000000, 0x080000, CRC(6c827520) SHA1(874289a1e8110312db48c5111fbf8e70b2426b5f) )
ROM_END
ROM_START( s_gbox )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gbox.sms", 0x000000, 0x040000, CRC(1890f407) SHA1(c0ee197e93c9ec81f5b788e8d6b20b3ab57d2259) )
ROM_END
ROM_START( s_gangst )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gangst.sms", 0x000000, 0x020000, CRC(5fc74d2a) SHA1(57f0972109cb6f9a74c65d70d6497bc02fdfc942) )
ROM_END
ROM_START( s_gfko )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gfko.sms", 0x000000, 0x040000, CRC(a64898ce) SHA1(d0ddb71c6ca823c53d7e927a0de7de4b56745331) )
ROM_END
ROM_START( s_gerald )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gerald.sms", 0x000000, 0x008000, CRC(956c416b) SHA1(d82145b582e21ae5cb562030b5042ec85d440add) )
ROM_END

ROM_START( s_ghous )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ghous.sms", 0x000000, 0x008000, CRC(f1f8ff2d) SHA1(cbce4c5d819be524f874ec9b60ca9442047a6681) )
ROM_END
ROM_START( s_ghousj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ghousj.sms", 0x000000, 0x008000,  CRC(c0f3ce7e) SHA1(051e74c499c6792f891668a7be23a11c2c4087af) )
ROM_END
ROM_START( s_gbus )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gbus.sms", 0x000000, 0x020000, CRC(1ddc3059) SHA1(8945a9dfc99a2081a6fb74bbabf8feaac83a7e1a) )
ROM_END
ROM_START( s_gdefp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gdefp.sms", 0x000000, 0x020000, CRC(91a0fc4e) SHA1(f92e1a9f7499b344e7865b18c042e09e7d614796) )
ROM_END
ROM_START( s_gdef )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gdef.sms", 0x000000, 0x020000, CRC(b746a6f5) SHA1(189ee1d4250a1f33e97053aa804a97b4e1467728) )
ROM_END
ROM_START( s_gloc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gloc.sms", 0x000000, 0x040000, CRC(05cdc24e) SHA1(144bd2f8f6e480829c50f71baa370a838e8cec41) )
ROM_END
ROM_START( s_dumpm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_dumpm.sms", 0x000000, 0x020000, CRC(a249fa9d) SHA1(77f1e788f43fb59456f982472f02f109f53c7918) )
ROM_END
ROM_START( s_gaxe )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gaxe.sms", 0x000000, 0x080000, CRC(c08132fb) SHA1(d92538cb16a7a456255fa0da2bd8d0f588cd12ab) )
ROM_END
ROM_START( s_gaxew )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gaxew.sms", 0x000000, 0x040000, CRC(c7ded988) SHA1(fbda0486b393708a89756bb57d116ad6007484e4) )
ROM_END
ROM_START( s_golfa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_golfa.sms", 0x000000, 0x040000, CRC(48651325) SHA1(d0b964dd7cd8ccdd730de4d8e4bb2e87bea7686e) )
ROM_END

ROM_START( s_golfap )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_golfap.sms", 0x000000, 0x040000, CRC(5dabfdc3) SHA1(da88dc3e84daa2f8b8d803b00a13b5fb3185d8c5) )
ROM_END
ROM_START( s_golvej )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_golvej.sms", 0x000000, 0x040000, CRC(bf0411ad) SHA1(f62c5c9dea4368e6475517c4220a62e40fedd35d) )
ROM_END
ROM_START( s_golve )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_golve.sms", 0x000000, 0x040000, CRC(a51376fe) SHA1(cb8c2de9a8e91c0e4e60e5d4d9958e671d84da4c) )
ROM_END
ROM_START( s_gbasej )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gbasej.sms", 0x000000, 0x008000, CRC(89e98a7c) SHA1(e6eaaec61bec32dee5161ae59a7a0902f0d05dc9) )
ROM_END
ROM_START( s_gbase )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gbase.sms", 0x000000, 0x020000, CRC(10ed6b57) SHA1(b332344eb529bad29dfb582633e787f7e42f71ae) )
ROM_END
ROM_START( s_gbask )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gbask.sms", 0x000000, 0x020000, CRC(2ac001eb) SHA1(2fdb7ebce61e316ded27b575535d4f475c9dd822) )
ROM_END
ROM_START( s_gfoot )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gfoot.sms", 0x000000, 0x020000, CRC(2055825f) SHA1(a768f44ce7e50083ffe8c4b5e3ac93ceb7bd3266) )
ROM_END
ROM_START( s_ggolf )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ggolf.sms", 0x000000, 0x020000, CRC(6586bd1f) SHA1(417739aa248032f5aebe05750a5de85346e36712) )
ROM_END
ROM_START( s_ggolmp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ggolmp.sms", 0x000000, 0x020000, CRC(4847bc91) SHA1(ce0662179bb0ca4a6491ef2be8839168b993c04e) )
ROM_END
ROM_START( s_ggolm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ggolm.sms", 0x000000, 0x020000, CRC(98e4ae4a) SHA1(3c0b12cfb70f2515429b6e88e0753d69dbb907ab) )
ROM_END

ROM_START( s_gice )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gice.sms", 0x000000, 0x020000, CRC(946b8c4a) SHA1(a7b76be9d3ed5d6a94917e444a188767ede1cc79) )
ROM_END
ROM_START( s_gsoc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gsoc.sms", 0x000000, 0x008000, CRC(0ed170c9) SHA1(7d1a381be96474f18ba1dac8eaf6710010b0e481) )
ROM_END
ROM_START( s_gsocj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gsocj.sms", 0x000000, 0x008000, CRC(2d7fd7ef) SHA1(110536303b7bccc193bef4437ba5a9eb6fd4ac8e) )
ROM_END
ROM_START( s_gsocws )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gsocws.sms", 0x000000, 0x020000,  CRC(72112b75) SHA1(bd385b69805c623ab9934174a19a30371584c4b0) )
ROM_END
ROM_START( s_gten )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gten.sms", 0x000000, 0x008000, CRC(95cbf3dd) SHA1(e7f3529689cd29be3fa02f94266e4ee8e0795d7d) )
ROM_END
ROM_START( s_gvolj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gvolj.sms", 0x000000, 0x020000, CRC(6819b0c0) SHA1(b9e3284c7b557eed84661c98bc733d32b46c7a07) )
ROM_END
ROM_START( s_gvol )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_gvol.sms", 0x000000, 0x020000,  CRC(8d43ea95) SHA1(133ffdde0a5a0ce951c667a4c5d7f9d9f35e9658) )
ROM_END
ROM_START( s_haja )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_haja.sms", 0x000000, 0x040000, CRC(b9fdf6d9) SHA1(46a032004d49fec58099aa6bf0dd796997e95142) )
ROM_END
ROM_START( s_hoaw )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hoaw.sms", 0x000000, 0x020000, CRC(1c5059f0) SHA1(ae29f676846fc740a2cfc69756875b6480265f97) )
ROM_END
ROM_START( s_hosh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hosh.sms", 0x000000, 0x020000, CRC(e167a561) SHA1(7c741493889788d4511979bcaa3c48708d6240ed) )
ROM_END

ROM_START( s_hango )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hango.sms", 0x000000, 0x008000, CRC(071b045e) SHA1(e601257f6477b85eb0b25a5b6d46ebc070d8a05a) )
ROM_END
ROM_START( s_hangoj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hangoj.sms", 0x000000, 0x008000, CRC(5c01adf9) SHA1(43552f58f0c0c292f3e4c1b1525fd0344dc220c6) )
ROM_END
ROM_START( s_hwchm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hwchm.sms", 0x000000, 0x040000,  CRC(fdab876a) SHA1(7e2523061df0c08b7df7b446b5504c4eb0fea163) )
ROM_END
ROM_START( s_hlance )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hlance.sms", 0x000000, 0x080000, CRC(cde13ffb) SHA1(16b2219a1493c06d18c973fc550ea563c3116207) )
ROM_END
ROM_START( s_hskim )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hskim.sms", 0x000000, 0x020000, CRC(9eb1aa4f) SHA1(d3c0aeeacccef77c45ab4219c7d6d8ed04d467cb) )
ROM_END
ROM_START( s_hokuto )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hokuto.sms", 0x000000, 0x020000,  CRC(24f5fe8c) SHA1(26c5da3ee48bc0f8fd3d20f9408e584242edcd9d) )
ROM_END
ROM_START( s_homeal )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_homeal.sms", 0x000000, 0x040000, CRC(c9dbf936) SHA1(9b9be300fdc386f864af516000ffa3a53f1613e2) )
ROM_END
ROM_START( s_hookp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hookp.sms", 0x000000, 0x040000, CRC(9ced34a7) SHA1(b7bbd78b301244d7ce83f79d72fd28c56a870905) )
ROM_END
ROM_START( s_hoshi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hoshi.sms", 0x000000, 0x040000, CRC(955a009e) SHA1(f9ce8b80d8671db6ab38ba5b7ce46324a65ebc3d) )
ROM_END
ROM_START( s_imiss )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_imiss.sms", 0x000000, 0x020000, CRC(64d6af3b) SHA1(d883f28e77e575edca6dcb1c4cd1f2b1f11393b2) )
ROM_END

ROM_START( s_icd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_icd.sms", 0x000000, 0x040000, CRC(b4584dde) SHA1(94a4ba183de82fc0066a0edab2acaee5e8bdd0e7) )
ROM_END
ROM_START( s_hulk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_hulk.sms", 0x000000, 0x080000, CRC(be9a7071) SHA1(235ad7d259023610d8aa59d066aaf0dba2ff8138) )
ROM_END
ROM_START( s_indlc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_indlc.sms", 0x000000, 0x040000, CRC(8aeb574b) SHA1(68e23692a12628dde805ded9de356c5e19e4eba6) )
ROM_END
ROM_START( s_007b )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_007b.sms", 0x000000, 0x040000, CRC(8feff688) SHA1(cc3eec4da3758fe9e407ab80fa88dc952d33cdd5) )
ROM_END
ROM_START( s_007 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_007.sms", 0x000000, 0x040000, CRC(8d23587f) SHA1(89f86869b90af986bee2acff44defe420e405a1e) )
ROM_END
ROM_START( s_jbdkb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_jbdkb.sms", 0x000000, 0x040000,  CRC(6a664405) SHA1(b07000feb0c74824f2e3e74fd415631a8f3c4da6) )
ROM_END
ROM_START( s_joemon )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_joemon.sms", 0x000000, 0x040000, CRC(0a9089e5) SHA1(7452f7286cee78ce4bbd05841a4d087fdfba12e3) )
ROM_END
ROM_START( s_jogos2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_jogos2.sms", 0x000000, 0x040000, CRC(45c50294) SHA1(4d6c46dedfe38fcfb740e948563b8eeec3bd4305) )
ROM_END
ROM_START( s_jpark )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_jpark.sms", 0x000000, 0x080000, CRC(0667ed9f) SHA1(67a6e6c132362f3d9263dda68d77c279b08f1fde) )
ROM_END
ROM_START( s_kensj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_kensj.sms", 0x000000, 0x040000, CRC(05ea5353) SHA1(cd349833ff41821635c6242a0b8cef7e071103d5) )
ROM_END

ROM_START( s_kens )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_kens.sms", 0x000000, 0x040000, CRC(516ed32e) SHA1(3fce661d57e8bc764e7190ddbee4bf3d3e214c6c) )
ROM_END
ROM_START( s_kquesp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_kquesp.sms", 0x000000, 0x020000, CRC(b7fe0a9d) SHA1(1b6330199444e303aafb9a2fd3f2119cedab0712) )
ROM_END
ROM_START( s_kques )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_kques.sms", 0x000000, 0x020000, CRC(f8d33bc4) SHA1(d05ae9652b85c858f4e7db0b7b7c457a4e0a6a49) )
ROM_END
ROM_START( s_klax )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_klax.sms", 0x000000, 0x020000, CRC(2b435fd6) SHA1(53ae621e66d8e5f2e7276e461e8771c3c2037a7a) )
ROM_END
ROM_START( s_krust )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_krust.sms", 0x000000, 0x040000, CRC(64a585eb) SHA1(54714a19c2d24260b117ebc5ae391d9b24ca9166) )
ROM_END
ROM_START( s_kujaku )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_kujaku.sms", 0x000000, 0x080000, CRC(d11d32e4) SHA1(6974d27bc31c2634bec54c4e9935a28461fb60f7) )
ROM_END
ROM_START( s_kungfu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_kungfu.sms", 0x000000, 0x020000, CRC(1e949d1f) SHA1(7e1c32f5abf9ff906ffe113ffab6eecd1c86b381) )
ROM_END
ROM_START( s_lghos )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lghos.sms", 0x000000, 0x040000, CRC(0ca95637) SHA1(a21286d282ca994c66d8e7a91ee0a05ff69c7981) )
ROM_END
ROM_START( s_lemp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lemp.sms", 0x000000, 0x040000, CRC(2c61ed88) SHA1(8d8692d363348f7c93f402c6485f5f831d1c8190) )
ROM_END
ROM_START( s_lof )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lof.sms", 0x000000, 0x080000, CRC(cb09f355) SHA1(985f78bcaf64bb088d64517f80b0acc7f5034b24) )
ROM_END


ROM_START( s_lordsj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lordsj.sms", 0x000000, 0x040000,  CRC(aa7d6f45) SHA1(6a08d913fd92a213b1ecf5aa7c5630362cccc6b4) )
ROM_END
ROM_START( s_lords )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_lords.sms", 0x000000, 0x040000, CRC(e8511b08) SHA1(a5326a0029f7c3101add3335a599a01ccd7634c5) )
ROM_END
ROM_START( s_loret )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_loret.sms", 0x000000, 0x020000, CRC(323f357f) SHA1(0ddd7448f5bd437d0d33b85a44f2bcc2bf2ea05e) )
ROM_END
ROM_START( s_luckp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_luckp.sms", 0x000000, 0x040000, CRC(7f6d0df6) SHA1(5ac3e68b34ee4499ddbdee28b47a1440782a9c04) )
ROM_END
ROM_START( s_mahsjp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mahsjp.sms", 0x000000, 0x020000, CRC(996b2a07) SHA1(169510c0575e4f53b9da8197fc48608993351182) )
ROM_END
ROM_START( s_mahsj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mahsj.sms", 0x000000, 0x020000,  CRC(bcfbfc67) SHA1(9b7cd3a25b2a1fc880683dcdca81457c93d46de5) )
ROM_END
ROM_START( s_makai )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_makai.sms", 0x000000, 0x020000, CRC(ca860451) SHA1(59d520fdb2b6cbd5736b2bd6045ad3ee3ad2e3a6) )
ROM_END
ROM_START( s_marble )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_marble.sms", 0x000000, 0x040000, CRC(bf6f3e5f) SHA1(f5efe0635e283a08f98272a9ff1bc7d37c35692c) )
ROM_END
ROM_START( s_marks )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_marks.sms", 0x000000, 0x020000, CRC(e8ea842c) SHA1(5491cce7b9c19cb49060da94ab8f9c4331e77cb3) )
ROM_END
ROM_START( s_mastd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mastd.sms", 0x000000, 0x040000, CRC(96fb4d4b) SHA1(ed3569be5d5a49ff5a09b2b04ec0101d4edfa81e) )
ROM_END

ROM_START( s_mastc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mastc.sms", 0x000000, 0x040000, CRC(93141463) SHA1(9596400394f2bb3dbf7eb4a26b820cdfe5cd6094) )
ROM_END
ROM_START( s_maze3d )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_maze3d.sms", 0x000000, 0x020000, CRC(31b8040b) SHA1(6d94c2159a67f3140d0c9158b58aa8f0474eaaba) )
ROM_END
ROM_START( s_mazew )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mazew.sms", 0x000000, 0x020000, CRC(871562b0) SHA1(ade756eccaf94c79e8b3636921b6f8669a163265) )
ROM_END
ROM_START( s_megumi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_megumi.sms", 0x000000, 0x020000, CRC(29bc7fad) SHA1(7bd156cf8dc2ad07c666ac58ccb3c0ff6671b93f) )
ROM_END
ROM_START( s_mercs )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mercs.sms", 0x000000, 0x080000, CRC(d7416b83) SHA1(f2cfad96a116bde9a91240eb1ad520dc448fa20f) )
ROM_END
ROM_START( s_mjmw )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mjmw.sms", 0x000000, 0x040000, CRC(56cc906b) SHA1(939416cebb381458d28ff628afb3d1f80293afa9) )
ROM_END
ROM_START( s_mmack )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mmack.sms", 0x000000, 0x040000, CRC(b67ceb76) SHA1(e2e2f45e43f0d4fa5974327e96d7c3ee7f057fad) )
ROM_END
ROM_START( s_multc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_multc.sms", 0x000000, 0x080000, CRC(2b86bcd7) SHA1(0b46e31f656fee2ac8d947ca9bb91881d1c72428) )
ROM_END
ROM_START( s_miracl )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_miracl.sms", 0x000000, 0x040000,  CRC(0e333b6e) SHA1(f952406bca4918ee91a89b27e949e224eae96d85) )
ROM_END
ROM_START( s_md3d )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_md3d.sms", 0x000000, 0x020000, CRC(fbe5cfbb) SHA1(86242fcc8b9f93cdad2241f40c3eebbf4c9ff213) )
ROM_END

ROM_START( s_monica )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_monica.sms", 0x000000, 0x040000, CRC(01d67c0b) SHA1(e05953a3772452f821c3815231a25940a1d93803) )
ROM_END
ROM_START( s_monop )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_monop.sms", 0x000000, 0x020000, CRC(69538469) SHA1(8dd7bb4f666f70f7f57687823d0068c9100af8e5) )
ROM_END
ROM_START( s_monope )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_monope.sms", 0x000000, 0x020000, CRC(026d94a4) SHA1(6bad7176011dd4bbd007498d167399daacde173d) )
ROM_END
ROM_START( s_montez )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_montez.sms", 0x000000, 0x020000, CRC(82fda895) SHA1(a2665093b8588d5d6f24c6b329080fb3ebee896e) )
ROM_END
ROM_START( s_mk3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mk3.sms", 0x000000, 0x080000, CRC(395ae757) SHA1(f1f43f57982dd22caa8869a85b1a05fa61c349dd) )
ROM_END
ROM_START( s_mk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mk.sms", 0x000000, 0x080000, CRC(302dc686) SHA1(3f67e0e702f391839b51a43125f971ae1c5fad92) )
ROM_END
ROM_START( s_mk2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_mk2.sms", 0x000000, 0x080000, CRC(2663bf18) SHA1(12bd887efb87f410d3b65bf9e6dfe2745b345539) )
ROM_END
ROM_START( s_myhero )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_myhero.sms", 0x000000, 0x008000, CRC(62f0c23d) SHA1(7583c5fb1963c070b7bda72b447cc3fd611ddf1a) )
ROM_END
ROM_START( s_nekky )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_nekky.sms", 0x000000, 0x040000, CRC(5b5f9106) SHA1(35e882723189178c1dc811a04125daec4487e693) )
ROM_END
ROM_START( s_ngaidp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ngaidp.sms", 0x000000, 0x040000, CRC(761e9396) SHA1(c67db6539dc609b08d3ca6f9e6f8f41daf150743) )
ROM_END


ROM_START( s_ngaid )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ngaid.sms", 0x000000, 0x040000, CRC(1b1d8cc2) SHA1(4c65db563e8407444020ab7fd93fc45193ae923a) )
ROM_END
ROM_START( s_ninjaj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ninjaj.sms", 0x000000, 0x020000, CRC(320313ec) SHA1(e9acdae112a898f7db090fc0b8f1ce9b86637234) )
ROM_END
ROM_START( s_ninja )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ninja.sms", 0x000000, 0x020000, CRC(66a15bd9) SHA1(76396a25902700e18adf6bc5c8668e2a8bdf03a9) )
ROM_END
ROM_START( s_opaopa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_opaopa.sms", 0x000000, 0x020000, CRC(bea27d5c) SHA1(38bd50181b98216c9ccf1d7dd6bc2c0a21e9a283) )
ROM_END
ROM_START( s_opwolf )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_opwolf.sms", 0x000000, 0x040000, CRC(205caae8) SHA1(064040452b6bacc75443dae7916a0fd573f1600d) )
ROM_END
ROM_START( s_otti )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_otti.sms", 0x000000, 0x040000, CRC(82ef2a7d) SHA1(465e7a8cdfad8fc96587f6516770eb81a171f036) )
ROM_END
ROM_START( s_or3d )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_or3d.sms", 0x000000, 0x040000, CRC(d6f43dda) SHA1(93c3fbe23848556fc6a737db1b5182537db5961d) )
ROM_END
ROM_START( s_orun )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_orun.sms", 0x000000, 0x040000, CRC(5589d8d2) SHA1(4f9b61b24f0d9fee0448cdbbe8fc05411dbb1102) )
ROM_END
ROM_START( s_orune )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_orune.sms", 0x000000, 0x040000, CRC(3932adbc) SHA1(c8fbf18eabdcf90cd70fc77444cf309ff47f5827) )
ROM_END
ROM_START( s_papb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_papb.sms", 0x000000, 0x020000, CRC(294e0759) SHA1(e91dae35a3ca3a475e30b7863c03375d656ec734) )
ROM_END

ROM_START( s_papbu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_papbu.sms", 0x000000, 0x020000, CRC(327a0b4c) SHA1(736718efab4737ebf9d06221ac35fa2fcc4593ce) )
ROM_END
ROM_START( s_parl )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_parl.sms", 0x000000, 0x020000, CRC(e030e66c) SHA1(06664daf208f07cb00b603b12eccfc3f01213a17) )
ROM_END
ROM_START( s_party )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_party.sms", 0x000000, 0x020000, CRC(7abc70e9) SHA1(dfbf0186d497cf53d1f21a9430991ed4124cc4c2) )
ROM_END
ROM_START( s_patrip )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_patrip.sms", 0x000000, 0x040000, CRC(9aefe664) SHA1(8b49c7772bd398665217bf81648353ff46485cac) )
ROM_END
ROM_START( s_pland )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pland.sms", 0x000000, 0x020000, CRC(f97e9875) SHA1(8762239c339a084dfb8443cc38515301476bde28) )
ROM_END
ROM_START( s_pga )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pga.sms", 0x000000, 0x040000, CRC(95b9ea95) SHA1(bdbb1337453234289fbd193d7d7fcf1ad3c3807c) )
ROM_END
ROM_START( s_pstb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pstb.sms", 0x000000, 0x080000, CRC(75971bef) SHA1(fd8dad6acb6fa75dc8e9bbaea2a7e9fd486fc2dd) )
ROM_END
ROM_START( s_pstmd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pstmd.sms", 0x000000, 0x080000, CRC(07301f83) SHA1(b3ae447dc739256616b44cbd77cb903c9f19e980) )
ROM_END
ROM_START( s_pstj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pstj.sms", 0x000000, 0x080000,  CRC(6605d36a) SHA1(c9a40ddd217c58dddcd6b5c0fe66c3a50d3e68e4) )
ROM_END
ROM_START( s_pstk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pstk.sms", 0x000000, 0x080000, CRC(747e83b5) SHA1(52b2aa52a1c96e15869498a8e42b074705070007) )
ROM_END

ROM_START( s_pst12 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pst12.sms", 0x000000, 0x080000, CRC(e4a65e79) SHA1(257ca76ebcd54c75a414ca7ce968fa59ea42f150) )
ROM_END
ROM_START( s_pst13 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pst13.sms", 0x000000, 0x080000, CRC(00bef1d7) SHA1(07fcf297be4f4c9d92cd3f119a7ac48467e06838) )
ROM_END
ROM_START( s_pfigb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pfigb.sms", 0x000000, 0x080000, CRC(aa4d4b5a) SHA1(409ccca9dcb78a20f7dd917699ce2c70f87f857f) )
ROM_END
ROM_START( s_pfig )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pfig.sms", 0x000000, 0x080000, CRC(b840a446) SHA1(e2856b4ae331aea100984ac778b5e726f5da8709) )
ROM_END
ROM_START( s_popu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_popu.sms", 0x000000, 0x040000, CRC(c7a1fdef) SHA1(d6142584b9796a96941b6a95bda14cf137b47085) )
ROM_END
ROM_START( s_pw3d )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pw3d.sms", 0x000000, 0x040000, CRC(abd48ad2) SHA1(c177effd5fd18a082393a2b4167c49bcc5db1f64) )
ROM_END
ROM_START( s_pstrk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pstrk.sms", 0x000000, 0x020000, CRC(4077efd9) SHA1(f6f245c41163b15bce95368e4684b045790a1148) )
ROM_END
ROM_START( s_pstrk2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pstrk2.sms", 0x000000, 0x080000, CRC(a109a6fe) SHA1(3bcffd47294f25b25cccb7f42c3a9c3f74333d73) )
ROM_END
ROM_START( s_pred2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pred2.sms", 0x000000, 0x040000, CRC(0047b615) SHA1(ee26c9f5fd08ac73b5e28b20f35889d24c88c6db) )
ROM_END
ROM_START( s_pop )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_pop.sms", 0x000000, 0x040000, CRC(7704287d) SHA1(7ef7b4e2fcec69946844c186c62836c0ae34665f) )
ROM_END

ROM_START( s_prow )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_prow.sms", 0x000000, 0x020000, CRC(fbde42d3) SHA1(b0e4832af04b4fb832092ad093d982ce11160eef) )
ROM_END
ROM_START( s_proyak )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_proyak.sms", 0x000000, 0x020000, CRC(da9be8f0) SHA1(1ee602f8711d82d13b006984cef95512a93c7783) )
ROM_END
ROM_START( s_promo3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_promo3.sms", 0x000000, 0x008000, CRC(30af0233) SHA1(53b50a1c574479359f85327a8000d3f03f0963d5) )
ROM_END
ROM_START( s_psyc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_psyc.sms", 0x000000, 0x040000, CRC(5c0b1f0f) SHA1(5fa54329692e680a190291d0744580968aa8b3fe) )
ROM_END
ROM_START( s_putt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_putt.sms", 0x000000, 0x020000, CRC(357d4f78) SHA1(760d330047a77b98e2fff786052741dc7e3760e8) )
ROM_END
ROM_START( s_puttp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_puttp.sms", 0x000000, 0x020000, CRC(8167ccc4) SHA1(c6d3f256f938827899807884253817f432379d7c) )
ROM_END
ROM_START( s_quart )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_quart.sms", 0x000000, 0x020000,  CRC(e0f34fa6) SHA1(08a3484e862a284f6038b7cd0dfc745a8b7c6c51) )
ROM_END
ROM_START( s_quest )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_quest.sms", 0x000000, 0x080000, CRC(f42e145c) SHA1(418ea57fdbd06aca4285447db2ecb2b0392a178d) )
ROM_END

ROM_START( s_rcgp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rcgp.sms", 0x000000, 0x040000, CRC(54316fea) SHA1(94cc64f6febdb0fe4e89ecd9deeca96694e5bead) )
ROM_END
ROM_START( s_rainbb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rainbb.sms", 0x000000, 0x040000, CRC(00ec173a) SHA1(a3958277129916a4fd32f3d5f2345b8d0fc23faf) )
ROM_END
ROM_START( s_rainb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rainb.sms", 0x000000, 0x040000, CRC(c172a22c) SHA1(acff4b6175aaaed9075f6e41cf387cbfd03eb330) )
ROM_END
ROM_START( s_rambo )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rambo.sms", 0x000000, 0x020000, CRC(bbda65f0) SHA1(dc44b090a01d6a4d9b5700ada764b00c62c00e91) )
ROM_END
ROM_START( s_rambo3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rambo3.sms", 0x000000, 0x040000, CRC(da5a7013) SHA1(6d9746c0e3c50e87fa773a64e2f0bb76f722d76c) )
ROM_END
ROM_START( s_rampag )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rampag.sms", 0x000000, 0x040000, CRC(42fc47ee) SHA1(cbf5b6f06bec42db8a99c7b9c00118521aded858) )
ROM_END
ROM_START( s_rampar )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rampar.sms", 0x000000, 0x040000, CRC(426e5c8a) SHA1(bfea7112a7e2eb2c5e7c20147197ffe3b06d5711) )
ROM_END
ROM_START( s_rastan )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rastan.sms", 0x000000, 0x040000, CRC(c547eb1b) SHA1(90f9ccf516db2a1cf20e199cfd5d31d4cfce0f1f) )
ROM_END
ROM_START( s_reggie )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_reggie.sms", 0x000000, 0x040000, CRC(6d94bb0e) SHA1(b7c387256e58a95bd3c3c4358a8a106f2304e9f2) )
ROM_END
ROM_START( s_renega )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_renega.sms", 0x000000, 0x040000, CRC(3be7f641) SHA1(c07a04f3ab811b52c97b9bf850670057148de6f0) )
ROM_END

ROM_START( s_rescue )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rescue.sms", 0x000000, 0x020000, CRC(79ac8e7f) SHA1(6a561bff2f8022261708b91722caf1ec0e63f9c4) )
ROM_END
ROM_START( s_roboc3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_roboc3.sms", 0x000000, 0x040000, CRC(9f951756) SHA1(459a01f6e240e6f81726f174fadcfe06badce841) )
ROM_END
ROM_START( s_roboct )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_roboct.sms", 0x000000, 0x080000, CRC(8212b754) SHA1(a5fe99ce04cb172714a431b12ce14e39fcc573e4) )
ROM_END
ROM_START( s_rocky )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_rocky.sms", 0x000000, 0x040000, CRC(1bcc7be3) SHA1(8601927ca17419f5d61f757e9371ce533228f7bb) )
ROM_END
ROM_START( s_runbat )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_runbat.sms", 0x000000, 0x040000, CRC(1fdae719) SHA1(8f680b3a9782304a2f879f6590dc78ea4e366163) )
ROM_END
ROM_START( s_sagaia )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sagaia.sms", 0x000000, 0x040000, CRC(66388128) SHA1(2a3e859139f8ca83494bb800dc848fe4d02db82a) )
ROM_END
ROM_START( s_sango3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sango3.sms", 0x000000, 0x100000, CRC(97d03541) SHA1(c0256188b15271bb814bb8356b3311340e53ea3e) )
ROM_END
ROM_START( s_sxom )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sxom.sms", 0x000000, 0x040000, CRC(890e83e4) SHA1(f14a7448f38766640c0bc9ea3410eb55194da58f) )
ROM_END
ROM_START( s_sxsos )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sxsos.sms", 0x000000, 0x020000, CRC(7ab2946a) SHA1(2583b9027fd2a4d84ed324ed03fc82b9eedd1ff4) )
ROM_END
ROM_START( s_sxvs )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sxvs.sms", 0x000000, 0x040000, CRC(9a608327) SHA1(fb4205bf1c1df55455a7ab4d4a6a9c5fe7d12a0b) )
ROM_END

ROM_START( s_sat7 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sat7.sms", 0x000000, 0x008000, CRC(16249e19) SHA1(88fc5596773ea31eda8ae5a8baf6f0ce5c3f7e5e) ) // hold
ROM_END
ROM_START( s_scram )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_scram.sms", 0x000000, 0x040000, CRC(9a8b28ec) SHA1(430154dc7ac1bb580c7663019ca476e5333e0508) )
ROM_END
ROM_START( s_sdi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sdi.sms", 0x000000, 0x020000, CRC(1de2c2d0) SHA1(2000b3b291dd7b76c3b8801a88fb0e293ca7e278) )
ROM_END
ROM_START( s_secret )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_secret.sms", 0x000000, 0x020000, CRC(00529114) SHA1(e7a952f8bd6dddbb365870e906801021b240a533) )
ROM_END
ROM_START( s_sches )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sches.sms", 0x000000, 0x040000, CRC(a8061aef) SHA1(2c386825dce99b340084b28bdf90fb4ee7107317) )
ROM_END
ROM_START( s_wtgolf )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wtgolf.sms", 0x000000, 0x040000, CRC(296879dd) SHA1(fd5a92e74a3c5fa16727637f1839b28595449bf6) )
ROM_END
ROM_START( s_seishu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_seishu.sms", 0x000000, 0x008000, CRC(f0ba2bc6) SHA1(6942f38e608cc7d70cf9cc8c13ee8c22e4b81679) )
ROM_END
ROM_START( s_shdanc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_shdanc.sms", 0x000000, 0x080000, CRC(3793c01a) SHA1(99a5995f31dcf6fbbef56d3ea0d2094ef039479f) )
ROM_END
ROM_START( s_shbeas )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_shbeas.sms", 0x000000, 0x040000, CRC(1575581d) SHA1(45943b021cbaee80a149b80ddb6f3fb5eb8b9e43) )
ROM_END
ROM_START( s_shangh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_shangh.sms", 0x000000, 0x020000, CRC(aab67ec3) SHA1(58f01556d1f2da0af9dfcddcb3ac26cb299220d3) )
ROM_END


ROM_START( s_shinoj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_shinoj.sms", 0x000000, 0x040000, CRC(e1fff1bb) SHA1(26070deaa2d3c170d31ac395a50231204250bdf3) )
ROM_END
ROM_START( s_shino )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_shino.sms", 0x000000, 0x040000, CRC(0c6fac4e) SHA1(7c0778c055dc9c2b0aae1d166dbdb4734e55b9d1) )
ROM_END
ROM_START( s_shgal )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_shgal.sms", 0x000000, 0x020000, CRC(4b051022) SHA1(6c22e3fa928c2aed468e925af65ea7f7c6292905) )
ROM_END
ROM_START( s_simpbw )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_simpbw.sms", 0x000000, 0x040000, CRC(f6b2370a) SHA1(038920e63437e05d8431e4cbab2131dd76fb3345) )
ROM_END
ROM_START( s_sitio )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sitio.sms", 0x000000, 0x100000,CRC(abdf3923) SHA1(9d626e9faa29a63ed396959894d4a481f1e7a01d) )
ROM_END
ROM_START( s_slapsa )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_slapsa.sms", 0x000000, 0x040000, CRC(d33b296a) SHA1(5979ee84570f7c930f20be473e895fc1d2b9e3f4) )
ROM_END
ROM_START( s_slaps )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_slaps.sms", 0x000000, 0x040000, CRC(c93bd0e9) SHA1(6327fb7129ecfcebdcd6b9c941703fda15a8a195) )
ROM_END
ROM_START( s_smurf2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smurf2.sms", 0x000000, 0x040000, CRC(97e5bb7d) SHA1(16f756e00314781e07af84c871e82ec8e0a68a57) )
ROM_END
ROM_START( s_smurf )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smurf.sms", 0x000000, 0x040000, CRC(3e63768a) SHA1(82f75232e195b8bdcd9c0d852076d999899cc92e) )
ROM_END
ROM_START( s_solomo )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_solomo.sms", 0x000000, 0x020000, CRC(11645549) SHA1(875f35d0775609776ec75ea2a8fa2297643e906c) )
ROM_END


ROM_START( s_sonbls )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sonbls.sms", 0x000000, 0x100000, CRC(96b3f29e) SHA1(4ad77a472e98002dc0d5c1463965720a257e1b8f) )
ROM_END
ROM_START( s_soncha )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_soncha.sms", 0x000000, 0x080000, CRC(aedf3bdf) SHA1(f64c8eea26a103582f09831c3e02c6045a6aff94) )
ROM_END
ROM_START( s_sonspi )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sonspi.sms", 0x000000, 0x080000, CRC(11c1bc8a) SHA1(a6aa8038feb54e84759fcdfced2270afbbef9bfc) )
ROM_END
ROM_START( s_son211 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_son211.sms", 0x000000, 0x080000, CRC(d6f2bfca) SHA1(689339bac11c3565dd774f8cd4d8ea1b27831118) )
ROM_END
ROM_START( s_spacg )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spacg.sms", 0x000000, 0x080000, CRC(a908cff5) SHA1(c5afd5fa7b26da55d243df6822c16dae3a401ac1) )
ROM_END
ROM_START( s_sh3d )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sh3d.sms", 0x000000, 0x040000, CRC(6bd5c2bf) SHA1(ce3507f62563f7d4cb4b2fc6497317685626af92) )
ROM_END
ROM_START( s_sh3dj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sh3dj.sms", 0x000000, 0x040000,  CRC(156948f9) SHA1(7caf44ecc3de6daffedf7a494d449202888a6156) )
ROM_END
ROM_START( s_shj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_shj.sms", 0x000000, 0x040000, CRC(beddf80e) SHA1(51ba2185a2b93957c1c51b0a2e2b80394463bed8) )
ROM_END
ROM_START( s_sh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sh.sms", 0x000000, 0x040000, CRC(ca1d3752) SHA1(9e92d8e27fad71635c71612e8bdd632d760f9a2d) )
ROM_END
ROM_START( s_scib )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_scib.sms", 0x000000, 0x040000, CRC(1b7d2a20) SHA1(a0b4be5b62a0a836e227983647e0140df3eafe4d) )
ROM_END


ROM_START( s_sci )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sci.sms", 0x000000, 0x040000, CRC(fa8e4ca0) SHA1(8daabf51099bd2702fe4418fd202eff532bc710a) )
ROM_END
ROM_START( s_sb2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sb2.sms", 0x000000, 0x040000, CRC(0c7366a0) SHA1(4e0b456441f0acef737e463e6ee5bbcb377ea308) )
ROM_END
ROM_START( s_sbm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sbm.sms", 0x000000, 0x020000, CRC(a57cad18) SHA1(e51cbdb9b74ce4b53747215b23b54eb62f8392b3) )
ROM_END
ROM_START( s_sb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sb.sms", 0x000000, 0x020000, CRC(5ccc1a65) SHA1(a4cf72c985d0fe51ac36388774c7f0bf982c19e3) )
ROM_END
ROM_START( s_spell )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spell.sms", 0x000000, 0x080000, CRC(4752cae7) SHA1(eed8b01fca86dbd8291d5ca2d1e6f6ca1b60fe68) )
ROM_END
ROM_START( s_smrss )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smrss.sms", 0x000000, 0x040000, CRC(ebe45388) SHA1(5d4fbf3873af14afcda10fadfdb3f4f8919b3b1e) )
ROM_END
ROM_START( s_smvsk )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smvsk.sms", 0x000000, 0x040000, CRC(908ff25c) SHA1(02ebee891d88bacdadd37a3e75e05763b7ad3c9b) )
ROM_END
ROM_START( s_spfoot )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spfoot.sms", 0x000000, 0x020000, CRC(e42e4998) SHA1(556d9ab4ba3c3a34440b36c6fc8e972f70f16d72) )
ROM_END
ROM_START( s_spsocc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spsocc.sms", 0x000000, 0x020000, CRC(41c948bf) SHA1(7634ce39e87049dad1ee4f32a80d728e4bd1f81f) )
ROM_END
ROM_START( s_spyspj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spyspj.sms", 0x000000, 0x008000, CRC(d41b9a08) SHA1(c5e004b34d6569e6e1d99bff6be940f308e2c39f) )
ROM_END

ROM_START( s_spyspd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_spyspd.sms", 0x000000, 0x040000, CRC(b87e1b2b) SHA1(03eec0d33d7c3b376fe08ffec79f84091f58366b) )
ROM_END
ROM_START( s_starw )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_starw.sms", 0x000000, 0x080000, CRC(d4b8f66d) SHA1(be75ca8ace66b72a063b4be2da2b1ed92f8449b0) )
ROM_END
ROM_START( s_sf2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sf2.sms", 0x000000, 0x0c8000, CRC(0f8287ec) SHA1(1866752a92abbf0eb55fbf9de1cd1b731ec62a54) )
ROM_END
ROM_START( s_sora )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sora.sms", 0x000000, 0x080000, CRC(4ab3790f) SHA1(5bdec24d9ba0f6ed359dcb3b11910ec86866ec98) )
ROM_END
ROM_START( s_sor2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sor2.sms", 0x000000, 0x080000, CRC(04e9c089) SHA1(cc18171a860711f6ad18ff89254dd7bd05c54654) )
ROM_END
ROM_START( s_strid2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_strid2.sms", 0x000000, 0x040000, CRC(b8f0915a) SHA1(f9d58bd9a5a99f2fd26fb411b0783fcd220249a4) )
ROM_END
ROM_START( s_subatt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_subatt.sms", 0x000000, 0x040000, CRC(d8f2f1b9) SHA1(44384ec8bf91e1ca5512ff88bbcf1ae1ce5a1a35) )
ROM_END
ROM_START( s_suke2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_suke2.sms", 0x000000, 0x020000, CRC(b13df647) SHA1(5cd041990c7418ba63cde54f83d3e0e323b42c3b) )
ROM_END
ROM_START( s_sumgam )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sumgam.sms", 0x000000, 0x020000, CRC(8da5c93f) SHA1(f9638b693ca3a7c65cfc589aaea0bbab56fc7238) )
ROM_END
ROM_START( s_supbbd )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supbbd.sms", 0x000000, 0x010000, CRC(0dbf3b4a) SHA1(616b95baad61086f08d8688c2bf06838b902ac75) )
ROM_END


ROM_START( s_smgpu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_smgpu.sms", 0x000000, 0x040000, CRC(3ef12baa) SHA1(fc933d9ec3a7e699c1e2cda89b957665e7321a80) )
ROM_END
ROM_START( s_suprac )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_suprac.sms", 0x000000, 0x040000, CRC(7e0ef8cb) SHA1(442f3ba8a66a0d9c49a46091df83fcdba4b63c3a) )
ROM_END
ROM_START( s_sstv )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_sstv.sms", 0x000000, 0x040000,CRC(e0b1aff8) SHA1(b4515aad1cad31980d041632e23d3be82aa31828) )
ROM_END
ROM_START( s_supten )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supten.sms", 0x000000, 0x008000, CRC(914514e3) SHA1(67787f3f29a5b5e74b5f6a636428da4517a0f992) )
ROM_END
ROM_START( s_supwbm )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supwbm.sms", 0x000000, 0x040000, CRC(b1da6a30) SHA1(f4cd1ee6f98bc77fb36e232bf755d61d88e219d7) )
ROM_END
ROM_START( s_supwb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supwb.sms", 0x000000, 0x020000, CRC(e2fcb6f3) SHA1(14210196f454b6d938f15cf7b52076796c5d0f7d) )
ROM_END
ROM_START( s_supman )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_supman.sms", 0x000000, 0x040000, CRC(6f9ac98f) SHA1(f12b0eddfc271888bbcb1de3df25072b96b024ec) )
ROM_END
ROM_START( s_t2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_t2.sms", 0x000000, 0x080000,  CRC(93ca8152) SHA1(cfa4a899185fced837991d14f011cdaca81e9dd7) )
ROM_END
ROM_START( s_chashq )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_chashq.sms", 0x000000, 0x040000,  CRC(85cfc9c9) SHA1(495e3ced83ccd938b549bc76905097dba0aaf32b) )
ROM_END
ROM_START( s_tazmar )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tazmar.sms", 0x000000, 0x080000, CRC(11ce074c) SHA1(36a67210ca9762f280364007fcacbd7b1416d6ee) )
ROM_END

ROM_START( s_taza )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_taza.sms", 0x000000, 0x040000, CRC(7cc3e837) SHA1(ac98f23ddc24609cb77bb13102e0386f8c2a4a76) )
ROM_END
ROM_START( s_twc92 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_twc92.sms", 0x000000, 0x040000, CRC(96e75f48) SHA1(bfc473bcfd849c8955f24e82347423fef4f7faf5) )
ROM_END
ROM_START( s_twc93 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_twc93.sms", 0x000000, 0x040000, CRC(5a1c3dde) SHA1(43a30da241f57cf086ab9b2ed25fe018171f2908) )
ROM_END
ROM_START( s_tbb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tbb.sms", 0x000000, 0x008000, CRC(2728faa3) SHA1(6ae39718703dbf7126f71387ce24ad956710a315) )
ROM_END
ROM_START( s_tbbmc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tbbmc.sms", 0x000000, 0x00c000, CRC(d7508041) SHA1(51014d89cd8770df8a3d837a0208cb69d5bf7903) )
ROM_END
ROM_START( s_tbbj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tbbj.sms", 0x000000, 0x008000, CRC(316727dd) SHA1(fb61c04f30c83733fdbf503b16e17aa8086932df) )
ROM_END
ROM_START( s_tenace )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tenace.sms", 0x000000, 0x040000, CRC(1a390b93) SHA1(0d202166d4a3bdfcf90514c711c77e4f20764552) )
ROM_END
ROM_START( s_tensai )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tensai.sms", 0x000000, 0x040000, CRC(8132ab2c) SHA1(ca802b7bdbc0b458d02fda2da32c2e27e50eef19) )
ROM_END
ROM_START( s_term2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_term2.sms", 0x000000, 0x040000, CRC(ac56104f) SHA1(a8fe50e27fa9d44f3bd05d249a964352a32d1799) )
ROM_END
ROM_START( s_term )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_term.sms", 0x000000, 0x040000, CRC(edc5c012) SHA1(eaf733d385e61526b90c1b194bf605078d43e2d3) )
ROM_END

ROM_START( s_tbladj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tbladj.sms", 0x000000, 0x040000, CRC(c0ce19b1) SHA1(535dbd339f4b5c5efd502cffbbe719d7b3e7f1c3) )
ROM_END
ROM_START( s_tblad )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tblad.sms", 0x000000, 0x040000, CRC(ae920e4b) SHA1(c38f3eea4e7224bc6042723e88b26c85b1a56ddc) )
ROM_END
ROM_START( s_tsol )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tsol.sms", 0x000000, 0x040000, CRC(51bd14be) SHA1(89842cb30cc3ba626901a5da41481f6d157ebd15) )
ROM_END
ROM_START( s_tomjp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tomjp.sms", 0x000000, 0x040000, CRC(0c2fc2de) SHA1(83133cc875f4124b6ede7a9afc29aa311b36c285) )
ROM_END
ROM_START( s_tomjmv )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tomjmv.sms", 0x000000, 0x040000, CRC(bf7b7285) SHA1(30224286c65beddf37dc83f688f1bd362f325227) )
ROM_END
ROM_START( s_totow3 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_totow3.sms", 0x000000, 0x040000, CRC(4f8d75ec) SHA1(55ca8a8b1f2a342fc8b8fc3f3ccd98ed44b2fe98) )
ROM_END
ROM_START( s_tranbt )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tranbt.sms", 0x000000, 0x008000, CRC(4bc42857) SHA1(73273e6d44ad7aea828b642d22f6f1c138be9d2b) )
ROM_END
ROM_START( s_trapsh )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_trapsh.sms", 0x000000, 0x020000, CRC(e8215c2e) SHA1(eaae5c9d9de24c0991500122042b3aa2210d50d9) )
ROM_END
ROM_START( s_trivia )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_trivia.sms", 0x000000, 0x080000, CRC(e5374022) SHA1(bafaaeaa376e109db5d52a484a3efcd6ed84d4d6) )
ROM_END
ROM_START( s_turma )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_turma.sms", 0x000000, 0x040000, CRC(22cca9bb) SHA1(77b4ec8086d81029b596020f202df3e210df985d) )
ROM_END

ROM_START( s_tvcol )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_tvcol.sms", 0x000000, 0x080000, CRC(e1714a88) SHA1(1b4f7eca8a3f04ead404e6f439a6c49a0d0500df) )
ROM_END
ROM_START( s_ult4p )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ult4p.sms", 0x000000, 0x080000, CRC(de9f8517) SHA1(bb1ae06b62a9f7d3259c51eee4cfded781eb5d30) )
ROM_END
ROM_START( s_ult4 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ult4.sms", 0x000000, 0x080000, CRC(b52d60c8) SHA1(a90e21e5961bcf2e10b715a009c04e7c2017a3b1) )
ROM_END
ROM_START( s_ultsoc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_ultsoc.sms", 0x000000, 0x040000, CRC(15668ca4) SHA1(e0adf52b6e1f54260dd6ea80e99ffb8bf76fd49a) )
ROM_END
ROM_START( s_vampp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_vampp.sms", 0x000000, 0x040000, CRC(20f40cae) SHA1(d03fb8fb31d6c49ce92e0a4c952768896f798dd5) )
ROM_END
ROM_START( s_vigil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_vigil.sms", 0x000000, 0x040000, CRC(dfb0b161) SHA1(0dc37f1104508c2c0e2593b10dffc3f268ae8ff9) )
ROM_END
ROM_START( s_vfanim )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_vfanim.sms", 0x000000, 0x100000, CRC(57f1545b) SHA1(ee24af11f2066dc8ffbefb72a5048a2471576229) )
ROM_END
ROM_START( s_walter )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_walter.sms", 0x000000, 0x040000, CRC(3d55759b) SHA1(2d1201523540ae1673fe75bd2c9d1db1cc61987d) )
ROM_END
ROM_START( s_wanted )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wanted.sms", 0x000000, 0x020000, CRC(5359762d) SHA1(6135e4d0f76812f9d35ddb5b3e7d34d56a5458b3) )
ROM_END
ROM_START( s_wwcsb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wwcsb.sms", 0x000000, 0x020000, CRC(88aa8ca6) SHA1(c848e92899dc9b6f664e98ad247fed86c0e46a41) )
ROM_END

ROM_START( s_wwcs )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wwcs.sms", 0x000000, 0x020000, CRC(428b1e7c) SHA1(683317c8d7d8b974066d7ddb3ed64f99801aa9de) )
ROM_END
ROM_START( s_wimb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wimb.sms", 0x000000, 0x040000, CRC(912d92af) SHA1(07bc7896efc9eae77a9be68c190071c99eb17a8a) )
ROM_END
ROM_START( s_wimb2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wimb2.sms", 0x000000, 0x040000,  CRC(7f3afe58) SHA1(55dd4aaa92de4bdaa1d8b6463a34887f4a8baa28) )
ROM_END
ROM_START( s_win94b )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_win94b.sms", 0x000000, 0x080000, CRC(2fec2b4a) SHA1(758f3dc68db93f35e315fbf2fcd904484cce2ad1) )
ROM_END
ROM_START( s_win94 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_win94.sms", 0x000000, 0x080000, CRC(a20290b6) SHA1(20b73d53e6868957f07b1a813b853943bfb90307) )
ROM_END
ROM_START( s_wolfc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wolfc.sms", 0x000000, 0x040000, CRC(1f8efa1d) SHA1(92873f8d7b81a57114b135a2dfffc58d45643703) )
ROM_END
ROM_START( s_wboy )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wboy.sms", 0x000000, 0x020000, CRC(73705c02) SHA1(63149f20bf69cd2f24d0e58841fcfcdace972f49) )
ROM_END
ROM_START( s_wbml )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wbml.sms", 0x000000, 0x040000, CRC(8cbef0c1) SHA1(51715db7a49a18452292dadb2bba0108aa6d6402) )
ROM_END
ROM_START( s_wbmwp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wbmwp.sms", 0x000000, 0x080000, CRC(81bff9bb) SHA1(7a26de7a544c602daeadbd3507028ac68ef35e91) )
ROM_END
ROM_START( s_wbmw )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wbmw.sms", 0x000000, 0x080000, CRC(7d7ce80b) SHA1(da0acdb1b9e806aa67a0216a675cb02ed24abf8b) )
ROM_END

ROM_START( s_wclb )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wclb.sms", 0x000000, 0x040000,  CRC(c9a449b7) SHA1(7bd82131944020a20167f37071ccc1dfa3ae0b3d) )
ROM_END
ROM_START( s_wc90 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wc90.sms", 0x000000, 0x020000, CRC(6e1ad6fd) SHA1(dfa51a4f982d0bec61532e16a679edae605d0aea) )
ROM_END
ROM_START( s_wc94 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wc94.sms", 0x000000, 0x080000, CRC(a6bf8f9e) SHA1(f41d81710f24b08a2a3ac28f2679338a47ca5890) )
ROM_END
ROM_START( s_wgamp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wgamp.sms", 0x000000, 0x020000, CRC(914d3fc4) SHA1(6361f796248c71ecfaaa02aafe6ddbbaebf6ebba) )
ROM_END
ROM_START( s_wgam )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wgam.sms", 0x000000, 0x020000, CRC(a2a60bc8) SHA1(4f3c04e40dd4b94d5d090068ba99b8461af56f51) )
ROM_END
ROM_START( s_wgp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wgp.sms", 0x000000, 0x020000, CRC(4aaad0d6) SHA1(650f15ebbd149f5c357f089d7bd305fcb20b068f) )
ROM_END
ROM_START( s_wgpu )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wgpu.sms", 0x000000, 0x020000, CRC(7b369892) SHA1(feff411732ca2dc17dab6d7868749d79326993d7) )
ROM_END
ROM_START( s_wwfsc )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_wwfsc.sms", 0x000000, 0x040000, CRC(2db21448) SHA1(6fd4f5af0f14e1e0a934cd9e39a6bb476eda7e97) )
ROM_END
ROM_START( s_x2iw )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_x2iw.sms", 0x000000, 0x040000, CRC(5c205ee1) SHA1(72cb8a24f63e9e79c65c26141abdf53f96c60c0c) )
ROM_END
ROM_START( s_x2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_x2.sms", 0x000000, 0x040000, CRC(ec726c0d) SHA1(860cff21eff077acd92b06a71d859bf3e81fe628) )
ROM_END

ROM_START( s_xmenmj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_xmenmj.sms", 0x000000, 0x080000, CRC(3e1387f6) SHA1(6405a2f8b6f220b4349f8006c3d75dfcdcd6db6d) )
ROM_END
ROM_START( s_zax3dp )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zax3dp.sms", 0x000000, 0x040000, CRC(bba74147) SHA1(85a67064c71dfa58eb150cc090beb5ae6639b527) )
ROM_END
ROM_START( s_zax3d )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zax3d.sms", 0x000000, 0x040000, CRC(a3ef13cb) SHA1(257946fe8a230ac1308fc60a8ed43851cfe6b915) )
ROM_END
ROM_START( s_zilli )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "s_zilli.sms", 0x000000, 0x020000, CRC(60c19645) SHA1(6a0a21426cadadb5567907d9cc4cdaf63195d5c3) )
ROM_END


ROM_START( gg_bust )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_bust.gg", 0x000000, 0x040000, CRC(c90f29ef) SHA1(e6bb5f72cffb11c8dd44ac3e378088b04cec1297) )
ROM_END

ROM_START( gg_puzlo )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_puzlo.gg", 0x000000, 0x040000, CRC(d173a06f) SHA1(8ea2e623858221c5d39eb1e0f6532a0b23b00305) )
ROM_END

ROM_START( gg_puyo2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_puyo2.gg", 0x000000, 0x080000, CRC(3ab2393b) SHA1(04ea9ff57faeeccd1bbb17490d22313ae94c86d6) )
ROM_END

ROM_START( gg_tempj )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_tempj.gg", 0x000000, 0x080000,  CRC(de466796) SHA1(a2d800d2836a03f81dd8f3dda23cd2d8bfff18a5) )
ROM_END

ROM_START( gg_tess )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_tess.gg", 0x000000, 0x020000, CRC(ca0e11cc) SHA1(2b2652c7e03218b212e4d6a6246bd70e925e7ee1) )
ROM_END

ROM_START( gg_popil )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_popil.gg", 0x000000, 0x020000, CRC(cf6d7bc5) SHA1(fb939f0810d0763b9abaeec1a2bfbabacaad5441) )
ROM_END

ROM_START( gg_nazo )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_nazo.gg", 0x000000, 0x020000, CRC(bcce5fd4) SHA1(6ad654e11067ed12437022fcf22b9fea9be7ac46) )
ROM_END
ROM_START( gg_nazo2 )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_nazo2.gg", 0x000000, 0x040000, CRC(73939de4) SHA1(dbd511aff622c618eac0c21de36965b869362dbd) )
ROM_END
ROM_START( gg_gear )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_gear.gg", 0x000000, 0x020000, CRC(e9a2efb4) SHA1(dc71c749c4a78e28922e10df645d23cc8289531f) )
ROM_END
ROM_START( gg_bean )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_bean.gg", 0x000000, 0x040000, CRC(3c2d4f48) SHA1(c36bda8a86994d430506b196922ec1365622e560 ) )
ROM_END
ROM_START( gg_cols )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_cols.gg", 0x000000, 0x008000, CRC(83fa26d9) SHA1(05b73f39a90fd59e01262cbd3f4e7a21575d468a) )
ROM_END
ROM_START( gg_baku )
	ROM_REGION( 0x400000, REGION_USER1, 0 ) /* z80 Code */
	ROM_LOAD( "gg_baku.gg", 0x000000, 0x040000, CRC(10ac9374) SHA1(5145967d0a3632cf6bb1a9d58b2ef04faecd40db) )
ROM_END



#ifdef HAZEMD
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
#endif /* HAZEMD */

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
	init_standagg(machine);
	smsgg_backupram = auto_malloc(0x2000);
	memset(smsgg_backupram, 0xff, 0x2000);
}

GAME( 199?, s_fantdz,        0,        sms,    sms,    codemast, ROT0,   "Codemasters", "(Master System) Fantastic Dizzy", 0)
GAME( 199?, s_micro,         0,        sms,    sms,    codemast, ROT0,   "Codemasters", "(Master System) Micro Machines", 0)
GAME( 199?, s_cosmic,        0,        sms,    sms,    codemast, ROT0,   "Codemasters", "(Master System) Cosmic Spacehead", 0)
GAME( 199?, s_dinob,         0,        sms,    sms,    codemast, ROT0,   "Codemasters", "(Master System) Dinobasher (proto)", 0) // pause crashes it (!)

GAME( 199?, s_landil,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mickey Mouse - Land of Illusion  [!]", 0)
GAME( 199?, s_tazman,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Taz-Mania  [!]", 0)
GAME( 199?, s_bubbob,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Bubble Bobble  [!]", 0)
GAME( 199?, s_chuck,         0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Chuck Rock  [!]", 0)
GAME( 199?, s_chuck2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Chuck Rock 2  [!]", 0)

GAME( 199?, s_adams,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Addams Family, The  [!]", GAME_NOT_WORKING )
GAME( 199?, s_aburn,         0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) After Burner  [!]", 0)
GAME( 199?, s_aladin,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Aladdin  [!]", 0)
GAME( 199?, s_alexmi,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alex Kidd in Miracle World  [!]", 0)
GAME( 199?, s_alsynd,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Alien Syndrome  [!]", 0)
GAME( 199?, s_alstor,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Alien Storm  [!]", 0)
GAME( 199?, s_actfgh,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Action Fighter  [!]", 0)
GAME( 199?, s_column,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Columns  [!]", 0)
GAME( 199?, s_bean,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dr. Robotnik's Mean Bean Machine  [!]", 0)
GAME( 199?, s_fzone,         0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Fantasy Zone", 0)
GAME( 199?, s_fzone2,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Fantasy Zone 2 - The Tears of Opa-Opa  [!]", 0)
GAME( 199?, s_fzone3,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Fantasy Zone 3 - The Maze  [!]", 0)
GAME( 199?, s_flint,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Flintstones, The  [!]", 0)

GAME( 199?, s_wb3dt,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wonder Boy 3 - The Dragon's Trap  [!]", 0)
GAME( 199?, s_woody,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Woody Pop (J) [!]", GAME_NOT_WORKING ) // input
GAME( 199?, s_zool,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Zool  [!]", GAME_NOT_WORKING ) // locks up..
GAME( 199?, s_smgpa,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Monaco GP  [a1][!]", 0)
GAME( 199?, s_sor,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Streets of Rage  [!]", 0)
GAME( 199?, s_lucky,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Lucky Dime Caper Starring Donald Duck, The  [!]", 0)
GAME( 199?, s_lionk,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Lion King, The  [!]", 0)
GAME( 199?, s_lemm,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Lemmings  [!]", 0)
GAME( 199?, s_jp2,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) James Pond 2 - Codename Robocod  [!]", GAME_NOT_WORKING ) // used to work..
GAME( 199?, s_gpride,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) GP Rider  [!]", 0)
GAME( 199?, s_jungbk,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Jungle Book, The  [!]", 0)
GAME( 199?, s_gaunt,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Gauntlet  [!]", 0)
GAME( 199?, s_gng,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ghouls 'n Ghosts  [!]", 0)

GAME( 199?, s_castil,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mickey Mouse - Castle of Illusion  [!]", 0)
GAME( 199?, s_sonic,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sonic the Hedgehog  [!]", 0)
GAME( 199?, s_sonic2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sonic the Hedgehog 2  (V1.0) [!]", 0)
GAME( 199?, s_spyspy,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Spy vs. Spy  [!]", 0)
GAME( 199?, s_suptet,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Super Tetris (Korea)", 0)
GAME( 199?, s_supko,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Kick Off  [!]", 0)
GAME( 199?, s_strid,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Strider  [!]", 0)
GAME( 199?, s_ssi,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Space Invaders  [!]", 0)
GAME( 199?, s_rrsh,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Road Rash  [!]", 0)
GAME( 199?, s_psycho,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Psycho Fox  [!]", 0)
GAME( 199?, s_tnzs,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) New Zealand Story, The  [!]", 0)

GAME( 199?, s_20em1,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) 20-em-1 (Brazil) [!]", 0)
GAME( 199?, s_aceace,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ace of Aces  [!]", GAME_NOT_WORKING ) // doesn't boot
GAME( 199?, s_actfgj,        0,        sms,    sms,    standard, ROT0,   "Sega", "(Master System) Action Fighter (J) [!]", 0)
GAME( 199?, s_aerial,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Aerial Assault  [!]", 0)
GAME( 199?, s_airesc,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Air Rescue  [!]", 0)
GAME( 199?, s_aleste,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Aleste [!]", 0)
GAME( 199?, s_alexls,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alex Kidd - The Lost Stars  [!]", 0)
GAME( 199?, s_alexbm,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alex Kidd BMX Trial  [!]", 0)
GAME( 199?, s_alexht,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alex Kidd in High Tech World  [!]", 0)
GAME( 199?, s_alf,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) ALF  [!]", 0)


GAME( 199?, s_alien3,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alien 3  [!]", 0)
GAME( 199?, s_altbea,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Altered Beast  [!]", 0)
GAME( 199?, s_ash,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Arcade Smash Hits  [!]", 0)
GAME( 199?, s_astrx,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Asterix  [!]", 0)
GAME( 199?, s_astrxa,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Asterix  [a1]", 0)
GAME( 199?, s_astgr,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Asterix and the Great Rescue  [!]", 0)
GAME( 199?, s_astsm,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Asterix and the Secret Mission  [!]", 0)
GAME( 199?, s_bttf2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Back to the Future 2  [!]", 0)
GAME( 199?, s_bttf3,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Back to the Future 3  [!]", 0)
GAME( 199?, s_baku,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Baku Baku Animals (Brazil) [!]", 0)


GAME( 199?, s_bartsm,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Bart vs. the Space Mutants  [!]", 0)
GAME( 199?, s_boutr,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Battle Out Run  [!]", 0)
GAME( 199?, s_calig,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) California Games  [!]", 0)
GAME( 199?, s_calig2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) California Games 2  [!]", GAME_NOT_WORKING ) // doesn't boot
GAME( 199?, s_coolsp,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Cool Spot  [!]", 0)
GAME( 199?, s_ddux,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dynamite Dux  [!]", 0)
GAME( 199?, s_legnil,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mickey Mouse - Legend of Illusion (Brazil) [!]", 0)
GAME( 199?, s_mspac,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ms. Pac-man  [!]", 0)
GAME( 199?, s_pmania,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Pac Mania  [!]", 0)
GAME( 199?, s_rtype,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) R-Type  [!]", 0)


GAME( 199?, s_sensi,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sensible Soccer  [!]", 0)
GAME( 199?, s_smgp2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Monaco GP 2  [!]", 0)
GAME( 199?, s_supoff,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Off-Road  [!]", 0)
GAME( 199?, s_zill,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Zillion  [!]", 0)
GAME( 199?, s_zill2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Zillion II - The Tri Formation  [!]", 0)







GAME( 199?, gg_exldz,        0,        sms,    gg,    codemagg, ROT0,   "Codemasters", "(Game Gear) Excellent Dizzy Collection (SMS2 Mode)", 0 ) // no start button
GAME( 1995, gg_bust,         0,        sms,    gg,    standagg, ROT0,   "Taito", "(Game Gear) Bust-A-Move  [!]", 0 )
GAME( 1993, gg_puzlo,        0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "(Game Gear) Puzlow Kids (J)", 0 )
GAME( 1994, gg_puyo2,        0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "(Game Gear) Puyo Puyo 2 (J) [!]", 0 )

GAME( 1995, gg_tempj,        0,        sms,    gg,    standagg, ROT0,   "Sega", "(Game Gear) Tempo Jr. (JUE) [!]", 0 )
GAME( 1993, gg_tess,         0,        sms,    gg,    standagg, ROT0,   "Gametek", "(Game Gear) Tesserae  [!]", 0 )
GAME( 1993, gg_popil,        0,        sms,    gg,    gg_popil, ROT0,   "Tengen", "(Game Gear) Magical Puzzle Popils (J)", 0 )

GAME( 1993, gg_nazo,         0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "(Game Gear) Nazo Puyo (J) [!]", 0 )
GAME( 1993, gg_nazo2,        0,        sms,    gg,    standagg, ROT0,   "Sega / Compile", "(Game Gear) Nazo Puyo 2 (J) [!]", 0 )
GAME( 1993, gg_gear,         0,        sms,    gg,    standagg, ROT0,   "Sony", "(Game Gear) Gear Works  [!]", 0 )
GAME( 1993, gg_bean,         0,        sms,    gg,    standagg, ROT0,   "Sega", "(Game Gear) Dr. Robotnik's Mean Bean Machine  [!]", 0 )
GAME( 1990, gg_cols,         0,        sms,    gg,    standagg, ROT0,   "Sega", "(Game Gear) Columns  [!]", 0 )
GAME( 1996, gg_baku,         0,        sms,    gg,    standagg, ROT0,   "Sega", "(Game Gear) Baku Baku Animals  [!]", 0 )

/* No-Intro Names */

GAME( 1900, s_alexmj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alex Kidd in Miracle World (J)", 0 )
GAME( 1900, s_alexm1,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alex Kidd in Miracle World (U) (v1.0)", 0 )
GAME( 1900, s_alexsh,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alex Kidd in Shinobi World (BUE)", 0 )
GAME( 1900, s_alsynj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Alien Syndrome (J)", 0 )
GAME( 1900, s_ambase,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) American Baseball (E)", 0 )
GAME( 1900, s_amprof,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) American Pro Football (E)", 0 )
GAME( 1900, s_anci,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ancient Ys Vanished Omen (UE)", 0 )
GAME( 1900, s_ancij,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ancient Ys Vanished Omen (J)", 0 )
GAME( 1900, s_aate,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Andre Agassi Tennis (E)", 0 )
GAME( 1900, s_anmit,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Anmitsu Hime (J)", 0 )

GAME( 1900, s_argos,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Argos no Juujiken (J)", 0 )
GAME( 1900, s_ariel,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ariel - The Little Mermaid (B)", 0 )
GAME( 1900, s_ashura,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ashura (J)", 0 )
GAME( 1900, s_assaul,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Assault City (E) (Pad)", 0 )
GAME( 1900, s_assau1,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Assault City (BE) (Light Phaser)", 0 )
GAME( 1900, s_astrof,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Astro Flash (J)", 0 )
GAME( 1900, s_awpp,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Astro Warrior & Pit Pot (E)", 0 )
GAME( 1900, s_astrow,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Astro Warrior (U)", 0 )
GAME( 1900, s_aztec,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Aztec Adventure - The Golden Road to Paradise (Nazca '88) (JUE)", 0 )
GAME( 1900, s_bankpa,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Bank Panic (E)", 0 )

GAME( 1900, s_bballn,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Basket Ball Nightmare (BE)", 0 )
GAME( 1900, s_batmr,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Batman Returns (E)", 0 )
GAME( 1900, s_batlmn,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Battlemaniacs (B)", 0 )
GAME( 1900, s_bbelt,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Black Belt (UE)", 0 )
GAME( 1900, s_bladee,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Blade Eagle (Blade Eagle 3-D) (JUE)", 0 )
GAME( 1900, s_bomber,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Bomber Raid (JUE)", 0 )
GAME( 1900, s_bnzabr,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Bonanza Bros. (E)", 0 )
GAME( 1900, s_bonker,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Bonkers Wax Up! (B)", 0 )
GAME( 1900, s_drac,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Bram Stoker's Dracula (E)", 0 )
GAME( 1900, s_buggy,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Buggy Run (BE)", 0 )

GAME( 1900, s_capsil,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Captain Silver (JE)", 0 )
GAME( 1900, s_capsiu,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Captain Silver (U)", 0 )
GAME( 1900, s_casino,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Casino Games (UE)", 0 )
GAME( 1900, s_castel,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Castelo Ra-Tim-Bum (B)", 0 )
GAME( 1900, s_cillu,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Castle of Illusion Starring Mickey Mouse (U)", 0 )
GAME( 1900, s_champe,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Champions of Europe (BE)", 0 )
GAME( 1900, s_champh,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Championship Hockey (E)", 0 )
GAME( 1900, s_chapol,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Chapolim x Dracula - Um Duelo Assustador (B)", 0 )
GAME( 1900, s_cheese,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Cheese Cat-astrophe Starring Speedy Gonzales' (E) (M4)", 0 )
GAME( 1900, s_chopjp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Choplifter (J) (Proto)", 0 )

GAME( 1900, s_chopl,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Choplifter (UE)", 0 )
GAME( 1900, s_chouon,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Chouon Senshi Borgman (J)", 0 )
GAME( 1900, s_chck2b,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Chuck Rock II - Son of Chuck (B)", 0 )
GAME( 1900, s_circui,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Circuit, The (J)", 0 )
GAME( 1900, s_cloudm,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Cloud Master (UE)", 0 )
GAME( 1900, s_cmj,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Comical Machinegun Joe (J)", 0 )
GAME( 1900, s_cybers,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Cyber Shinobi, The - Shinobi Part 2 (E)", 0 )
GAME( 1900, s_cyborg,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Cyborg Hunter (BUE)", 0 )
GAME( 1900, s_daffy,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Daffy Duck in Hollywood (E) (M5)", 0 )
GAME( 1900, s_danan,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Danan - The Jungle Fighter (E)", 0 )

GAME( 1900, s_deadan,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dead Angle (UE)", 0 )
GAME( 1900, s_deepd,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Deep Duck Trouble Starring Donald Duck (E)", 0 )
GAME( 1900, s_dspeed,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Desert Speedtrap Starring Road Runner and Wile E. Coyote (BE) (M5)", 0 )
GAME( 1900, s_dstrik,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Desert Strike (E) (M4)", 0 )
GAME( 1900, s_dicktr,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dick Tracy (UE)", 0 )
GAME( 1900, s_plandj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dokidoki Penguin Land - Uchuu Daibouken (J)", 0 )
GAME( 1900, s_ddr,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Double Dragon (UE)", 0 )
GAME( 1900, s_dhawk,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Double Hawk (E)", 0 )
GAME( 1900, s_dbltar,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Double Target - Cynthia no Nemuri (J)", 0 )
GAME( 1900, s_blee,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dragon - The Bruce Lee Story (E)", 0 )

GAME( 1900, s_dcrys,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dragon Crystal (E)", 0 )
GAME( 1900, s_dduke,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dynamite Duke (E)", 0 )
GAME( 1900, s_dhead,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Dynamite Headdy (B)", 0 )
GAME( 1900, s_ejim,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Earthworm Jim (B)", 0 )
GAME( 1900, s_ecco2,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ecco - The Tides of Time (B)", 0 )
GAME( 1900, s_ecco,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ecco the Dolphin (E)", 0 )
GAME( 1900, s_enduro,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Enduro Racer (UE)", 0 )
GAME( 1900, s_endurj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Enduro Racer (J)", 0 )
GAME( 1900, s_eswat,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) E-SWAT - City Under Siege (BUE) (Easy Version)", 0 )
GAME( 1900, s_eswat1,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) E-SWAT - City Under Siege (BUE) (Hard Version)", 0 )
GAME( 1900, s_exdzp,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Excellent Dizzy Collection, The (UE) (M3) (Proto)", 0 )



GAME( 1900, s_f1,             0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) F1 (E)", 0 )
GAME( 1900, s_f16,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) F-16 Fighter (UE)", 0 )
GAME( 1900, s_f16fj,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) F-16 Fighting Falcon (J)", 0 )
GAME( 1900, s_f16fu,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) F-16 Fighting Falcon (U)", 0 )
GAME( 1900, s_fzon2j,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Fantasy Zone II - The Tears of OPA-OPA (J)", 0 )
GAME( 1900, s_ferias,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ferias Frustradas do Pica-Pau (B)", 0 )
GAME( 1900, s_fifa,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Fifa International Soccer (B) (M3)", 0 )
GAME( 1900, s_fbubbo,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Final Bubble Bobble (J)", 0 )
GAME( 1900, s_fire,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Fire & Forget II (E)", 0 )
GAME( 1900, s_fireic,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Fire & Ice (B)", 0 )

GAME( 1900, s_flash,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Flash, The (E)", 0 )
GAME( 1900, s_forgot,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Forgotten Worlds (E)", 0 )
GAME( 1900, s_pitpoj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Fushigi no Oshiro Pit Pot (J)", 0 )
GAME( 1900, s_gain,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Gain Ground (E)", 0 )
GAME( 1900, s_galac,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Galactic Protector (J)", 0 )
GAME( 1900, s_gforc,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Galaxy Force (BE)", 0 )
GAME( 1900, s_gforcu,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Galaxy Force (U)", 0 )
GAME( 1900, s_gbox,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Game Box Esportes Radicais (B)", 0 )
GAME( 1900, s_gangst,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Gangster Town (BUE)", 0 )
GAME( 1900, s_gfko,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) George Foreman's KO Boxing (E)", 0 )

GAME( 1900, s_gerald,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Geraldinho (B)", 0 )
GAME( 1900, s_ghous,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ghost House (BUE)", 0 )
GAME( 1900, s_ghousj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ghost House (J)", 0 )
GAME( 1900, s_gbus,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ghostbusters (UE)", 0 )
GAME( 1900, s_gdefp,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Global Defense (UE) (Proto)", 0 )
GAME( 1900, s_gdef,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Global Defense (UE)", 0 )
GAME( 1900, s_gloc,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) G-Loc Air Battle (BE)", 0 )
GAME( 1900, s_dumpm,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Gokuaku Doumei Dump Matsumoto (J)", 0 )
GAME( 1900, s_gaxe,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Golden Axe (UE)", 0 )
GAME( 1900, s_gaxew,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Golden Axe Warrior (BUE)", 0 )

GAME( 1900, s_golfa,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Golfamania (E)", 0 )
GAME( 1900, s_golfap,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Golfamania (UE) (Proto)", 0 )
GAME( 1900, s_golvej,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Golvellius (J)", 0 )
GAME( 1900, s_golve,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Golvellius (UE)", 0 )
GAME( 1900, s_gbasej,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Baseball (J)", 0 )
GAME( 1900, s_gbase,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Baseball (UE)", 0 )
GAME( 1900, s_gbask,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Basketball (UE)", 0 )
GAME( 1900, s_gfoot,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Football (UE)", 0 )
GAME( 1900, s_ggolf,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Golf (J)", 0 )
GAME( 1900, s_ggolmp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Golf (Masters Golf) (JUE) (Proto)", 0 )

GAME( 1900, s_ggolm,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Golf (Masters Golf) (JUE)", 0 )
GAME( 1900, s_gice,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Ice Hockey (U)", 0 )
GAME( 1900, s_gsoc,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Soccer (E)", 0 )
GAME( 1900, s_gsocj,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Soccer (J)", 0 )
GAME( 1900, s_gsocws,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Soccer (World Soccer) (JBUE)", 0 )
GAME( 1900, s_gten,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Tennis [Super Tennis] (J)", 0 )
GAME( 1900, s_gvolj,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Volleyball (J)", 0 )
GAME( 1900, s_gvol,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Great Volleyball (UE)", 0 )
GAME( 1900, s_haja,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Haja no Fuuin (J)", 0 )
GAME( 1900, s_hoaw,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Hang-On & Astro Warrior (U)", 0 )

GAME( 1900, s_hosh,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Hang-On & Safari Hunt (U)", 0 )
GAME( 1900, s_hango,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Hang-On (BE)", 0 )
GAME( 1900, s_hangoj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Hang-On (J)", 0 )
GAME( 1900, s_hwchm,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Heavyweight Champ (E)", 0 )
GAME( 1900, s_hlance,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Heroes of the Lance (E)", 0 )
GAME( 1900, s_hskim,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) High School! Kimengumi (J)", 0 )
GAME( 1900, s_hokuto,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Hokuto no Ken (J)", 0 )
GAME( 1900, s_homeal,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Home Alone (E)", 0 )
GAME( 1900, s_hookp,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Hook (E) (Proto)", 0 )
GAME( 1900, s_hoshi,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Hoshi wo Sagasite (J)", 0 )

GAME( 1900, s_imiss,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Impossible Mission (E)", 0 )
GAME( 1900, s_icd,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Incredible Crash Dummies, The (E)", 0 )
GAME( 1900, s_hulk,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Incredible Hulk, The (E)", 0 )
GAME( 1900, s_indlc,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Indiana Jones and the Last Crusade (E)", 0 )
GAME( 1900, s_007b,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) James Bond 007 - The Duel (B)", 0 )
GAME( 1900, s_007,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) James Bond 007 - The Duel (E)", 0 )
GAME( 1900, s_jbdkb,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) James 'Buster' Douglas Knockout Boxing (U)", 0 )
GAME( 1900, s_joemon,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Joe Montana Football (UE)", 0 )
GAME( 1900, s_jogos2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Jogos de Verao II [California Games II] (B)", 0 )
GAME( 1900, s_jpark,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Jurassic Park (E)", 0 )

GAME( 1900, s_kensj,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Kenseiden (J)", 0 )
GAME( 1900, s_kens,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Kenseiden (UE)", 0 )
GAME( 1900, s_kquesp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) King's Quest - Quest for the Crown (U) (Proto)", 0 )
GAME( 1900, s_kques,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) King's Quest - Quest for the Crown (U)", 0 )
GAME( 1900, s_klax,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Klax (E)", 0 )
GAME( 1900, s_krust,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Krusty's Fun House (E)", 0 )
GAME( 1900, s_kujaku,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Kujaku Ou (J)", 0 )
GAME( 1900, s_kungfu,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Kung Fu Kid (UE)", 0 )
GAME( 1900, s_lghos,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Laser Ghost (E)", 0 )
GAME( 1900, s_lemp,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Lemmings (E) (Proto)", 0 )

GAME( 1900, s_lof,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Line of Fire (E)", 0 )
GAME( 1900, s_lordsj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Lord of Sword (J)", 0 )
GAME( 1900, s_lords,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Lord of the Sword (UE)", 0 )
GAME( 1900, s_loret,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Loretta no Shouzou (J)", 0 )
GAME( 1900, s_luckp,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Lucky Dime Caper Starring Donald Duck, The (BE) (Proto)", 0 )
GAME( 1900, s_mahsjp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mahjong Sengoku Jidai (J) (Proto)", 0 )
GAME( 1900, s_mahsj,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mahjong Sengoku Jidai (J)", 0 )
GAME( 1900, s_makai,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Makai Retsuden (J)", 0 )
GAME( 1900, s_marble,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Marble Madness (E)", 0 )
GAME( 1900, s_marks,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Marksman Shooting & Trap Shooting (BU)", 0 )

GAME( 1900, s_mastd,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Master of Darkness (E)", 0 )
GAME( 1900, s_mastc,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Masters of Combat (E)", 0 )
GAME( 1900, s_maze3d,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Maze Hunter 3-D (UE)", 0 )
GAME( 1900, s_mazew,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Maze Walker (J)", 0 )
GAME( 1900, s_megumi,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Megumi Rescue (J)", 0 )
GAME( 1900, s_mercs,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mercs (E)", 0 )
GAME( 1900, s_mjmw,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Michael Jackson's Moonwalker (UE)", 0 )
GAME( 1900, s_mmack,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mick & Mack as the Global Gladiators (E)", 0 )
GAME( 1900, s_multc,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mickey's Ultimate Challenge (B)", 0 )
GAME( 1900, s_miracl,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Miracle Warriors - Seal of the Dark Land (UE)", 0 )

GAME( 1900, s_md3d,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Missile Defense 3-D (UE)", 0 )
GAME( 1900, s_monica,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Monica no Castelo do Dragao (B)", 0 )
GAME( 1900, s_monop,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mono Poly (U)", 0 )
GAME( 1900, s_monope,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Monopoly (UE)", 0 )
GAME( 1900, s_montez,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Montezuma's Revenge (U)", 0 )
GAME( 1900, s_mk3,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mortal Kombat 3 (B)", 0 )
GAME( 1900, s_mk,             0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mortal Kombat (E)", 0 )
GAME( 1900, s_mk2,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Mortal Kombat II (E)", 0 )
GAME( 1900, s_myhero,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) My Hero (BUE)", 0 )
GAME( 1900, s_nekky,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Nekkyuu Koushien (J)", 0 )

GAME( 1900, s_ngaidp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ninja Gaiden (BE) (Proto)", 0 )
GAME( 1900, s_ngaid,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ninja Gaiden (BE)", 0 )
GAME( 1900, s_ninjaj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ninja, The (J)", 0 )
GAME( 1900, s_ninja,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ninja, The (UE)", 0 )
GAME( 1900, s_opaopa,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Opa Opa (J)", 0 )
GAME( 1900, s_opwolf,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Operation Wolf (E)", 0 )
GAME( 1900, s_otti,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ottifants, The (E) (M5)", 0 )
GAME( 1900, s_or3d,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Out Run 3-D (BE)", 0 )
GAME( 1900, s_orun,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Out Run (UE)", 0 )
GAME( 1900, s_orune,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Out Run Europa (E)", 0 )

GAME( 1900, s_papb,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Paperboy (E)", 0 )
GAME( 1900, s_papbu,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Paperboy (U)", 0 )
GAME( 1900, s_parl,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Parlour Games (UE)", 0 )
GAME( 1900, s_party,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Party Games (J)", 0 )
GAME( 1900, s_patrip,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Pat Riley Basketball (U) (Proto)", 0 )
GAME( 1900, s_pland,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Penguin Land (UE)", 0 )
GAME( 1900, s_pga,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) PGA Tour Golf (E)", 0 )
GAME( 1900, s_pstb,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Phantasy Star (B)", 0 )
GAME( 1900, s_pstmd,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Phantasy Star (J) (Mega Drive)", 0 )
GAME( 1900, s_pstj,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Phantasy Star (J)", 0 )

GAME( 1900, s_pstk,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Phantasy Star (K)", 0 )
GAME( 1900, s_pst12,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Phantasy Star (UE) (v1.2)", 0 )
GAME( 1900, s_pst13,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Phantasy Star (UE) (v1.3)", 0 )
GAME( 1900, s_pfigb,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Pit Fighter - The Ultimate Challenge (B)", 0 )
GAME( 1900, s_pfig,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Pit Fighter - The Ultimate Challenge (E)", 0 )
GAME( 1900, s_popu,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Populous (BE)", 0 )
GAME( 1900, s_pw3d,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Poseidon Wars 3-D (UE)", 0 )
GAME( 1900, s_pstrk,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Power Strike (UE)", 0 )
GAME( 1900, s_pstrk2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Power Strike II (E)", 0 )
GAME( 1900, s_pred2,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Predator 2 (E)", 0 )

GAME( 1900, s_pop,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Prince of Persia (BE)", 0 )
GAME( 1900, s_prow,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Pro Wrestling (UE)", 0 )
GAME( 1900, s_proyak,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Pro Yakyuu Pennant Race, The (J)", 0 )
GAME( 1900, s_promo3,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Promocao Especial M. System III Compact (B) (Promo)", 0 )
GAME( 1900, s_psyc,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Psychic World (E)", 0 )
GAME( 1900, s_putt,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Putt & Putter (BE)", 0 )
GAME( 1900, s_puttp,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Putt & Putter (E) (Proto)", 0 )
GAME( 1900, s_quart,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Quartet (UE)", 0 )
GAME( 1900, s_quest,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Quest for the Shaven Yak Starring Ren Hoek & Stimpy (B)", 0 )


GAME( 1900, s_rcgp,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) R.C. Grand Prix (UE)", 0 )
GAME( 1900, s_rainbb,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rainbow Islands - The Story of Bubble Bobble 2 (B)", 0 )
GAME( 1900, s_rainb,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rainbow Islands - The Story of Bubble Bobble 2 (E)", 0 )
GAME( 1900, s_rambo,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rambo - First Blood Part II (U)", 0 )
GAME( 1900, s_rambo3,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rambo III (UE)", 0 )
GAME( 1900, s_rampag,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rampage (BUE)", 0 )
GAME( 1900, s_rampar,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rampart (E)", 0 )
GAME( 1900, s_rastan,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rastan (BUE)", 0 )
GAME( 1900, s_reggie,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Reggie Jackson Baseball (U)", 0 )
GAME( 1900, s_renega,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Renegade (E)", 0 )

GAME( 1900, s_rescue,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rescue Mission (UE)", 0 )
GAME( 1900, s_roboc3,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) RoboCop 3 (E)", 0 )
GAME( 1900, s_roboct,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) RoboCop versus The Terminator (BE)", 0 )
GAME( 1900, s_rocky,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Rocky (UE)", 0 )
GAME( 1900, s_runbat,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Running Battle (E)", 0 )
GAME( 1900, s_sagaia,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sagaia (E)", 0 )
GAME( 1900, s_sango3,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sangokushi 3 (K) (Unl)", 0 )
GAME( 1900, s_sxom,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sapo Xule - O Mestre do Kung Fu (B)", 0 )
GAME( 1900, s_sxsos,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sapo Xule - SOS Lagoa Poluida (B)", 0 )
GAME( 1900, s_sxvs,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sapo Xule vs. Os Invasores do Brejo (B)", 0 )

GAME( 1900, s_sat7,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Satellite 7 (J)", 0 )
GAME( 1900, s_scram,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Scramble Spirits (E)", 0 )
GAME( 1900, s_sdi,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) SDI (J)", 0 )
GAME( 1900, s_secret,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Secret Command (E)", 0 )
GAME( 1900, s_sches,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sega Chess (E)", 0 )
GAME( 1900, s_wtgolf,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sega World Tournament Golf (E)", 0 )
GAME( 1900, s_seishu,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Seishun Scandal (J)", 0 )
GAME( 1900, s_shdanc,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Shadow Dancer (E)", 0 )
GAME( 1900, s_shbeas,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Shadow of the Beast (E)", 0 )
GAME( 1900, s_shangh,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Shanghai (UE)", 0 )

GAME( 1900, s_shinoj,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Shinobi (J)", 0 )
GAME( 1900, s_shino,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Shinobi (UE)", 0 )
GAME( 1900, s_shgal,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Shooting Gallery (UE)", 0 )
GAME( 1900, s_simpbw,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Simpsons, The - Bart vs. The World (E)", 0 )
GAME( 1900, s_sitio,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sitio do Picapau Amarelo (B)", 0 )
GAME( 1900, s_slapsa,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Slap Shot (E) [a]", 0 )
GAME( 1900, s_slaps,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Slap Shot (E)", 0 )
GAME( 1900, s_smurf2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Smurfs 2, The (E) (M4)", 0 )
GAME( 1900, s_smurf,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Smurfs, The (E) (M4)", 0 )
GAME( 1900, s_solomo,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Solomon no Kagi - Oujo Rihita no Namida (J)", 0 )

GAME( 1900, s_sonbls,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sonic Blast (B)", 0 )
GAME( 1900, s_soncha,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sonic Chaos (E)", 0 )
GAME( 1900, s_sonspi,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sonic Spinball (E)", 0 )
GAME( 1900, s_son211,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sonic the Hedgehog 2 (E) (v1.1)", 0 )
GAME( 1900, s_spacg,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Space Gun (E)", 0 )
GAME( 1900, s_sh3d,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Space Harrier 3-D (BUE)", 0 )
GAME( 1900, s_sh3dj,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Space Harrier 3D (J)", 0 )
GAME( 1900, s_shj,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Space Harrier (J)", 0 )
GAME( 1900, s_sh,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Space Harrier (UE)", 0 )
GAME( 1900, s_scib,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Special Criminal Investigation (E) (Beta)", 0 )

GAME( 1900, s_sci,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Special Criminal Investigation (E)", 0 )
GAME( 1900, s_sb2,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Speedball 2 (E)", 0 )
GAME( 1900, s_sbm,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Speedball (E) (Mirrorsoft)", 0 )
GAME( 1900, s_sb,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Speedball (E) (Virgin)", 0 )
GAME( 1900, s_spell,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Spellcaster (UE)", 0 )
GAME( 1900, s_smrss,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Spider-Man - Return of the Sinister Six (E)", 0 )
GAME( 1900, s_smvsk,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Spider-Man vs. The Kingpin (UE)", 0 )
GAME( 1900, s_spfoot,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sports Pad Football (U)", 0 )
GAME( 1900, s_spsocc,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sports Pad Soccer (J)", 0 )
GAME( 1900, s_spyspj,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Spy vs. Spy (J)", 0 )

GAME( 1900, s_spyspd,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Spy vs. Spy (U) (Display-Unit Cart)", 0 )
GAME( 1900, s_starw,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Star Wars (E)", 0 )
GAME( 1900, s_sf2,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Street Fighter II (B)", 0 )
GAME( 1900, s_sora,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Streets of Rage (E)", 0 )
GAME( 1900, s_sor2,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Streets of Rage II (E)", 0 )
GAME( 1900, s_strid2,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Strider II (E)", 0 )
GAME( 1900, s_subatt,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Submarine Attack (E)", 0 )
GAME( 1900, s_suke2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Sukeban Deka 2 (J)", 0 )
GAME( 1900, s_sumgam,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Summer Games (E)", 0 )
GAME( 1900, s_supbbd,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Basketball (U) (Demo)", 0 )

GAME( 1900, s_smgpu,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Monaco GP (U)", 0 )
GAME( 1900, s_suprac,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Racing (J)", 0 )
GAME( 1900, s_sstv,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Smash T.V. (E)", 0 )
GAME( 1900, s_supten,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Tennis (UE)", 0 )
GAME( 1900, s_supwbm,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Wonder Boy - Monster World (J)", 0 )
GAME( 1900, s_supwb,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Super Wonder Boy (J)", 0 )
GAME( 1900, s_supman,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Superman (E)", 0 )
GAME( 1900, s_t2,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) T2 - The Arcade Game (E)", 0 )
GAME( 1900, s_chashq,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Taito Chase H.Q. (E)", 0 )
GAME( 1900, s_tazmar,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Taz in Escape from Mars (B)", 0 )

GAME( 1900, s_taza,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Taz Mania (BE)", 0 )
GAME( 1900, s_twc92,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Tecmo World Cup '92 (E) (Proto)", 0 )
GAME( 1900, s_twc93,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Tecmo World Cup '93 (E)", 0 )
GAME( 1900, s_tbb,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Teddy Boy (BUE)", 0 )
GAME( 1900, s_tbbmc,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Teddy Boy Blues (J) (Ep-MyCard) (Proto)", 0 )
GAME( 1900, s_tbbj,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Teddy Boy Blues (J)", 0 )
GAME( 1900, s_tenace,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Tennis Ace (BE)", 0 )
GAME( 1900, s_tensai,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Tensai Bakabon (J)", 0 )
GAME( 1900, s_term2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Terminator 2 - Judgment Day (E)", 0 )
GAME( 1900, s_term,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Terminator, The (E)", 0 )

GAME( 1900, s_tbladj,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Thunder Blade (J)", 0 )
GAME( 1900, s_tblad,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Thunder Blade (UE)", 0 )
GAME( 1900, s_tsol,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Time Soldiers (UE)", 0 )
GAME( 1900, s_tomjp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Tom & Jerry (E) (Proto)", 0 )
GAME( 1900, s_tomjmv,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Tom and Jerry - The Movie (E)", 0 )
GAME( 1900, s_totow3,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Toto World 3 (K) (Unl)", 0 )
GAME( 1900, s_tranbt,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) TransBot (BUE)", 0 )
GAME( 1900, s_trapsh,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Trap Shooting & Marksman Shooting & Safari Hunt (E)", 0 )
GAME( 1900, s_trivia,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Trivial Pursuit (E) (M4)", 0 )
GAME( 1900, s_turma,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Turma da Monica em O Resgate (B)", 0 )

GAME( 1900, s_tvcol,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) TV Colosso (B)", 0 )
GAME( 1900, s_ult4p,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ultima IV - Quest of the Avatar (E) (Proto)", 0 )
GAME( 1900, s_ult4,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ultima IV - Quest of the Avatar (E)", 0 )
GAME( 1900, s_ultsoc,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Ultimate Soccer (E) (M5)", 0 )
GAME( 1900, s_vampp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Vampire (UE) (Proto)", 0 )
GAME( 1900, s_vigil,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Vigilante (UE)", 0 )
GAME( 1900, s_vfanim,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Virtua Fighter Animation (B)", 0 )
GAME( 1900, s_walter,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Walter Payton Football (U)", 0 )
GAME( 1900, s_wanted,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wanted (UE)", 0 )
GAME( 1900, s_wwcsb,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Where in the World is Carmen Sandiego (B)", 0 )

GAME( 1900, s_wwcs,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Where in the World is Carmen Sandiego (U)", 0 )
GAME( 1900, s_wimb,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wimbledon (E)", 0 )
GAME( 1900, s_wimb2,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wimbledon II (E)", 0 )
GAME( 1900, s_win94b,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Winter Olympics - Lillehammer '94 (B) (M8)", 0 )
GAME( 1900, s_win94,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Winter Olympics - Lillehammer '94 (E) (M8)", 0 )
GAME( 1900, s_wolfc,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wolfchild (E)", 0 )
GAME( 1900, s_wboy,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wonder Boy (BUE)", 0 )
GAME( 1900, s_wbml,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wonder Boy in Monster Land (UE)", 0 )
GAME( 1900, s_wbmwp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wonder Boy in Monster World (E) (Proto)", 0 )
GAME( 1900, s_wbmw,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Wonder Boy in Monster World (E)", 0 )

GAME( 1900, s_wclb,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) World Class Leader Board (E)", 0 )
GAME( 1900, s_wc90,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) World Cup Italia '90 (BE)", 0 )
GAME( 1900, s_wc94,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) World Cup USA 94 (E) (M8)", 0 )
GAME( 1900, s_wgamp,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) World Games (BE) (Proto)", 0 )
GAME( 1900, s_wgam,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) World Games (BE)", 0 )
GAME( 1900, s_wgp,           0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) World Grand Prix (E)", 0 )
GAME( 1900, s_wgpu,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) World Grand Prix (U)", 0 )
GAME( 1900, s_wwfsc,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) WWF Wrestlemania - Steel Cage Challenge (BE)", 0 )
GAME( 1900, s_x2iw,          0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Xenon 2 - Megablast (E) (Image Works)", 0 )
GAME( 1900, s_x2,            0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Xenon 2 - Megablast (E) (Virgin)", 0 )

GAME( 1900, s_xmenmj,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) X-Men - Mojo World (B)", 0 )
GAME( 1900, s_zax3dp,        0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Zaxxon 3-D (JUE) (Proto)", 0 )
GAME( 1900, s_zax3d,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Zaxxon 3-D (JUE)", 0 )
GAME( 1900, s_zilli,         0,        sms,    sms,    standpal, ROT0,   "Sega", "(Master System) Zillion (Akai Koudan Zillion) (JE) (M2)", 0 )


















#ifdef HAZEMD
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
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp1->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}

	}

	for (y=0;y<192;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp2->r_bitmap, y, 0);

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

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(0) // Vblank handled manually.
	MDRV_MACHINE_RESET(systeme)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)

	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 223)


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
	init_segasyse(machine);

	memory_install_read8_handler(0, ADDRESS_SPACE_IO, 0xf8, 0xf8, 0, 0, segae_ridleofp_port_f8_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xfa, 0xfa, 0, 0, segae_ridleofp_port_fa_w);
}


static DRIVER_INIT( hangonjr )
{
	init_segasyse(machine);

	memory_install_read8_handler(0, ADDRESS_SPACE_IO, 0xf8, 0xf8, 0, 0, segae_hangonjr_port_f8_r);
	memory_install_write8_handler(0, ADDRESS_SPACE_IO, 0xfa, 0xfa, 0, 0, segae_hangonjr_port_fa_w);
}

GAME( 1985, hangonjr, 0,        systeme, hangonjr, hangonjr, ROT0,  "Sega", "(Arcade, System E) Hang-On Jr.", 0 )
GAME( 1986, transfrm, 0,        systeme, transfrm, segasyse, ROT0,  "Sega", "(Arcade, System E) Transformer ", 0 )
GAME( 1986, ridleofp, 0,        systeme, ridleofp, ridleofp, ROT90, "Sega / Nasco", "(Arcade, System E) Riddle of Pythagoras (Japan)", 0 )
GAME( 1988, tetrisse, 0,        systeme, tetrisse, segasyse, ROT0,  "Sega", "(Arcade, System E) Tetris (Japan, System E)", 0 )
#endif /* HAZEMD */

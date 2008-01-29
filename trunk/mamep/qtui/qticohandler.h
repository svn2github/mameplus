/****************************************************************************
**
** Copyright (C) 2003-2006 Trolltech AS. All rights reserved.
**
** This file is part of a Qt Solutions component.
**
** Licensees holding valid Qt Solutions licenses may use this file in
** accordance with the Qt Solutions License Agreement provided with the
** Software.
**
** See http://www.trolltech.com/products/solutions/index.html 
** or email sales@trolltech.com for information about Qt Solutions
** License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QTICOHANDLER_H
#define QTICOHANDLER_H

#include <QtGui/QImageIOHandler>

// These next two structs represent how the icon information is stored
// in an ICO file.
typedef struct
{
    quint8	bWidth;               // Width of the image
    quint8	bHeight;              // Height of the image (times 2)
    quint8	bColorCount;          // Number of colors in image (0 if >=8bpp) [ not ture ]
    quint8	bReserved;            // Reserved
    quint16	wPlanes;              // Color Planes
    quint16	wBitCount;            // Bits per pixel
    quint32	dwBytesInRes;         // how many bytes in this resource?
    quint32	dwImageOffset;        // where in the file is this image
} ICONDIRENTRY, *LPICONDIRENTRY;
#define ICONDIRENTRY_SIZE 16

typedef struct
{
    quint16	idReserved;   // Reserved
    quint16	idType;       // resource type (1 for icons)
    quint16	idCount;      // how many images?
    ICONDIRENTRY	idEntries[1]; // the entries for each image
} ICONDIR, *LPICONDIR;

#define ICONDIR_SIZE    6       // Exclude the idEntries field

typedef struct {				// BMP information header
    quint32  biSize;				// size of this struct
    quint32  biWidth;				// pixmap width
    quint32  biHeight;				// pixmap height
    quint16  biPlanes;				// should be 1
    quint16  biBitCount;			// number of bits per pixel
    quint32  biCompression;			// compression method
    quint32  biSizeImage;				// size of image
    quint32  biXPelsPerMeter;			// horizontal resolution
    quint32  biYPelsPerMeter;		// vertical resolution
    quint32  biClrUsed;				// number of colors used
    quint32  biClrImportant;			// number of important colors
} BMP_INFOHDR ,*LPBMP_INFOHDR;
#define BMP_INFOHDR_SIZE 40


class ICOReader
{
public:
    ICOReader(QIODevice * iodevice);
    int count();
    QImage iconAt(int index);
    static bool canRead(QIODevice *iodev);

    static QList<QImage> read(QIODevice * device);

    static bool write(QIODevice * device, const QList<QImage> & images);

private:
    bool readHeader();
    bool readIconEntry(int index, ICONDIRENTRY * iconEntry);

    bool readBMPHeader(ICONDIRENTRY & iconEntry, BMP_INFOHDR * header);
    void findColorInfo(QImage & image);
    void readColorTable(QImage & image);
    
    void readBMP(QImage & image);
    void read1BitBMP(QImage & image);
    void read4BitBMP(QImage & image);
    void read8BitBMP(QImage & image);
    void read16_24_23BMP(QImage & image);

    struct IcoAttrib
    {
        int nbits;
        int ncolors;
        int h;
        int w;
        int red_mask, green_mask, blue_mask;
        int red_shift, green_shift, blue_shift;
        int red_scale, green_scale, blue_scale;
        int depth;
    } icoAttrib;

    QIODevice * iod;
    qint64 startpos;
    bool headerRead;
    ICONDIR iconDir;

};

class QtIcoHandler: public QImageIOHandler
{
public:
    QtIcoHandler(QIODevice *device);
    virtual ~QtIcoHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;
    
    int imageCount() const;
    bool jumpToImage(int imageNumber);
    bool jumpToNextImage();
    
    static bool canRead(QIODevice *device);
    
private:
    int m_currentIconIndex;
    ICOReader *m_pICOReader;

};

#endif /* QTICOHANDLER_H */


#ifndef _MAMEPGUITYPES_H_
#define _MAMEPGUITYPES_H_

#include <QtGui>

class BiosSet : public QObject
{
public:
	QString description;
	bool isDefault;

	BiosSet(QObject *parent = 0);
};

class RomInfo : public QObject
{
public:
	QString name;
	QString bios;
	quint64 size;
	//quint32 crc is the key
	//md5
	//sha1
	QString merge;
	QString region;
	//offset
	QString status;
	//dispose

	/* internal */
	bool available;

	RomInfo(QObject *parent = 0);
};

class DiskInfo : public QObject
{
public:
	QString name;
	//md5
	//QString sha1 is the key
	QString merge;
	QString region;
	quint8 index;
	QString status;
	//dispose

	/* internal */
	bool available;

	DiskInfo(QObject *parent = 0);
};

class ChipInfo : public QObject
{
public:
	QString name;
	QString tag;
	QString type;
	quint32 clock;

	ChipInfo(QObject *parent = 0);
};

class DisplayInfo : public QObject
{
public:
	QString type;
	QString rotate;
	bool flipx;
	quint16 width;
	quint16 height;
	QString refresh;
//	int pixclock;
	quint16 htotal;
	quint16 hbend;
	quint16 hbstart;
	quint16 vtotal;
	quint16 vbend;
	quint16 vbstart;

	DisplayInfo(QObject *parent = 0);
};

class ControlInfo : public QObject
{
public:
	QString type;
	quint16 minimum;
	quint16 maximum;
	quint16 sensitivity;
	quint16 keydelta;
	bool reverse;

	ControlInfo(QObject *parent = 0);
};

class DeviceInfo : public QObject
{
public:
	QString type;
	QString tag;
	bool mandatory;
	bool isConst;
	QString mountedPath;

//	QString instanceName is the key
	QStringList extensionNames;

	DeviceInfo(QObject *parent = 0);
};

class TreeItem;
class GameInfo : public QObject
{
public:
	/* game */
	QString sourcefile;
	bool isBios;
//	bool runnable;
	QString cloneof;
	QString romof;
	QString sampleof;
	QString description;
	QString year;
	QString manufacturer;

	/* biosset */
	QHash<QString /*name*/, BiosSet *> biosSets;

	/* rom */
	QMultiHash<quint32 /*crc*/, RomInfo *> roms;

	/* disk */
	QHash<QString /*sha1*/, DiskInfo *> disks;

	/* sample */
	QStringList samples;
	
	/* chip */
	QList<ChipInfo *> chips;

	/* display */
	QList<DisplayInfo *> displays;

	/* sound */
	quint8 channels;

	/* input */
	bool service;
	bool tilt;
	quint8 players;
	quint8 buttons;
	quint8 coins;
	QList<ControlInfo *> controls;

	//dipswitch 

	/* driver, impossible for a game to have multiple drivers */
	quint8 status;
	quint8 emulation;
	quint8 color;
	quint8 sound;
	quint8 graphic;
	quint8 cocktail;
	quint8 protection;
	quint8 savestate;
	quint32 palettesize;

	/* device */
	QMap<QString /* instanceName */, DeviceInfo *> devices;

	/*ramoption */
	QList<quint32> ramOptions;
	quint32 defaultRamOption;

	/* extension */
	QByteArray extraInfo;

	/* updater only */
	QString url;
	qint64 size;
	quint32 crc;
	QString directory;
	QString filter;	//only for filenames, no paths

	/* internal */
	QString lcDesc;
	QString lcMftr;
	QString reading;

	bool isExtRom;
	bool isHorz;
	bool isMechanical;
	bool isGamble;

	qint8 available;
	QByteArray icondata;
	TreeItem *pModItem;
	QSet<QString> clones;

	GameInfo(QObject *parent = 0);
	~GameInfo();

	QString biosof();
	DeviceInfo *getDevice(QString type, int = 0);
	QString getDeviceInstanceName(QString type, int = 0);
};

class MameDat : public QObject
{
Q_OBJECT

public:
	QString defaultIni;
	QString mameVersion;
	QHash<QString, GameInfo *> games;

	MameDat(QObject * = 0, int = 0);
	MameDat(const QByteArray&);
	int load();
	void save();
	int completeData();

private:
	QProcess *loadProc;
	int numTotalGames;
	QString mameOutputBuf;

	void parseListXml(int = 0);

private slots:
	// external process management
	void loadListXmlReadyReadStandardOutput();
	void loadListXmlFinished(int, QProcess::ExitStatus);
	void loadDefaultIniReadyReadStandardOutput();
	void loadDefaultIniFinished(int, QProcess::ExitStatus);
};

extern MameDat *pMameDat;
extern MameDat *pFixDat;
extern MameDat *pTempDat;

#endif

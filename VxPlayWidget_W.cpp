#include "stdafx.h"
#include "VxPlayWidget_W.h"

#ifdef Q_OS_WIN
#include <WinSock.h>
#include <sys/stat.h>
#include <io.h>
#endif

#include <algorithm>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include "monitorWidget.h"
#include "VxPlayViewAPI_X.h"
#include "VxPlayButtonController_W.h"
#include "VxPlayViewController_X.h"

#include "inc_UiCtrl/VxPlayRuler.h"
#include "inc_UiCtrl/VxAudioIndicatorWnd.h"
#include "inc_UiLib/VxTimeCodeEdit.h"
#include "inc_UiLib/VxButtonItem.h"
#include "inc_CommonLib/turbo_edit_msg.h"
#include "inc_CommonLib/customEvent.h"
#include "inc_CommonLib/IMsgObserver.h"
#include "inc_CommonLib/IMsg.h"
#include "inc_CommonLib/StringConvert.h"
#include "inc_CommonLib/FrameCovertTools.h"
#include "inc_CommonLib/VxFileDir.h"
#include "inc_CommonLib/stringUtil.h"

#include "vxfilename.h"
#include "VxImage.h"
#include "VxNLEEngineIF.h"
#include "vxasyncfiletasksif.h"
#include "vxdatacoreif.h"

#include "VxDemuxerInitIF.h"


static int rscreccn_l = 0,  rscreccn_t = 0, rscreccn_r = 0, rscreccn_b = 0;


unsigned g_maxsize = 4000000000;

SHANDLE_PTR g_iehwnd = 0;
// 启动默认最大声
float g_mixer[AUDIO_INOUT_CHANEL][AUDIO_INOUT_CHANEL] = \
	{
			1, 0, 1, 0, 1, 0, 1, 0,
			0, 1, 0, 1, 0, 1, 0, 1,
			0, 0, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 1, 0, 0, 0, 0,
			0, 0, 0, 0, 1, 0, 0, 0,
			0, 0, 0, 0, 0, 1, 0, 0,
			0, 0, 0, 0, 0, 0, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 1,	

	};

static DWORD vxGetVideoStandard(int nRate, int nScale)
{
	if(nRate > 0 && nScale > 0)
	{
		double dbFrameRate = (double)nRate / nScale;

		if(fabs(dbFrameRate - 25.0) < 1.0 || fabs(dbFrameRate - 50.0) < 1.0)
			return vxPalType;

		if(fabs(dbFrameRate - 29.97) < 1.0 || fabs(dbFrameRate - 59.94) < 1.0)
			return vxNtscType;
	}

	return UnknownStandardType;
}




void CVxPlayWidget_W::GetFileMatteInfo( QString filePath, MFILEINFO &mfileInfo)
{
	//vxFreeMFileInfo(&mfileInfo);
	memset(&mfileInfo, 0, sizeof(mfileInfo));

	string vidPath = QStringToString(filePath);
	VXFILEPATH vx_filePath;
	VidPath2VxFilePath(vidPath, vx_filePath);
	CVxComPtr<IVxCompoundFile> compoundFile = NULL;
	vxCreateCompoundFile("bay", "760417", &compoundFile);
	compoundFile->Add(vx_filePath);

	// 1流多声道
	VXSIZE size;
	size.cx = 80;
	size.cy = 60;
	compoundFile->SetDemuxType(cd_native);
	int iCurNo = compoundFile->GetAudStreamNo();
	compoundFile->SetAudStreamNo(-1);
	mfileInfo = vxGetMFileInfo(compoundFile, 0, m_InfoTools, false, true, 0, size);
	compoundFile->SetAudStreamNo(iCurNo);
	compoundFile->SetDemuxType(cd_spliteaudio);		
	compoundFile->StateEvent(cereload,NULL);

	//bool bMultAStream = isMultiChannels(m_InfoTools, compoundFile);
	//if (bMultAStream)
	//{
		//CVxComPtr <IVxCompoundFile> pTempFile;
		//vxCreateCompoundFile("bay", "760417", &pTempFile);
		//for(int i = 0; i < compoundFile->GetCount(); ++i)
		//{
		//	pTempFile->Add(compoundFile->GetAt(i));
		//}
		//pTempFile->SetDemuxType(cd_native);
		//int iCurNo = pTempFile->GetAudStreamNo();
		//pTempFile->SetAudStreamNo(-1);
		//mfileInfo = vxGetMFileInfo(pTempFile, 0, m_InfoTools, false, true, 0, size);
		//pTempFile->SetAudStreamNo(iCurNo);
		//pTempFile->SetDemuxType(cd_spliteaudio);		
		//pTempFile->StateEvent(cereload,NULL);
	//}
	//else
	//{
	//	mfileInfo = vxGetMFileInfo(compoundFile, 0, m_InfoTools, 0, 0, 0, VXSIZE(), 0);

	//}

	if (m_fileInfo.info.type != MI_AV)
	{
	//	QFile file(filePath);
	//	qint64 size = file.size();
	//	int rate = 25;
	//	if (m_fileInfo.info.vinfo.scale > 0)
	//		rate = m_fileInfo.info.vinfo.rate / m_fileInfo.info.vinfo.scale;
	//	if (rate == 0) rate = 25;
	//	m_fileInfo.info.muxinfo.bitrate = (float)(size * 8) / (float)(m_fileInfo.info.vinfo.frames / rate);
	//}
	//else
	//{
		m_fileInfo.info.muxinfo.bitrate = 0;
	}
	
}


void FCClipPlayCallback::CurrentFrame(STATEPARAM * theParam,void * theObject)
{
	if(theParam->newState == theParam->oldState)
	{ 
		int playheadIndicator = theParam->lFrame;
		CVxPlayWidget_W *VxPlayWidget_W = static_cast<CVxPlayWidget_W*>(theObject);
		if (VxPlayWidget_W->IsClipNeedCallbackPlayFrame())
		{
			IMsgObserver *pMsg = GetIMsgObserver();
			pMsg->SendMsg((QObject*)theObject, TURBO_EDIT_CLIPPLAY_CURRENTFRAME, (void *)theParam->lFrame, true);
		}
		else
		{
			int xx = 0;
		}
	}
	if(HALSFREE == theParam->newState)
	{
		int playheadIndicator = theParam->lFrame;
		IMsgObserver *pMsg = GetIMsgObserver();
		pMsg->SendMsg((QObject*)theObject, TURBO_EDIT_CLIPPLAY_COMPLETE, true);
	}

}

static QEvent *_createAudioDataEvent(void *theParam,int type)
{
	CAudioDataEvent *pAudioEvent = new CAudioDataEvent(QEvent::Type(type));
	memcpy(&pAudioEvent->m_audioData, theParam, sizeof(AUDIOUVMETER));
	return pAudioEvent;
}

void FCClipPlayAudioCallback::__audiovufunc(AUDIOUVMETER * theParam,void * theObject)
{
	IMsgObserver *pMsg = GetIMsgObserver();
	pMsg->PostMsg((QObject*)theObject, _createAudioDataEvent, TURBO_EDIT_PLAYVIEW_AUDIODATA,theParam );
}


CVxPlayWidget_W::CVxPlayWidget_W(QWidget *parent)
	: CBasePlayWidget(parent)
{
	m_bHasSource = false;
	m_bFullScreen = false;
	m_bClickDbl = false;
	m_bSeekTo = false;
	m_player = NULL;
	m_curFrame = -1;
	m_duration = 0;
	m_speedPlayEnd = -1;
	m_playRulerDragType = -1;
	m_doubleTimer  = new QTimer(this);
	m_doubleTimer->setSingleShot(true);
	connect(m_doubleTimer, SIGNAL(timeout()), this, SLOT(SlotDoubleClickPlay()));
	m_resetDoubleTimer = new QTimer(this);
	connect(m_resetDoubleTimer, SIGNAL(timeout()), this, SLOT(SlotResetDoubleClickPlay()));

	memset(&m_fileInfoAttribute, 0, sizeof(m_fileInfoAttribute));
	init();
	LoadSettings();
}

CVxPlayWidget_W::~CVxPlayWidget_W()
{
	if(m_player)
	{
		VXRECT rect;
		m_player->SetHwnd((HVXWND)NULL,&rect);
	}

	m_demultiplexer = NULL;
	m_compoundFile = NULL;
	vxFreeMFileInfo(&m_fileInfo);
	vxFreeMFileInfo(&m_fileInfoAttribute);
	if(m_pPlayViewController)
	{
		delete m_pPlayViewController;
	}
	IMsgObserver *pMsgObservers = GetIMsgObserver();
	pMsgObservers->RemoveObserver(this);


	SaveSettings();
}

void  CVxPlayWidget_W::InitAllShortcut()
{
	m_pGoHeadShortcut			=  new QShortcut(tr("Home"),this);
	m_pGoInPointShortcut		=  new QShortcut(tr("Q"),this);
	m_pPreFrameShortcut			=  new QShortcut(tr("Left"),this);
	m_pFastBackwardShortcut		=  new QShortcut(tr("Down"),this);
	m_pPlayShortcut				=  new QShortcut(tr("Space"),this);
	m_pFastForwardShortcut		=  new QShortcut(tr("Up"),this);
	m_pNextFrameShortcut		=  new QShortcut(tr("Right"),this);
	m_pGoOutPointShortcut       =  new QShortcut(tr("W"),this);
	m_pGoEndShortcut			=  new QShortcut(tr("End"),this);
	m_pSetInPointShortcut       =  new QShortcut(tr("I"),this);
	m_pSetOutPointShortcut      =  new QShortcut(tr("O"),this);
	m_pAudioUpShortcut			=  new QShortcut(tr("Ctrl+Up"),this);
	m_pAudioDownShortcut        =  new QShortcut(tr("Ctrl+Down"),this);
	m_pOpenFileShortcut         =  new QShortcut(tr("Ctrl+O"), this);
	

	connect(m_pGoHeadShortcut, SIGNAL(activated()),this, SLOT(home()));
	connect(m_pGoInPointShortcut, SIGNAL(activated()),this, SLOT(goToInPoint()));
	connect(m_pPreFrameShortcut, SIGNAL(activated()),this, SLOT(prevFrame()));
	connect(m_pFastBackwardShortcut, SIGNAL(activated()),this, SLOT(SlotFastBackward()));
	connect(m_pPlayShortcut, SIGNAL(activated()),this, SLOT(play()));
	connect(m_pFastForwardShortcut, SIGNAL(activated()),this, SLOT(SlotFastForward()));
	connect(m_pNextFrameShortcut, SIGNAL(activated()),this, SLOT(nextFrame()));
	connect(m_pGoOutPointShortcut, SIGNAL(activated()),this, SLOT(goToOutPoint()));
	connect(m_pGoEndShortcut, SIGNAL(activated()),this, SLOT(end()));
	connect(m_pSetInPointShortcut, SIGNAL(activated()),this, SLOT(setInPosSlot()));
	connect(m_pSetOutPointShortcut,SIGNAL(activated()),this, SLOT(setOutPosSlot()));
	connect(m_pAudioUpShortcut,SIGNAL(activated()),this, SLOT(AudioUpSlot()));
	connect(m_pAudioDownShortcut,SIGNAL(activated()),this,SLOT(AudioDownSlot()));
	connect(m_pOpenFileShortcut, SIGNAL(activated()), this, SLOT(openFileDlgSlot()));
}

void CVxPlayWidget_W::init()
{
	memset(&m_fileInfo, 0, sizeof(m_fileInfo));
	m_pNLEEngine = NULL;
	m_pPlayWidget = new CMonitorWidget(this);
	//m_pPlayWidget = new CMonitorWidgetContainet(this);
	m_pManager = new CVxPlayButtonController_W(this);	
	m_pPlayRuler = new CVxPlayRuler(this);
	m_pPlayRuler->SetDuration(200);
	m_pPlayRuler->SetVisibleRangle(0,200);
	m_pPlayRuler->SetCurrentFrame(0);
	m_pPlayRuler->EnableRightClicked(false);
	m_pPlayRuler->installEventFilter(this);
	m_pPlayRuler->SetDragInOut(true);
	m_pPlayRuler->EnableInOutHasDrag(true);
	m_pPlayViewController = new CVxPlayViewController_X(this);
	m_pPlayViewController->SetPlayRuler(m_pPlayRuler);
	m_playRectHeight = 0;


	connect(m_pManager, SIGNAL(buttonClicked(int)), this, SLOT(onBtnClicked(int)));
	connect(qApp, SIGNAL(aboutToQuit()),this,SLOT(appWillQuitSlot()));


	m_pCurLineEdit      = new CTimeCodeEdit(this);
	m_pCurLineEdit->setCurFrame(0);
	m_pCurLineEdit->setTotalFrames(200);
	m_pCurLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
	connect(m_pCurLineEdit, SIGNAL(valueChangedSignal(const int&, bool)), this, SLOT(SlotCurLineEdit(const int&, bool)));
	m_pCurLineEdit->installEventFilter(this);
	m_pCurLineEdit->setFont(QApplication::font());

	m_pAreaLineEdit     = new CVxTimeCodeEdit(this);
	m_pAreaLineEdit->setCurFrame(0);
	m_pAreaLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
	m_pAreaLineEdit->setLineEditStyle(CVxTimeCodeEdit::AREATIME_OFFSET);
	m_pAreaLineEdit->setReadOnly(true);
	m_pAreaLineEdit->installEventFilter(this);
	m_pAreaLineEdit->setFont(QApplication::font());

	m_pInPointEdit      = new CVxTimeCodeEdit(this);
	m_pInPointEdit->setCurFrame(0);
	m_pInPointEdit->setContextMenuPolicy(Qt::NoContextMenu);
	m_pInPointEdit->setReadOnly(true);
	m_pInPointEdit->installEventFilter(this);
	m_pInPointEdit->setFont(QApplication::font());

	m_pOutPointEdit     = new CVxTimeCodeEdit(this);
	m_pOutPointEdit->setCurFrame(0);
	m_pOutPointEdit->setContextMenuPolicy(Qt::NoContextMenu);
	m_pOutPointEdit->setReadOnly(true);
	m_pOutPointEdit->installEventFilter(this);
	m_pOutPointEdit->setFont(QApplication::font());

	m_pTotalLineEdit     = new CVxTimeCodeEdit(this);
	m_pTotalLineEdit->setCurFrame(200);
	m_pTotalLineEdit->setTotalFrames(200);
	m_pTotalLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
	m_pTotalLineEdit->setReadOnly(true);
	m_pTotalLineEdit->installEventFilter(this);
	m_pTotalLineEdit->setFont(QApplication::font());

	QPalette palette; 
	palette.setColor(QPalette::WindowText,QColor(190,190,190));



	//m_pPixBg = new QPixmap(tr(":/VxPlayViewX/Resources/toolbar_bg.png"));
	connect(m_pPlayRuler,SIGNAL(frameChanged(int ,bool, int)),this,SLOT(playRulerFrameChangeSlot(int ,bool, int)));
	registerObserver();


	connect(m_pPlayRuler, SIGNAL(inPointChanging(int)), this, SLOT(playRulerInPointChanging(int)));
	connect(m_pPlayRuler, SIGNAL(outPointChanging(int)), this, SLOT(playRulerOutPointChanging(int)));
	connect(m_pPlayRuler, SIGNAL(inPointChanging(int)), m_pInPointEdit, SLOT(setCurFrame(int)));
	connect(m_pPlayRuler, SIGNAL(outPointChanging(int)), m_pOutPointEdit, SLOT(setCurFrame(int)));
	connect(m_pPlayRuler, SIGNAL(inOutPointChanging()), this, SLOT(playRulerInOutPointChanging()));


	m_bNeedFulls = false;
	m_bHasAudio = true;
	m_bMouseDown = false;
	m_audioVolume = 1.0;
	m_audioRect = QRect();


	m_pInOverImage = new QImage(tr(":/VxPlayViewX/Resources/rudian_d.png"));
	m_pInNorImage  = new QImage(tr(":/VxPlayViewX/Resources/rudian_u.png"));
	m_pInDisImage  = new QImage(tr(":/VxPlayViewX/Resources/rudian_x.png"));
	m_pOutOverImage = new QImage(tr(":/VxPlayViewX/Resources/chudian_d.png"));
	m_pOutNorImage  = new QImage(tr(":/VxPlayViewX/Resources/chudian_u.png"));
	m_pOutDisImage  = new QImage(tr(":/VxPlayViewX/Resources/chudian_x.png"));
	m_pSetInBtn = new CVxButtonItemX(this);
	m_pSetInBtn->SetImage(m_pInOverImage, m_pInNorImage, m_pInDisImage);
	m_pSetInBtn->setToolTip(tr("设置入点(I)"));
	connect(m_pSetInBtn, SIGNAL(clicked()), this, SLOT(setInPosSlot()));
	m_pSetOutBtn = new CVxButtonItemX(this);
	m_pSetOutBtn->SetImage(m_pOutOverImage, m_pOutNorImage, m_pOutDisImage);
	m_pSetOutBtn->setToolTip(tr("设置出点(O)"));
	connect(m_pSetOutBtn, SIGNAL(clicked()), this, SLOT(setOutPosSlot()));

	m_pSafeFrameAct	   = new QAction(tr("安全框"), this);
	m_pShowFileInfoAct = new QAction(tr("文件属性"), this);
	m_pClearInPointAct = new QAction(tr("刪除入点"),this);
	m_pClearInPointAct->setEnabled(false);
	m_pClearOutPointAct = new QAction(tr("删除出点"),this);
	m_pClearOutPointAct->setEnabled(false);
	m_pFullScreenAct = new QAction("全屏显示", this);
	InitAct();


	connect(m_pShowFileInfoAct, SIGNAL(triggered()), this, SLOT(ShowFileInfo()));
	connect(m_pSafeFrameAct, SIGNAL(triggered()), this, SLOT(ShowSafeFrame()));
	connect(m_pClearInPointAct, SIGNAL(triggered()), this, SLOT(clearInPoint()));
	connect(m_pClearOutPointAct, SIGNAL(triggered()), this, SLOT(clearOutPoint()));
	connect(m_pFullScreenAct, SIGNAL(triggered()), this, SLOT(SlotFullScreen()));


	m_pSafeFrameAct->setCheckable(true);
	m_pSafeFrameAct->setChecked(true);
	

	m_meter1          = new CAudioMeterCtrl(this );
	m_meter1->setLeft();
	m_meter2          = new CAudioMeterCtrl(this );
	m_meter2->setRight();

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(ShowBar()));
	m_pManager->installEventFilter(this);
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	m_pAudio = new CAudio(this);
	m_pAudio->SetDirection(VET);
	m_pAudio->setWindowOpacity(0.5f);
	m_pAudio->SetBg(QColor(50,50,50));
	m_pAudio->hide();

	m_inputOutputTimer = new QTimer(this);
	connect(m_inputOutputTimer, SIGNAL(timeout()), this, SLOT(SlotInputOutTimer()));
	//m_pAudio->setVisible(false);
	//后面用QShortCut在网页中无效
	//InitAllShortcut(); 
	m_bHasSource = false;
}

void CVxPlayWidget_W::openFileDlgSlot()
{
	emit OpenFileSig(m_preFilePath);	
}

static long GetFreeDiskSpaceInKB(const char *pFile) 
{ 
#ifdef Q_OS_WIN

	DWORD dwFreeClusters, dwBytesPerSector, dwSectorsPerCluster, dwClusters;
	char RootName[MAX_PATH];
	LPSTR ptmp;         
	ULARGE_INTEGER ulA, ulB, ulFreeBytes;      
	GetFullPathName(pFile, sizeof(RootName), RootName, &ptmp);
	if (RootName[0] == '\\' && RootName[1] == '\\') 
	{  // 是否网络路径
		ptmp = &RootName[2]; 
		while (*ptmp && (*ptmp != '\\')) 
		{     ptmp++; 
		} 
		if (*ptmp) 
		{     // “\\\” ?   
			ptmp++; 
		}     
	} else 
	{ 
		ptmp = RootName;     
	}      
	while (*ptmp && (*ptmp != '\\')) 
	{ 
		ptmp++;     
	}     
	if (*ptmp) 
	{  
		ptmp++; *ptmp = '\0';     
	}
	//装载kernel32.dll
	HINSTANCE h = LoadLibraryA("kernel32.dll");     
	if (h) 
	{
		typedef BOOL (WINAPI *MyFunc)(LPCSTR RootName, 
			PULARGE_INTEGER pulA, PULARGE_INTEGER pulB, 
			PULARGE_INTEGER pulFreeBytes); 
		MyFunc pfnGetDiskFreeSpaceEx = (MyFunc)GetProcAddress(h, "GetDiskFreeSpaceExA"); 
		FreeLibrary(h); 
		if (pfnGetDiskFreeSpaceEx)
		{     
			if (!pfnGetDiskFreeSpaceEx(RootName, &ulA, &ulB, &ulFreeBytes)) 
				return -1;     
			return (long)(ulFreeBytes.QuadPart / 1024); 
		}     
	}      
	if (!GetDiskFreeSpace(RootName, 
		&dwSectorsPerCluster, 
		&dwBytesPerSector, 
		&dwFreeClusters, &dwClusters)) 
		return -1;  

	return(MulDiv(dwSectorsPerCluster * dwBytesPerSector,   
		dwFreeClusters,    
		1024)); 

#endif
	return 0;
} 

void CVxPlayWidget_W::GetDiskFreeSpace(const QString &path, long &nFreeSpace)
{
	string filePath = QStringToString(path);
	nFreeSpace = GetFreeDiskSpaceInKB(filePath.c_str());
	//返回 G
	nFreeSpace = (nFreeSpace/1024)/1024;
}

QString CVxPlayWidget_W::GetFileSize(const QString &fileName)
{
	QString fileSize;
#ifdef Q_OS_WIN
	string filePath = QStringToString(fileName);
	struct _stati64 STA;
	__int64 i64Size = 0;
	int ret = _stati64(filePath.c_str(), &STA);
	if(ret != 0)
	{//没有取得信息
		return fileSize;
	}
	i64Size = STA.st_size;
	fileSize.setNum(i64Size);
#endif
	return fileSize;
}

bool CVxPlayWidget_W::GetFileAvInfo(QString strXML, MATTERINFO2 &info, QString &filePath)
{   
	QDomDocument doc;
	QString         strError;  
	int             errLin = 0, errCol = 0;  
	if( !doc.setContent(strXML) ) 
	{  
		return false;  
	}  

	if( doc.isNull() ) 
	{  
		return false;  
	}
	QDomElement fileInfoNode = doc.firstChildElement(tr("FILEINFO"));
	QDomNodeList nodeList = fileInfoNode.elementsByTagName(tr("FILE"));
	if (!nodeList.isEmpty())
	{
		for(int i = 0; i < nodeList.count(); ++i)
		{
			QDomNode node = nodeList.at(i);
			if (node.hasChildNodes())
			{
				QDomElement pathNode = node.firstChildElement(tr("FILE_PATH"));
				QString filePath1 = pathNode.text();
				filePath = filePath1;
				//MATTERINFO2 &minfo = m_fileInfo.info;
				info = m_fileInfo.info;
				//if (minfo.type == MI_CANNOTUSE)
				//{
				//	return false;
				//}
				//else if (minfo.type == MI_AV)
				//{
				//	memcpy(&info, &minfo, sizeof(minfo));
				//}
				//else if (minfo.type == MI_VIDEO)
				//{
				//	info.type = minfo.type;
				//	memcpy(&(info.muxinfo), &(minfo.muxinfo), sizeof(VX_VIDEOINFO));
				//	memcpy(&(info.vinfo), &(minfo.vinfo), sizeof(VX_VIDEOINFO));
				//}
				//else  if (minfo.type == MI_AUDIO)
				//{
				//	info.type = minfo.type;
				//	memcpy(&(info.muxinfo), &(minfo.muxinfo), sizeof(VX_VIDEOINFO));
				//	memcpy(&(info.ainfo), &(minfo.ainfo), sizeof(VX_AUDIOINFO));
				//}
			}

		}
	}
	return true;
}

void CVxPlayWidget_W::GetMediaInfo(const QString &filePath, QString &strXml)
{
	strXml.clear();

	MATTERINFO2 &matterInfo = m_fileInfo.info;
	string vidPath = QStringToString(m_preFilePath);
	VXFILEPATH vx_filePath;
	VidPath2VxFilePath(vidPath, vx_filePath);


	strXml += tr("<avFormatTemplate><templateName>MAM</templateName><fileFormat>");
	strXml += FindPathExt(m_preFilePath);
	strXml += tr("</fileFormat><audioFileFormat>");
	if (strlen(vx_filePath.audiofiles[0]) == 0)
	{
		strXml += FindPathExt(m_preFilePath);
	}
	else
	{
		strXml += FindPathExt(vx_filePath.audiofiles[0]);
	}
	strXml += tr("</audioFileFormat><muxDataRate>");
	strXml += QSTR(matterInfo.muxinfo.bitrate);
	strXml += tr("</muxDataRate>");
	strXml += "<videoFormat>";
	strXml += L8(vxGetFormatType(matterInfo.vinfo.fourcc));
	strXml += tr("</videoFormat>");
	//strXml += "<videoProfile>0</videoProfile>";

	int duration = matterInfo.vinfo.frames;
	if (matterInfo.type == MI_AUDIO)
	{
		if (matterInfo.ainfo.freq > 0)
		{
			duration = 25 * matterInfo.ainfo.samples / matterInfo.ainfo.freq;
		}
	}
	strXml += "<duration>";
	strXml += QSTR(duration);
	strXml += "</duration>";
	strXml += "<videoDataRateMode>";
	strXml += QSTR(matterInfo.vinfo.bitrate >> 31);
	strXml += "</videoDataRateMode>";
	strXml += "<videoDataRate>";
	strXml += QSTR(matterInfo.vinfo.bitrate);
	strXml += tr("</videoDataRate><videoFrameRate>");
	strXml += QSTR(matterInfo.vinfo.scale > 0 ? matterInfo.vinfo.rate / matterInfo.vinfo.scale : 25);
	strXml += tr("</videoFrameRate><videoWidth>");
	strXml += QString::number(matterInfo.vinfo.display_width);
	strXml += tr("</videoWidth><videoHeight>");
	strXml += QString::number(matterInfo.vinfo.display_height);
	strXml += tr("</videoHeight>");
	strXml += "<videoAspectRatio>";
	strXml += "1";
	strXml += "</videoAspectRatio>";
	strXml += "<videoKeyframeSpace>";
	strXml += QSTR(matterInfo.vinfo.dwKeyframeSpace);
	strXml += "</videoKeyframeSpace><videoChroma>";
	strXml += QSTR(matterInfo.vinfo.chromafmt);
	strXml += tr("</videoChroma>");
	strXml += "<videoTopFirst>";
	strXml += QSTR(matterInfo.vinfo.topfirst);
	strXml += "</videoTopFirst>";

	strXml += tr("<audioFormat>");
	strXml += L8(vxGetFormatType(matterInfo.ainfo.fourcc));
	strXml += tr("</audioFormat><audioDataRate>");
	strXml += QSTR(matterInfo.ainfo.bitrate);
	strXml += tr("</audioDataRate><audioChannels>");
	int channels = matterInfo.ainfo.channels;
	int auds = matterInfo.audios;
	VX_AUDIOINFO2 *pbitrate = matterInfo.painfo;
	while (--auds > 0 && pbitrate)
	{
		channels += pbitrate->channels;
		pbitrate++;
	}
	strXml += QSTR(channels);
	strXml += tr("</audioChannels><audioSampleRate>");
	strXml += QSTR(matterInfo.ainfo.freq);
	strXml += tr("</audioSampleRate><audioBitsPerSample>");
	strXml += QSTR(matterInfo.ainfo.bitpersample);
	strXml += tr("</audioBitsPerSample>");
	//afd
	int afd = 0;
	if (m_preFilePath.endsWith(".mxf", Qt::CaseInsensitive))
	{
		afd = (matterInfo.vinfo.aspect & 0xf0000000)>>28;
		afd = (afd << 3);
		if (matterInfo.vinfo.display_width > 720)
			afd |= 0x4;
	}
	strXml += "<afd>";
	strXml += QSTR(afd);
	strXml += "</afd>";
	strXml += ("</avFormatTemplate>");
		
}

void CVxPlayWidget_W::GrabFrame(long pos, const QString &filepath, long format, long width, long height)
{
	if (m_player)
	{
		QString err;
		string strFilePath = QStringToString(filepath);
		bool bPathExists = TRUE;

		replace(strFilePath.begin(), strFilePath.end(), '/', '\\');
		string directoryPath = strFilePath.substr(0, strFilePath.find_last_of('\\'));
		if(!vxDirExists(directoryPath.c_str()))
		{
			QDir dir;
			bPathExists = dir.mkpath(directoryPath.c_str());
		}
		if(bPathExists)
		{
			int frameWidth, frameHeight;

			if (m_bSd)
			{
				frameWidth = 720;
				frameHeight = 576;
			}
			else
			{
				frameWidth = 1920;
				frameHeight = 1080;
			}

			int frameSize = frameWidth * frameHeight * 4;
			DWORD	*pData = (DWORD	*)_vxmalloc(frameSize);
			VXSIZE size = { frameWidth, frameHeight};
			//vxSleep(400);
			m_player->SingleCapture(pData, &size);
			VXIMAGE saveImg;
			memset(&saveImg, 0, sizeof(VXIMAGE));
			saveImg.Width = frameWidth;
			saveImg.Height = frameHeight;
			saveImg.Pitch = saveImg.Width * 4;
			saveImg.BPP = 32;
			saveImg.BPPSource = 32;
			saveImg.data = pData;

			bool bRet = VxImage_Save(strFilePath.c_str(), &saveImg, 32, 80);
			if (!bRet)
			{
				//err = "采集单帧失败";
				//CIEMsgBox box;
				//box.Show(this, "警告", err);
				
			}
			_vxfree(pData);
			
		}
		else
		{
			//err = "无法访问文件夹:";
			//err += directoryPath.c_str() + QString(PATH_SPLITTER);
			//CIEMsgBox box;
			//box.Show(this, "警告", err);
		}
		
	}
}

bool CVxPlayWidget_W::OpenFile(const QString &fileName)
{
	m_fileName = fileName;
	string strDir = QStringToString(fileName);
	VX_AVPATH avPath = {0};
	sprintf(avPath.szPath,"%s",strDir.c_str());
	m_duration = 0;
	return SetSource(avPath);

}


// 调用过程:OpenFileDialog->GetMediaInfo->OpenFile->GetPlayFileInfo->OpenFile;
void CVxPlayWidget_W::OpenFileDialog(QString &strOpenFileXML, long iewnd)
{
	// zhy 2012.06.28 修改为只能选一个文件, 
	strOpenFileXML = tr("");
	QStringList fileNames;

	g_iehwnd = iewnd;
	CWinFileDialog winFileDlg(this, g_iehwnd);
	winFileDlg.SetPreFilePath(m_preFilePath);
	winFileDlg.DoModal();
	QString filePathTemp = winFileDlg.SelectedFile();
	if (filePathTemp.isEmpty())
	{
		return;
	}
	fileNames << filePathTemp;


	m_preFilePath = filePathTemp;


	VX_AVPATH avPath = {0};
	QString fileName;
	QString strFileXML[9];
	for (int i = 0; i < 9; ++i)
	{
		strFileXML[i] = tr("");
	}
	int type = 10;//组类型：//1  视频编目  2 音频编目 3 图片编目 10 未定义
	bool bHaveVideo = false, bHaveAudio = false;
	int fileCount = fileNames.count();
	for (int index = 0; index < fileCount; ++index)
	{
		fileName = fileNames.at(index);
		//SigRebulidIndex(fileName);
		fileName = QDir::toNativeSeparators(fileName);
		GetFileMatteInfo(fileName, m_fileInfo);
		break;
	}

	filePathTemp = QDir::toNativeSeparators(filePathTemp);
	string vidPath = QStringToString(filePathTemp);
	VXFILEPATH vx_filePath;
	VidPath2VxFilePath(vidPath, vx_filePath);
	
	// 在JDVN中, 全openfile最后一个文件, 所以返回有音频时他会只打开音频, 这里改为只返回视频路径 by zhy 2012.11.30
	for (int i = 0; i < 8; ++i)
	{
		if (strlen(vx_filePath.audiofiles[i]))
		{
			fileNames << StringToQString(vx_filePath.audiofiles[i]);
		}
		else
		{
			break;
		}
	}

	
	for (int index = 0; index < fileNames.size(); ++index)
	{
		MFILEINFO mfileInfo;
		memset(&mfileInfo, sizeof(mfileInfo), 0);
		fileName = fileNames.at(index);
		//MFILEINFO &mfileInfo = m_fileInfo;
		GetFileMatteInfo(fileName, mfileInfo);
		MATTERINFO2 &matterInfo = mfileInfo.info;
		
		QString strFileType;               //类型 //0-video+audio 1-video 2-audio 
		nFileType = 4;                     //0-video+audio 1-video 2-audio 4-其他 
		if (matterInfo.type == MI_AV)
		{
			if (fileNames.size()-1 == matterInfo.audios)
			{
				// 是 1
				nFileType = 1;
				strFileType = tr("1");
			}
			else
			{
				// 是 0

				nFileType = 0;
				strFileType = tr("0");
			}

		}
		else if (matterInfo.type == MI_VIDEO)
		{
			nFileType = 1;
			strFileType = tr("1");
		}
		else if (matterInfo.type == MI_AUDIO)
		{
			nFileType = 2;
			strFileType = tr("2");
		}
		else
		{
			nFileType = 4;
			strFileType = tr("3");
		}

		vxFreeMFileInfo(&mfileInfo);
		QString strSize = GetFileSize(fileName);
		strFileXML[index] += "<FILE>";
		strFileXML[index] += "<FILE_PATH>";
		strFileXML[index] += fileName;//文件路径名
		strFileXML[index] += "</FILE_PATH>";
		strFileXML[index] += "<FILE_SIZE>";
		strFileXML[index] += strSize;//大小
		strFileXML[index] += "</FILE_SIZE>";
		strFileXML[index] += "<FILE_TYPE>";
		strFileXML[index] += strFileType;
		strFileXML[index] += "</FILE_TYPE>";
		strFileXML[index] += "</FILE>";

	
		//只能有一个视频，如果有多个返回错误
		if(nFileType == 0 || nFileType == 1)
		{
			if(bHaveVideo)
			{
				type = 10;
				break;
			}
			type = 1;
			bHaveVideo = true;
		}
		else if(nFileType == 2)	//音频
		{
			bHaveAudio = true;
			if(type != 1)	//没有视频的时候
			{	
				type = 2;
			}
		}
		else
		{	
			if(type > 2)		//不能识别的格式都认为是图片
			{	
				type = 3;
			}
		}
	}
		
	
	strOpenFileXML += QString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><FILEINFO type=\"");
	strOpenFileXML += QString::number(type);
	strOpenFileXML += QString("\">");
	for(int i = 0;i < fileNames.size(); i++)
	{
		strOpenFileXML += strFileXML[i];
	}
	strOpenFileXML += "</FILEINFO>";
}

void CVxPlayWidget_W::OpenSingleFileDialog(QString &strOpenFileXML)
{
	strOpenFileXML = tr("");
	QStringList fileNames = QFileDialog::getOpenFileNames(
		this,
		tr("选择一个或多个媒体文件"),
		tr(""),
		tr("All Media Files (*.*)"));
	int fileCount = fileNames.count();
	if (fileCount == 0)
	{
		return; 
	}
	if (fileCount > 10)
	{
		fileCount = 10;
	}

	MATTERINFO matterInfo = {0};
	VX_AVPATH avPath = {0};
	QString fileName;
	QString strFileXML[5];
	for (int i = 0; i < 5; ++i)
	{
		strFileXML[i] = tr("");
	}
	for (int index = 0; index < fileCount; ++index)
	{
		fileName = fileNames.at(index);
		string strDir = QStringToString(fileName);	
		sprintf(avPath.szPath,"%s",strDir.c_str());
		m_InfoTools->GetInfo(&avPath,matterInfo);
		QString strFileType;               //类型 //0-video+audio 1-video 2-audio 
		int nFileType;                     //0-video+audio 1-video 2-audio 4-其他 
		if (matterInfo.type == MI_AV)
		{
			nFileType = 0;
			strFileType = tr("0");
		}
		else if (matterInfo.type == MI_VIDEO)
		{
			nFileType = 1;
			strFileType = tr("1");
		}
		else if (matterInfo.type == MI_AUDIO)
		{
			nFileType = 2;
			strFileType = tr("2");
		}
		else
		{
			nFileType = 4;
			strFileType = tr("10");
		}
		QString strSize = GetFileSize(fileName);
		strFileXML[nFileType] += "<FILE>";
		strFileXML[nFileType] += "<FILE_PATH>";
		strFileXML[nFileType] += fileName;//文件路径名
		strFileXML[nFileType] += "</FILE_PATH>";
		strFileXML[nFileType] += "<FILE_SIZE>";
		strFileXML[nFileType] += strSize;//大小
		strFileXML[nFileType] += "</FILE_SIZE>";
		strFileXML[nFileType] += "<FILE_TYPE>";
		strFileXML[nFileType] += strFileType;
		strFileXML[nFileType] += "</FILE_TYPE>";
		strFileXML[nFileType] += "</FILE>";
	}
	strOpenFileXML += QString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?><FILEINFO type=\"0\">");
	for(int i = 0;i < 5; i++)
	{
		strOpenFileXML += strFileXML[i];
	}
	strOpenFileXML += "</FILEINFO>";
}

void CVxPlayWidget_W::openFileSlot(QString &filePath)
{
	string strDir = QStringToString(filePath);
	VX_AVPATH avPath = {0};
	sprintf(avPath.szPath,"%s",strDir.c_str());
	SetSource(avPath);
}

void CVxPlayWidget_W::setInPosSlot()
{
	if (m_bHasSource)
	{
		
		if (m_pPlayViewController->isPlay())
		{
			m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);
			m_pPlayViewController->stop();
		}

		int inPut = m_pPlayViewController->getPlayHeadIndicator();

		int inPoint;
		int outPoint;
		m_pPlayRuler->GetInOutPoint(inPoint, outPoint);
		if (outPoint >= 0) // has outPoint
		{
			if (outPoint <= inPut)
			{
				m_pPlayRuler->RemoveOutPoint();
				m_pOutPointEdit->setCurFrame(0);
				m_pAreaLineEdit->setCurFrame(0);
				m_pClearOutPointAct->setEnabled(false);
			}
			else
			{
				m_pAreaLineEdit->setCurFrame((outPoint - inPut));
			}
			m_pPlayRuler->SetInPoint(inPut);
			m_pInPointEdit->setCurFrame(inPut);
		}
		else
		{
			m_pPlayRuler->SetInPoint(inPut);
			m_pInPointEdit->setCurFrame(inPut);
			m_pAreaLineEdit->setCurFrame(0);
			m_pOutPointEdit->setCurFrame(0);
		}

		m_pClearInPointAct->setEnabled(true);
	}
}

void CVxPlayWidget_W::setOutPosSlot()
{
	if (m_bHasSource)
	{
		if (m_pPlayViewController->isPlay())
		{
			m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);
			m_pPlayViewController->stop();
		}

		int outPut = m_pPlayViewController->getPlayHeadIndicator();

		int inPoint;
		int outPoint;
		m_pPlayRuler->GetInOutPoint(inPoint, outPoint);
		if (inPoint >= 0) // has inPoint
		{
			if (outPut <= inPoint)
			{
				m_pPlayRuler->RemoveInPoint();
				m_pInPointEdit->setCurFrame(0);
				m_pAreaLineEdit->setCurFrame(0);
				m_pClearInPointAct->setEnabled(false);
			}
			else
			{
				m_pAreaLineEdit->setCurFrame((outPut - inPoint));
			}
			m_pPlayRuler->SetOutPoint(outPut);
			m_pOutPointEdit->setCurFrame(outPut);
		}
		else
		{
			m_pPlayRuler->SetOutPoint(outPut);
			m_pOutPointEdit->setCurFrame(outPut);
			m_pInPointEdit->setCurFrame(0);
			m_pAreaLineEdit->setCurFrame(0);
		}

		m_pClearOutPointAct->setEnabled(true);

	}
}

void CVxPlayWidget_W:: SetCtrlRange(long nFrom, long nTo)
{
	if (m_bHasSource)
	{
		if (nTo >= m_duration)
		{
			nTo = m_duration -1;
		}
		m_pPlayRuler->SetInOutPoint(nFrom, nTo);
		setCurFrame(nFrom);
		m_pPlayRuler->SetCurrentFrame(nFrom);
		m_pCurLineEdit->setCurFrame(nFrom);

		m_pInPointEdit->setCurFrame(nFrom);
		m_pOutPointEdit->setCurFrame(nTo);
		m_pAreaLineEdit->setCurFrame(nTo - nFrom);

		m_pClearOutPointAct->setEnabled(true);
		m_pClearInPointAct->setEnabled(true);
	}
}

void CVxPlayWidget_W::SetInOutPoint(long nInPoint, long nOutPoint)
{
	if (m_bHasSource)
	{
		m_pPlayRuler->SetInOutPoint(nInPoint, nOutPoint);
		m_pPlayRuler->SetCurrentFrame(nInPoint);
		setCurFrame(nInPoint);
		m_pCurLineEdit->setCurFrame(nInPoint);
		m_pInPointEdit->setCurFrame(nInPoint);

		int inPoint = 0, outPoint = 0;
		m_pPlayRuler->GetInOutPoint(inPoint, outPoint);
		if (outPoint == -1)
		{
			outPoint = m_pPlayRuler->GetDuration()-1;
		}
		m_pOutPointEdit->setCurFrame(nOutPoint);
		m_pAreaLineEdit->setCurFrame(nOutPoint - nInPoint );
		m_pPlayRuler->SetDragInOut(true);

		m_pClearInPointAct->setEnabled(true);
		m_pClearOutPointAct->setEnabled(true);
	}
}

void CVxPlayWidget_W::GetInOutPoint(int &inpoint,int &outpoint)
{
	m_pPlayRuler->GetInOutPoint(inpoint, outpoint);
	if(inpoint<0)
	{
		inpoint = 0;
	}

	if(outpoint<0)
	{
		int length = m_pPlayRuler->GetDuration();
		outpoint = length - 1;        //帧数从0开始，所以减1
		if (!m_bHasSource)
			outpoint = 0;
	}
}

void CVxPlayWidget_W::GetPosition(int &nPosition)
{
	nPosition = m_pPlayViewController->getPlayHeadIndicator();
}

void CVxPlayWidget_W::GetInPoint(int &inpoint)
{
	int outpoint;
	GetInOutPoint(inpoint, outpoint);
}

void CVxPlayWidget_W::GetOutPoint(int &outpoint)
{
	int inpoint;
	GetInOutPoint(inpoint, outpoint);
}

void CVxPlayWidget_W::playRulerInPointChanging(int iFrame)
{
	int inPoint = 0, outPoint = 0;
	m_pPlayRuler->GetInOutPoint(inPoint, outPoint);

	if (outPoint != -1)
	{
		m_pAreaLineEdit->setCurFrame((outPoint - inPoint ));
	}
	else
	{
		m_pAreaLineEdit->setCurFrame((m_pPlayRuler->GetDuration() - inPoint));
	}
}

void CVxPlayWidget_W::playRulerInOutPointChanging()
{
	int inPoint = 0, outPoint = 0;
	m_pPlayRuler->GetInOutPoint(inPoint, outPoint);
	m_pInPointEdit->setCurFrame(inPoint);
	m_pOutPointEdit->setCurFrame(outPoint);
	m_pAreaLineEdit->setCurFrame(outPoint - inPoint);
}

void CVxPlayWidget_W::playRulerOutPointChanging(int iFrame)
{
	int inPoint = 0, outPoint = 0;
	m_pPlayRuler->GetInOutPoint(inPoint, outPoint);

	if (inPoint != -1)
	{
		m_pAreaLineEdit->setCurFrame((outPoint - inPoint ));
	}
	else
	{
		m_pAreaLineEdit->setCurFrame((outPoint ));
	}
}

void CVxPlayWidget_W::GetPlayFile(QString &fileName)
{
	if (m_bHasSource)
	{
		fileName = m_fileName;
	}
	else
	{
		fileName.clear();
	}
}

void CVxPlayWidget_W::GetPlayFileInfo(const QString &strFilePath, QString &strXml)
{
	bool xmlFlag = false;
	QString filePath;
	for(int i = 0; i < strFilePath.length(); i ++)
	{
		if(strFilePath.at(i) == '<')
		{
			xmlFlag = true;
			break;
		}
	}
	MATTERINFO2 matterInfo;
	memset(&matterInfo, 0 ,sizeof(matterInfo));
	VX_AVPATH avPath = {0};
	if (xmlFlag)  //传入字符串是个XML字符串，从XML字符串中获取MATTERINFO
	{
		if (!GetFileAvInfo( strFilePath, matterInfo, filePath))
		{
			return;
		}
	}
	else
	{
		filePath = strFilePath;
		matterInfo = m_fileInfo.info;
		if (matterInfo.type == MI_CANNOTUSE)
		{
			return;
		}
	}
	
	SepCharacter2XmlCharacter(filePath, true);
	strXml = GetXFileItemXmlInfo(filePath, matterInfo);
	//if (matterInfo.type == MI_AV || matterInfo.type == MI_VIDEO)
	//{
	//	strXml = CreateVideoXMLString(matterInfo);
	//}
	//else if (matterInfo.type == MI_AUDIO)
	//{
	//	strXml = CreateAudioXMLString(matterInfo);
	//}
}

QString CVxPlayWidget_W::CreateAudioXMLString(const MATTERINFO2 &info)
{
	QString strXml = tr("");
	unsigned long frames = 25 * info.ainfo.samples / info.ainfo.freq;
	QString timeCode = StringToQString(VxFrameToString(frames, 25));
	strXml = tr("<FILEINFO><DURATION>");
	strXml += timeCode;
	strXml += tr("</DURATION><CODE_FORMAT>");
	// 音频编码格式
	strXml += vxGetFormatType(info.ainfo.fourcc);
	strXml += tr("</CODE_FORMAT><AUDIO_CHANNEL>");
	QString audioChannel;
	switch(info.ainfo.channels)
	{
	case 1:
		audioChannel = tr("单声道");
		break;
	case 2:
		audioChannel = tr("双声道");
		break;
	default:
		//audioChannel = tr("其它");
		audioChannel = QString::number(info.audios);
	}
	strXml += audioChannel;
	strXml += tr("</AUDIO_CHANNEL></FILEINFO>");
	return strXml;
}

QString CVxPlayWidget_W::CreateVideoXMLString(const MATTERINFO2 &info)
{
	// 音视频混一起解释
	bool baudio = (info.type == MI_AUDIO);
	QString strXml = tr("");
	unsigned long frames = 0;
	frames = info.vinfo.frames;
	float fframeRate;
	if (info.vinfo.scale != 0)
	{
		fframeRate = (float)info.vinfo.rate / info.vinfo.scale;
	}
	else
	{
		fframeRate = 0;
	}

	QString timeCode;
	if (baudio)
	{
		unsigned long frames = 25 * info.ainfo.samples / info.ainfo.freq;
		timeCode = StringToQString(VxFrameToString(frames, 25));
	}
	else
	{
		timeCode = StringToQString(VxFrameToString(frames, fframeRate));
	}

	strXml = tr("<FILEINFO><DURATION>");
	strXml += timeCode;
	strXml += tr("</DURATION><CHANNEL_FORMAT>");
	// 声道格式
	int channels = info.ainfo.channels;
	int auds = info.audios;
	VX_AUDIOINFO2 *pbitrate = info.painfo;
	while (--auds > 0 && pbitrate)
	{
		channels += pbitrate->channels;
		pbitrate++;
	}

	QString audioChannel;
	switch(channels)
	{
	case 1:
		audioChannel = "单声道";
		break;
	case 2:
		audioChannel = "双声道";
		break;
	default:
		audioChannel = QString::number(channels) + "声道";
	}

	strXml += audioChannel;
	strXml += tr("</CHANNEL_FORMAT>");
	strXml += "<COMPLEX_RATE>";
	int muxbitrate = info.muxinfo.bitrate > g_maxsize ? 0 : info.muxinfo.bitrate;
	strXml += QString::number(baudio ? 0 : muxbitrate);
	strXml += "</COMPLEX_RATE><ASPECT_RATIO>";
	// 画面宽高比
	QString aspectRatio;

	int asw = m_aspect & 0x0000ffff;
	int ash = (m_aspect & 0x0fff0000) >> 16;

	float F1 = asw*1.0 / ash;
	if (F1>=(float)1.23 && F1<(float)1.27) aspectRatio=("5:4");
	else if (F1>=(float)1.30 && F1<(float)1.37) aspectRatio=("4:3");
	else if (F1>=(float)1.45 && F1<(float)1.55) aspectRatio=("3:2");
	else if (F1>=(float)1.74 && F1<(float)1.82) aspectRatio=("16:9");
	else if (F1>=(float)1.82 && F1<(float)1.88) aspectRatio=("1.85:1");
	else if (F1>=(float)2.15 && F1<(float)2.22) aspectRatio=("2.2:1");
	else if (F1>=(float)2.23 && F1<(float)2.30) aspectRatio=("2.25:1");
	else if (F1>=(float)2.30 && F1<(float)2.37) aspectRatio=("2.35:1");
	else if (F1>=(float)2.37 && F1<(float)2.45) aspectRatio=("2.40:1");
	else              aspectRatio="其它";

	//switch(info.vinfo.aspect & 0x0fffffff)
	//{
	//case 0x00030004:
	//	aspectRatio = tr("4:3");
	//	break;
	//case 0x00040005:
	//	aspectRatio = tr("5:4");
	//	break;
	//case 0x00090010:
	//	aspectRatio = tr("16:9");
	//	break;
	//case 0x000a0010:
	//	aspectRatio = tr("16:10");
	//	break;
	//default:
	//	aspectRatio = baudio ? "" : tr("其它");
	//}
	strXml += aspectRatio;	
	strXml += tr("</ASPECT_RATIO><SYSTEM>");
	// 制式
	QString strType;
	switch(vxGetVideoStandard(info.vinfo.rate, info.vinfo.scale))
	{
	case vxNtscType:
		strType = tr("NTSC");
		break;
	case vxPalType:
		strType = tr("PAL");
		break;
	case vxSecamType:
		strType = tr("SECAM");
		break;
	default:
		{
			strType = baudio ? "" : tr("其它");
		}

	}
	strXml += strType;
	strXml += tr("</SYSTEM><AUDIO_DATARATE>");
	// 音频数据码率
	QString strAudioBitrate;
	int abitrate = info.ainfo.bitrate > g_maxsize ? 0 : info.ainfo.bitrate;
	strXml += strAudioBitrate.setNum(abitrate);
	strXml += tr("</AUDIO_DATARATE><AUCODING_FORMAT>");
	// 音频编码格式

	QString fourccStr;
	if (info.ainfo.fourcc == vxFormat_MPA && info.ainfo.reserved[1] != 0)
	{
		const char* MpaLayer[]={"mp1","mp2","mp3"};
		float version = info.ainfo.reserved[1];
		if (version == 3.0 )
		{
			version = 2.5;
		}
		fourccStr = QString("MPEG Audio Version %1 Layer %2\n").arg(version, 0, 'f', 2).arg(info.ainfo.reserved[0]);
	}
	else
	{
		fourccStr = QString::fromLocal8Bit(vxGetFormatType(info.ainfo.fourcc));
	}

	if (info.audios>1)
	{
		for ( int i = 0 ; i < info.audios - 1; i ++)
		{
			if (info.painfo == NULL)
			{
				break;
			}
			VX_AUDIOINFO2 AInfoTemp = info.painfo[i];
			if (AInfoTemp.channels == 6) // 如果是杜比素材, 特殊处理
			{
				fourccStr = "Dolby E";
				break;
			}
		}
	}


	strXml += fourccStr;
	strXml += tr("</AUCODING_FORMAT><AUSAMPLING_FRE>");
	// 音频采样频率
	QString strSamplePerSec;
	switch(info.ainfo.freq)
	{
	case 192000:
		strSamplePerSec = tr("192 KHz");
		break;
	case 96000:
		strSamplePerSec = tr("96 KHz");
		break;
	case 48000:
		strSamplePerSec = tr("48 KHz");
		break;
	case 44100:
		strSamplePerSec = tr("44.1 KHz");
		break;
	case 32000:
		strSamplePerSec = tr("32 KHz");
		break;
	default:
		strSamplePerSec = tr("其它");
	}
	strXml += strSamplePerSec;
	strXml += tr("</AUSAMPLING_FRE><AUDIO_BITDEPTH>");
	// 音频位深度
	QString strBitDepth;
	switch(info.ainfo.bitpersample)
	{
	case 8:
		strBitDepth = tr("8 bits");
		break;
	case 16:
		strBitDepth = tr("16 bits");
		break;
	case 24:
		strBitDepth = tr("24 bits");
		break;
	default:
		strBitDepth = tr("其它");
	}
	strXml += strBitDepth;
	strXml += tr("</AUDIO_BITDEPTH><VIDEO_BITRATE>");
	// 视频数据码率
	QString strVideoBitrate;
	int vbitrate = info.vinfo.bitrate > g_maxsize ? 0 : info.vinfo.bitrate;
	strXml += strAudioBitrate.setNum(vbitrate);
	strXml += tr("</VIDEO_BITRATE><VICODING_FORMAT>");
	// 视频编码格式
	strXml += vxGetFormatType(info.vinfo.fourcc);
	strXml += tr("</VICODING_FORMAT><VISAMPLING_TYPE>");
	// 视频取样格式
	QString strChroma;
	switch(info.vinfo.chromafmt)
	{
	case 1:
		strChroma = tr("4:2:0");
		break;
	case 2:
		strChroma = tr("4:2:2");
		break;
	case 3:
		strChroma = tr("4:4:4");
		break;
	default:
		strChroma = baudio ? "" : tr("其它");
	}
	strXml += strChroma;
	strXml += tr("</VISAMPLING_TYPE><FILE_FORMAT>");
	// 文件格式
	strXml += vxGetFormatType(info.muxinfo.fourcc);
	strXml += tr("</FILE_FORMAT><FRAME_RATE>");
	// 帧率
	QString strRate;
	strRate.setNum(info.vinfo.rate);
	strXml += strRate;
	strXml += tr("</FRAME_RATE><FRAME_SCALE>");
	// 帧率倍数
	QString strScale;
	strRate.setNum(info.vinfo.scale);
	strXml += strScale;
	strXml += tr("</FRAME_SCALE><BROWSE_WIDTH>");
	// 视频窗口大小－宽
	QString strWidth;
	strWidth.setNum(info.vinfo.display_width);
	strXml += strWidth;
	strXml += tr("</BROWSE_WIDTH><BROWSE_LENGTH>");
	// 视频窗口大小－高
	QString strHeight;
	strHeight.setNum(info.vinfo.display_height);
	strXml += strHeight;
	strXml += tr("</BROWSE_LENGTH>");

	strXml += tr("</FILEINFO>");
	return strXml;
}

void CVxPlayWidget_W::GetFrameRate(double &dFrameRate)
{
	if (m_bHasSource)
	{
		dFrameRate = m_frameRate;
	}
	else
	{
		dFrameRate = 0.0;
	}
}

void CVxPlayWidget_W::registerObserver()
{
	IMsgObserver *pMsgObservers = GetIMsgObserver();
	pMsgObservers->AddObserver(this,TURBO_EDIT_CLIPPLAY_CURRENTFRAME,this);
	pMsgObservers->AddObserver(this,TE_ClipSeekNotification,this);
	pMsgObservers->AddObserver(this,TURBO_EDIT_PLAYVIEW_AUDIODATA,this);
	pMsgObservers->AddObserver(this,TURBO_EDIT_CLIPPLAY_COMPLETE,this);
	pMsgObservers->AddObserver(this,TE_SwitchToClipPlayModeNotification,this);           // 添加安全框时需要  add 2012/4/9 CB
}

bool CVxPlayWidget_W::event(QEvent *event)
{
	QEvent::Type type = event->type();



	if (type > QEvent::User)
	{
		switch (type)
		{
		case TURBO_EDIT_CLIPPLAY_CURRENTFRAME:
			{
				if(!m_pPlayViewController->GetClipNeedCallbackPlayFrame())//如果不需要回调当前帧，一般都是seek了后，根本不需要再回调了。
					break;

				if (m_curFrame != -1) //到头和到尾
				{
					break;
				}

				CVxDataEvent *vxDataEvent = static_cast<CVxDataEvent *>(event);
				int playHeadIndicator = (int)(qptrdiff)vxDataEvent->data();
				if (playHeadIndicator > m_duration) break;  //防止播放到最后一帧后很多帧
				m_pPlayViewController->setPlayHeadIndicator(playHeadIndicator);
				m_pCurLineEdit->setCurFrame((playHeadIndicator));
				m_pPlayRuler->SetCurrentFrame(playHeadIndicator, false);
				m_pPlayRuler->update();
			}
			break;
		case TE_ClipSeekNotification:
			{
				CVxPointerIntEvent *vxPointerIntEvent = static_cast<CVxPointerIntEvent *>(event);
				clipSeek((int)vxPointerIntEvent->data());
			}
			break;
		case TURBO_EDIT_PLAYVIEW_AUDIODATA:
			{
				CAudioDataEvent *vxDataEvent = static_cast<CAudioDataEvent *>(event);
				AUDIOUVMETER audioData = vxDataEvent->m_audioData;
				if (m_pPlayViewController->isPlay() || m_bSeekTo)
				{
					m_meter1->setLevel(audioData.outlevel[0]);
					m_meter2->setLevel(audioData.outlevel[1]);
				}
				else
				{
					m_meter1->setLevel(-100);
					m_meter2->setLevel(-100);
				}

				
			}
			break;
		case TURBO_EDIT_CLIPPLAY_COMPLETE:
			{
				if(m_pPlayViewController->isPlay())
				{
					break;
				}
				m_pManager->displayPlayImage(true);
			}
			break;		
		default:
			break;
		}
	}
	return QWidget::event(event);
}



void CVxPlayWidget_W::resizeEvent(QResizeEvent * event)
{
	const QRect frameRect =  this->rect();
	const int timeCodeEditHeight = 20;
	int controlButtomHeight = Controller_Bottom_Inteval - 20;

	m_playRectHeight = frameRect.height() - m_pPlayRuler->height() - timeCodeEditHeight;
	m_pPlayRuler->setGeometry(Left_Interval, frameRect.bottom() - m_pPlayRuler->height() - timeCodeEditHeight, frameRect.width() - Left_Interval * 2, m_pPlayRuler->height());

	/* 排列5个TimeCodeEdit */
	int center = frameRect.left()+frameRect.width()/2;
	int top = frameRect.bottom() -timeCodeEditHeight;
	int width = (frameRect.width() - m_pInNorImage->rect().width()*2 - 20) / 5;
	if (width > 96)
	{
		width = 96;
	}
	m_pCurLineEdit->setGeometry(1, top, width, 20);
	m_pTotalLineEdit->setGeometry(frameRect.right()-width-1, top, width, 20);
	m_pAreaLineEdit->setGeometry(center - width/2, top, width, 20);
	m_pInPointEdit->setGeometry(center-width/2 - width -4, top, width, 20);

	m_pSetInBtn->setGeometry(center-width/2 - width -4 -m_pInNorImage->rect().width() -2 , top, m_pInNorImage->rect().width(), 
		m_pInNorImage->rect().height());
	m_pOutPointEdit->setGeometry(center+ width/2 + 4, top, width, 20);
	m_pSetOutBtn->setGeometry(center+width/2 + width + 2, top, m_pOutNorImage->rect().width(), 
		m_pOutNorImage->rect().height());



	//reSetPlayRect(m_playRectHeight, frameRect.width() - Left_Interval * 2, rscreccn_l, rscreccn_t, rscreccn_r, rscreccn_b);
	int uvmeter_w = 12;
	if (m_bHasAudio)
	{
		m_meter1->setGeometry(0,0,uvmeter_w,m_playRectHeight);
		m_meter2->setGeometry(frameRect.width()-uvmeter_w, 0, uvmeter_w, m_playRectHeight);
		m_meter1->show();
		m_meter2->show();
	}
	else
	{
		uvmeter_w = 0;
		m_meter1->hide();
		m_meter2->hide();
	}


	QRect playRect(uvmeter_w+1, 0, frameRect.width()-2*uvmeter_w, m_playRectHeight);
	m_needFullRect = playRect;
	NeedFulls();
	DrawExt();
	m_pPlayWidget->setGeometry(playRect);

	int managerWidth = m_pManager->GetWindowWidth();
	m_pManager->setGeometry((frameRect.width() - managerWidth)/2, frameRect.bottom()-m_pPlayRuler->height()-timeCodeEditHeight-controlButtomHeight,   
		managerWidth, controlButtomHeight);
	m_pManager->resizeButtonFrame();
	m_pManager->hide();

	m_pAudio->setGeometry(frameRect.width()-28-20, frameRect.bottom()-m_pPlayRuler->height()-timeCodeEditHeight-controlButtomHeight-100
		, 10, 80);
}

void CVxPlayWidget_W::ShowFileInfo()
{
	if (m_fileName != m_fileNameAttribute && m_compoundFile)
	{
		{
			string vidPath = m_fileName.toLocal8Bit().data();
			VXFILEPATH vx_filePath;
			VidPath2VxFilePath(vidPath, vx_filePath);
			CVxComPtr <IVxCompoundFile> pTempFile;
			vxCreateCompoundFile("bay", "760417", &pTempFile);
			pTempFile->Add(vx_filePath);
			pTempFile->SetDemuxType(cd_native);
			int iCurNo = pTempFile->GetAudStreamNo();
			pTempFile->SetAudStreamNo(-1);
			VXSIZE size;
			size.cx = 80;
			size.cy = 60;
			vxFreeMFileInfo(&m_fileInfoAttribute);
			memset(&m_fileInfoAttribute, 0, sizeof(m_fileInfoAttribute));
			m_fileInfoAttribute = vxGetMFileInfo(pTempFile, 0, m_InfoTools, true, true, 0, size);
			pTempFile->SetAudStreamNo(iCurNo);
			pTempFile->SetDemuxType(cd_spliteaudio);		
			pTempFile->StateEvent(cereload,NULL);
		}
	}
	m_fileNameAttribute = m_fileName;
	emit ShowFileInfoSig(m_fileNameAttribute, m_fileInfoAttribute);
}

void CVxPlayWidget_W::ShowBar()
{
	if (!m_pManager->isHidden())
	{
		m_pManager->hide();
	}
}

void CVxPlayWidget_W::SetFullScreen(bool FullScreen)
{
	
	if (FullScreen)
	{
		SetAudioEnabled(false);
		m_normalRect = this->rect();
		setWindowFlags(Qt::Dialog);
		showFullScreen();
		//showMaximized();
	}
	else
	{
		SetAudioEnabled(true);
		setWindowFlags(Qt::SubWindow);
		showNormal();
		setFocus();
		setGeometry(m_normalRect.left(), m_normalRect.top(), m_normalRect.width(), m_normalRect.height());
	}
}

#define REPEAT(repeat) if (repeat) return;
void CVxPlayWidget_W::keyPressEvent(QKeyEvent * event)
{
	bool repeat = event->isAutoRepeat();

	switch (event->key())
	{
	//case Qt::Key_Home:
	case Qt::Key_A:
		{
			REPEAT(repeat);
			if (event->modifiers() != Qt::NoModifier) return;
			if (m_bHasSource)
			{
				home();
			}			
			return;
		}
	case Qt::Key_Q:
		{
			REPEAT(repeat);
			if (event->modifiers() != Qt::NoModifier) return;
			goToInPoint();
			return;
		}
	case Qt::Key_Left:
		{
			if (event->modifiers() != Qt::NoModifier) return;
			prevFrame();
			return;
		}
	case Qt::Key_Down:
		{
			if (event->modifiers() & Qt::ControlModifier)            //Ctrl + Down
			{
				AudioVolumeChange(false);
			}
			else
			{
				backward();
			}

			return;
		}
	case Qt::Key_Space:
		{
			REPEAT(repeat);
			if (event->modifiers() != Qt::NoModifier) return;
			m_playMode = PLAY;
			m_pPlayViewController->setIsSpecialPlay(false);
			play();
			return;
		}
	case Qt::Key_Up:
		{
			if (event->modifiers() & Qt::ControlModifier)            //Ctrl + Up
			{
				AudioVolumeChange(true);
			}
			else
			{
				forward();
			}
			return;
		}
	case Qt::Key_Right:
		{
			if (event->modifiers() != Qt::NoModifier) return;
			nextFrame();
			return;
		}
	case Qt::Key_W:
		{
			REPEAT(repeat);
			if (event->modifiers() != Qt::NoModifier) return;
			goToOutPoint();
			return;
		}
	//case Qt::Key_End:
	case Qt::Key_S:
		{
			REPEAT(repeat);
			if (event->modifiers() != Qt::NoModifier) return;
			if (m_bHasSource)
			{
				end();
			}

			return;
		}
	case Qt::Key_I:
		{
			REPEAT(repeat);
			if (event->modifiers() != Qt::NoModifier) return;
			setInPosSlot();
			return;
		}
	case Qt::Key_O:
		{
			REPEAT(repeat);

			if (event->modifiers() & Qt::ControlModifier)            //Ctrl + O
			{
				openFileDlgSlot();
			}
			else
			{
				if (event->modifiers() != Qt::NoModifier) return;
				setOutPosSlot();
			}
			return;
		}
	case Qt::Key_Escape:
		{
			REPEAT(repeat);
			if (event->modifiers() != Qt::NoModifier) return;
			if (m_bFullScreen)
			{
				m_bFullScreen = false;
				m_pFullScreenAct->setText("全屏显示");
				SetFullScreen(m_bFullScreen);
			}
			return;
		}

	default:
		;
	}
	CBasePlayWidget::keyReleaseEvent(event);
}




void CVxPlayWidget_W::mouseMoveEvent(QMouseEvent * event)
{
	QRect rc = m_pPlayWidget->geometry();
	if (rc.contains(event->pos()))
	{
		// 当窗口有MouseMove时 显示m_pManager并设置隐藏显示的定时器
		if (m_pManager->isHidden())
		{
			m_pManager->show();
		}
		if (m_timer->isActive())
		{
			m_timer->stop();
		}	
		m_timer->start(600);
	}

}

void CVxPlayWidget_W::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	//QBrush brush(QColor(38,38,38));
	QRect rect = this->rect();
	painter.fillRect(rect,QColor(50,50,50));
}

void CVxPlayWidget_W::onBtnClicked(int iOptType)
{
	switch (iOptType)
	{
	case Option_Play:
		{			
			m_playMode = PLAY;
			m_pPlayViewController->setIsSpecialPlay(false);
			play();
		}
		break;
	case Option_Pause:
		{
			play();
		}
		break;
	case Option_GoHome:
		{
			home();
		}
		break;
	case Option_GoEnd:
		{
			end();
		}
		break;
	case Option_GoInPoint:
		{
			goToInPoint();
		}
		break;
	case Option_GoOutPoint:
		{
			goToOutPoint();
		}
		break;
	case Option_PreFrame:
		{
			prevFrame();
		}
		break;
	case Option_NextFrame:
		{
			nextFrame();
		}
		break;
	case Option_Backward:
		{
			backward();
		}
		break;
	case Option_Forward:
		{
			forward();
		}
		break;
	default :
		break;
	}
}

void CVxPlayWidget_W::goToInPoint()
{
	if (!m_bHasSource)
	{
		return;
	}

	int inPoint = 0, outPoint = 0;
	m_pPlayRuler->GetInOutPoint(inPoint, outPoint);
	if (inPoint != -1)
	{
		if (m_pPlayViewController->isPlay())
		{
			m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);
			m_pPlayViewController->stop();
		}
		m_pPlayViewController->goToInPoint(inPoint);
	}
	else
	{
		//m_pPlayViewController->goToInPoint(0);
	}
}

void CVxPlayWidget_W::goToOutPoint()
{
	if (!m_bHasSource)
	{
		return;
	}

	int inPoint = 0, outPoint = 0;
	m_pPlayRuler->GetInOutPoint(inPoint, outPoint);
	if (outPoint != -1)
	{
		if (m_pPlayViewController->isPlay())
		{
			m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);
			m_pPlayViewController->stop();
		}
		m_pPlayViewController->goToOutPoint(outPoint);
	}
	else
	{
		//m_pPlayViewController->goToOutPoint(m_pPlayRuler->GetDuration() - 1);
	}
}

void CVxPlayWidget_W::nextFrame()
{
	m_pPlayViewController->nextFrame();
}

void CVxPlayWidget_W::prevFrame()
{
	m_pPlayViewController->prevFrame();
}

void CVxPlayWidget_W::home()
{
	m_curFrame = m_pPlayViewController->getPlayHeadIndicator();
	m_pPlayViewController->home();
}

void CVxPlayWidget_W::end()
{
	m_curFrame = m_pPlayViewController->getPlayHeadIndicator();
	m_pPlayViewController->end();
}

void CVxPlayWidget_W::nextSecond()
{
	m_pPlayViewController->nextSecond();
}

void CVxPlayWidget_W::prevSecond()
{
	m_pPlayViewController->prevSecond();
}

void CVxPlayWidget_W::backward()
{
	int newPos = m_pPlayViewController->getPlayHeadIndicator();
	if (0==newPos)
	{
		return;
	}
	if (PlayInCtrlRange())
	{
		return;
	}

	QAction *pAct = m_pSpeedGroupAct->checkedAction();
	int speed = pAct->data().toInt();
	m_pPlayViewController->SetSpeedStep(speed);

	if (m_pPlayViewController->isPlay())
	{
		m_pPlayViewController->backward();
	}
	else
	{
		m_pPlayViewController->setSpeed(0);
		m_pPlayViewController->backward();
		m_pPlayViewController->setIsSpecialPlay(true);
		play();
	}
	m_playMode = BACKWARDPALY;
	IVxAVPlayer *pplay = m_player;
	IVxAVPlayer2 *pplay2 = dynamic_cast<IVxAVPlayer2*>(pplay);
	pplay2->SetPlaySpeed(m_pPlayViewController->getSpeed());

	m_speedPlayEnd = m_duration % speed ;

}

void CVxPlayWidget_W::forward()
{
	int curPos = m_pPlayViewController->getPlayHeadIndicator();
	int duration = m_pPlayViewController->getDuration();
	if (curPos >= duration - 1)
	{
		return;
	}

	if (PlayInCtrlRange())
	{
		return;
	}

	QAction *pAct = m_pSpeedGroupAct->checkedAction();
	int speed = pAct->data().toInt();
	m_pPlayViewController->SetSpeedStep(speed);

	if (m_pPlayViewController->isPlay())
	{
		m_pPlayViewController->forward(); // 已经播放了就加速
	}
	else
	{
		m_pPlayViewController->forward();
		m_pPlayViewController->setIsSpecialPlay(true);
		play();
	}
	m_playMode = FORWARDPLAY;

	IVxAVPlayer *pplay = m_player;
	IVxAVPlayer2 *pplay2 = dynamic_cast<IVxAVPlayer2*>(pplay);
	pplay2->SetPlaySpeed(m_pPlayViewController->getSpeed());

	m_speedPlayEnd = m_duration % speed ;
}

void CVxPlayWidget_W::play()
{
	if (!m_bHasSource)
	{
		return;
	}

	/* 0021308 播放中点击时间尺迅速空格播放时间头会跳回上次点继续播放, 
	   原因分析: 底层还在返回数据.
	   解决方案: 如果是点击时间尺,刚延时播放
	*/
	if (m_playRulerDragType == CVxRuler::DraggingStatus_Begin)
	{
		QElapsedTimer timer;
		timer.start();
		while (timer.elapsed() < 200)
		{
			qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
		}
		m_playRulerDragType = -1;
	}

	m_bSeekTo = false;
	m_pPlayViewController->SetIsCtrlRange(true);
	m_pPlayViewController->SetClipNeedCallbackPlayFrame(true);
	m_pManager->displayPlayImage(false);
	m_pPlayViewController->play();
	m_curFrame = -1;
	m_inputOutputTimer->start(300);

}

void CVxPlayWidget_W::pause()
{
	if (!m_bHasSource)
	{
		return;
	}
	stop();
}

bool CVxPlayWidget_W::SendData(char* Ip, int Port, char* sendData, int sendLength, int recvLength, char* recvData)
{
	bool     bRet = false;

#ifdef Q_OS_WIN
	SOCKET   client_sock;
	int      ret = -1;

	struct sockaddr_in	dest_sin;
	memset(&dest_sin , 0 , sizeof( dest_sin ) );
	dest_sin.sin_family = AF_INET;
	dest_sin.sin_addr.s_addr = inet_addr(Ip);
	dest_sin.sin_port = htons(Port);

	client_sock = socket(AF_INET, SOCK_STREAM, 0);


	if (::connect(client_sock, (const struct sockaddr*)&dest_sin, sizeof(dest_sin)) != 0)
	{
		goto end;
	}

	ret = send( client_sock , sendData , sendLength , 0 );
	if (ret < sendLength)  //数据发送不完全
	{
		goto end;
	}

	ret = recv( client_sock , recvData , recvLength , 0 );

end:
	closesocket(client_sock);
#endif
	return 	bRet;
}

void CVxPlayWidget_W::stop()
{
	m_pPlayViewController->SetClipNeedCallbackPlayFrame(true);
	if (m_pPlayViewController->isPlay())
	{
		m_pManager->displayPlayImage(true);
		m_pPlayViewController->stop();
	}
}

void CVxPlayWidget_W::CloseFile()
{
	if (m_bHasSource)
	{

		stop();
		m_bHasSource = false;
		m_fileName.clear();
		m_pPlayRuler->RemoveInPoint(true);
		m_pPlayRuler->RemoveOutPoint(true);
		SeekTo(0);
		ResetPlay();
		ResetPlayWidget();

	}
	else
	{
		m_pPlayViewController->setPlayHeadIndicator(0);
	}
}

void CVxPlayWidget_W::setCurFrame(int frame)
{
	m_pPlayViewController->setPlayHeadIndicator(frame, true);
}

void CVxPlayWidget_W::SetSetup(IVxSystemSetup *aSetup)
{
	m_Setup = aSetup;
}

void CVxPlayWidget_W::SetPlayer(void *pplayer)
{
	m_player = (IVxAVPlayer*)pplayer;
	m_pPlayViewController->SetPlayer(pplayer);
	//QRect rc = m_pPlayWidget->rect();
	//m_pPlayViewController->setHwnd(m_pPlayWidget, rc);
	m_pPlayWidget->SetPlayer(m_player);
}

void CVxPlayWidget_W::SetInfoTool(IVxClipInfoTools *aInfoTools)
{
	m_InfoTools = (IVxClipInfoTools2*)aInfoTools;
}



bool CVxPlayWidget_W::SetSource(VX_AVPATH avPath)
{
	m_bHasSource = false;
	m_pPlayViewController->SetCurDataCore(NULL); // datacore获取真实帧数(少于100)
	ResetPlay(false);

	string vidPath = avPath.szPath;
	VXFILEPATH vx_filePath;
	VidPath2VxFilePath(vidPath, vx_filePath);

	m_compoundFile = NULL;
	vxCreateCompoundFile("bay", "760417", &m_compoundFile);
	m_compoundFile->Add(vx_filePath);

	// check clips
	vxFreeMFileInfo(&m_fileInfo);
	memset(&m_fileInfo, 0, sizeof(m_fileInfo));

	VXSIZE size;
	size.cx = 80;
	size.cy = 60;




	//bool bMultAStream = isMultiChannels(m_InfoTools, m_compoundFile);
	//if (bMultAStream)
	{
		CVxComPtr <IVxCompoundFile> pTempFile;
		vxCreateCompoundFile("bay", "760417", &pTempFile);
		for(int i = 0; i < m_compoundFile->GetCount(); ++i)
		{
			pTempFile->Add(m_compoundFile->GetAt(i));
		}
		pTempFile->SetDemuxType(cd_native);
		int iCurNo = pTempFile->GetAudStreamNo();
		pTempFile->SetAudStreamNo(-1);
		m_fileInfo = vxGetMFileInfo(pTempFile, 0, m_InfoTools, false, true, 0, size);
		pTempFile->SetAudStreamNo(iCurNo);
		pTempFile->SetDemuxType(cd_spliteaudio);		
		pTempFile->StateEvent(cereload,NULL);
	}
	//else
	//{
	//	m_fileInfo = vxGetMFileInfo(m_compoundFile, 0, m_InfoTools, false, true, 0, size, 0);
	//}
	
	
	bool canplay = true;
	if (m_fileInfo.info.type == MI_CANNOTUSE)
	{
		canplay = false;
	}
	else
	{
		// 如果是视频文件但无icon
		if (m_fileInfo.info.type == MI_VIDEO && !m_fileInfo.icon)
		{
			canplay = false;
		}
		//// 如果是AV文件但无A且V
		if (m_fileInfo.info.type == MI_AV && 
			(!m_fileInfo.icon || !m_fileInfo.info.audios))
		{
			canplay = false;
		}
	}
	if (!canplay)
	{
		m_compoundFile = NULL;
		//CIEMsgBox box;
		//box.Show(this, "警告", "无法播放此文件");
		//m_preFilePath = prefilePathTemp;
		return false;
	}

		
	MATTERINFO2 &matterInfo = m_fileInfo.info;
	if (matterInfo.vinfo.scale != 0)
	{
		m_frameRate = matterInfo.vinfo.rate / matterInfo.vinfo.scale;
	}
	else
	{
		m_frameRate = 0.0;
	}

	QString filePathSrc(avPath.szPath);
	if (filePathSrc.endsWith(".flv", Qt::CaseInsensitive) &&
		m_frameRate < 23)
	{
		m_compoundFile = NULL;
		//CIEMsgBox box;
		//box.Show(this, "警告", "暂不支持帧数小于23的FLV");
		return false;
	}

	ResetFreq(m_frameRate);
	// can setsource
	m_demultiplexer = NULL;
	m_Setup->CreateDemultiplexer2(m_compoundFile, &m_demultiplexer);
	if (!m_demultiplexer)
	{
		return false;
	}
	m_player->SetSource(m_demultiplexer);
	m_player->SeekTo(0);
	m_pPlayViewController->setPlayHeadIndicator(0);

	
	m_duration = m_pPlayViewController->getDuration();
	
	if (0 == m_duration)
	{
		QString errMsg;
		errMsg = tr(avPath.szPath);
		durationChangedSlot(200);
		return false;
	}

	m_preFilePath = avPath.szPath;


	durationChangedSlot(m_duration);

	m_bSd = matterInfo.vinfo.display_width <= 720;
	m_bHasSource = true;
	double m_WidHeiRatio = 0;
	
	ShowSafeFrame();

	int type = matterInfo.type;
	if (MI_CANNOTUSE == type || MI_AUDIO == type)
	{
		m_WidHeiRatio = 1.0*16/9;
		m_pPlayWidget->setRatio(m_WidHeiRatio);
		m_pPlayWidget->ResetHwnd();
		return true;
	}
	m_aspect = matterInfo.vinfo.aspect;


	// 
	int asw = m_aspect & 0x0000ffff;
	int ash = (m_aspect & 0x0fff0000) >> 16;
	if (ash)
	{
		m_WidHeiRatio = 1.0 * asw / ash;
	}
	else
	{
		// 当m_aspect为0时
		bool sd = m_fileInfo.info.vinfo.display_width <= 720;
		if (sd)
		{
			m_WidHeiRatio = 1.0 * 4/3;
		}
		else
		{
			m_WidHeiRatio = 1.0 * 16/9;
		}
	}






	m_player->SetClipMixer(g_mixer); //调音量
	m_pAudio->SetValue(m_audioVolume, false);


	m_audioRect.setRect(0,0,matterInfo.vinfo.display_width, matterInfo.vinfo.display_height);
	NeedFulls();
	DrawExt();

	m_pPlayWidget->setRatio(m_WidHeiRatio);
	m_pPlayWidget->ResetHwnd();

	SlotResetHwnd();

	return true;
}

bool CVxPlayWidget_W::SetSource( IVxDataCore *pDataCore, bool bSD )
{
		m_bSd = bSD;
		m_bHasSource = false;
	//	SetPlayer(m_player);
		IVxAVPlayer *pCurPlay = m_player;
		IVxAVPlayer2 *pPlay2 = (IVxAVPlayer2*)pCurPlay;

		// 要先设成mlp_single模式,setsource后会自动变回来
		pPlay2->SetMode(mlp_single);
		ShowSafeFrame(); 

		// 播放序列的时候大于400*300就开启全部
		m_audioRect.setRect(0,0,400*300*4, 1);
		DrawExt();

		double m_WidHeiRatio = 0;
		pPlay2->SetSource2(pDataCore);
		pPlay2->SeekTo(0);
		m_pPlayViewController->setPlayHeadIndicator(0);
		m_pPlayViewController->SetCurDataCore(pDataCore); // 其它不是工作文件的都要设为NULL
		//long duration = m_player->GetDuration();
		int left = 0;
		int duration = 0;
		pDataCore->GetDuration(left, duration);
		m_duration = duration;
		if (m_duration <= 0)
		{
			durationChangedSlot(200);
			return false;
		}

		durationChangedSlot(m_duration);

		if (bSD)
		{
			m_WidHeiRatio = 1.0*4/3;
		}
		else
		{
			m_WidHeiRatio = 1.0*16/9;
		}

		m_frameRate = 25;
		m_bHasSource = true;
		m_player->SetClipMixer(g_mixer); //直接混

		m_pPlayWidget->setRatio(m_WidHeiRatio); // 暂时这样处理
		m_pPlayWidget->ResetHwnd();

		SlotResetHwnd();

		return true;
}


void CVxPlayWidget_W::SetSource(IVxSystemSetup *setup,IVxSource *source)
{
	m_bHasSource = false;
	m_pPlayViewController->SetCurDataCore(NULL);
	double m_WidHeiRatio = 1.0*16/9;	
	m_pPlayWidget->setRatio(m_WidHeiRatio);

	if(source == NULL)
	{
		m_player->SetSource((IVxReadStream*)NULL,(IVxReadStream*)NULL,0,NULL);
		m_pPlayWidget->ResetHwnd();
		m_player->Redraw(0);
		m_player->SeekTo(0,true);
		return ;
	}
	CVxComPtr<IVxDemultiplexer> bluray;
	if(setup->CreateDemultiplexer(source,vxUnparkSub_TS,&bluray)!=0) return;

	CVxComPtr<IVxReadStream> vstream,astream,tstream;
	bluray->GetStream(vxstreamVIDEO,0,&vstream);
	bluray->GetStream(vxstreamAUDIO,0,&astream);
	bluray->GetStream(vxstreamSUBPIC,0,&tstream);

	IVxReadStream* auds[] = {astream};
	m_player->SetSource(vstream,tstream,1,auds);

	float mixer[AUDIO_INOUT_CHANEL][AUDIO_INOUT_CHANEL] = {0};
	for(int i=0;i<AUDIO_INOUT_CHANEL;i++)
		mixer[i][i] = 1.f;
	//mixer[0][2] = mixer[1][2] = 1.f;
	m_player->SetClipMixer(mixer);

	m_duration = m_player->GetDuration();
	durationChangedSlot(m_duration);
	m_pPlayWidget->ResetHwnd();
	m_player->Redraw(0);
	m_bHasSource = true;
}

void CVxPlayWidget_W::SetSource(IVxSystemSetup *setup,IVxCompoundFile* compoundFie,float aspect)
{
	m_bHasSource = false;
	m_pPlayViewController->SetCurDataCore(NULL);
	if (aspect > 0)
	{
		m_pPlayWidget->setRatio(aspect);
	}
	else
	{
		m_pPlayWidget->setRatio(1.0*16/9);
	}

	if (setup)
	{
		CVxComPtr<IVxDemultiplexer> demux;
		setup->CreateDemultiplexer2(compoundFie, &demux);
		if (demux)
		{
			CVxComPtr<IVxReadStream> streamv;
			CVxComPtr<IVxReadStream> streama[8];
			IVxReadStream *streamaa[8];
			demux->GetStream(vxstreamVIDEO, 0, &streamv);
			for(int j = 0; j < 8; ++j)
			{
				CVxComPtr<IVxReadStream> tempStream;
				demux->GetStream(vxstreamAUDIO, j, &tempStream);
				streama[j] = tempStream;
			}
			for(int j = 0; j < 8; ++j)
			{
				streamaa[j] = streama[j];
			}
			m_player->SetSource(streamv,8,streamaa);
			m_duration = m_player->GetDuration();
			durationChangedSlot(m_duration);
			m_pPlayWidget->ResetHwnd();
			m_player->Redraw(0);
			m_bHasSource = true;
		}
	}
}




void CVxPlayWidget_W::ResetPlayWidget()
{
	m_pPlayWidget->ResetHwnd();
}

void CVxPlayWidget_W::monitorSizeChangeSlot(QRect rc)
{
	QAction *pAct = m_pDrawExtGroupAct->checkedAction();
	int enable = 0;
	if (pAct)
	{
		enable = pAct->data().toInt();
	}
	m_pPlayViewController->setHwnd(m_pPlayWidget,rc, enable);
}

bool CVxPlayWidget_W::MoveMediaFile(const QString &srcFilePath,const QString &dstFilePath)
{
	bool bRet =false;
#ifdef Q_OS_WIN
	string szSrcFilePath = QStringToString(srcFilePath);
	string szDesFilePath = QStringToString(dstFilePath);
	if(_access(szSrcFilePath.c_str(),6) != 0)
	{
		return bRet;
	}
	bRet = MoveFileEx(szSrcFilePath.c_str(), szSrcFilePath.c_str(), MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING); 
	if(bRet)
	{
		bRet = MoveFileEx(szSrcFilePath.c_str(), szDesFilePath.c_str(), MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING); 
	}
#endif
	return bRet;
}

void CVxPlayWidget_W::appWillQuitSlot()
{
	if (m_pPlayViewController->isPlay())
	{
		m_pPlayViewController->stop();
	}
}

void CVxPlayWidget_W::playRulerFrameChangeSlot(int frame, bool isQuickSeek, int dragStatus)
{
	if (dragStatus == CVxRuler::DraggingStatus_Begin)
	{
		m_playRulerDragType = dragStatus;
	}
	else if (dragStatus == CVxRuler::DraggingStatus_Dragging)
	{
		m_playRulerDragType = -1;
	}

	if (m_bHasSource)
	{
		m_pPlayViewController->SeekTo(frame, isQuickSeek);
	}
	m_bSeekTo = true;
	m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);

	m_pPlayViewController->setPlayHeadIndicator(frame);
	m_pCurLineEdit->setCurFrame(frame);
	m_pPlayRuler->SetCurrentFrame(frame);

	//if (dragStatus==0)
	//{
	//	QElapsedTimer time;
	//	time.start();
	//	while (time.elapsed()<1000)
	//	{
	//		qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
	//	}
	//}
}

void CVxPlayWidget_W::durationChangedSlot(long duration)
{
	//++duration; // 多加一帧
	m_pPlayRuler->SetDuration(duration);
	m_pPlayRuler->SetVisibleRangle(0,duration);
	m_pPlayRuler->SetCurrentFrame(0);
	m_pPlayRuler->update();
	m_pAreaLineEdit->setTotalFrames(duration);
	m_pAreaLineEdit->setCurFrame(0);
	m_pInPointEdit->setCurFrame(0);
	m_pOutPointEdit->setTotalFrames(duration);
	m_pOutPointEdit->setCurFrame(0);
	m_pCurLineEdit->setTotalFrames(duration);
	m_pCurLineEdit->setCurFrame(0);
	m_pTotalLineEdit->setTotalFrames(duration +1);
	m_pTotalLineEdit->setCurFrame(duration);
}

void CVxPlayWidget_W::clipSeek(int frame)
{
	m_pPlayViewController->setPlayHeadIndicator(frame);
	m_pCurLineEdit->setCurFrame(frame);
	m_pPlayRuler->SetCurrentFrame(frame);	
}

void CVxPlayWidget_W::SeekTo(int index,bool bFlag)
{
	m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);
	clipSeek(index);
	m_pPlayViewController->SeekTo(index, false);
#ifdef Q_OS_WIN
	::Sleep(500);
#endif
}

VXPLAYVIEW_X_EXT CBasePlayWidget *CreatePlayWnd3(QWidget *parent)
{
	return new CVxPlayWidget_W(parent);
}

void CVxPlayWidget_W::mouseReleaseEvent( QMouseEvent* event)
{
	if (event->button()==Qt::RightButton)
	{
		CVxComPtr<IVxLiveExt2> liveext;
		m_player->QueryInterface(LIID_IVxLiveExt2,(void**)&liveext);

		QMenu PopUpMenu(this);
		if (!m_bAxFactory) // 单机起来的时候才有
		{
			PopUpMenu.addAction(m_pOpenFileAct);
			PopUpMenu.addAction(m_pOpenHttpFileAct);
		}

		PopUpMenu.addAction(m_pRebuildIndexAct);
		PopUpMenu.addAction(m_pClearInPointAct);
		PopUpMenu.addAction(m_pClearOutPointAct);
		//if (liveext)
		//{
			PopUpMenu.addAction(m_pSafeFrameAct);
		//}
		PopUpMenu.addMenu(m_pDrawExtMenu);
		PopUpMenu.addMenu(m_pPlaySpeedMenu);
		//PopUpMenu.addAction(m_pFullScreenAct);
		PopUpMenu.addSeparator();
		PopUpMenu.addAction(m_pShowFileInfoAct);

		// 版本信息
		PopUpMenu.addSeparator();
		PopUpMenu.addAction("版本号: 2.1.0411");
		PopUpMenu.exec(event->globalPos());
		setFocus();
	}
	if (event->button() == Qt::LeftButton)
	{ 

		QRect rc = m_pPlayWidget->geometry();
		if (rc.contains(event->pos()))
		{
			m_pPlayViewController->setIsSpecialPlay(false);
			if (!m_bClickDbl)
			{
				m_doubleTimer->start(200);
				m_resetDoubleTimer->start(1000);
			}		
		}

	}

}

void CVxPlayWidget_W::mouseDoubleClickEvent(QMouseEvent * event)
{

	if ( event->button() != Qt::LeftButton )
	{
		return;
	}


	//qDebug() << "mouseDoubleClickEvent";
	QRect rc = m_pPlayWidget->geometry();
	if (rc.contains(event->pos()))
	{
		m_bClickDbl = true;
		m_bFullScreen = !m_bFullScreen;
		SetFullScreen(m_bFullScreen);
	}

	CBasePlayWidget::mouseDoubleClickEvent(event);
}

void CVxPlayWidget_W::ShowSafeFrame()
{
	bool isShow = m_pSafeFrameAct->isChecked();
	//int status = VSF_SR_MATTER;
	PlayShow_SF(m_status,isShow);
}

bool CVxPlayWidget_W::IsClipNeedCallbackPlayFrame()
{
	return m_pPlayViewController->GetClipNeedCallbackPlayFrame();
}

void CVxPlayWidget_W::initPlayData()
{
	m_player->RegisterStateFunc(FCClipPlayCallback::CurrentFrame, this);
	m_player->RegisterUVMeterFunc(FCClipPlayAudioCallback::__audiovufunc, this);
	QRect rc = m_pPlayWidget->rect();

	VXRECT cxRect; 
	QPoint point = rc.topLeft();
	cxRect.left = point.x();
	cxRect.top = point.y();
	cxRect.right = point.x() + rc.width();
	cxRect.bottom = point.y() + rc.height();

	m_player->SetHwnd((HVXWND)m_pPlayWidget->winId(), &cxRect);
}

void CVxPlayWidget_W::PlayShow_SF(int status, bool isShow)
{
	if (m_pNLEEngine == NULL)
	{
		return;
	}

	CVxComPtr<IVxLiveCliper> cliper;
	m_player->QueryInterface(LIID_IVxLiveCliper,(void**)&cliper);
	if(cliper) 
	{
		cliper->SetDrawExt(DRAWEXT_FULLRES,m_bNeedFulls); //
		cliper->SetDrawExt(DRAWEXT_SAFTRECT,isShow);
		m_pPlayWidget->ResetHwnd();
	}
	SlotResetHwnd();
}

void CVxPlayWidget_W::clearInPoint()
{
	m_pPlayRuler->RemoveInPoint();
	m_pInPointEdit->setCurFrame(0);
	m_pAreaLineEdit->setCurFrame(0);
	m_pPlayRuler->SetDragInOut(true);
	m_pClearInPointAct->setEnabled(false);
	emit inPointChanged(0);
}

void CVxPlayWidget_W::clearOutPoint()
{
	m_pPlayRuler->RemoveOutPoint();
	m_pOutPointEdit->setCurFrame(0);
	m_pAreaLineEdit->setCurFrame(0);

	m_pClearOutPointAct->setEnabled(false);
	m_pPlayRuler->SetDragInOut(true);

}


void CVxPlayWidget_W::ResetPlay(bool b)
{
	if (m_player)
	{
		m_player->SetSource((IVxReadStream*)NULL,(IVxReadStream*)NULL,0,NULL);
		m_player->Redraw(0);
		VXRECT rect;
		if (b)
		{
			m_player->SetHwnd((HVXWND)NULL,&rect);
		}
		

		m_pPlayRuler->SetCurrentFrame(0);
		m_pPlayRuler->update();
		m_pAreaLineEdit->setCurFrame(0);
		m_pInPointEdit->setCurFrame(0);
		m_pOutPointEdit->setCurFrame(0);
		m_pCurLineEdit->setCurFrame(0);
		m_pCurLineEdit->setTotalFrames(200);
		m_pTotalLineEdit->setCurFrame(200);
		m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);

		m_compoundFile = NULL;
		m_demultiplexer = NULL;
		m_InfoTools->Reset();
	}
}

void CVxPlayWidget_W::DrawExt( bool bDraw /*= false*/ )
{
	QAction *pAct = (QAction*)sender();
	pAct = bDraw ? pAct : m_pDrawExtGroupAct->checkedAction();

	int enable = pAct->data().toInt();
	
	CHECK_PLAY_PRT_NUL(m_player);
	CVxComPtr<IVxLiveCliper> cliper;
	m_player->QueryInterface(LIID_IVxLiveCliper,(void**)&cliper);
	
	if(cliper/*&&!m_bMouseDown*/) 
	{
		cliper->SetDrawExt(DRAWEXT_FULLRES,m_bNeedFulls || enable); // m_bNeedFulls是播放窗口,enbale选择了反交织
		cliper->SetDrawExt(DRAWEXT_DEINTERLACE,enable);
		m_player->Redraw(0);
	}

}

void CVxPlayWidget_W::SlotPlaySpeed( bool b /*= true*/ )
{
	QAction *pAct = static_cast<QAction*>(sender());
	if (!pAct)
	{
		return;
	}

	int curSpeed = pAct->data().toInt();
	m_pPlayViewController->SetSpeedStep(curSpeed);
	

	if (m_pPlayViewController->isPlay() &&
		m_pPlayViewController->getIsSpecialPlay())
	{
		float oldSpeed = m_pPlayViewController->getSpeed();
		IVxAVPlayer *pplay = m_player;
		IVxAVPlayer2 *pplay2 = dynamic_cast<IVxAVPlayer2*>(pplay);
		if (pplay2)
		{
			pplay2->SetPlaySpeed(oldSpeed < 0.0f ? -curSpeed : curSpeed);
		}

	}	

}

void CVxPlayWidget_W::SlotFastForward()
{
	onBtnClicked(Option_Forward);
}

void CVxPlayWidget_W::SlotFastBackward()
{
	onBtnClicked(Option_Backward);
}

void CVxPlayWidget_W::InitAct()
{
	m_pDrawExtMenu = new QMenu(tr("反隔行"), this);
	m_pDrawExtAct0 = new QAction(tr("无"), this);
	m_pDrawExtAct0->setCheckable(true);
	m_pDrawExtAct0->setChecked(true);
	m_pDrawExtAct0->setData(0);
	m_pDrawExtAct1 = new QAction(tr("混合"),this);
	m_pDrawExtAct1->setCheckable(true);
	m_pDrawExtAct1->setChecked(false);
	m_pDrawExtAct1->setData(1);
	m_pDrawExtAct2 = new QAction(tr("边缘检测"),this);
	m_pDrawExtAct2->setCheckable(true);
	m_pDrawExtAct2->setChecked(false);
	m_pDrawExtAct2->setData(2);
	m_pDrawExtAct3 = new QAction(tr("中值阈值"),this);
	m_pDrawExtAct3->setCheckable(true);
	m_pDrawExtAct3->setChecked(false);
	m_pDrawExtAct3->setData(3);
	m_pDrawExtAct4 = new QAction(tr("自适应垂直/时态滤波"),this);
	m_pDrawExtAct4->setCheckable(true);
	m_pDrawExtAct4->setChecked(false);
	m_pDrawExtAct4->setData(4);
	m_pDrawExtAct5 = new QAction(tr("中值"),this);
	m_pDrawExtAct5->setCheckable(true);
	m_pDrawExtAct5->setChecked(false);
	m_pDrawExtAct5->setData(5);
	m_pDrawExtAct6 = new QAction(tr("时空内插"),this);
	m_pDrawExtAct6->setCheckable(true);
	m_pDrawExtAct6->setChecked(false);
	m_pDrawExtAct6->setData(6);
	m_pDrawExtAct7 = new QAction(tr("运动自适应混合"),this);
	m_pDrawExtAct7->setCheckable(true);
	m_pDrawExtAct7->setChecked(false);
	m_pDrawExtAct7->setData(7);
	m_pDrawExtGroupAct = new QActionGroup(this);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct0);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct1);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct2);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct3);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct4);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct5);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct6);
	m_pDrawExtGroupAct->addAction(m_pDrawExtAct7);


	connect(m_pDrawExtAct0, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));
	connect(m_pDrawExtAct1, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));
	connect(m_pDrawExtAct2, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));
	connect(m_pDrawExtAct3, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));
	connect(m_pDrawExtAct4, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));
	connect(m_pDrawExtAct5, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));
	connect(m_pDrawExtAct6, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));
	connect(m_pDrawExtAct7, SIGNAL(triggered(bool)), this, SLOT(DrawExt(bool)));

	m_pDrawExtMenu->addAction(m_pDrawExtAct0);
	m_pDrawExtMenu->addAction(m_pDrawExtAct1);
	m_pDrawExtMenu->addAction(m_pDrawExtAct2);
	m_pDrawExtMenu->addAction(m_pDrawExtAct3);
	m_pDrawExtMenu->addAction(m_pDrawExtAct4);
	m_pDrawExtMenu->addAction(m_pDrawExtAct5);
	m_pDrawExtMenu->addAction(m_pDrawExtAct6);
	m_pDrawExtMenu->addAction(m_pDrawExtAct7);



	m_pPlaySpeedMenu = new QMenu(tr("快进/快退倍数"), this);


	m_pSpeedAct2 = new QAction(tr("x2"),this);
	m_pSpeedAct2->setCheckable(true);
	m_pSpeedAct2->setChecked(true);
	m_pSpeedAct2->setData(2);
	m_pSpeedAct4 = new QAction(tr("x4"),this);
	m_pSpeedAct4->setCheckable(true);
	m_pSpeedAct4->setChecked(false);
	m_pSpeedAct4->setData(4);
	m_pSpeedAct8 = new QAction(tr("x8"),this);
	m_pSpeedAct8->setCheckable(true);
	m_pSpeedAct8->setChecked(false);
	m_pSpeedAct8->setData(8);

	m_pSpeedGroupAct = new QActionGroup(this);
	m_pSpeedGroupAct->addAction(m_pSpeedAct2);
	m_pSpeedGroupAct->addAction(m_pSpeedAct4);
	m_pSpeedGroupAct->addAction(m_pSpeedAct8);

	//connect(m_pSpeedAct1, SIGNAL(triggered(bool)), this, SLOT(SlotPlaySpeed(bool)));
	connect(m_pSpeedAct2, SIGNAL(triggered(bool)), this, SLOT(SlotPlaySpeed(bool)));
	connect(m_pSpeedAct4, SIGNAL(triggered(bool)), this, SLOT(SlotPlaySpeed(bool)));
	connect(m_pSpeedAct8, SIGNAL(triggered(bool)), this, SLOT(SlotPlaySpeed(bool)));
	//m_pPlaySpeedMenu->addAction(m_pSpeedAct1);
	m_pPlaySpeedMenu->addAction(m_pSpeedAct2);
	m_pPlaySpeedMenu->addAction(m_pSpeedAct4);
	m_pPlaySpeedMenu->addAction(m_pSpeedAct8);

	m_pOpenFileAct = new QAction("打开文件", this );
	connect(m_pOpenFileAct, SIGNAL(triggered(bool)), this, SLOT(SlotOpenFile()));
	m_pOpenHttpFileAct = new QAction("打开Http文件", this );
	connect(m_pOpenHttpFileAct, SIGNAL(triggered(bool)), this, SLOT(SlotHttpOpenFile()));

	m_pRebuildIndexAct = new QAction("重新加载", this );
	m_pRebuildIndexAct->setEnabled(false);
	connect(m_pRebuildIndexAct, SIGNAL(triggered(bool)), this, SLOT(SlotRebuildIndex()));
}




void CVxPlayWidget_W::AudioUpSlot()
{
	AudioVolumeChange();
}

void CVxPlayWidget_W::AudioDownSlot()
{
	AudioVolumeChange(false);
}


void CVxPlayWidget_W::AudioVolumeChange( bool b /*= true*/ )
{
	const float step = 0.1f;
	if (b)
	{
		m_audioVolume += step;
	}
	else
	{
		m_audioVolume -= step;
	}
	m_audioVolume = m_audioVolume < 0 ? 0:m_audioVolume;
	m_audioVolume = m_audioVolume > 1 ? 1:m_audioVolume;

	// 混音方案 默认1357声道到1,2468声道到2
	for(int i=0;i<AUDIO_INOUT_CHANEL;i++)
		g_mixer[i][i] = m_audioVolume;

	g_mixer[0][2] = m_audioVolume;
	g_mixer[0][4] = m_audioVolume;
	g_mixer[0][6] = m_audioVolume;
	g_mixer[1][3] = m_audioVolume;
	g_mixer[1][5] = m_audioVolume;
	g_mixer[1][7] = m_audioVolume;



	m_player->SetClipMixer(g_mixer);
	m_pAudio->SetValue(m_audioVolume);

}

void CVxPlayWidget_W::NeedFulls()
{
	// 播放容器是视频分辨率的1/4
	if (m_needFullRect.width()*m_needFullRect.height() > (m_audioRect.width()*m_audioRect.height()) /4)
	{
		m_bNeedFulls = true;
	}
	else
	{
		m_bNeedFulls = false;
	}
}


void CVxPlayWidget_W::SetNLEEngine( IVxNLEEngine *pNLEEngine )
{
	m_pNLEEngine = pNLEEngine;
}

void CVxPlayWidget_W::SlotCurLineEdit( const int &frame, bool b )
{
	if (m_bHasSource)
	{
		m_bSeekTo = true;
		m_pPlayViewController->SetClipNeedCallbackPlayFrame(false);
		m_pPlayViewController->SeekTo(frame, true);
		m_pPlayViewController->setPlayHeadIndicator(frame);
		m_pPlayRuler->SetCurrentFrame(frame);
	}
	else
	{
		m_pPlayRuler->SetCurrentFrame(frame);
	}
}

void CVxPlayWidget_W::wheelEvent( QWheelEvent *event )
{
	int numDegree = event->delta() / 8;
	int numStep = numDegree / 15;
	//qDebug("%i", numDegree);

	if (numDegree>0)
	{
		AudioVolumeChange(true);
	}
	else
	{
		AudioVolumeChange(false);
	}
}

void CVxPlayWidget_W::GetMixer( float mixer[8][8] )
{
	// 先固定 默认1357声道到1,2468声道到2
	//	1, 0, 1, 0, 1, 0, 1, 0,
	//	0, 1, 0, 1, 0, 1, 0, 1,
	//	0, 0, 0, 0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0, 0, 0, 0,
	mixer[0][0] = 1;
	mixer[0][2] = 1;
	mixer[0][4] = 1;
	mixer[0][6] = 1;
	mixer[1][1] = 1;
	mixer[1][3] = 1;
	mixer[1][5] = 1;
	mixer[1][7] = 1;

}



void CVxPlayWidget_W::SetIEHandle( SHANDLE_PTR iewnd )
{
	g_iehwnd = iewnd;
}

void CVxPlayWidget_W::SlotOpenFile()
{
	openFileDlgSlot();
}

void CVxPlayWidget_W::SlotRebuildIndex()
{
	emit SigRebulidIndex(m_preFilePath);		
}

QRect CVxPlayWidget_W::GetMonitorRect()
{
	return m_pPlayWidget->geometry(); 
}

void CVxPlayWidget_W::SlotDoubleClickPlay()
{
	qDebug() << "doubleClickPaly()";

	if (!m_bClickDbl)
	{
		m_pPlayViewController->setIsSpecialPlay(false);
		play();
	}
	else
	{
		m_bClickDbl = false;
	}
}

void CVxPlayWidget_W::SlotResetDoubleClickPlay()
{
	if (m_bClickDbl)
	{
		//killTimer(m_resetDoubleTimer->timerId());
		m_bClickDbl = false;
	}
}
void CVxPlayWidget_W::ResetFreq( int freq )
{
	freq = 25;
	m_pPlayRuler->SetFreq(freq);
	m_pCurLineEdit->setDFreg(freq);                            
	m_pAreaLineEdit->setDFreg(freq);                           
	m_pInPointEdit->setDFreg(freq);                            
	m_pOutPointEdit->setDFreg(freq);
	m_pTotalLineEdit->setDFreg(freq);
	m_pPlayViewController->setFrag(freq);
}

void CVxPlayWidget_W::EnableMenu( bool enable )
{
	if (enable)
	{
		// 有入出点才true
		int input = -1;
		int output = -1;
		m_pPlayRuler->GetInOutPoint(input, output);
		if (input != -1)
		{
			m_pClearInPointAct->setEnabled(enable);
		}
		if (output != -1)
		{
			m_pClearOutPointAct->setEnabled(enable);
		}
		m_pPlayRuler->SetDragInOut(true);
	}
	else
	{
		m_pClearInPointAct->setEnabled(enable);
		m_pClearOutPointAct->setEnabled(enable);
		m_pPlayRuler->SetDragInOut(false);
	}



}

void CVxPlayWidget_W::SlotInputOutTimer()
{
	if(HALSPLAY == m_player->GetState()) 
	{
		EnableMenu(false);
		m_pCurLineEdit->setEnabled(false);
	}
	else
	{
		m_inputOutputTimer->stop();
		EnableMenu(true);
		m_player->Redraw(0);
		m_pCurLineEdit->setEnabled(true);
	}
}



bool CVxPlayWidget_W::eventFilter( QObject *watcher, QEvent *event )
{
	int type = event->type();
	switch (type)
	{
	case QEvent::MouseMove:
		{
			if (watcher == m_pManager)
			{

				//处理 m_pManager 中的 MouseMoveEvent 
				m_timer->stop();
				return true;
				
			}
		}
	case QEvent::ToolTip:
		{
			// 因为CVxTimeCodeEdit使用了stylesheet所以只有通过此方法
			CVxTimeCodeEdit *codeEdit = static_cast<CVxTimeCodeEdit*>(watcher);
			QHelpEvent *helpEvent = dynamic_cast<QHelpEvent*>(event);
			if (!helpEvent) break;
			QString toolTipStr;
			//if (m_pCurLineEdit == codeEdit)
			//{
			//	toolTipStr = "当前帧位置";
			//}
			if (m_pCurLineEdit == static_cast<CTimeCodeEdit*>(watcher))
			{
				toolTipStr = "当前帧位置";
			}
			else if (m_pAreaLineEdit == codeEdit)
			{
				toolTipStr = "入出点长度";
			}
			else if (m_pInPointEdit == codeEdit)
			{
				toolTipStr = "入点位置";
			}
			else if (m_pOutPointEdit == codeEdit)
			{
				toolTipStr = "出点位置";
			}
			else if (m_pTotalLineEdit == codeEdit)
			{
				toolTipStr = "总时长度";
			}
			else
			{
				break;
			}
			QToolTip::showText(helpEvent->globalPos(), toolTipStr);
			return true;
		}
	default:
		;
	}

	return QWidget::eventFilter(watcher, event);
}

void CVxPlayWidget_W::SlotContinuePlay()
{
	switch (m_playMode)
	{
	case PLAY:
		{
			m_pPlayViewController->setIsSpecialPlay(false);
			play();
		}
		break;
	case BACKWARDPALY:
		{
			backward();
		}
		break;
	case FORWARDPLAY:
		{
			forward();
		}
		break;
	}

}

int CVxPlayWidget_W::GrabMovieFirstFrame( const QString &filePath, const QString &iconPath )
{
	return 0;

}

void CVxPlayWidget_W::SlotFullScreen()
{
	QAction *fullScreenAct = qobject_cast<QAction*>(sender());
	fullScreenAct->setText(m_bFullScreen ? "全屏显示" : "窗口显示");
	m_bFullScreen = !m_bFullScreen;

	SetFullScreen(m_bFullScreen);
}

void CVxPlayWidget_W::SlotResetHwnd()
{
	// 移动一人像素再移回来, 解决安全框缺失
	QRect rect = m_pPlayWidget->geometry();
	QRect rctemp = rect.adjusted(0, 0, 0, 1);
	m_pPlayWidget->setGeometry(rctemp);
	m_pPlayWidget->setGeometry(rect);
	m_pPlayWidget->ResetHwnd();
}

void CVxPlayWidget_W::SpeedPlaySeekToEnd( int headerIndex )
{
	if (m_pPlayViewController->getIsSpecialPlay())
	{
		switch (m_playMode)
		{
		case FORWARDPLAY:
			{
				qDebug() << headerIndex << " " << m_duration - m_pPlayViewController->getSpeed();
				if (headerIndex >= m_duration - m_pPlayViewController->getSpeed() - 3)
				{
					static int curHeader = m_duration - 1;
					m_curFrame = headerIndex;
					m_player->SeekTo(curHeader, false);
					m_pPlayRuler->SetCurrentFrame(curHeader);
					m_pPlayViewController->setPlayHeadIndicator(curHeader, false);
					m_pCurLineEdit->setCurFrame(curHeader);
				}
			}
			break;
		case BACKWARDPALY:
			{
				qDebug() << headerIndex << " " << m_pPlayViewController->getSpeed() + 3;
				if (headerIndex <= -m_pPlayViewController->getSpeed() + 3)
				{
					m_curFrame = headerIndex;
					m_player->SeekTo(0, false);
					m_pPlayRuler->SetCurrentFrame(0);
					m_pPlayViewController->setPlayHeadIndicator(0, false);
					m_pCurLineEdit->setCurFrame(0);
				}
			}
			break;
		}
	}
}

bool CVxPlayWidget_W::PlayInCtrlRange()
{
	int input = -1;
	int output = -1;
	m_pPlayRuler->GetInOutPoint(input, output);
	if (m_pPlayViewController->isPlay() && 
		!m_pPlayViewController->getIsSpecialPlay() &&
		input != -1 && output != -1)
	{
		return true;
	}
	return false;
}



void CVxPlayWidget_W::showEvent( QShowEvent *event )
{
	CBasePlayWidget::showEvent(event);
}

void CVxPlayWidget_W::SaveSettings()
{
	QSettings settings(settingPath, QSettings::IniFormat);
	settings.setValue("prefilepath", QFileInfo(m_preFilePath).absolutePath());
}

void CVxPlayWidget_W::LoadSettings()
{
	QSettings settings(settingPath, QSettings::IniFormat);
	m_preFilePath = settings.value("prefilepath", QApplication::applicationDirPath()).toString();
}

QString CVxPlayWidget_W::FindPathExt( const QString &filePath )
{
	QString ext;
	int pos = filePath.lastIndexOf(".");
	if (pos != -1)
	{
		ext = filePath.mid(pos + 1);
	}

	return ext;
}

void CVxPlayWidget_W::SlotHttpOpenFile()
{
	emit OpenHttpFileSig(m_preFilePath);
}



CAudio::CAudio(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	m_value = 0.0;
	m_direction = HOR;
	//m_color = QColor(Qt::green);
	m_color = QColor(80,250,80);
	m_bg = parent->palette().color(QPalette::Background);
	m_timer = new QTimer(this);
	m_timer->setSingleShot(true);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(hide()));
}

CAudio::~CAudio()
{

}

void CAudio::SetValue( float v , bool bshow /*= true*/)
{

	m_value = v;
	m_value = qBound<float>(0.0, m_value, 1.0);
	m_value = 1 - m_value;
	update();
	if (bshow)
	{
		show();
		m_timer->start(1000);
	}

}

void CAudio::paintEvent( QPaintEvent *event )
{
	QWidget::paintEvent(event);
	QRect rect = this->rect();
	int w = rect.width();
	int h = rect.height();
	QPainter painter(this);
	painter.fillRect(rect, QBrush(m_color));

	int rl = m_value * rect.height();
	if (rl <= 0) return;
	QRect temp(rect);
	temp.setBottomLeft(QPoint(0, rl));
	temp.setBottomRight(QPoint(w, rl));
	painter.fillRect(temp, QBrush(m_bg));
}

QPolygon CAudio::GetFillRect( /*int rl */)
{
	QPolygon polygon;
	QPoint p[3];
	QRect rect = this->rect();

	polygon << QPoint(rect.topLeft());
	polygon << QPoint(rect.topRight()/*+QPoint(1,0)*/);
	polygon << QPoint(rect.bottomRight()/*+QPoint(1,0)*/);

	return polygon;
}

QPolygon CAudio::GetFillRect2(int rl)
{
	QPolygon polygon;
	QPoint p[3];
	QRect rect = this->rect();
	int w = rect.width();
	int h = rect.height();

	polygon << QPoint((w-(w*rl/h)), rl);
	polygon << QPoint(rect.width(), rl);
	polygon << QPoint(rect.bottomRight()+QPoint(1,0));

	return polygon;
}

void CAudio::resizeEvent( QResizeEvent *event )
{
	QRect rect = this->rect();
	if (m_direction == HOR)
	{
		m_endP = rect.topRight();
	}
	else
	{
		m_endP = rect.topLeft();
	}

}

void CAudio::ContinueShow()
{

}

#ifdef Q_OS_WIN32
#define ENABLEWINDOW(h,b) EnableWindow((HWND)h, b)
#else
#define ENABLEWINDOW(h,b)
#endif

CWinFileDialog::CWinFileDialog( QWidget *parent, long hwnd /*= 0*/ )
	: m_parent(parent)
	, m_hwnd(hwnd)
{

}

void CWinFileDialog::DoModal()
{
#ifdef Q_OS_WIN32
	startTimer(50);
#endif

	ENABLEWINDOW(m_hwnd, false);
	m_filePath = QFileDialog::getOpenFileName(m_parent, "选择文件", QFileInfo(m_preFilePath).path());
	ENABLEWINDOW(m_hwnd, true);
} 

void CWinFileDialog::timerEvent( QTimerEvent *event )
{
	// 刷新IE
#ifdef Q_OS_WIN32
	if (m_hwnd)
	{
		UpdateWindow((HWND)m_hwnd);
	}
	else
	{
		killTimer(event->timerId());
	}
#endif
}


//
//CIEMsgBox::CIEMsgBox( QObject *parent /*= NULL*/ )
//{
//
//}
//
//CIEMsgBox::~CIEMsgBox()
//{
//
//}
//
//void CIEMsgBox::Show( QWidget *parent, const QString title, const QString context )
//{
//	QMessageBox msgbox(parent);
//	//msgbox.setParent(parent); // 为什么用这个不行
//	msgbox.setStandardButtons(QMessageBox::Ok);
//	msgbox.setWindowTitle(title);
//	msgbox.setText(context);
//
//#ifdef Q_OS_WIN32
//	startTimer(50);
//#endif
//
//	msgbox.exec();
//}
//
//void CIEMsgBox::timerEvent( QTimerEvent *event )
//{
//	// 刷新IE
//#ifdef Q_OS_WIN32
//	if (g_iehwnd)
//	{
//		UpdateWindow((HWND)g_iehwnd);
//	}
//	else
//	{
//		killTimer(event->timerId());
//	}
//#endif
//
//}


//////////////////////////////////////////////////////////////////////////



CTimeCodeEditSub::CTimeCodeEditSub( QWidget *parent ,int TimeType, int Codetype /*= 0*/ )
	: QLineEdit(parent)
{
	setFrame(false);
	setAlignment(Qt::AlignCenter);
	setAcceptDrops(false);
	setFocusPolicy(Qt::StrongFocus);
	setContextMenuPolicy(Qt::NoContextMenu);
	setValidator( new QIntValidator(0, 900, this));

	switch (Codetype)
	{
	case CTimeCodeEditSub_Sixty:
		setValidator( new QIntValidator(0, 59, this));
		break;
	case CTimeCodeEditSub_TwentyFive:
		setValidator( new QIntValidator(0, 24, this));
		break;
	default:
		;
	}
	
	setStyleSheet("background-color:rgb(50,50,50);color:white"); 
	setFont( QFont( tr("JsTimeCode"), 10 ,QFont::Light ) );
	m_timeType = TimeType;
	m_bMouseMove = false;

	
	connect(this, SIGNAL(editingFinished()), this, SLOT(SlotEditFinished()));
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(SlotTextChanged(const QString&)));

	setText("00");

}

CTimeCodeEditSub::~CTimeCodeEditSub()
{

}

void CTimeCodeEditSub::mousePressEvent( QMouseEvent *event )
{
	setFocus();
	m_bMouseMove = true;
	m_oldMouseMovePoint = event->globalPos();
	//QLineEdit::mousePressEvent(event);
	selectAll();
	
}

void CTimeCodeEditSub::focusOutEvent(QFocusEvent *event)
{
	QString curTime = text();

	if (curTime.isEmpty())
	{
		curTime = "00";
	}
	else if (curTime.size() == 1)
	{
		curTime = "0" + curTime;
	}
	blockSignals(true);
	setText(curTime);
	blockSignals(false);
	ValueChange(curTime.toInt());
	QLineEdit::focusOutEvent(event);
}

void CTimeCodeEditSub::mouseReleaseEvent( QMouseEvent *event )
{
	m_bMouseMove = false;
	this->setCursor(QCursor(Qt::ArrowCursor));
	QLineEdit::mouseReleaseEvent(event);
	setFocus();
}

void CTimeCodeEditSub::mouseMoveEvent( QMouseEvent *event )
{
	if (m_bMouseMove)
	{
		if (event->globalPos().y() - m_oldMouseMovePoint.y() > 0)
		{
			DragValueChanged(m_timeType, false);
		}
		else
		{
			DragValueChanged(m_timeType, true);
		}
		m_oldMouseMovePoint = event->globalPos();
		this->setCursor(QCursor(Qt::SizeVerCursor));
	}
	QLineEdit::mouseMoveEvent(event);
}

void CTimeCodeEditSub::mouseDoubleClickEvent( QMouseEvent *event )
{
	selectAll();
	//QLineEdit::mouseDoubleClickEvent(event);
}

void CTimeCodeEditSub::SlotEditFinished()
{
	//QString curTime = text();
	//
	//curTime = curTime.size() == 1 ? "0"+curTime : curTime;
	//blockSignals(true);
	//setText(curTime);
	//emit ValueChange(text().toInt());
	//blockSignals(false);
	//((QWidget*)parentWidget())->setFocus();
}

void CTimeCodeEditSub::SlotTextChanged( const QString &text )
{
	if (text == QLatin1String("+"))
	{
		setText("");
		return;
	}
	//if (text.toInt() > m_maxValue)
	//{
	//	if (text.size() == 2)
	//	{
	//		blockSignals(true);
	//		setText(text.left(1));
	//		blockSignals(false);
	//	}
	//	else if (text.size() == 3)
	//	{
	//		if (text.left(2).toInt() <= m_maxValue)
	//		{
	//			blockSignals(true);
	//			setText(text.left(2));
	//			blockSignals(false);
	//			emit NextFocus();
	//		}						
	//	}
	//}
	//else
	{
		if (text.size() == 2)
		{
			//if (text.left(1) != QLatin1String("0") ||
			//	(text == QLatin1String("00")))
			//{
				emit NextFocus();
			//}

		}
		//else if (text.size() == 3)
		//{
		//	blockSignals(true);
		//	setText(text.right(2));
		//	blockSignals(false);
		//	emit NextFocus();

		//}
	}
}

void CTimeCodeEditSub::keyPressEvent( QKeyEvent *event )
{
	QLineEdit::keyPressEvent(event);
	if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
	{
		((QWidget*)(parent()->parent()))->setFocus();
	}

}

void CTimeCodeEditSub::SetText( const QString &text )
{
	blockSignals(true);
	setText(text);
	blockSignals(false);
}



//////////////////////////////////////////////////////////////////////////

CTimeCodeEdit::CTimeCodeEdit( QWidget *parent )
	: QWidget(parent)
{
	m_hasFocus = false;
	m_freq = 25;
	m_curFrame = 0;
	m_totalFrame = 0;

	m_hour = new CTimeCodeEditSub(this, CTimeCodeEditSub_Hour,CTimeCodeEditSub_TwentyFive);\
	m_minute = new CTimeCodeEditSub(this, CTimeCodeEditSub_Minute);
	m_second = new CTimeCodeEditSub(this, CTimeCodeEditSub_Second);
	m_frame = new CTimeCodeEditSub(this, CTimeCodeEditSub_Frame, CTimeCodeEditSub_TwentyFive);
	m_hour->installEventFilter(this);
	m_minute->installEventFilter(this);
	m_second->installEventFilter(this);
	m_frame->installEventFilter(this);
	m_hour->setFont(QApplication::font());
	m_minute->setFont(QApplication::font());
	m_second->setFont(QApplication::font());
	m_frame->setFont(QApplication::font());
	setTabOrder(m_hour, m_minute);
	setTabOrder(m_minute, m_second);
	setTabOrder(m_second, m_frame);
	//setTabOrder(m_frame, m_hour);

	connect(m_hour, SIGNAL(ValueChange(int)), this, SLOT(SlotValueChanged(int)));
	connect(m_minute, SIGNAL(ValueChange(int)), this, SLOT(SlotValueChanged(int)));
	connect(m_second, SIGNAL(ValueChange(int)), this, SLOT(SlotValueChanged(int)));
	connect(m_frame, SIGNAL(ValueChange(int)), this, SLOT(SlotValueChanged(int)));

	connect(m_frame, SIGNAL(DragValueChanged(int,bool)), this, SLOT(SlotDragValueChanged(int,bool)));
	connect(m_minute, SIGNAL(DragValueChanged(int,bool)), this, SLOT(SlotDragValueChanged(int,bool)));
	connect(m_second, SIGNAL(DragValueChanged(int,bool)), this, SLOT(SlotDragValueChanged(int,bool)));
	connect(m_frame, SIGNAL(DragValueChanged(int,bool)), this, SLOT(SlotDragValueChanged(int,bool)));

	connect(m_hour, SIGNAL(NextFocus()), this, SLOT(SlotNextFocus()));
	connect(m_minute, SIGNAL(NextFocus()), this, SLOT(SlotNextFocus()));
	connect(m_second, SIGNAL(NextFocus()), this, SLOT(SlotNextFocus()));
	connect(m_frame, SIGNAL(NextFocus()), this, SLOT(SlotNextFocus()));


	m_colon = new CTimeCodeEditColon(this);
	m_colon2 = new CTimeCodeEditColon(this);
	m_colon3 = new CTimeCodeEditColon(this);

	
}

void CTimeCodeEdit::setCurFrame( int frame )
{
	if (frame >= m_totalFrame)
	{
		frame = m_totalFrame - 1;
	}

	m_curFrame = frame;
	QString timeCodeStr = VxFrameToString(m_curFrame, m_freq).c_str();
	QStringList timeCodeList = timeCodeStr.split(":", QString::SkipEmptyParts);
	if (timeCodeList.size() == 4)
	{
		m_hour->SetText(timeCodeList[0]);
		m_minute->SetText(timeCodeList[1]);
		m_second->SetText(timeCodeList[2]);
		m_frame->SetText(timeCodeList[3]);
	}
}


void CTimeCodeEdit::setTotalFrames( int frame )
{
	m_totalFrame = frame;
}

string CTimeCodeEdit::frameToTimeCode( const int frame )
{
	return "";
}

void CTimeCodeEdit::SlotValueChanged( int frame )
{
	int curFrame = m_hour->text().toInt()*60*60*m_freq + m_minute->text().toInt()*60*m_freq + 
		m_second->text().toInt()*m_freq + m_frame->text().toInt();

	if (curFrame >=0 && curFrame < m_totalFrame && curFrame != m_curFrame)
	{

		setCurFrame(curFrame);
		valueChangedSignal(m_curFrame, true);
	}
	else
	{
		setCurFrame(m_curFrame);
	}
}

void CTimeCodeEdit::SlotDragValueChanged( int type, bool inscrea )
{
	int step = inscrea ? 1 : -1;
	int hourStep = 0;
	int minuteStep = 0;
	int secondStep = 0;
	int frameStep = 0;
	switch (type)
	{
	case CTimeCodeEditSub_Hour:
		hourStep = step;
		break;
	case CTimeCodeEditSub_Minute:
		minuteStep = step;
		break;
	case CTimeCodeEditSub_Second:
		secondStep = step;
		break;
	case CTimeCodeEditSub_Frame:
		frameStep = step;
		break;
	}

	int curFrame = (m_hour->text().toInt()+hourStep)*60*60*m_freq + (m_minute->text().toInt()+minuteStep)*60*m_freq + 
		(m_second->text().toInt()+secondStep)*m_freq + (m_frame->text().toInt()+frameStep);

	if (curFrame >=0 && curFrame < m_totalFrame)
	{
		setCurFrame(curFrame);
		valueChangedSignal(m_curFrame, true);
	}
	else
	{
		setCurFrame(m_curFrame);
	}
	
}

void CTimeCodeEdit::resizeEvent(QResizeEvent *event)
{
	const int margin = 1;
	const int wcolon = 4;
	int w = this->rect().width();
	int h = this->rect().height();
	int wsub = (w -(wcolon*3))/ 4;
	int left = margin;
	m_hour->setGeometry(left, margin, wsub, h);
	left = m_hour->geometry().right();
	m_colon->setGeometry(left, margin, wcolon, h);
	left = m_colon->geometry().right();
	m_minute->setGeometry(left, margin, wsub, h);
	left = m_minute->geometry().right();
	m_colon2->setGeometry(left, margin, wcolon, h);
	left = m_colon2->geometry().right();
	m_second->setGeometry(left, margin, wsub, h);
	left = m_second->geometry().right();
	m_colon3->setGeometry(left, margin, wcolon, h);
	left = m_colon3->geometry().right();
	m_frame->setGeometry(left, margin, wsub, h);
}

bool CTimeCodeEdit::eventFilter ( QObject * watched, QEvent * event )
{
	int type = event->type();
	if (type == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = (QKeyEvent*)event;
		if (keyEvent->key() == Qt::Key_Tab)
		{
			if (watched == m_frame && m_frame->hasFocus())
			{
				qApp->postEvent(m_hour, new QMouseEvent(QEvent::MouseButtonPress, QPoint(0,0), Qt::LeftButton,Qt::LeftButton, Qt::NoModifier));
				return true;
			}
		}

	}
	return false;
}
//
//bool CTimeCodeEdit::event( QEvent *event )
//{
//	if (event->type() == Qt::Key_Tab)
//	{
//		int i = 0;
//	}
//
//	return QWidget::event(event);
//}

void CTimeCodeEdit::SlotNextFocus()
{
	if (HasFocus())
	{
		QKeyEvent e(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
		event(&e);
	}
	else
	{
		((QWidget*)parent())->setFocus();
	}
}

bool CTimeCodeEdit::HasFocus()
{
	return m_hour->hasFocus() || m_minute->hasFocus() || m_second->hasFocus() /*|| m_frame->hasFocus()*/;
}

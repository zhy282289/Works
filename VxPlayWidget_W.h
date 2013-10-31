#ifndef VX_PLAY_VIEW_WINDOW_X_DANIEL_
#define VX_PLAY_VIEW_WINDOW_X_DANIEL_

#include "inc_PlayViewX/dllInterface.h"
#include "VxCompoundIF.h"
#include "vxasyncfiletasksif.h"
#include "VxAVImport.h"

#include "inc_CommonLib/IMsg.h"
#include "inc_CommonLib/IMsgObserver.h"



class CBasePlayWidget;
class CMonitorWidget;
class CMonitorWidgetContainet;
class CVxPlayViewController_X;
class CVxPlayButtonController_W;

class CVxPlayRuler;
class CVxTimeCodeEdit;
class CAudioMeterCtrl;
class CVxButtonItemX;
class CAudio;
class CVxMatterWaitDialog;
class CTsImportDlg;
struct IMPORTDLLINFO;
class CTimeCodeEdit;



/*
	切换到安全框模式下需要    add 2012/4/9 CB
*/
class FCClipPlayCallback
{
public:
	static void CurrentFrame(STATEPARAM * theParam,void * theObject);
};

class FCClipPlayAudioCallback
{
public:
	static void __audiovufunc(AUDIOUVMETER * theParam,void * theObject);
};



class  CVxPlayWidget_W : public CBasePlayWidget
{
	typedef enum PlayMode {NONE, PLAY, BACKWARDPALY, FORWARDPLAY};

	Q_OBJECT
public:
	CVxPlayWidget_W(QWidget *parent = NULL);
	virtual ~CVxPlayWidget_W();
public:
	virtual void SetSetup(IVxSystemSetup *aSetup);
	virtual void SetPlayer(void *pplayer);
	virtual void SetInfoTool(IVxClipInfoTools *aInfoTools);
	virtual bool SetSource(VX_AVPATH avPath);
	virtual void SetSource(IVxSystemSetup *setup, IVxSource *source);
	virtual void SetSource(IVxSystemSetup *setup,IVxCompoundFile* compoundFie,float aspect);
	virtual bool SetSource(IVxDataCore *pDataCore, bool bSD);
	virtual void SeekTo(int index, bool bFlag = false);
	virtual void ResetPlayWidget();
	virtual void SetInOutPoint(long nInPoint, long nOutPoint);
	virtual void SetCtrlRange(long nFrom, long nTo);
	virtual void GetInOutPoint(int &inpoint,int &outpoint);
	virtual void GetInPoint(int &inpoint);
	virtual void GetOutPoint(int &outpoint);
	virtual void GetPosition(int &nPosition);
	virtual void GetFrameRate(double &dFrameRate);
	virtual void GetPlayFile(QString &fileName);
	virtual void ResetPlay(bool b = true);
	void GrabFrame(long pos, const QString &filepath, long format, long width, long height);
	void GetDiskFreeSpace(const QString &path, long &nFreeSpace);
	void GetPlayFileInfo(const QString &strFilePath, QString &strXml);
	void GetMediaInfo(const QString &filePath, QString &strXml);
	bool MoveMediaFile(const QString &srcFilePath,const QString &dstFilePath);
	bool SendData(char* Ip, int Port, char* sendData, int sendLength, int recvLength, char* recvData);
	void pause();
	void stop();
	virtual void CloseFile();
	void OpenSingleFileDialog(QString &strOpenFileXML);
	void OpenFileDialog(QString &strOpenFileXML, long iehwnd);
	bool OpenFile(const QString &fileName);
	bool IsClipNeedCallbackPlayFrame();								//添加安全框需要 add 2012/4/9 CB 
	void SetPlayerSD(void *pplayer) {m_playerSD = (IVxAVPlayer*)pplayer;}
	void SetPlayerHD(void *pplayer) {m_playerHD = (IVxAVPlayer*)pplayer;}
	void SetNLEEngine(IVxNLEEngine *pNLEEngine);
	void SetAudioEnabled(bool bHasAudio){m_bHasAudio = bHasAudio;}
	void SetPreFilePath(const QString &filePath) { m_preFilePath = filePath; }
	QString GetPreFilePath(){ return m_preFilePath; }
	void SetIEHandle(SHANDLE_PTR iewnd);
	void SetAxFactoryServer(bool enable) { m_bAxFactory = enable; }
	QRect GetMonitorRect(); 
	void EnableCreateIndex(bool enable) { m_pRebuildIndexAct->setEnabled(enable); }

	int	GrabMovieFirstFrame(const QString &filePath, const QString &iconPath);

public slots:
	void onBtnClicked(int iOptType);
	void monitorSizeChangeSlot(QRect rc);
	void appWillQuitSlot();
	void playRulerFrameChangeSlot(int ,bool, int);
	void durationChangedSlot(long);
	void openFileDlgSlot();
	void openFileSlot(QString &filePath);
	void play();
	

signals:
	void ShowFileInfoSig(QString, MFILEINFO&);
	void OpenFileSig(QString filePath);
	void SigRebulidIndex(QString);
	void OpenHttpFileSig(QString filePath);

private slots:
	void nextFrame();
	void prevFrame();
	void home();
	void end();
	void nextSecond();
	void prevSecond();
	void goToInPoint();
	void goToOutPoint();
	void forward();
	void backward();
	void setCurFrame(int frame);
	void setInPosSlot();
	void setOutPosSlot();
	void playRulerInPointChanging(int iFrame);
	void playRulerOutPointChanging(int iFrame);
	void playRulerInOutPointChanging();
	void ShowBar();
	void ShowFileInfo();
	void ShowSafeFrame();
	void PlayShow_SF(int status, bool isShow);
	void clearInPoint();
	void clearOutPoint();
	void DrawExt(bool bDraw = false);
	void AudioUpSlot();
	void AudioDownSlot();
	void SlotCurLineEdit(const int &frame, bool b);
	void SlotPlaySpeed(bool b = true);
	void SlotFastForward();
	void SlotFastBackward();
	void SlotOpenFile();
	void SlotRebuildIndex();
	void SlotDoubleClickPlay();
	void SlotResetDoubleClickPlay();
	void SlotInputOutTimer();
	void SlotContinuePlay();
	void SlotFullScreen();
	void SlotResetHwnd();
	void SlotHttpOpenFile();
protected:
	void showEvent(QShowEvent *event);
	void resizeEvent(QResizeEvent * event);
	void paintEvent(QPaintEvent *e);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent * event);
	void keyPressEvent(QKeyEvent * event); 
	//bool eventFilter(QObject *obj, QEvent *ev);
	bool event(QEvent *event);
	void mouseReleaseEvent( QMouseEvent* event);              //添加右键弹出菜单（安全框） 2012/4/6 CB
	void wheelEvent(QWheelEvent *event);
	//void closeEvent(QCloseEvent *event);
	void AudioVolumeChange(bool b = true);
	void GetFileMatteInfo(QString filePath, MFILEINFO &materInfo);
	bool eventFilter(QObject *watched, QEvent *event);
private:
	void init();
	void initPlayData();                                     //切换到安全框模式下的初始化  2012/4/6 CB
	void registerObserver();
	void clipSeek(int frame);
	bool GetFileAvInfo(QString strXML, MATTERINFO2 &info, QString &filePath);
	QString CreateVideoXMLString(const MATTERINFO2 &info);
	QString CreateAudioXMLString(const MATTERINFO2 &info);
	QString GetFileSize(const QString& fileName);
	void SetFullScreen(bool);
	//bool	IsSdVideo(const char *videoPath);
	void	InitAct();
	void	NeedFulls();
	void	GetMixer(float mixer[8][8]);
	void    InitAllShortcut();
	void	ResetFreq(int freq);
	void	EnableMenu(bool enable);
	void	SpeedPlaySeekToEnd(int headerIndex);
	bool	PlayInCtrlRange();
	void    SaveSettings();
	void    LoadSettings();
	QString FindPathExt(const QString &filePath);
private:
	IVxSystemSetup			*m_Setup;
	IVxAVPlayer				*m_player;
	IVxAVPlayer				*m_playerSD;
	IVxAVPlayer				*m_playerHD;
	IVxClipInfoTools2		*m_InfoTools;
	IVxNLEEngine				*m_pNLEEngine;
	

	bool                        m_bHasSource;                                //是否加载了文件
	bool                        m_bFullScreen;                               //是否全屏
	bool						m_bMouseDown;
	bool						m_bSd;
	QRect                       m_normalRect;

	double                      m_frameRate;
	QString						m_fileName;
	QString						m_fileNameAttribute;
	DWORD                       m_aspect;                                    //控制抓帧时图片的尺寸

	QTimer                      *m_timer;                                    //用于控制拦的显示定时 
	CMonitorWidget				 *m_pPlayWidget;                              //显示器
	//CMonitorWidgetContainet		*m_pPlayWidget; 
	CVxPlayViewController_X     *m_pPlayViewController;
	CVxPlayButtonController_W   *m_pManager;                                 //工具栏按钮
	int                         m_playRectHeight;
	CVxPlayRuler                *m_pPlayRuler;                               //时间尺
	//CVxTimeCodeEdit             *m_pCurLineEdit;                             //当前位置时码
	CTimeCodeEdit             *m_pCurLineEdit;                             //当前位置时码
	CVxTimeCodeEdit             *m_pAreaLineEdit;                            //入出点间长度时码
	CVxTimeCodeEdit             *m_pInPointEdit;                            
	CVxTimeCodeEdit             *m_pOutPointEdit;
	CVxTimeCodeEdit             *m_pTotalLineEdit;                          //片长时码

	CVxButtonItemX              *m_pSetInBtn;
	CVxButtonItemX              *m_pSetOutBtn;
	QImage                      *m_pInOverImage;
	QImage                      *m_pInNorImage;
	QImage                      *m_pInDisImage;
	QImage                      *m_pOutOverImage;
	QImage                      *m_pOutNorImage;
	QImage                      *m_pOutDisImage;

	QShortcut                   *m_pTranscoder;                              // 打包快捷键
	QShortcut                   *m_pTranscoderProfile;                       // 打包配置快捷键

	QShortcut                   *m_pGoHeadShortcut;                          // 到头部
	QShortcut                   *m_pGoInPointShortcut;                       // 到入点
    QShortcut                   *m_pPreFrameShortcut;                        // 上一帧
	QShortcut                   *m_pFastBackwardShortcut;                    // 快退
	QShortcut                   *m_pPlayShortcut;                            // 播放
	QShortcut                   *m_pFastForwardShortcut;                     // 快进
	QShortcut                   *m_pNextFrameShortcut;                       // 下一帧
	QShortcut                   *m_pGoOutPointShortcut;                      // 到出点
	QShortcut                   *m_pGoEndShortcut;                           // 到尾部
	QShortcut                   *m_pSetInPointShortcut;                      // 设置入点
	QShortcut                   *m_pSetOutPointShortcut;                     // 设置出点
	QShortcut                   *m_pAudioUpShortcut;                         // 加大音量
	QShortcut                   *m_pAudioDownShortcut;                       // 调小音量
	QShortcut                   *m_pOpenFileShortcut;                        // 打开文件

	QShortcut                   *m_pPreSecondShortcut;                       // 上一秒
	QShortcut                   *m_pNextSecondShortcut;                      // 下一秒
	
	QAction                     *m_pShowFileInfoAct;
	QAction                     *m_pSafeFrameAct;                  
	QAction						*m_pClearInPointAct;
	QAction						*m_pClearOutPointAct;
	QAction						*m_pFullScreenAct;


	float						m_audioVolume;
	bool						m_bHasAudio;
	bool						m_bNeedFulls;
	QRect						m_audioRect;
	
	QMenu						*m_pDrawExtMenu;
	QAction						*m_pDrawExtAct0;
	QAction						*m_pDrawExtAct1;
	QAction						*m_pDrawExtAct2;
	QAction						*m_pDrawExtAct3;
	QAction						*m_pDrawExtAct4;
	QAction						*m_pDrawExtAct5;
	QAction						*m_pDrawExtAct6;
	QAction						*m_pDrawExtAct7;
	QActionGroup				*m_pDrawExtGroupAct;
	
	QMenu						*m_pPlaySpeedMenu;
	QAction						*m_pSpeedAct2;
	QAction						*m_pSpeedAct4;
	QAction						*m_pSpeedAct8;
	QActionGroup				*m_pSpeedGroupAct;

	CAudioMeterCtrl		        *m_meter1;
	CAudioMeterCtrl		        *m_meter2;

	int						m_status;
	CAudio				*m_pAudio;
	QString				m_preFilePath;

	MFILEINFO			m_fileInfo;
	MFILEINFO			m_fileInfoAttribute;
	CVxComPtr<IVxCompoundFile> m_compoundFile;
	CVxComPtr<IVxDemultiplexer> m_demultiplexer;

	QAction				*m_pOpenFileAct;
	QAction				*m_pOpenHttpFileAct;
	bool				m_bAxFactory;
	QAction				*m_pRebuildIndexAct;
	bool				m_bClickDbl;
	int					m_curFrame;
	QTimer				*m_doubleTimer;
	QTimer				*m_resetDoubleTimer;
	QTimer				*m_inputOutputTimer;
	long				m_duration;
	bool				m_bSeekTo;
	PlayMode			m_playMode;
	int					m_speedPlayEnd;
	QRect				m_needFullRect;

	int					m_playRulerDragType;

	int					nFileType;
};


enum DIRECTION{HOR, VET};
class CAudio : public QWidget
{
	Q_OBJECT

public:
	CAudio(QWidget *parent);
	~CAudio();

public slots:
	void SetValue(float v, bool bshow = true);
	void SetBg(QColor bg) {m_bg = bg;};
	void SetColor(QColor color){m_color = color;}
	void SetDirection(DIRECTION d){m_direction = d;}

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	QPolygon GetFillRect();
	QPolygon GetFillRect2(int rl);
	void ContinueShow();
private:
	float		m_value;
	QPoint		m_endP;
	QColor		m_bg;
	QColor		m_color;
	DIRECTION	m_direction;
	QTimer		*m_timer;
};


class CWinFileDialog : public QObject
{
	Q_OBJECT
public:
	explicit	CWinFileDialog(QWidget *parent, long hwnd = 0);

	void		SetHwnd(long hwnd) { m_hwnd = hwnd; }
	void		SetPreFilePath(const QString &filePath) { m_preFilePath = filePath; }
	QString     SelectedFile() { return m_filePath; }
	void		DoModal();
protected:
	void		resizeEvent(QResizeEvent *event);
	void		timerEvent(QTimerEvent *event);
private:
	long		m_hwnd;
	QFileDialog	*m_fileDlg;
	QWidget		*m_parent;
	QString     m_preFilePath;
	QString     m_filePath;
};


//class CIEMsgBox : public QObject
//{
//	Q_OBJECT
//public:
//	CIEMsgBox(QObject *parent , long hwnd);
//	~CIEMsgBox();
//
//	void	Show(QWidget *parent, const QString title, const QString context);
//
//protected:
//	void	timerEvent(QTimerEvent *event);
//
//private:
//	long		m_hwnd;
//};

struct IMPORTDLLINFO
{
	QString         dllName;
	HMODULE         dllHandle;
	PIAVIMINFOFUNCS dllInfo;
	AVIMPORTPROC    ImprotInfo;
	QAction         *ImportAction;
	bool            isFiltered;
	QStringList     openFilters;
	QStringList     dllFileNames;
	IMPORTDLLINFO   *next;
	IMPORTDLLINFO()
	{
		memset(&ImprotInfo, 0, sizeof(AVIMPORTPROC));
	}
};


//////////////////////////////////////////////////////////////////////////
enum {CTimeCodeEditSub_Sixty, CTimeCodeEditSub_TwentyFive};
enum {CTimeCodeEditSub_Hour, CTimeCodeEditSub_Minute, CTimeCodeEditSub_Second, CTimeCodeEditSub_Frame};
class CTimeCodeEditSub : public QLineEdit
{
	Q_OBJECT
public:
	CTimeCodeEditSub(QWidget *parent ,int TimeType, int Codetype = CTimeCodeEditSub_Sixty);
	~CTimeCodeEditSub();

	void	Clear() { setText("00"); }
	void    SetText(const QString &text);
signals:
	void	ValueChange(int frame);
	void	DragValueChanged(int type, bool increase);
	void    NextFocus();
protected:
	//void event(QEvent *event);
	void focusOutEvent(QFocusEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void mouseMoveEvent( QMouseEvent *event );
	void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
	void	SlotEditFinished();
	void    SlotTextChanged(const QString &text);
private:
	QPoint              m_oldMouseMovePoint;
	int					m_timeType;
	bool				m_bMouseMove;
	int					m_maxValue;
	bool				m_textChaneged;
};

class CTimeCodeEditColon : public QLabel
{
	Q_OBJECT
public:
	CTimeCodeEditColon(QWidget *parent)
		: QLabel(parent)
	{		
		setFrameShape(QFrame::NoFrame);
		setAlignment(Qt::AlignCenter);
		setText(":");
		//setFont( QFont( tr("JsTimeCode"), 10 ,QFont::Light ) );
		setStyleSheet("background-color:rgb(50,50,50); color:rgb(190,190,190);"); 
	}



};

class CTimeCodeEdit : public QWidget
{
	Q_OBJECT
public:
	CTimeCodeEdit(QWidget *parent);
	//~CTimeCodeEdit();

	void setCurFrame(int frame);
	void setDFreg(int freq) { m_freq = freq; }
	void setTotalFrames(int frame);

signals:
	void valueChangedSignal(int, bool);


protected:
	void resizeEvent(QResizeEvent *event);
	bool eventFilter ( QObject * watched, QEvent * event );
	//bool event(QEvent *event);
	//void focusInEvent(QFocusEvent *event);
	//void focusOutEvent(QFocusEvent *event);
	//void mousePressEvent(QMouseEvent *event);
	//void mouseReleaseEvent(QMouseEvent *event);
	//void keyPressEvent(QKeyEvent *event);
	//void mouseMoveEvent( QMouseEvent *event );
	//void mouseDoubleClickEvent(QMouseEvent *event);

	string frameToTimeCode(const int frame);
	bool HasFocus();
private slots:
	void	SlotValueChanged(int frame);
	void	SlotDragValueChanged(int type, bool inscrea);
	void    SlotNextFocus();
private:
	CTimeCodeEditSub	*m_hour;
	CTimeCodeEditSub	*m_minute;
	CTimeCodeEditSub	*m_second;
	CTimeCodeEditSub	*m_frame;

	CTimeCodeEditColon	*m_colon;
	CTimeCodeEditColon	*m_colon2;
	CTimeCodeEditColon	*m_colon3;

	bool			m_hasFocus;
	int				m_curFrame;
	int				m_freq;
	int				m_totalFrame;
};



#endif


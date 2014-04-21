#ifndef SOUNDCONVERTER_H
#define SOUNDCONVERTER_H

#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>
#include <QtGui/QStandardItemModel>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QMimeData>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QSettings>
#include <QtCore/QThread>

//---------------------------------------------------------
enum eData
{
    eDATA_FOLDER,
    eDATA_FLAC,
    eDATA_APE,
    eDATA_WAV,
    eDATA_M4A,
};

//---------------------------------------------------------
class TreeViewEx : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeViewEx(QWidget *parent);

private:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

signals:
    void signalDrop(const QList<QUrl> &listUrls) const;
};

//---------------------------------------------------------
class SoundConverter;

//---------------------------------------------------------
class TabConvert : public QWidget
{
    Q_OBJECT
public:
    TabConvert(SoundConverter *p_App, const bool b_Lame);

private:
    SoundConverter *const pApp;
    QStandardItemModel *sitemModel;

    TreeViewEx *trViewEx;
    QLineEdit *leSaveTo;
    QComboBox *cbLameOptions;
    QSpinBox *sbCount;

    const bool bLame;//@@?
    bool bM4a;//@@?=
    const QIcon iconFlac,
    iconApe,
    iconWav,
    iconM4a,
    iconMp3;
    const QChar chSep;

    QString strPathM4a;
    QString strEncodersPath;

    void fDelParentItem(const QModelIndex &modInd) const;
    void fAddSubFolder(const QString &strPath, QStandardItem *sitemParent) const;
    void fSubItem(const QStandardItem *const sitemParent, const QString &strLameParam,
                  const QString &strSaveTo, const QString &strAbsolute, const int iCut,
                  QStringList &slistMkDirs, QStringList &slistCommands, QStringList &slistFiles) const;



    //LOLO:
    QProgressBar *prbTotal;
    QWidget *wgtArea;
    QScrollArea *scrArea;
    QLabel *lblIcon,
    *lblMessage;
    QStackedWidget *stwgtArea;


    QComboBox *cbEnd;
    QPushButton *pbErrors;
    //TextEditEx *teErrors;
    QStringList slistCommands5;
    int iCommand,
    iComComplete,
    iComAll,
    iProcComplete,
    iProcAll,
    iErrors;
    //bool bLame;
    QVector<QProgressBar*> vectPrBars;
    QVector<int> vectProgress;
    QList<QByteArray> listLastOut;
    QByteArray baErrors;
    //--LOLO:

private slots:
    void slotDrop(const QList<QUrl> &listUrls);
    void slotDel();
    void slotAddFiles();
    void slotAddFolder();
    void slotSaveTo();
    void slotRun();




    //LOLO:
    void MYFYNC(const QStringList &slistCommands, const int iNumProc, const bool bLame, const QList< QPair<QString, QString> > &listActions);
};

//---------------------------------------------------------
class SoundConverter : public QStackedWidget
{
    Q_OBJECT
public:
    SoundConverter();
    ~SoundConverter();

private:
    QTabWidget *twMain;
    TabConvert *tabconv1,
    *tabconv2;
};









//class WgtRunning : public QWidget
//{
//    Q_OBJECT
//public:
//    WgtRunning(QWidget *parent);

//private:
//    QProgressBar *prbTotal;
//    QWidget *wgtArea;
//    QScrollArea *scrArea;
//    QLabel *lblIcon,
//    *lblMessage;
//    QStackedWidget *stwgtArea;
//    QCheckBox *chbExit,
//    *chbEndRun;
//    QComboBox *cbEnd;
//    QPushButton *pbErrors;
//    //TextEditEx *teErrors;
//    QStringList slistCommands;
//    int iCommand,
//    iComComplete,
//    iComAll,
//    iProcComplete,
//    iProcAll,
//    iErrors;
//    bool bLame;
//    QVector<QProgressBar*> vectPrBars;
//    QVector<int> vectProgress;
//    QList<QByteArray> listLastOut;
//    QByteArray baErrors;

//public slots:
//    void slotStart(const QStringList &slistCommands_, const int iNumProc,
//                   const QList<QPair<QString, QString> > &listActions, const bool bLame_);
//    void slotProcReadLame();
//    void slotProcReadNoLame();
//    void slotProcFinish();
//    void slotShowErrors();
//    void slotOk();

//signals:
//    void signalProgress(const int iValue, const int iMax);
//#ifdef Q_OS_WIN
//    void signalState(const TBPFLAG tbpFlags);
//#endif
//    void signalEnd();
//    void signalOk();
//};


#endif // SOUNDCONVERTER_H














#ifndef RUNNING_H
#define RUNNING_H

#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStackedWidget>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#ifdef Q_OS_WIN
#include <initguid.h>
#include <shobjidl.h>
#endif

class TextEditEx : public QPlainTextEdit
{
    Q_OBJECT
public:
    TextEditEx(QWidget * const parent);
    ~TextEditEx();
};

class WgtRunning : public QWidget
{
    Q_OBJECT
private:
    QProgressBar *prbTotal;
    QWidget *wgtArea;
    QScrollArea *scrArea;
    QLabel *lblIcon,
    *lblMessage;
    QStackedWidget *stwgtArea;
    QCheckBox *chbExit,
    *chbEndRun;
    QComboBox *cbEnd;
    QPushButton *pbErrors;
    TextEditEx *teErrors;
    QStringList slistCommands;
    int iCommand,
    iComComplete,
    iComAll,
    iProcComplete,
    iProcAll,
    iErrors;
    bool bLame;
    QVector<QProgressBar*> vectPrBars;
    QVector<int> vectProgress;
    QList<QByteArray> listLastOut;
    QByteArray baErrors;

public:
    WgtRunning(QWidget * const parent);

public slots:
    void slotStart(const QStringList &slistCommands_, const int &iNumProc,
                   const QList<QPair<QString, QString> > &listActions, const bool &bLame_);
    void slotProcReadLame();
    void slotProcReadNoLame();
    void slotProcFinish();
    void slotShowErrors();
    void slotOk();

signals:
    void signalProgress(const int &iValue, const int &iMax);
#ifdef Q_OS_WIN
    void signalState(const TBPFLAG &tbpFlags);
#endif
    void signalEnd();
    void signalOk();
};

#endif // RUNNING_H

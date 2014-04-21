#include "running.h"

//-------------------------------------------------------------------------------------------------
TextEditEx::TextEditEx(QWidget * const parent) : QPlainTextEdit(parent)
{
    this->setWindowFlags(Qt::Window);
    this->setWindowTitle(qApp->applicationDisplayName() + ": " + tr("Errors"));
    this->setWordWrapMode(QTextOption::NoWrap);
    if (!this->restoreGeometry(QSettings("UserPrograms", qApp->applicationName()).value("GeometryErrors").toByteArray()))
        this->resize(this->sizeHint());
}

//-------------------------------------------------------------------------------------------------
TextEditEx::~TextEditEx()
{
    QSettings("UserPrograms", qApp->applicationName()).setValue("GeometryErrors", this->saveGeometry());
}

//-------------------------------------------------------------------------------------------------
WgtRunning::WgtRunning(QWidget * const parent) : QWidget(parent)
{
    prbTotal = new QProgressBar(this);
    prbTotal->setAlignment(Qt::AlignHCenter);

    wgtArea = new QWidget(this);
    scrArea = new QScrollArea(this);
    scrArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrArea->setWidgetResizable(true);
    scrArea->setWidget(wgtArea);
    lblIcon = new QLabel(this);
    lblMessage = new QLabel(this);
    QHBoxLayout * const hblMessageTop = new QHBoxLayout;
    hblMessageTop->addWidget(lblIcon);
    hblMessageTop->addWidget(lblMessage);
    QPushButton * const pbClose = new QPushButton("&OK", this);
    QFrame * const frame = new QFrame(this);
    frame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout * const vblMessage = new QVBoxLayout(frame);
    vblMessage->addLayout(hblMessageTop);
    vblMessage->addWidget(pbClose, 0, Qt::AlignHCenter);
    QWidget * const wgtAreaMessage = new QWidget(this);
    (new QVBoxLayout(wgtAreaMessage))->addWidget(frame, 0, Qt::AlignCenter);
    stwgtArea = new QStackedWidget(this);
    stwgtArea->addWidget(scrArea);
    stwgtArea->addWidget(wgtAreaMessage);

    QLabel * const lblEnd = new QLabel(tr("At end:"), this);
    chbExit = new QCheckBox(tr("&Exit"), this);
    chbEndRun = new QCheckBox(tr("R&un"), this);
    cbEnd = new QComboBox(this);
    cbEnd->setEnabled(false);
    pbErrors = new QPushButton(this);

    QHBoxLayout * const hblEnd = new QHBoxLayout;
    hblEnd->addWidget(lblEnd);
    hblEnd->addSpacing(5);
    hblEnd->addWidget(chbExit);
    hblEnd->addWidget(chbEndRun);
    hblEnd->addWidget(cbEnd, 1);
    hblEnd->addWidget(pbErrors);

    QVBoxLayout * const vblMain = new QVBoxLayout(this);
    vblMain->setContentsMargins(0, 0, 0, 0);
    vblMain->setSpacing(0);
    vblMain->addWidget(prbTotal);
    vblMain->addWidget(stwgtArea);
    vblMain->addLayout(hblEnd);

    teErrors = new TextEditEx(this);

    //connects
    connect(pbClose, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(pbErrors, SIGNAL(clicked()), this, SLOT(slotShowErrors()));
    connect(chbEndRun, SIGNAL(toggled(bool)), cbEnd, SLOT(setEnabled(bool)));
}

//-------------------------------------------------------------------------------------------------
void WgtRunning::slotStart(const QStringList &slistCommands_, const int &iNumProc, const QList<QPair<QString, QString> > &listActions, const bool &bLame_)
{
    chbExit->setEnabled(true);
    chbExit->setChecked(false);
    chbEndRun->setEnabled(true);
    chbEndRun->setChecked(false);
    cbEnd->addItem(tr("Nothing"));
    if (listActions.isEmpty())
        chbEndRun->setEnabled(false);
    else
    {
        int i = 1;
        for (QList<QPair<QString, QString> >::const_iterator it = listActions.constBegin(); it != listActions.constEnd(); ++it, ++i)
        {
            cbEnd->addItem(it->first);
            cbEnd->setItemData(i, it->second, Qt::ToolTipRole);
        }
    }
    cbEnd->setEnabled(false);
    pbErrors->setText(tr("No errors"));
    pbErrors->setEnabled(false);

    slistCommands = slistCommands_;
    iErrors = iProcComplete = iComComplete = iCommand = 0;
    iComAll = slistCommands.count();
    iProcAll = iComAll < iNumProc ? iComAll : iNumProc;
    bLame = bLame_;

    prbTotal->setValue(0);
    prbTotal->setFormat("0/" + QString::number(iComAll) + "     %p%");

    QVBoxLayout * const vblArea = new QVBoxLayout(wgtArea);
    vblArea->addStretch();
    for (int i = 0; i < iProcAll; ++i)
    {
        QProgressBar * const prb = new QProgressBar(wgtArea);
        prb->setAlignment(Qt::AlignHCenter);
        vectPrBars.append(prb);
        vectProgress.append(0);
        listLastOut.append(0);
        const QString strNum = QString::number(i+1);
        QGroupBox * const gb = new QGroupBox(tr("Process:") + ' ' + strNum, wgtArea);
        (new QVBoxLayout(gb))->addWidget(prb);
        vblArea->addWidget(gb);
        vblArea->addStretch();

        QProcess * const proc = new QProcess(prb);
        proc->setProperty("Num", i);
        if (bLame)
        {
            prb->setValue(0);
            prb->setFormat('#' + strNum + "     %p%");
            connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(slotProcReadLame()));
        }
        else
        {
            prb->setMaximum(0);
            connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(slotProcReadNoLame()));
        }
        connect(proc, SIGNAL(readChannelFinished()), this, SLOT(slotProcFinish()));
        const QString strCommand = slistCommands.at(iCommand++);
        prb->setToolTip(strCommand);
        proc->start(strCommand);
    }
    stwgtArea->setCurrentIndex(0);
}

//-------------------------------------------------------------------------------------------------
void WgtRunning::slotProcReadLame()
{
    QProcess * const proc = /*qobject_cast*/static_cast<QProcess*>(sender());
    if (!proc)
        return;
    const int iNum = proc->property("Num").toInt();
    const QByteArray baOut = proc->readAllStandardError();
    listLastOut[iNum] = baOut;
    const int iPercentEnd = baOut.indexOf("%)|");
    if (iPercentEnd > 0)
    {
        int iPercent = baOut.lastIndexOf('(', iPercentEnd - 2);        //[2 = "(?"]
        if (iPercent >= 0 && (iPercent = baOut.mid(iPercent + 1, iPercentEnd - iPercent - 1).toInt()))
        {
            QProgressBar * const prBar = vectPrBars.at(iNum);
            prBar->setValue(iPercent);
            vectProgress[iNum] = iPercent;
            int iTotalProgress = iComComplete*100;        //complete = 100%
            for (QVector<int>::const_iterator it = vectProgress.constBegin(); it != vectProgress.constEnd(); ++it)
                iTotalProgress += *it;
            prbTotal->setValue(iTotalProgress/iComAll);
            signalProgress(iTotalProgress, iComAll);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void WgtRunning::slotProcReadNoLame()
{
    if (QProcess * const proc = /*qobject_cast*/static_cast<QProcess*>(sender()))
        listLastOut[proc->property("Num").toInt()] = proc->readAllStandardError();
}

//-------------------------------------------------------------------------------------------------
void WgtRunning::slotProcFinish()
{
    QProcess * const proc = /*qobject_cast*/static_cast<QProcess*>(sender());
    if (!proc)
        return;

    //Progress Bars
    const int iNum = proc->property("Num").toInt();
    QProgressBar * const prBar = vectPrBars.at(iNum);
    vectProgress[iNum] = 0;
    int iTotalProgress = ++iComComplete*100;        //complete = 100%
    if (bLame)
    {
        prBar->setValue(100);
        for (QVector<int>::const_iterator it = vectProgress.constBegin(); it != vectProgress.constEnd(); ++it)
            iTotalProgress += *it;
    }

    prbTotal->setValue(iTotalProgress/iComAll);
    prbTotal->setFormat(QString::number(iComComplete) + '/' + QString::number(iComAll) + "     %p%");
    signalProgress(iTotalProgress, iComAll);

    //Error
    const QByteArray baLastOut = listLastOut.at(iNum).trimmed();
    if (bLame)
    {
        if (!baLastOut.endsWith("done") && !baLastOut.endsWith("dB"))
        {
            //Lame output UTF-8
            baErrors += "==================================================\n" + prBar->toolTip() + ":\n" + baLastOut + '\n';
            pbErrors->setText(tr("E&rrors:") + ' ' + QString::number(++iErrors));
            pbErrors->setEnabled(true);
#ifdef Q_OS_WIN
            signalState(TBPF_ERROR);
#endif
        }
    }
    else if (!baLastOut.endsWith("done") && !baLastOut.endsWith("Success...") && !baLastOut.endsWith("%)") && !baLastOut.endsWith('%'))
    {
        //Flac, Mac, Wvunpack - output ANSI, Avconv - output UTF-8
        baErrors += "==================================================\n" + prBar->toolTip() + ":\n" + (prBar->toolTip().indexOf(".m4a\" \"") > 0 ? baLastOut : QString::fromLocal8Bit(baLastOut)) + '\n';
        pbErrors->setText(tr("E&rrors:") + ' ' + QString::number(++iErrors));
        pbErrors->setEnabled(true);
#ifdef Q_OS_WIN
        signalState(TBPF_ERROR);
#endif
    }

    //New command
    const QString strCommand = slistCommands.value(iCommand++);
    if (strCommand.isEmpty())
    {
        prBar->setToolTip(0);
        if (bLame)
            prBar->setFormat("100%");
        else
        {
            prBar->setMaximum(100);
            prBar->setValue(100);
        }
        if (++iProcComplete == iProcAll)
        {
            const QString strAppName = qApp->applicationName();
            QSettings("UserPrograms", strAppName).setValue("Error", 0);
            if (iErrors)
            {
                const QString strErrorDir = strAppName + "_log";
                const QDir dir;
                if (dir.exists(strErrorDir) || dir.mkdir(strErrorDir))
                {
                    QFile file(strErrorDir + '/' + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".log");
                    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
                        file.write(baErrors);
                    //file.close();
                }
            }
#ifdef Q_OS_WIN
            MessageBeep(MB_OK);
#endif
            if (chbEndRun->isChecked())
            {
                const QString strRun = cbEnd->itemData(cbEnd->currentIndex(), Qt::ToolTipRole).toString();
                if (!strRun.isEmpty())
                    QProcess::startDetached(strRun);
            }
            if (chbExit->isChecked())
                qApp->quit();
            else
            {
                signalEnd();
#ifdef Q_OS_WIN
                signalState(TBPF_NOPROGRESS);
#endif
                slistCommands.clear();
                vectPrBars.clear();
                vectProgress.clear();
                listLastOut.clear();
                const QObjectList listObj = wgtArea->children();
                if (iErrors)
                {
                    lblIcon->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxWarning));
                    lblMessage->setText(tr("Convert completed with errors"));
                }
                else
                {
                    lblIcon->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxInformation));
                    lblMessage->setText(tr("Convert completed successfully"));
                }
                chbExit->setEnabled(false);
                chbEndRun->setEnabled(false);
                cbEnd->setEnabled(false);
                stwgtArea->setCurrentIndex(1);
                for (QObjectList::const_iterator it = listObj.constBegin(); it != listObj.constEnd(); ++it)
                    (*it)->deleteLater();
            }
        }
    }
    else
    {
        prBar->setToolTip(strCommand);
        if (bLame)
        {
            prBar->setValue(0);
            prBar->setFormat('#' + QString::number(iCommand) + "     %p%");
        }
        proc->start(strCommand);
    }
}

//-------------------------------------------------------------------------------------------------
void WgtRunning::slotOk()
{
    baErrors.clear();
    teErrors->clear();
    cbEnd->clear();
    signalOk();
}

//-------------------------------------------------------------------------------------------------
void WgtRunning::slotShowErrors()
{
    teErrors->setPlainText(baErrors);
    if (teErrors->isHidden())
        teErrors->show();
    else
        teErrors->activateWindow();
}

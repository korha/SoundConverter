#include "soundconverter.h"

#define MAX_PROC 100

#ifdef Q_OS_WIN
#define APP_FLAC "flac.exe"
#define APP_APE  "mac.exe"
#define APP_WV   "wvunpack.exe"
#define APP_LAME "lame.exe"
#else
#define APP_FLAC "flac"
#define APP_APE  "mac"
#define APP_WV   "wvunpack"
#define APP_LAME "lame"
#endif

//-------------------------------------------------------------------------------------------------
TreeViewEx::TreeViewEx(QWidget *parent) : QTreeView(parent)
{
    this->header()->hide();
    this->setAlternatingRowColors(true);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->setAcceptDrops(true);
}

//-------------------------------------------------------------------------------------------------
void TreeViewEx::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

//-------------------------------------------------------------------------------------------------
void TreeViewEx::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

//-------------------------------------------------------------------------------------------------
void TreeViewEx::dropEvent(QDropEvent *event)
{
    Q_ASSERT(event->mimeData()->hasUrls());
    this->setUpdatesEnabled(false);
    signalDrop(event->mimeData()->urls());
    this->setUpdatesEnabled(true);
}

//-------------------------------------------------------------------------------------------------
TabConvert::TabConvert(SoundConverter *p_App, const bool b_Lame) : QWidget(p_App),
    pApp(p_App),
    sitemModel(new QStandardItemModel(this)),
    bLame(b_Lame),
    iconFlac(":/img/flac.png"),
    iconApe(":/img/ape.png"),
    iconWav(":/img/wav.png"),
    iconM4a(":/img/m4a.png"),
    iconMp3(":/img/mp3.png"),
    chSep(QDir::separator()),
    strEncodersPath(qAppName() + "_bin" + chSep)
{
    QAction *actDel = new QAction(this);
    actDel->setShortcut(QKeySequence::Delete);

    trViewEx = new TreeViewEx(this);
    trViewEx->setModel(sitemModel);
    trViewEx->addAction(actDel);

    QPushButton *pbDel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), tr("Remove"), this);
    QPushButton *pbAddFiles = new QPushButton(QIcon(":/img/addfiles.png"), tr("Add Files"), this);
    QPushButton *pbAddFolder = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Add Folder"), this);
    QHBoxLayout *hblActions = new QHBoxLayout;
    hblActions->addWidget(pbDel);
    hblActions->addWidget(pbAddFiles);
    hblActions->addWidget(pbAddFolder);

    QLabel *lblSaveTo = new QLabel(tr("Save to:"), this);
    leSaveTo = new QLineEdit(this);
    QToolButton *tbSaveTo = new QToolButton(this);
    tbSaveTo->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    QHBoxLayout *hblSaveTo = new QHBoxLayout;
    hblSaveTo->addWidget(lblSaveTo);
    hblSaveTo->addWidget(leSaveTo);
    hblSaveTo->addWidget(tbSaveTo);

    QVBoxLayout *vblMain = new QVBoxLayout(this);
    vblMain->addWidget(trViewEx);
    vblMain->addLayout(hblActions);
    vblMain->addLayout(hblSaveTo);

    if (bLame)
    {
        QLabel *lblOptions = new QLabel(tr("Lame options:"), this);
        cbLameOptions = new QComboBox(this);
        QHBoxLayout *hblOptions = new QHBoxLayout;
        hblOptions->addWidget(lblOptions);
        hblOptions->addWidget(cbLameOptions, 1);
        vblMain->addLayout(hblOptions);
    }

    QLabel *lblCount = new QLabel(tr("Number of processes:"));
    sbCount = new QSpinBox(this);
    sbCount->setRange(1, MAX_PROC);

    QPushButton *pbRun = new QPushButton(style()->standardIcon(QStyle::SP_CommandLink), tr("Run"), this);
    QHBoxLayout *hblRun = new QHBoxLayout;
    hblRun->addWidget(lblCount);
    hblRun->addWidget(sbCount);
    hblRun->addWidget(pbRun, 1);

    vblMain->addLayout(hblRun);

    //connects
    connect(trViewEx, SIGNAL(signalDrop(QList<QUrl>)), this, SLOT(slotDrop(QList<QUrl>)));
    connect(actDel, SIGNAL(triggered()), this, SLOT(slotDel()));
    connect(pbDel, SIGNAL(clicked()), this, SLOT(slotDel()));
    connect(pbAddFiles, SIGNAL(clicked()), this, SLOT(slotAddFiles()));
    connect(pbAddFolder, SIGNAL(clicked()), this, SLOT(slotAddFolder()));
    connect(tbSaveTo, SIGNAL(clicked()), this, SLOT(slotSaveTo()));
    connect(pbRun, SIGNAL(clicked()), this, SLOT(slotRun()));

    prbTotal = new QProgressBar(this);
    prbTotal->setAlignment(Qt::AlignHCenter);

    wgtArea = new QWidget(this);

    scrArea = new QScrollArea(this);
    scrArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrArea->setWidgetResizable(true);
    scrArea->setWidget(wgtArea);

    lblIcon = new QLabel(this);
    lblMessage = new QLabel(this);
    QHBoxLayout *hblMessageTop = new QHBoxLayout;
    hblMessageTop->addWidget(lblIcon);
    hblMessageTop->addWidget(lblMessage);

    QPushButton *pbClose = new QPushButton("OK", this);
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::StyledPanel);
    QVBoxLayout *vblMessage = new QVBoxLayout(frame);
    vblMessage->addLayout(hblMessageTop);
    vblMessage->addWidget(pbClose, 0, Qt::AlignHCenter);
    QWidget *wgtAreaMessage = new QWidget(this);
    (new QVBoxLayout(wgtAreaMessage))->addWidget(frame, 0, Qt::AlignCenter);
    stwgtArea = new QStackedWidget(this);
    stwgtArea->addWidget(scrArea);
    stwgtArea->addWidget(wgtAreaMessage);

    QLabel *lblEnd = new QLabel(tr("At end:"), this);
    cbEnd = new QComboBox(this);
    cbEnd->setEnabled(false);
    pbErrors = new QPushButton(this);

    QHBoxLayout *hblEnd = new QHBoxLayout;
    hblEnd->addWidget(lblEnd);
    hblEnd->addSpacing(5);
    hblEnd->addWidget(cbEnd, 1);
    hblEnd->addWidget(pbErrors);

    QVBoxLayout *vblMainw = new QVBoxLayout(this);
    vblMainw->setContentsMargins(0, 0, 0, 0);
    vblMainw->setSpacing(0);
    vblMainw->addWidget(prbTotal);
    vblMainw->addWidget(stwgtArea);
    vblMainw->addLayout(hblEnd);
}

//-------------------------------------------------------------------------------------------------
void TabConvert::slotDrop(const QList<QUrl> &listUrls)
{
    for (int i = 0; i < listUrls.size(); ++i)
    {
        const QString strPath = listUrls.at(i).toLocalFile();
        if (QFileInfo(strPath).isDir())
        {
            QStandardItem *sitem = new QStandardItem(style()->standardIcon(QStyle::SP_DirIcon), QDir::toNativeSeparators(strPath));
            sitem->setData(eDATA_FOLDER);
            sitem->setEditable(false);
            sitemModel->appendRow(sitem);
            fAddSubFolder(strPath, sitem);
        }
        else if (bLame)
        {
            if (strPath.endsWith(".wav", Qt::CaseInsensitive))
            {
                QStandardItem *sitem = new QStandardItem(iconWav, QDir::toNativeSeparators(strPath));
                sitem->setEditable(false);
                sitemModel->appendRow(sitem);
            }
            else if (strPath.endsWith(".mp3", Qt::CaseInsensitive))
            {
                QStandardItem *sitem = new QStandardItem(iconMp3, QDir::toNativeSeparators(strPath));
                sitem->setEditable(false);
                sitemModel->appendRow(sitem);
            }
        }
        else if (strPath.endsWith(".flac", Qt::CaseInsensitive) || strPath.endsWith(".fla", Qt::CaseInsensitive))
        {
            QStandardItem *sitem = new QStandardItem(iconFlac, QDir::toNativeSeparators(strPath));
            sitem->setData(eDATA_FLAC);
            sitem->setEditable(false);
            sitemModel->appendRow(sitem);
        }
        else if (strPath.endsWith(".ape", Qt::CaseInsensitive))
        {
            QStandardItem *sitem = new QStandardItem(iconApe, QDir::toNativeSeparators(strPath));
            sitem->setData(eDATA_APE);
            sitem->setEditable(false);
            sitemModel->appendRow(sitem);
        }
        else if (strPath.endsWith(".wv", Qt::CaseInsensitive))
        {
            QStandardItem *sitem = new QStandardItem(iconWav, QDir::toNativeSeparators(strPath));
            sitem->setData(eDATA_WAV);
            sitem->setEditable(false);
            sitemModel->appendRow(sitem);
        }
        else if (bM4a && strPath.endsWith(".m4a", Qt::CaseInsensitive))
        {
            QStandardItem *sitem = new QStandardItem(iconM4a, QDir::toNativeSeparators(strPath));
            sitem->setData(eDATA_M4A);
            sitem->setEditable(false);
            sitemModel->appendRow(sitem);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void TabConvert::fDelParentItem(const QModelIndex &modInd) const
{
    if (modInd.isValid() && !sitemModel->itemFromIndex(modInd)->hasChildren())
    {
        const QModelIndex modIndParent = modInd.parent();
        sitemModel->removeRow(modInd.row(), modIndParent);
        fDelParentItem(modIndParent);
    }
}

//-------------------------------------------------------------------------------------------------
void TabConvert::slotDel()
{
    trViewEx->setUpdatesEnabled(false);
    QModelIndexList modIndList = trViewEx->selectionModel()->selectedRows();
    qSort(modIndList.begin(), modIndList.end(), qGreater<QModelIndex>());
    for (int i = 0; i < modIndList.size(); ++i)
    {
        const QModelIndex modIndParent = modIndList.at(i).parent();
        sitemModel->removeRow(modIndList.at(i).row(), modIndParent);
        fDelParentItem(modIndParent);
    }
    trViewEx->setUpdatesEnabled(true);
}

//-------------------------------------------------------------------------------------------------
void TabConvert::slotAddFiles()
{
    if (bLame)
    {
        const QStringList slistPaths = QFileDialog::getOpenFileNames(this, 0, 0, "Audio (*.wav *.mp3)");
        if (!slistPaths.isEmpty())
        {
            trViewEx->setUpdatesEnabled(false);
            for (int i = 0; i < slistPaths.size(); ++i)
            {
                Q_ASSERT(slistPaths.at(i).endsWith(".wav", Qt::CaseInsensitive) || slistPaths.at(i).endsWith(".mp3", Qt::CaseInsensitive));
                QStandardItem *sitem = new QStandardItem(slistPaths.at(i).endsWith(".wav", Qt::CaseInsensitive) ?
                                                             iconWav : iconMp3, QDir::toNativeSeparators(slistPaths.at(i)));
                sitem->setEditable(false);
                sitemModel->appendRow(sitem);
            }
            trViewEx->setUpdatesEnabled(true);
        }
    }
    else
    {
        const QStringList slistPaths = QFileDialog::getOpenFileNames(this, 0, 0, bM4a ? "Lossless Audio (*.flac *.fla *.ape *.wv *.m4a)" :
                                                                                        "Lossless Audio (*.flac *.fla *.ape *.wv)");
        if (!slistPaths.isEmpty())
        {
            trViewEx->setUpdatesEnabled(false);
            for (int i = 0; i < slistPaths.size(); ++i)
                if (slistPaths.at(i).endsWith(".ape", Qt::CaseInsensitive))
                {
                    QStandardItem *sitem = new QStandardItem(iconApe, QDir::toNativeSeparators(slistPaths.at(i)));
                    sitem->setData(eDATA_APE);
                    sitem->setEditable(false);
                    sitemModel->appendRow(sitem);
                }
                else if (slistPaths.at(i).endsWith(".wv", Qt::CaseInsensitive))
                {
                    QStandardItem *sitem = new QStandardItem(iconWav, QDir::toNativeSeparators(slistPaths.at(i)));
                    sitem->setData(eDATA_WAV);
                    sitem->setEditable(false);
                    sitemModel->appendRow(sitem);
                }
                else if (slistPaths.at(i).endsWith(".m4a", Qt::CaseInsensitive))
                {
                    QStandardItem *sitem = new QStandardItem(iconM4a, QDir::toNativeSeparators(slistPaths.at(i)));
                    sitem->setData(eDATA_M4A);
                    sitem->setEditable(false);
                    sitemModel->appendRow(sitem);
                }
                else
                {
                    Q_ASSERT(slistPaths.at(i).endsWith(".flac", Qt::CaseInsensitive) || slistPaths.at(i).endsWith(".fla", Qt::CaseInsensitive));
                    QStandardItem *sitem = new QStandardItem(iconFlac, QDir::toNativeSeparators(slistPaths.at(i)));
                    sitem->setData(eDATA_FLAC);
                    sitem->setEditable(false);
                    sitemModel->appendRow(sitem);
                }
            trViewEx->setUpdatesEnabled(true);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void TabConvert::fAddSubFolder(const QString &strPath, QStandardItem *sitemParent) const
{
    const QDir dir(strPath);
    QStringList slistPaths = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < slistPaths.size(); ++i)
    {
        QStandardItem *sitem = new QStandardItem(style()->standardIcon(QStyle::SP_DirIcon), slistPaths.at(i));
        sitem->setData(eDATA_FOLDER);
        sitem->setEditable(false);
        sitemParent->appendRow(sitem);
        fAddSubFolder(dir.absoluteFilePath(slistPaths.at(i)), sitem);
    }
    if (bLame)
    {
        slistPaths = dir.entryList(QStringList() << "*.wav" << "*.mp3", QDir::Files);
        for (int i = 0; i < slistPaths.size(); ++i)
        {
            Q_ASSERT(slistPaths.at(i).endsWith(".wav", Qt::CaseInsensitive) || slistPaths.at(i).endsWith(".mp3", Qt::CaseInsensitive));
            QStandardItem *sitem = new QStandardItem(slistPaths.at(i).endsWith(".wav", Qt::CaseInsensitive) ?
                                                         iconWav : iconMp3, QDir::toNativeSeparators(slistPaths.at(i)));
            sitem->setEditable(false);
            sitemParent->appendRow(sitem);
        }
    }
    else
    {
        slistPaths = dir.entryList(bM4a ? (QStringList() << "*.flac" << "*.fla" << "*.ape" << "*.wv" << "*.m4a") :
                                          (QStringList() << "*.flac" << "*.fla" << "*.ape" << "*.wv"), QDir::Files);
        for (int i = 0; i < slistPaths.size(); ++i)
            if (slistPaths.at(i).endsWith(".ape", Qt::CaseInsensitive))
            {
                QStandardItem *sitem = new QStandardItem(iconApe, slistPaths.at(i));
                sitem->setData(eDATA_APE);
                sitem->setEditable(false);
                sitemParent->appendRow(sitem);
            }
            else if (slistPaths.at(i).endsWith(".wv", Qt::CaseInsensitive))
            {
                QStandardItem *sitem = new QStandardItem(iconWav, slistPaths.at(i));
                sitem->setData(eDATA_WAV);
                sitem->setEditable(false);
                sitemParent->appendRow(sitem);
            }
            else if (slistPaths.at(i).endsWith(".m4a", Qt::CaseInsensitive))
            {
                QStandardItem *sitem = new QStandardItem(iconM4a, slistPaths.at(i));
                sitem->setData(eDATA_M4A);
                sitem->setEditable(false);
                sitemParent->appendRow(sitem);
            }
            else
            {
                Q_ASSERT(slistPaths.at(i).endsWith(".flac", Qt::CaseInsensitive) || slistPaths.at(i).endsWith(".fla", Qt::CaseInsensitive));
                QStandardItem *sitem = new QStandardItem(iconFlac, slistPaths.at(i));
                sitem->setData(eDATA_FLAC);
                sitem->setEditable(false);
                sitemParent->appendRow(sitem);
            }
    }
    if (!sitemParent->hasChildren())
        sitemModel->removeRow(sitemParent->row(), sitemModel->indexFromItem(sitemParent->parent()));
}

//-------------------------------------------------------------------------------------------------
void TabConvert::slotAddFolder()
{
    const QString strPath = QFileDialog::getExistingDirectory(this);
    if (!strPath.isEmpty())
    {
        trViewEx->setUpdatesEnabled(false);
        QStandardItem *sitem = new QStandardItem(style()->standardIcon(QStyle::SP_DirIcon), QDir::toNativeSeparators(strPath));
        sitem->setData(eDATA_FOLDER);
        sitem->setEditable(false);
        sitemModel->appendRow(sitem);
        fAddSubFolder(strPath, sitem);
        trViewEx->setUpdatesEnabled(true);
    }
}

//-------------------------------------------------------------------------------------------------
void TabConvert::slotSaveTo()
{
    const QString strPath = QFileDialog::getExistingDirectory(this);
    if (!strPath.isEmpty())
        leSaveTo->setText(QDir::toNativeSeparators(strPath));
}

//-------------------------------------------------------------------------------------------------
void TabConvert::fSubItem(const QStandardItem *const sitemParent, const QString &strLameParam,
                          const QString &strSaveTo, const QString &strAbsolute, const int iCut,
                          QStringList &slistMkDirs, QStringList &slistCommands, QStringList &slistFiles) const
{
    slistMkDirs.append(strSaveTo + strAbsolute.mid(iCut));
    for (int i = 0; i < sitemParent->rowCount(); ++i)
    {
        const QStandardItem *sitem = sitemParent->child(i);
        const QString strItemText = sitem->text();
        if (sitem->data() == eDATA_FOLDER)
            fSubItem(sitem, strLameParam, strSaveTo, strAbsolute + strItemText + chSep, iCut, slistMkDirs, slistCommands, slistFiles);
        else
        {
            const QString strFile = strAbsolute.mid(iCut) + QFileInfo(strItemText).completeBaseName();
            slistFiles.append(strFile);
            if (bLame)
                slistCommands.append(strEncodersPath + APP_LAME + ' ' + strLameParam + " \"" + strAbsolute + strItemText + "\" \"" + strSaveTo + strFile + ".mp3\"");
            else if (sitem->data() == eDATA_APE)
                slistCommands.append(strEncodersPath + APP_APE + " \"" + strAbsolute + strItemText + "\" \"" + strSaveTo + strFile + ".wav\" -d");
            else if (sitem->data() == eDATA_WAV)
                slistCommands.append(strEncodersPath + APP_WV + " -y \"" + strAbsolute + strItemText + "\" \"" + strSaveTo + strFile + ".wav\"");
            else if (sitem->data() == eDATA_M4A)
                slistCommands.append(strPathM4a + " -y -i \"" + strAbsolute + strItemText + "\" \"" + strSaveTo + strFile + ".wav\"");
            else
            {
                Q_ASSERT(sitem->data() == eDATA_FLAC);
                slistCommands.append(strEncodersPath + APP_FLAC + " -d -f -o \"" + strSaveTo + strFile + ".wav\" \"" + strAbsolute + strItemText + '\"');
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
void TabConvert::slotRun()
{
    const QStandardItem *sitemRoot = sitemModel->invisibleRootItem();
    if (!sitemRoot->hasChildren())
    {
        QMessageBox::warning(this, 0, tr("Missing files to convert"));
        return;
    }
    QString strSaveTo = QDir::toNativeSeparators(leSaveTo->text());
    if (strSaveTo.isEmpty())
    {
        leSaveTo->setFocus();
        QMessageBox::warning(this, 0, tr("Choose save folder"));
        return;
    }
    if (strSaveTo.at(strSaveTo.length()-1) != chSep)
        strSaveTo += chSep;
    const QDir dir;
    if (!dir.exists(strSaveTo) && !dir.mkpath(strSaveTo))
    {
        leSaveTo->setFocus();
        leSaveTo->selectAll();
        QMessageBox::warning(this, 0, tr("Don't create folder:"));
        return;
    }

    QString strLameParam;
    if (bLame)
    {
        const int iCurIndex = cbLameOptions->currentIndex();
        if (iCurIndex >= 0)
            strLameParam = cbLameOptions->itemData(iCurIndex, Qt::ToolTipRole).toString();
    }

    QStringList slistMkDirs,
            slistCommands,
            slistFiles;
    for (int i = 0; i < sitemRoot->rowCount(); ++i)
    {
        const QStandardItem *sitem = sitemRoot->child(i);
        const QString strItemText = sitem->text();
        if (sitem->data() == eDATA_FOLDER)
            fSubItem(sitem, strLameParam, strSaveTo, strItemText + chSep, strItemText.length() - QDir(strItemText).dirName().length(), slistMkDirs, slistCommands, slistFiles);
        else
        {
            const QString strFile = QFileInfo(strItemText).completeBaseName();
            slistFiles.append(strFile);
            if (bLame)
                slistCommands.append(strEncodersPath + APP_LAME + ' ' + strLameParam + " \"" + strItemText + "\" \"" + strSaveTo + strFile + ".mp3\"");
            else if (sitem->data() == eDATA_APE)
                slistCommands.append(strEncodersPath + APP_APE + " \"" + strItemText + "\" \"" + strSaveTo + strFile + ".wav\" -d");
            else if (sitem->data() == eDATA_WAV)
                slistCommands.append(strEncodersPath + APP_WV + " -y \"" + strItemText + "\" \"" + strSaveTo + strFile + ".wav\"");
            else if (sitem->data() == eDATA_M4A)
                slistCommands.append(strPathM4a + " -y -i \"" + strItemText + "\" \"" + strSaveTo + strFile + ".wav\"");
            else
            {
                Q_ASSERT(sitem->data() == eDATA_FLAC);
                slistCommands.append(strEncodersPath + APP_FLAC + " -d -f -o \"" + strSaveTo + strFile + ".wav\" \"" + strItemText + '\"');
            }
        }
    }

    for (int i = 0; i < slistFiles.size(); ++i)
        slistFiles[i] = slistFiles.at(i).toLower();

    for (int i = 0; i < slistFiles.size(); ++i)
        for (int j = i+1; j < slistFiles.size(); ++j)
            if (slistFiles.at(i) == slistFiles.at(j))
            {
                QMessageBox::warning(this, 0, tr("Some files are repeated"));
                return;
            }

    slistMkDirs.removeDuplicates();
    for (int i = 0; i < slistMkDirs.size(); ++i)
        if (!dir.mkpath(slistMkDirs.at(i)))
        {
            QMessageBox::warning(this, 0, tr("Don't create folder:") + '\n' + slistMkDirs.at(i));
            return;
        }

    sitemModel->clear();
}

//-------------------------------------------------------------------------------------------------
SoundConverter::SoundConverter() : QStackedWidget()
{
    tabconv1 = new TabConvert(this, false);
    tabconv2 = new TabConvert(this, true);

    QPushButton *pbSettings = new QPushButton(QIcon(":/img/settings.png"), tr("Settings"), this);
    QLabel *lblNotes = new QLabel(
                "FLAC: <a href=\"http://xiph.org/flac\">http://xiph.org/flac</a><br />"
                "Monkey's Audio: <a href=\"http://monkeysaudio.com\">http://monkeysaudio.com</a><br />"
                "WAVPack: <a href=\"http://wavpack.com\">http://wavpack.com</a><br />"
                "LAME MP3 Encoder: <a href=\"http://lame.sourceforge.net\">http://lame.sourceforge.net</a><br />"
                "Libav: <a href=\"http://libav.org\">http://libav.org</a>", this);
    lblNotes->setOpenExternalLinks(true);
    lblNotes->setAlignment(Qt::AlignCenter);
    QWidget *wgtTabSettings = new QWidget(this);
    QVBoxLayout *vblTabSettings = new QVBoxLayout(wgtTabSettings);
    vblTabSettings->addWidget(pbSettings, 0, Qt::AlignHCenter);
    vblTabSettings->addWidget(lblNotes);

    //==
    twMain = new QTabWidget(this);
    twMain->addTab(tabconv1, "flac/ape/wv -> wav");
    twMain->addTab(tabconv2, "wav/mp3 -> mp3");
    twMain->addTab(wgtTabSettings, tr("Settings"));


    this->addWidget(twMain);

    const QSettings stg("UserPrograms", qAppName());
    this->restoreGeometry(stg.value("Geometry").toByteArray());
}

//-------------------------------------------------------------------------------------------------
SoundConverter::~SoundConverter()
{
    QSettings("UserPrograms", qAppName()).setValue("Geometry", this->saveGeometry());
}

TabConvert::MYFYNC(const QStringList &slistCommands, const int iNumProc, const bool bLame, const QList<QPair<QString, QString> > &listActions)
{
    cbEnd->clear();
    cbEnd->addItem("-----");
    if (listActions.isEmpty())
        cbEnd->setEnabled(false);
    else
    {
        for (int i = 0; i < listActions.size(); )
        {
            cbEnd->addItem(listActions.at(i).first);
            cbEnd->setItemData(++i, listActions.at(i).second, Qt::ToolTipRole);
        }
        cbEnd->setEnabled(true);
    }

    pbErrors->setText(tr("No errors"));
    pbErrors->setEnabled(false);

    slistCommands5 = slistCommands;
    iErrors = iProcComplete = iComComplete = iCommand = 0;
    iComAll = slistCommands.count();
    iProcAll = iComAll < iNumProc ? iComAll : iNumProc;

    prbTotal->setValue(0);
    prbTotal->setFormat("0/" + QString::number(iComAll) + "     %p%");

    QVBoxLayout *vblArea = new QVBoxLayout(wgtArea);
    vblArea->addStretch();
    for (int i = 0; i < iProcAll; ++i)
    {
        QProgressBar *prb = new QProgressBar(wgtArea);
        prb->setAlignment(Qt::AlignHCenter);
        vectPrBars.append(prb);
        vectProgress.append(0);
        listLastOut.append(0);
        const QString strNum = QString::number(i+1);
        QGroupBox *gb = new QGroupBox(tr("Process:") + ' ' + strNum, wgtArea);
        (new QVBoxLayout(gb))->addWidget(prb);
        vblArea->addWidget(gb);
        vblArea->addStretch();

        QProcess *proc = new QProcess(prb);
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

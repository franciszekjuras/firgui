#ifndef BOXUPDATE_H
#define BOXUPDATE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QProcess>
#include "cautoupdatergithub.h"

class BoxUpdate : public QWidget
{
    Q_OBJECT
public:
    explicit BoxUpdate(QWidget *parent = nullptr);

private:
    QLabel* updateLabel;
    QPushButton* downloadButton;
    QPushButton* updateFailedMoreButton;
    QLabel* downloadLabel;
    QProgressBar* downloadProgress;

    CAutoUpdaterGithub* updater;
    void onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onUpdateDownloadFinished();
    void onUpdateError(QString errorMessage);
    void onDownloadUpdate();
    void onUpdateFailedMore();
    void onArchiveUnpacked(int exitCode, QProcess::ExitStatus exitStatus);
    bool prepareEnvironment();
    bool allowPreReleases;
    CAutoUpdaterGithub::VersionEntry proposedUpdate;

signals:

public slots:
};

#endif // BOXUPDATE_H

#pragma once

#include <QFile>
#include <QNetworkAccessManager>
#include <QString>

#include <functional>
#include <vector>

#if defined _WIN32
#define UPDATE_FILE_EXTENSION QLatin1String("-win32.zip")
#elif defined __APPLE__
#define UPDATE_FILE_EXTENSION QLatin1String("-osx.zip")
#else
#define UPDATE_FILE_EXTENSION QLatin1String("-linux.zip")
#endif

namespace UpdateInstaller {

bool install(const QString& downloadedUpdateFilePath);

}

class CAutoUpdaterGithub : public QObject
{
    Q_OBJECT
public:
	struct VersionEntry {
		QString versionString;
		QString versionChanges;
		QString versionUpdateUrl;
	};

	using ChangeLog = std::vector<VersionEntry>;

signals:
    void updateAvailable(ChangeLog changelog);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void updateDownloadFinished();
    void updateError(QString errorMessage);

public:
	// If the string comparison functior is not supplied, case-insensitive natural sorting is used (using QCollator)
	CAutoUpdaterGithub(const QString& githubRepositoryAddress,
					   const QString& currentVersionString,
					   const std::function<bool (const QString&, const QString&)>& versionStringComparatorLessThan = std::function<bool (const QString&, const QString&)>());

	CAutoUpdaterGithub& operator=(const CAutoUpdaterGithub& other) = delete;

	void checkForUpdates();
    void downloadUpdate(const QString& updateUrl, const QString& downloadDir);

private:
	void updateCheckRequestFinished();
    void updateDownloaded();
	void onNewDataDownloaded();

private:
	QFile _downloadedBinaryFile;
	const QString _updatePageAddress;
	const QString _currentVersionString;
	const std::function<bool (const QString&, const QString&)> _lessThanVersionStringComparator;

	QNetworkAccessManager _networkManager;
};


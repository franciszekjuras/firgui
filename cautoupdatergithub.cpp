#include "cautoupdatergithub.h"

#include <QCollator>
#include <QCoreApplication>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProcess>
#include <QDebug>
#include <assert.h>

static const auto naturalSortQstringComparator = [](const QString& l, const QString& r) {
	static QCollator collator;
	collator.setNumericMode(true);
	collator.setCaseSensitivity(Qt::CaseInsensitive);
	return collator.compare(l, r) == -1;
};

CAutoUpdaterGithub::CAutoUpdaterGithub(const QString& githubRepositoryAddress, const QString& currentVersionString, const std::function<bool (const QString&, const QString&)>& versionStringComparatorLessThan) :
	_updatePageAddress(githubRepositoryAddress + "/releases/"),
	_currentVersionString(currentVersionString),
	_lessThanVersionStringComparator(versionStringComparatorLessThan ? versionStringComparatorLessThan : naturalSortQstringComparator)
{
	assert(githubRepositoryAddress.contains("https://github.com/"));
	assert(!currentVersionString.isEmpty());
}

void CAutoUpdaterGithub::checkForUpdates()
{
	QNetworkReply * reply = _networkManager.get(QNetworkRequest(QUrl(_updatePageAddress)));
	if (!reply)
	{
        updateError("Network request rejected.");
		return;
	}

	connect(reply, &QNetworkReply::finished, this, &CAutoUpdaterGithub::updateCheckRequestFinished, Qt::UniqueConnection);
}

void CAutoUpdaterGithub::downloadUpdate(const QString& updateUrl, const QString &downloadDir)
{
	assert(!_downloadedBinaryFile.isOpen());
    qDebug() << downloadDir + "appupdate" + UPDATE_FILE_EXTENSION;
    _downloadedBinaryFile.setFileName(downloadDir + "appupdate" + UPDATE_FILE_EXTENSION);
	if (!_downloadedBinaryFile.open(QFile::WriteOnly))
	{
        updateError("Failed to open temporary file " + _downloadedBinaryFile.fileName());
		return;
	}

	QNetworkRequest request((QUrl(updateUrl)));
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration()); // HTTPS
	request.setMaximumRedirectsAllowed(5);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	QNetworkReply * reply = _networkManager.get(request);
	if (!reply)
	{
        updateError("Network request rejected.");
		return;
	}

	connect(reply, &QNetworkReply::readyRead, this, &CAutoUpdaterGithub::onNewDataDownloaded);
    connect(reply, &QNetworkReply::downloadProgress, this, &CAutoUpdaterGithub::downloadProgress);
	connect(reply, &QNetworkReply::finished, this, &CAutoUpdaterGithub::updateDownloaded, Qt::UniqueConnection);
}

static QString match(const QString& pattern, const QString& text, int from, int& end)
{
	end = -1;

	const auto delimiters = pattern.split('*');
	if (delimiters.size() != 2)
	{
		Q_ASSERT_X(delimiters.size() != 2, __FUNCTION__, "Invalid pattern");
		return {};
	}

	const int leftDelimiterStart = text.indexOf(delimiters[0], from);
	if (leftDelimiterStart < 0)
		return {};

	const int rightDelimiterStart = text.indexOf(delimiters[1], leftDelimiterStart + delimiters[0].length());
	if (rightDelimiterStart < 0)
		return {};

	const int resultLength = rightDelimiterStart - leftDelimiterStart - delimiters[0].length();
	if (resultLength <= 0)
		return {};

	end = rightDelimiterStart + delimiters[1].length();
	return text.mid(leftDelimiterStart + delimiters[0].length(), resultLength);
}

void CAutoUpdaterGithub::updateCheckRequestFinished()
{
	auto reply = qobject_cast<QNetworkReply *>(sender());
	if (!reply)
		return;

	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
	{
        updateError(reply->errorString());

		return;
	}

	if (reply->bytesAvailable() <= 0)
	{
        updateError("No data downloaded.");
		return;
	}

	ChangeLog changelog;
	const QString changelogPattern = QStringLiteral("<div class=\"markdown-body\">\n*</div>");
	const QString versionPattern = QStringLiteral("/releases/tag/*\">");
	const QString releaseUrlPattern = QStringLiteral("<a href=\"*\"");

	const auto releases = QString(reply->readAll()).split("release-header");
	// Skipping the 0 item because anything before the first "release-header" is not a release
	for (int releaseIndex = 1, numItems = releases.size(); releaseIndex < numItems; ++releaseIndex)
	{
		const QString& releaseText = releases[releaseIndex];

		int offset = 0;
		QString updateVersion = match(versionPattern, releaseText, offset, offset);
		if (updateVersion.startsWith(QStringLiteral(".v")))
			updateVersion.remove(0, 2);
		else if (updateVersion.startsWith('v'))
			updateVersion.remove(0, 1);

		if (!naturalSortQstringComparator(_currentVersionString, updateVersion))
			continue; // version <= _currentVersionString, skipping

		const QString updateChanges = match(changelogPattern, releaseText, offset, offset);

		QString url;
		offset = 0; // Gotta start scanning from the beginning again, since the release URL could be before the release description
		while (offset != -1)
		{
			const QString newUrl = match(releaseUrlPattern, releaseText, offset, offset);
			if (newUrl.endsWith(UPDATE_FILE_EXTENSION))
			{
				Q_ASSERT_X(url.isEmpty(), __FUNCTION__,"More than one suitable update URL found");
				url = newUrl;
			}
		}

		if (!url.isEmpty())
			changelog.push_back({ updateVersion, updateChanges, "https://github.com" + url });
	}

    updateAvailable(changelog);
}

void CAutoUpdaterGithub::updateDownloaded()
{
	_downloadedBinaryFile.close();

	auto reply = qobject_cast<QNetworkReply *>(sender());
	if (!reply)
		return;

	reply->deleteLater();

	if (reply->error() != QNetworkReply::NoError)
	{
        updateError(reply->errorString());

		return;
	}

    updateDownloadFinished();

    if (!UpdateInstaller::install(_downloadedBinaryFile.fileName()))
        updateError("Failed to launch the downloaded update.");
}

void CAutoUpdaterGithub::onNewDataDownloaded()
{
	auto reply = qobject_cast<QNetworkReply*>(sender());
	if (!reply)
		return;

	_downloadedBinaryFile.write(reply->readAll());
}


bool UpdateInstaller::install(const QString& downloadedUpdateFilePath)
{
    qDebug() << "Installing" << downloadedUpdateFilePath;
    return true;
    //return QProcess::startDetached('\"' + downloadedUpdateFilePath + '\"');
}


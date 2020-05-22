#ifndef QTMATERIALLABEL_H
#define QTMATERIALLABEL_H

#include <QtWidgets/QLabel>
#include <QFont>
#include <QColor>
#include <QString>

class QtMaterialLabel : public QLabel
{
    Q_OBJECT

public:
    QtMaterialLabel(const QString &text, QWidget *parent = nullptr);

    void setUseThemeColors(bool value);
    bool useThemeColors() const;

    void setUseThemeFont(bool value);
    bool useThemeFont() const;

    void setMainFont(const QFont &font);

    void setTextColor(const QColor &color);

    void setThemeFont(const QString &font);

    void setThemeColor(const QString &color);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    void updateTypeset();
    void updateColorset();
    void checkThemeChange();
    void setupTheme();

    int _themeIdx;
    bool _useThemeColors;
    bool _useThemeFont;
    QString _themeFont;
    QString _themeColor;
    QFont _mainFont;
    QColor _textColor;

private:
    Q_DISABLE_COPY(QtMaterialLabel)
};

#endif // QTMATERIALLABEL_H

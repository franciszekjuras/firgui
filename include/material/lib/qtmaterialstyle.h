#ifndef QTMATERIALSTYLE_H
#define QTMATERIALSTYLE_H

#include <QtWidgets/QCommonStyle>
//#include /*"lib/*/"qtmaterialstyle_p.h"
#include /*"lib/*/"qtmaterialtheme.h"

#define MATERIAL_DISABLE_THEME_COLORS \
    if (d->useThemeColors == true) { d->useThemeColors = false; }

class QtMaterialStylePrivate;
class QtMaterialTheme;

class QtMaterialStyle : public QCommonStyle
{
    Q_OBJECT

public:    
    ~QtMaterialStyle();
    inline static QtMaterialStyle &instance();
    inline static QColor transparentized(QColor clr, double alpha_mult);

    void setTheme(QtMaterialTheme *theme);
    int themeIdx();
    QFont themeFont(const QString &key) const;
    QColor themeColor(const QString &key) const;
    QColor themeColor(const QString &key, double alpha_mult) const;

protected:
    const QScopedPointer<QtMaterialStylePrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(QtMaterialStyle)

    QtMaterialStyle();

    QtMaterialStyle(QtMaterialStyle const &);
    void operator=(QtMaterialStyle const &);
};

inline QtMaterialStyle &QtMaterialStyle::instance()
{
    static QtMaterialStyle instance;
    return instance;
}

inline QColor QtMaterialStyle::transparentized(QColor clr, double alpha_mult){
    clr.setAlphaF(clr.alphaF()*alpha_mult);
    return  clr;
}

#endif // QTMATERIALSTYLE_H

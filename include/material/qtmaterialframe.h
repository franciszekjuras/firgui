#ifndef QTMATERIALFRAME_H
#define QTMATERIALFRAME_H

#include <QtWidgets/QWidget>

class QtMaterialFrame : public QWidget
{
    Q_OBJECT

public:
    explicit QtMaterialFrame(QWidget *parent = nullptr);

    void setUseThemeColors(bool value);
    bool useThemeColors() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setCornerRadius(qreal radius);
    qreal cornerRadius() const;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    bool _useThemeColors;
    qreal _cornerRadius;
    QColor _backgroundColor;

private:
    Q_DISABLE_COPY(QtMaterialFrame)
};

#endif // QTMATERIALFRAME_H

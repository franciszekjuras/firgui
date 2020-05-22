#ifndef QTMATERIALRAISEDBUTTON_H
#define QTMATERIALRAISEDBUTTON_H

#include "qtmaterialdropdown.h"

class QtMaterialComboPrivate;

class QtMaterialCombo : public QtMaterialDropdown
{
    Q_OBJECT

public:
    explicit QtMaterialCombo(QWidget *parent = nullptr);
    ~QtMaterialCombo();

    void setItems(const QStringList &data);

    virtual QSize sizeHint() const override;

signals:
    void selectedText(QString text);
    void selectedIndex(int index);

protected:
    QtMaterialCombo(QtMaterialComboPrivate &d, QWidget *parent = nullptr);

    virtual bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

    void itemSelected(bool dummy);

private:
    Q_DISABLE_COPY(QtMaterialCombo)
    Q_DECLARE_PRIVATE(QtMaterialCombo)
};

#endif // QTMATERIALRAISEDBUTTON_H

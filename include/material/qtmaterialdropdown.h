#ifndef QTMATERIALDROPDOWN_H
#define QTMATERIALDROPDOWN_H

#include <QtWidgets/QWidget>

class QtMaterialDropdownPrivate;

class QtMaterialDropdown : public QWidget
{
    Q_OBJECT

public:
    explicit QtMaterialDropdown(QWidget *parent = nullptr);
    ~QtMaterialDropdown();

signals:

protected slots:

protected:

    QtMaterialDropdown(QtMaterialDropdownPrivate &d, QWidget *parent = nullptr);

    void toggleDeffered();
//    bool event(QEvent *event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

    const QScopedPointer<QtMaterialDropdownPrivate> d_ptr;

private:
    Q_DISABLE_COPY(QtMaterialDropdown)
    Q_DECLARE_PRIVATE(QtMaterialDropdown)
};

#endif // QTMATERIALDROPDOWN_H

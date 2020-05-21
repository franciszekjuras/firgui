#ifndef QTMATERIALAUTOCOMPLETE_H
#define QTMATERIALAUTOCOMPLETE_H

#include <QColor>
#include "lib/qtmaterialtheme.h"
#include "qtmaterialflatbutton.h"

class QtMaterialAutoCompletePrivate;

class QtMaterialAutoComplete : public QtMaterialFlatButton
{
    Q_OBJECT

public:
    explicit QtMaterialAutoComplete(QWidget *parent = 0);
    ~QtMaterialAutoComplete();

    void setDataSource(const QStringList &data);

    //QColor foregroundColor() const override;

signals:
    void itemSelected(QString);

protected slots:
    void updateResults();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(QtMaterialAutoComplete)
    Q_DECLARE_PRIVATE(QtMaterialAutoComplete)
};

#endif // QTMATERIALAUTOCOMPLETE_H

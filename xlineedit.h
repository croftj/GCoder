#ifndef XLINEEDIT_H
#define XLINEEDIT_H

#include <QLineEdit>

class XLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    XLineEdit(QWidget *parent = 0) : QLineEdit(parent)
    {}

protected:
//    virtual void keyPressEvent(QKeyEvent *kevt);
};

#endif // XLINEEDIT_H

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <windows.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    void refreshTable();
private slots:
    void on_exitBtn_clicked();

    void on_driver_toggled(bool checked);

    void on_win32_toggled(bool checked);

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

private:
    Ui::Widget *ui;
    // 服务控制管理数据库句柄
    SC_HANDLE mHSCM;
    // 枚举的服务指针
    char* mBuf;
};

#endif // WIDGET_H

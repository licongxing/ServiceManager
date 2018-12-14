#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->serviceTable->setColumnWidth(0,100);
    ui->serviceTable->setColumnWidth(1,200);
    ui->serviceTable->setColumnWidth(2,100);
    // 打开SCM数据句柄
    mHSCM = OpenSCManagerA(nullptr,nullptr,SC_MANAGER_ALL_ACCESS);
    if( mHSCM == nullptr)
        qDebug() << "打开SCM失败";

    refreshTable();
}
void Widget::refreshTable(){
    int count = ui->serviceTable->rowCount();
    // 先清空表
    for(int i = 0; i < count;i++)
        ui->serviceTable->removeRow(0);
    // 清空之前申请的空间
    if(mBuf!=nullptr)
        delete[] mBuf;


    DWORD needLen,serviceNum,result,type;
    BOOL ret;

    // 刷新表格，枚举服务
    if(ui->win32->isChecked()){
        // win32服务
        type = SERVICE_WIN32;
    }else{
        // 驱动服务
        type = SERVICE_DRIVER;
    }

    // 第一次 获取需要的字节长度
    ret = EnumServicesStatusExA(mHSCM,SC_ENUM_PROCESS_INFO ,type
                                ,SERVICE_STATE_ALL,NULL,0,&needLen
                                ,&serviceNum,NULL,NULL);
    mBuf = new char[needLen]{0};
    if( ret == 0){
        // 下面这段switch代码要写，在qt5.9.7版本下 不写就是不行
        switch (GetLastError()) {
        case ERROR_ACCESS_DENIED:
            qDebug() << "ERROR_ACCESS_DENIED";
            break;
        case ERROR_MORE_DATA:
            qDebug() << "ERROR_MORE_DATA";
            break;
        case ERROR_INVALID_PARAMETER:
            qDebug() << "ERROR_INVALID_PARAMETER";
            break;
        case ERROR_INVALID_HANDLE:
            qDebug() << "ERROR_INVALID_HANDLE";
            break;
        case ERROR_INVALID_LEVEL:
            qDebug() << "ERROR_INVALID_LEVEL";
            break;
        case ERROR_SHUTDOWN_IN_PROGRESS:
            qDebug() << "ERROR_SHUTDOWN_IN_PROGRESS";
            break;
        default:
            qDebug() << "other error";
            break;
        }
        // EnumServicesStatusExA 枚举服务API
        ret = EnumServicesStatusExA(mHSCM,SC_ENUM_PROCESS_INFO ,type
                                    ,SERVICE_STATE_ALL,reinterpret_cast<LPBYTE>(mBuf),needLen,&needLen
                                    ,&serviceNum,&result,NULL);

        LPENUM_SERVICE_STATUS_PROCESSA p = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESSA>(mBuf);

        if(ret == 0){
            qDebug() << "获取服务失败";
        }else{
            qDebug() << "获取服务成功";
            // 往表格中添加数据
            for(DWORD i = 0; i < serviceNum; i++){
                ui->serviceTable->insertRow(i);
                ui->serviceTable->setItem(i,0,new QTableWidgetItem(QString(p[i].lpServiceName)));
                ui->serviceTable->setItem(i,1,new QTableWidgetItem(QString::fromLocal8Bit(p[i].lpDisplayName)));
                QTableWidgetItem* item;
                switch(p[i].ServiceStatusProcess.dwCurrentState){
                case SERVICE_PAUSED:
                    item = new QTableWidgetItem("暂停");
                    break;
                case SERVICE_STOPPED:
                    item = new QTableWidgetItem("停止");
                    break;
                case SERVICE_RUNNING:
                    item = new QTableWidgetItem("运行");
                    break;
                default:
                    item = new QTableWidgetItem("其他");
                    break;
                }
                ui->serviceTable->setItem(i,2,item);
            }
//            delete
        }
    }
}

Widget::~Widget()
{
    delete ui;
    // 关闭SCM数据句柄
    if( mHSCM != nullptr)
        CloseServiceHandle(mHSCM);
}

void Widget::on_exitBtn_clicked()
{
    this->close();
}

void Widget::on_driver_toggled(bool checked)
{
    if(checked)
        refreshTable();
}

void Widget::on_win32_toggled(bool checked)
{
    if(checked)
        refreshTable();
}

void Widget::on_startBtn_clicked()
{
    // 启动服务
    int row = ui->serviceTable->currentRow();
    LPENUM_SERVICE_STATUS_PROCESSA p =  reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESSA>(mBuf);
    if(p[row].ServiceStatusProcess.dwCurrentState == SERVICE_RUNNING)
        return;

    const char* name = ui->serviceTable->item(row,0)->text().toStdString().c_str();
    SC_HANDLE service = OpenServiceA(mHSCM, name, SERVICE_ALL_ACCESS);
    if(service == nullptr){
        qDebug() << "打开服务失败";
        return;
    }

    // StartServiceA启动服务API
    BOOL ret = StartServiceA(service,0,nullptr);
    if( ret ){
        qDebug() << "启动服务成功";
        ui->serviceTable->setItem(row,2,new QTableWidgetItem("运行"));
        p[row].ServiceStatusProcess.dwCurrentState = SERVICE_RUNNING;
    }else{
        qDebug() << "启动服务失败";
    }
}

void Widget::on_stopBtn_clicked()
{
    // 停止服务
    int row = ui->serviceTable->currentRow();
    LPENUM_SERVICE_STATUS_PROCESSA p =  reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESSA>(mBuf);
    if(p[row].ServiceStatusProcess.dwCurrentState == SERVICE_STOPPED)
        return;

    const char* name = ui->serviceTable->item(row,0)->text().toStdString().c_str();
    if( name == nullptr)
        return;

    SC_HANDLE service = OpenServiceA(mHSCM, name, SERVICE_ALL_ACCESS);
    if(service == nullptr){
        qDebug() << "打开服务失败";
        return;
    }

    SERVICE_STATUS status;
    // StartServiceA停止服务API
    BOOL ret = ControlService(service,SERVICE_CONTROL_STOP,&status);
    if( ret ){
        qDebug() << "停止服务成功";
        ui->serviceTable->setItem(row,2,new QTableWidgetItem("停止"));
        p[row].ServiceStatusProcess.dwCurrentState = SERVICE_STOPPED;
    }else{
        qDebug() << "停止服务失败";
    }
}

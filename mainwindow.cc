#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QVariantList>
#include <QJsonObject>
#include <QJsonArray>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->listWidget,
                     &QListWidget::itemActivated,
                     [=](QListWidgetItem* item) {
        if (item != nullptr)
            emit languageRequested(item->data(Qt::UserRole + 1).toString());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setLanguages(const QHash<QString, QString>& languages)
{
    ui->listWidget->clear();

    for (auto key : languages.keys()) {
        auto item = new(std::nothrow) QListWidgetItem;
        Q_CHECK_PTR(item);

        item->setText(languages[key]);
        item->setData(Qt::UserRole + 1, key);

        ui->listWidget->addItem(item);
    }
}

void MainWindow::changeEvent(QEvent* ev)
{
    QMainWindow::changeEvent(ev);

    if (ev->type() != QEvent::LanguageChange)
        return;

    ui->retranslateUi(this);
}

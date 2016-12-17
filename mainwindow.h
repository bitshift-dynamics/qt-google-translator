#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QNetworkAccessManager;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void setLanguages(const QHash<QString, QString>& languages);

signals:
    void languageRequested(const QString& lang);

protected:
    void changeEvent(QEvent* ev);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* qnam = nullptr;

    void retranslateUi();
};

#endif // MAINWINDOW_H

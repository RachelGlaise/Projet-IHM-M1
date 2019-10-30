#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTranslator>
#include "ui_mainwindow.h"
#include <random>

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool  eventFilter(QObject *watched, QEvent *event);
private:
    bool typeView = false; //vue tab
    unsigned typeSave = 0;
    bool colorChanged = false;
    int xPixel;
    int yPixel;

    //QFrame *picture;
    QImage *pic;
    QTranslator m_translator; // contains the translations for this application.
    std::string m_currLang = "FR";// contains the currently loaded language.
    std::default_random_engine generator;

    void init_tab(int nb_lignes, int nb_colonnes, int min, int max);
    void init_picture(int nb_rows, int nb_columns);
    void loadTranslator();
    void colorDupInColumn(QColor color);
    void colorDupInTab(QColor color);
    void colorTabToPic();
    bool savePicture();
    void changeColor(QColor color);
    QColor hsv_to_rgb(float h, float s, float v);
    void initToolTip();

protected slots:
    //menu
    void openGenerate();
    void importFile();
    bool saveTab();
    void chooseAndSaveTypeBMP();
    void chooseAndSaveTypePNG();
    void openAPropos();
    void setEnLanguage();
    void setFrLanguage();

    void save();
    void swapLines();
    void swapColumns();
    void changeView();
    void changePreviewColor();
    void chooseColorFast();
    void chooseColorDial();
    void chooseRandomColor();
    void colorAutoRand();
    void colorAutoGradient();
    void colorAuto();
    void updatePic();
    void showCurrentCell(QTableWidgetItem*);
    void showCoordinatesStatusBar(QTableWidgetItem*);
    void showCurrentPixel();



};



#endif // MAINWINDOW_H

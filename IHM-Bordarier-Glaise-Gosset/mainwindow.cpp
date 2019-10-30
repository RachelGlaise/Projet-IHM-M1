#include "mainwindow.h"
#include <QDebug>
#include "generate.h"
#include <iostream>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <random>
#include <QColorDialog>
#include <QString>
#include <QEvent>
#include <QMouseEvent>
#include <vector>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    generator.seed(time(nullptr));
    loadTranslator();
    setupUi(this);
    stackedWidget->setCurrentIndex(0); //Force to be on tableView on start.

    //menu
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(importFile()));
    connect(actionGenerate, SIGNAL(triggered()), this, SLOT(openGenerate()));
    connect(actionExportTab, SIGNAL(triggered()), this, SLOT(saveTab()));
    connect(actionBMP, SIGNAL(triggered()), this, SLOT(chooseAndSaveTypeBMP()));
    connect(actionPNG, SIGNAL(triggered()), this, SLOT(chooseAndSaveTypePNG()));
    connect(actionFR, SIGNAL(triggered()), this, SLOT(setFrLanguage()));
    connect(actionEN, SIGNAL(triggered()), this, SLOT(setEnLanguage()));
    connect(actionA_propos, SIGNAL(triggered()), this, SLOT(openAPropos()));

    //mainWindow
    connect(btn_changeView, SIGNAL(clicked()), this, SLOT(changeView()));
    connect(btn_majPic, SIGNAL(clicked()), this, SLOT(updatePic()));
    connect(tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(showCurrentCell(QTableWidgetItem*)));
    //connect(btn_changeView, SIGNAL(clicked()), this, SLOT(resizeContent()));
    connect(tableWidget, SIGNAL(itemEntered(QTableWidgetItem*)), this, SLOT(showCoordinatesStatusBar(QTableWidgetItem*)));
    connect(btn_validFastColor, SIGNAL(clicked()), this, SLOT(chooseColorFast()));
    connect(btn_modifieColor, SIGNAL(clicked()), this, SLOT(chooseColorDial()));
    connect(btn_autoColor, SIGNAL(clicked()), this, SLOT(chooseRandomColor()));
    connect(btn_colorAuto, SIGNAL(clicked()), this, SLOT(colorAuto()));
    connect(btn_colorAutoRand, SIGNAL(clicked()), this, SLOT(colorAutoGradient()));
    connect(btn_swapCol, SIGNAL(clicked()), this, SLOT(swapColumns()));
    connect(btn_swapLine, SIGNAL(clicked()), this, SLOT(swapLines()));
    connect(btn_export, SIGNAL(clicked()), this, SLOT(save()));

    //color
    connect(sliderColorR, SIGNAL(valueChanged(int)), this, SLOT(changePreviewColor()));
    connect(sliderColorV, SIGNAL(valueChanged(int)), this, SLOT(changePreviewColor()));
    connect(sliderColorB, SIGNAL(valueChanged(int)), this, SLOT(changePreviewColor()));

    //event label picture (to choose a pixel)
    labelPicture->setMouseTracking(true);
    labelPicture->installEventFilter(this);

    //init preview color
    changePreviewColor();

    initToolTip();

}
/**
 * Attribute pixel with coordonnates on the click in the picture. (Problem of inaccuracy)
 * @brief MainWindow::eventFilter
 * @param watched
 * @param event
 * @return
 */
bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == labelPicture && event->type() == QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        qDebug() << QString("Mouse click (%1, %2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y());
        /************Warning!**********/
        /*
         * x,y coordinates on pictures are inversed compared to i,j coordinates in tableWidget.
         * This can be verified by activating the qDebug above show mouse click position inside
         * the label containing the picture.
        */
        /************End Warning*******/
        int taillePixelX = labelPicture->size().width()/tableWidget->columnCount();
        int taillePixelY = labelPicture->size().height()/tableWidget->rowCount();

        xPixel = (mouseEvent->pos().y())/taillePixelY +1; //tab begin with 1
        yPixel = (mouseEvent->pos().x())/taillePixelX +1;
        if (xPixel > tableWidget->columnCount()) xPixel = tableWidget->columnCount();
        if (yPixel > tableWidget->rowCount()) yPixel = tableWidget->rowCount();

        qDebug() << "Pixel courant : " << xPixel << " , " << yPixel;
        showCurrentPixel();
        return true;
    }
    return false;
}
/**
 * Initialize a table with data of generator
 * @brief MainWindow::init_tab
 * @param nb_rows
 * @param nb_columns
 * @param min
 * @param max
 */
void MainWindow::init_tab(int nb_rows, int nb_columns, int min, int max){
    tableWidget->setRowCount(nb_rows);
    tableWidget->setColumnCount(nb_columns);

    uniform_int_distribution<int> distribution(min, max);

    int value;

    for(int i = 0; i < nb_rows; ++i){
        for(int j = 0; j < nb_columns; ++j){
            value = distribution(generator);  // generates number in the range min..max
            QTableWidgetItem *item = new QTableWidgetItem;
            item->setData(Qt::DisplayRole, value);
            tableWidget->setItem(i, j, item);
        }
    }
}
/**
 * Create a picture with the same size that table and enable button related of picture
 * @brief MainWindow::init_picture
 * @param nb_rows
 * @param nb_columns
 */
void MainWindow::init_picture(int nb_rows, int nb_columns){
    pic = new QImage(nb_columns, nb_rows,  QImage::Format_RGB16 );
    qDebug() << __FUNCTION__ << " rows " << nb_rows << " col " << nb_columns;
    labelPicture->setPixmap(QPixmap::fromImage(*pic));
    qDebug() << " pic->size().width() " << pic->size().width() << " pic->size().height() " << pic->size().height();
    //*pic = pic->scaled(pic->size().width()*3, pic->size().height()*3);
    //qDebug() << " pic->size().width() " << pic->size().width() << " pic->size().height() " << pic->size().height();
    btn_changeView->setDisabled(false);
    btn_majPic->setDisabled(false);
}
/**
 * Load the file for translation
 * @brief MainWindow::loadTranslator
 */
void MainWindow::loadTranslator(){
    if(!m_translator.load(":/visutab_en.qm")){
        if(!m_translator.load("./visutab_en.qm")){
            QMessageBox::information(this, tr("Erreur"), tr("Fichier de traduction introuvable. Essayez de copier le fichier 'visutab_en.qm' du projet dans le dossier build que vous avez construit."));
        }
    }



}
/**
 * Color the picture from the table colored
 * @brief MainWindow::colorTabToPic
 */
void MainWindow::colorTabToPic(){
    pic = new QImage(tableWidget->columnCount(),tableWidget->rowCount(), QImage::Format_RGB16);
    for(int i = 0; i < pic->size().height(); ++i){
        for(int j = 0; j < pic->size().width(); ++j){
            QTableWidgetItem* item = tableWidget->item(i, j);
            QColor currentColor = item->backgroundColor();
            pic->setPixel(j, i, currentColor.rgb());
        }
    }
    *pic = pic->scaled(stackedWidget->width(), stackedWidget->height());
    labelPicture->setPixmap(QPixmap::fromImage(*pic));
    //delete pic;
}
/**
 * Save the picture in PNG (default) or BMP
 * @brief MainWindow::savePicture
 * @return
 */
bool MainWindow::savePicture(){
    qDebug() << __FUNCTION__ << "type : " << typeSave;
    QString fileName;
    if (typeSave == 1){ //BMP
        fileName = QFileDialog::getSaveFileName(this,
                tr("Sauvegarder votre image"), "image.bmp",
                tr("BMP (*.bmp);;PNG (*.png);;Tous les fichiers(*)"));
    } else { //par defaut PNG (0)
        fileName = QFileDialog::getSaveFileName(this,
                tr("Sauvegarder votre image"), "image.png",
                tr("PNG (*.png);;BMP (*.bmp);;Tous les fichiers(*)"));
    }
    QFile file(fileName);
    return pic->save(fileName);
}
/**
 * Change the color of all item with the same value (in all the table or in the column)
 * @brief MainWindow::changeColor
 * @param color
 */
void MainWindow::changeColor(QColor color){
    colorChanged = true;
    if(only_column->isChecked())
        colorDupInColumn(color);
    else if (entire_tab->isChecked())
        colorDupInTab(color);
    else QMessageBox::information(this, tr("Erreur"), tr("Une erreur est survenue."));
    /*
    QTableWidgetItem* itemColumn;
    for(QTableWidgetItem* item : tableWidget->selectedItems()){
        item->setBackgroundColor(color);
        if(only_column->isChecked()){
            int j = tableWidget->column(item);
            for(int i = 0; i < tableWidget->rowCount(); i++){
                itemColumn = tableWidget->item(i, j);
                if(itemColumn->text() == item->text())
                    itemColumn->setBackgroundColor(color);
            }
        } else if(entire_tab->isChecked()){
            QList<QTableWidgetItem *> items = tableWidget->findItems(item->text(), Qt::MatchExactly);
            for(QTableWidgetItem *item : items)
                item->setBackgroundColor(color);
        } else QMessageBox::information(this, tr("Erreur"), tr("Une erreur est survenue."));

        item->setSelected(false);
    }*/
}
/**
 * Change color in the column
 * @brief MainWindow::colorDupInColumn
 * @param color
 */
void MainWindow::colorDupInColumn(QColor color){
    QTableWidgetItem* itemColumn;
    for(QTableWidgetItem* item : tableWidget->selectedItems()){
        item->setBackgroundColor(color);
        int j = tableWidget->column(item);
        for(int i = 0; i < tableWidget->rowCount(); i++){
            itemColumn = tableWidget->item(i, j);
            if(itemColumn->text() == item->text())
                itemColumn->setBackgroundColor(color);
        }
        item->setSelected(false);
    }
}
/**
 * Change color in all the table
 * @brief MainWindow::colorDupInTab
 * @param color
 */
void MainWindow::colorDupInTab(QColor color){
    for(QTableWidgetItem* item : tableWidget->selectedItems()){
        item->setBackgroundColor(color);
        QList<QTableWidgetItem *> items = tableWidget->findItems(item->text(), Qt::MatchExactly);
        for(QTableWidgetItem *dup : items){
            //changement couleur texte
            int red = color.red();
            int green = color.green();
            int blue = color.blue();

            if( (0.2125*red) + (0.7154*green) + (0.0721*blue) <= 128)
                dup->setForeground(Qt::white);
            else dup->setForeground(Qt::black);
            dup->setBackgroundColor(color);
        }
        item->setSelected(false);
    }
}
/**
 * Convert color hsv in rgb
 * @brief MainWindow::hsv_to_rgb
 * @param h
 * @param s
 * @param v
 * @return rgb color in QColor
 */
QColor MainWindow::hsv_to_rgb(float h, float s, float v){

  int h_i = static_cast<int>(h*6);
  float f = h*6 - h_i;
  float p = v * (1 - s);
  float q = v * (1 - f*s);
  float t = v * (1 - (1 - f) * s);
  float r, g, b;
  if (h_i == 0){
      r = v;
      g = t;
      b = p;
  } else if (h_i == 1) {
      r = q;
      g = v;
      b = p;
  }else if (h_i == 2) {
      r = p;
      g = v;
      b = t;
  }else if (h_i == 3) {
      r = p;
      g = q;
      b = v;
  }else if (h_i == 4) {
      r = t;
      g = p;
      b = v;
  }else if (h_i == 5) {
      r = v;
      g = p;
      b = q;
  } else {
      r = 0;
      g = 0;
      b = 0;
  }
  int red = static_cast<int>(r*256);
  int green = static_cast<int>(g*256);
  int blue = static_cast<int>(b*256);

  return QColor(red, green, blue);
}
/**
 * Give the text for the tool tip
 * @brief MainWindow::initToolTip
 */
void MainWindow::initToolTip(){
    btn_changeView->setToolTip(tr("Bascule entre les vues tableau et image."));
    btn_majPic->setToolTip(tr("Met à jour l'image."));

    locationCurrent_num->setToolTip(tr("Ligne, Colonne"));
    valCurrent_num->setToolTip(tr("La valeur sélectionné."));

    label->setToolTip(tr("Rouge."));
    label_2->setToolTip(tr("Bleu."));
    label_3->setToolTip(tr("Vert."));

    sliderColorR->setToolTip(tr("Valeur du rouge."));
    sliderColorB->setToolTip(tr("Valeur du bleu."));
    sliderColorV->setToolTip(tr("Valeur du vert."));

    spinBox_R->setToolTip(tr("Valeur du rouge."));
    spinBox_B->setToolTip(tr("Valeur du bleu."));
    spinBox_V->setToolTip(tr("Valeur du vert."));

    entire_tab->setToolTip(tr("Colorie les valeurs identiques du tableau."));
    only_column->setToolTip(tr("Colorie les valeurs identiques de la colonne."));

    frame_color->setToolTip(tr("Couleur courante."));
    btn_modifieColor->setToolTip(tr("Ouvre une fenêtre pour un choix plus poussé de la couleur."));
    btn_validFastColor->setToolTip(tr("Valide la couleur courante."));
    btn_autoColor->setToolTip(tr("Choisi aléatoirement une couleur."));
    btn_colorAuto->setToolTip(tr("Colorie toutes les valeurs du tableau avec un dégradé de gris."));
    btn_colorAutoRand->setToolTip(tr("Colorie toutes les valeurs non coloriées du tableau aléatoirement (une couleur par valeur)."));

    btn_swapCol->setToolTip(tr("Interverti 2 colonnes."));
    btn_swapLine->setToolTip(tr("Interverti 2 lignes."));

    swapCol1->setToolTip(tr("Première colonne."));
    swapCol2->setToolTip(tr("Deuxième colonne."));

    swapLine1->setToolTip(tr("Première ligne."));
    swapLine2->setToolTip(tr("Deuxième ligne."));

    btn_export->setToolTip(tr("Exporte le tableau ou l'image courant(e)"));
}
/*
 *
 *  SLOTS
 *
 */
/*
 *  MENU
 */
/**
 * Open the dialog box Generate to generate a new table
 * @brief MainWindow::openGenerate
 */
void MainWindow::openGenerate () {
    qDebug() << "ouverture de 'générer'";
    Generate *gen = new Generate(this);
    while(true){
        int res = gen->exec();
        if(res != QDialog::Accepted) return;

        int nb_rows = gen->getNb_rows();
        int nb_columns = gen->getNb_columns();
        int min = gen->getMin();
        int max = gen->getMax();

        if(max >= min){
            init_tab(nb_rows, nb_columns, min, max);
            init_picture(nb_rows, nb_columns);
            break;
        }
        else QMessageBox::information(this, tr("Erreur de valeurs"), tr("Le max doit être superieur ou egal à min."));
    }
}
/**
 * Import a file csv in the app
 * @brief MainWindow::importFile
 */
void MainWindow::importFile(){
    QString fileName = QFileDialog::getOpenFileName(this,
              tr("Ouvrir fichier"), "",
              tr("Comma Semicolon Value (*.csv);;"
                 "Tous les fichiers(*)"));
       if (fileName.isEmpty())
               return;
       else {
           QStringList rowOfData;
           QStringList rowData;
           QString data;
           QFile file(fileName);
           //statusBar()->showMessage(tr("Trying to open ") + fileName + ".",2000);
           if (!file.open(QIODevice::ReadOnly)) {
              QMessageBox::information(this, tr("Impossible d'ouvrir le fichier"),
                  file.errorString());
              return;
           }
           QMainWindow::statusBar()->showMessage(fileName + tr(" ouvert."));
           data = file.readAll();
           rowOfData = data.split("\r\n");
           tableWidget->setRowCount(rowOfData.size());

           for (int x = 0; x < rowOfData.size(); x++)
           {

               rowData = rowOfData[x].split(";");
               if(rowData.isEmpty()) continue;
               if(tableWidget->columnCount() < rowData.size())
                   tableWidget->setColumnCount(rowData.size());

               for (int y = 0; y < rowData.size(); y++)
               {

                   tableWidget->setItem(x,y,new QTableWidgetItem(rowData[y]));
               }
           }
           file.close();

       }
}
/**
 * Save the table in csv
 * @brief MainWindow::saveTab
 * @return
 */
bool MainWindow::saveTab(){
    // Création d'un objet QFile
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Sauvegarder votre tableau"), "tab.csv",
            tr("Comma Semicolon Value (*.csv);;Tous les fichiers(*)"));

    QFile file(fileName);

    // On ouvre notre fichier en écriture seule et on vérifie l'ouverture
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    // Création d'un objet QTextStream à partir de notre objet QFile
    QTextStream flux(&file);
    // On choisit le codec correspondant au jeu de caractères que l'on souhaite ; ici, UTF-8
    flux.setCodec("UTF-8");
    // Écriture des différentes lignes dans le fichier
    for(int i = 0; i < tableWidget->rowCount(); ++i){
        for(int j = 0; j < tableWidget->columnCount(); ++j){
            flux << tableWidget->item(i, j)->text();
            if(j < tableWidget->columnCount() - 1)  flux << ";";
        }
        if(i <tableWidget->rowCount() - 1)  flux << endl;
    }
    QMainWindow::statusBar()->showMessage(fileName + tr(" sauvegardé."));

    return true;
}
/**
 * Determine the type of file to save in BMP
 * @brief MainWindow::chooseAndSaveTypeBMP
 */
void MainWindow::chooseAndSaveTypeBMP(){
    typeSave = 1;
    savePicture();
}
/**
 * Determine the type of file to save in PNG
 * @brief MainWindow::chooseAndSaveTypePNG
 */
void MainWindow::chooseAndSaveTypePNG(){
    typeSave = 0;
    savePicture();
}
/**
 * Open the message with the project's description
 * @brief MainWindow::openAPropos
 */
void MainWindow::openAPropos(){
    QMessageBox::information(this, tr("A propos..."),
                             tr("VisualTab - Application permettant la représentation de données tabulaire en image.\n"
                                "Elle a été créé par Olivier Bordarier, Rachel Glaise et Yannick Gosset dans le cadre d'un projet d'IHM au cours du Master 1 d'Informatique de Luminy (2018/2019)"));
}
/**
 * Set the language to english and retranslate the ui.
 * @brief MainWindow::setEnLanguage
 */
void MainWindow::setEnLanguage(){
    qDebug() << __FUNCTION__;
    if(m_currLang == "EN") return;
    m_currLang = "EN";
    QApplication::installTranslator(&m_translator);
    retranslateUi(this);
}
/**
 * Set the language to french and retranslate the ui.
 * @brief MainWindow::setFrLanguage
 */
void MainWindow::setFrLanguage(){
    qDebug() << __FUNCTION__;
    if(m_currLang == "FR") return;
    m_currLang = "FR";
    QApplication::removeTranslator(&m_translator);
    retranslateUi(this);
}
/*
 * AUTRES
 */
/**
 * Determine the type of save (picture or table) with the view
 * @brief MainWindow::save
 */
void MainWindow::save() {
    qDebug() << __FUNCTION__;
    if (typeView == 0)
        saveTab();
    else{
        typeSave=0;  //format PNG par defaut
        savePicture();
    }
}
/**
 * Swap two lines of the table (indicate in the main window)
 * @brief MainWindow::swapLines
 */
void MainWindow::swapLines(){
    tableWidget->setSortingEnabled(false);

    int l1 = swapLine1->value()-1;
    int l2 = swapLine2->value()-1;
    if(l1 == l2)return;
    if(l1 >= tableWidget->rowCount() || l2 >= tableWidget->rowCount()){
        QMessageBox::information(this, tr("Erreur échange de lignes"),
                                 tr("Ligne(s) inexistante(s)."));

        return;
    }
    qDebug() << __FUNCTION__ << l1 << l2;
    QMainWindow::statusBar()->showMessage(tr("Echange des lignes ") + QString::number(l1) + tr(" et ") + QString::number(l2) + "...");
    for(int i = 0; i < tableWidget->columnCount(); ++i){
        QTableWidgetItem *temp = tableWidget->takeItem(l1,i);
        tableWidget->setItem(l1,i,tableWidget->takeItem(l2,i));
        tableWidget->setItem(l2,i,temp);
    }
    QMainWindow::statusBar()->showMessage(tr("Lignes échangées."));

    tableWidget->setSortingEnabled(true);
}
/**
 * Swap two columns of the table (indicate in the main window)
 * @brief MainWindow::swapColumns
 */
void MainWindow::swapColumns(){
    qDebug() << __FUNCTION__;
    qDebug() << "1 ere cellule " << tableWidget->item(0,0)->text();
    tableWidget->setSortingEnabled(false);
    int c1 = swapCol1->value()-1;
    int c2 = swapCol2->value()-1;
    if(c1 == c2)return;
    if(c1 >= tableWidget->columnCount() || c2 >= tableWidget->columnCount()){
        QMessageBox::information(this, tr("Erreur échange de colonnes"),
                                 tr("Colonne(s) inexistante(s)."));
        return;
    }
    QMainWindow::statusBar()->showMessage(tr("Echange des colonnes ") + QString::number(c1) + tr(" et ") + QString::number(c2) + "...");
    for(int i = 0; i < tableWidget->rowCount(); ++i){
        QTableWidgetItem *temp = tableWidget->takeItem(i,c1);
        tableWidget->setItem(i,c1,tableWidget->takeItem(i,c2));
        tableWidget->setItem(i,c2,temp);
    }
    QMainWindow::statusBar()->showMessage(tr("Colonnes échangées."));
    tableWidget->setSortingEnabled(true);
}
/**
 * Change the view (picture or table)
 * @brief MainWindow::changedView
 */
void MainWindow::changeView(){
    //qDebug() << __FUNCTION__ << "view = " << typeView;
    typeView = !typeView;
    if (typeView){
        QMainWindow::statusBar()->showMessage(tr("Affichage de la vue image..."));
        colorTabToPic();
        locationCurrent->setText(tr("Pixel courant :"));
    } else {
        QMainWindow::statusBar()->showMessage(tr("Affichage de la vue tabulaire..."));
        locationCurrent->setText(tr("Cellule courante :"));
        QMainWindow::statusBar()->showMessage(tr("Vue image affichée."));
    }
    stackedWidget->setCurrentIndex(typeView);
}
/**
 * Change the color of the preview to choose color fastly
 * @brief MainWindow::changePreviewColor
 */
void MainWindow::changePreviewColor(){
    QColor color;
    //color.setHsv(_hueSlider->value(),_saturationSlider->value(),_valueSlider->value());
    color.setRgb(sliderColorR->value(),sliderColorV->value(),sliderColorB->value());
    //qDebug() << "The color is " << color;
    QPalette pal;
    pal.setColor(QPalette::Window, color);
    frame_color->setPalette(pal);
}
/**
 * Change color with the fast choose
 * @brief MainWindow::chooseColorFast
 */
void MainWindow::chooseColorFast(){
    QColor color = QColor(spinBox_R->value(), spinBox_V->value(), spinBox_B->value());
    changeColor(color);
}
/**
 * Open a dialog to choose a color.
 * @brief MainWindow::chooseColorDial
 */
void MainWindow::chooseColorDial(){
    qDebug() << __FUNCTION__;
    QColor color = QColorDialog::getColor(Qt::darkGray, this, "Choisissez une couleur");
    if(color.isValid()){
        changeColor(color);
    }
    QMainWindow::statusBar()->showMessage(tr("Couleur choisi."));
}
/**
 * Choose a random color and color the item (and the others)
 * @brief MainWindow::chooseRandomColor
 */
void MainWindow::chooseRandomColor(){
    float golden_ratio_conjugate = 0.618033988749895f;
    uniform_int_distribution<int> distributionH(0, 99);
    float h = distributionH(generator);
    h = h/100;
    h += golden_ratio_conjugate;
    h = fmod(h, 1.0f);

    uniform_int_distribution<int> distributionU(20, 90);
    float u = distributionU(generator);
    u = u/100;

    uniform_int_distribution<int> distributionV(50, 90);
    float v = distributionV(generator);
    v = v/100;

    QColor color = hsv_to_rgb(h, u, v);
    changeColor(color);
    //QMainWindow::statusBar()->showMessage(tr("Couleur généré."));
}
/**
 * Color the table with random colors - NOT USE
 * @brief MainWindow::colorAutoRand
 */
void MainWindow::colorAutoRand(){
    QMainWindow::statusBar()->showMessage(tr("Coloration automatique en cours...\n"), 10000);
    for(int i = 0; i < tableWidget->rowCount(); ++i){
        for(int j = 0; j < tableWidget->columnCount(); ++j){
            if(!tableWidget->item(i, j)->backgroundColor().isValid()){
                tableWidget->item(i, j)->setSelected(true);
                chooseRandomColor();
                tableWidget->item(i, j)->setSelected(false);
            }
        }
    }
    QMainWindow::statusBar()->showMessage(tr("Coloration terminée"), 2000);
}
/**
 * Color the table with gradient colors
 * @brief MainWindow::colorAutoRand
 */
void MainWindow::colorAutoGradient(){
    QMainWindow::statusBar()->showMessage(tr("Coloration automatique en cours...\n"), 10000);

    vector<int> list;
    list.push_back(tableWidget->item(0, 0)->text().toInt());
    bool b = true;
    for(int i = 0; i < tableWidget->rowCount(); ++i){
        for(int j = 0; j < tableWidget->columnCount(); ++j){
            for(unsigned long long k = 0; k < list.size(); ++k){
                if(tableWidget->item(i, j)->text().toInt() == list.at(k)){
                    b = false;
                    break;
                }
            }
            if(b) list.push_back(tableWidget->item(i, j)->text().toInt());
            else b = true;
        }
    }

    sort(list.rbegin(), list.rend());

    int targetVal = list.back();
    float gap_hue = 300.0f/(list.size()-1);
    float gap_sat = 170.0f/(list.size()-1);

    float hue = 300;
    float sat = 85;
    QColor color;

    while(true){
        for(int i = 0; i < tableWidget->rowCount(); ++i){
            for(int j = 0; j < tableWidget->columnCount(); ++j){
                if(targetVal == tableWidget->item(i, j)->text().toInt()){
                    tableWidget->item(i, j)->setSelected(true);
                    color.setHsv(static_cast<int>(hue), static_cast<int>(sat), 255);
                    changeColor(color.convertTo(QColor::Hsv));
                    tableWidget->item(i, j)->setSelected(false);
                    hue -= gap_hue;
                    sat += gap_sat;
                    list.pop_back();
                    if(list.empty()){
                        QMainWindow::statusBar()->showMessage(tr("Coloration terminée"), 2000);
                        return;
                    }
                    targetVal = list.back();
                }
            }
        }
    }
}
/**
 * Color the table with a gray gradient
 * @brief MainWindow::colorAuto
 */
void MainWindow::colorAuto(){
    QMainWindow::statusBar()->showMessage(tr("Coloration automatique en cours...\n"), 10000);

    vector<int> list;
    list.push_back(tableWidget->item(0, 0)->text().toInt());
    bool b = true;
    for(int i = 0; i < tableWidget->rowCount(); ++i){
        for(int j = 0; j < tableWidget->columnCount(); ++j){
            for(unsigned long long k = 0; k < list.size(); ++k){
                if(tableWidget->item(i, j)->text().toInt() == list.at(k)){
                    b = false;
                    break;
                }
            }
            if(b) list.push_back(tableWidget->item(i, j)->text().toInt());
            else b = true;
        }
    }

    sort(list.rbegin(), list.rend());

    int targetVal = list.back();
    float gap = 255.0f/(list.size()-1);
    int val = 255;

    while(true){
        for(int i = 0; i < tableWidget->rowCount(); ++i){
            for(int j = 0; j < tableWidget->columnCount(); ++j){
                if(targetVal == tableWidget->item(i, j)->text().toInt()){
                    tableWidget->item(i, j)->setSelected(true);
                    QColor color = QColor(val, val, val);
                    changeColor(color);
                    tableWidget->item(i, j)->setSelected(false);
                    val -= gap;
                    if(val < 0) val = 0;
                    list.pop_back();
                    if(list.empty()){
                        QMainWindow::statusBar()->showMessage(tr("Coloration terminée"), 2000);
                        return;
                    }
                    targetVal = list.back();
                }
            }
        }
    }
}
/**
 * Update the picture
 * @brief MainWindow::updatePic
 */
void MainWindow::updatePic(){
    colorTabToPic();
}
/**
 * Show the cell's num selected in a textLabel
 * @brief MainWindow::showCurrentCell
 * @param item
 */
void MainWindow::showCurrentCell(QTableWidgetItem* item){
    qDebug() << __FUNCTION__;
    QString row = QString::number(item->row() + 1);
    QString column = QString::number(item->column() + 1);
    valCurrent_num->setText(tableWidget->item(item->row() , item->column() )->text());
    QMainWindow::statusBar()->showMessage(row + "," + column);
    locationCurrent_num->setText(row + "," + column);
}
/**
 * Show the last cell's num hovering in the status bar
 * @brief MainWindow::showCoordinatesStatusBar
 * @param item
 */
void MainWindow::showCoordinatesStatusBar(QTableWidgetItem* item){
    //qDebug() << __FUNCTION__;
    QString row = QString::number(item->row() + 1);
    QString column = QString::number(item->column() + 1);
    QMainWindow::statusBar()->showMessage(row + "," + column, 2000);
}
/**
 * Show the last pixel selected in a textLabel and the statusBar
 * @brief MainWindow::showCurrentPixel
 */
void MainWindow::showCurrentPixel(){
    QList<QTableWidgetItem*> list = tableWidget->selectedItems();
    for (QTableWidgetItem *item : list){
        item->setSelected(false);
    }
    locationCurrent_num->setText(QString::number(xPixel) + "," + QString::number(yPixel));
    valCurrent_num->setText(tableWidget->item(xPixel-1, yPixel-1)->text());
    tableWidget->item(xPixel-1, yPixel-1)->setSelected(true);
    QMainWindow::statusBar()->showMessage(QString::number(xPixel) + "," + QString::number(yPixel), 2000);

}

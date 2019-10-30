#include "generate.h"
#include <QDebug>

Generate::Generate(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi(this);

    connect(btn_generate, SIGNAL(clicked()), this, SLOT(recup_values()));
}

int Generate::getNb_rows() const
{
    return nbLine_value->value();
}

int Generate::getNb_columns() const
{
    return nbCol_value->value();
}

int Generate::getMin() const
{
    return valMin_value->value();
}

int Generate::getMax() const
{
    return valMax_value->value();
}
/**
 * Valid the generator
 * @brief Generate::recup_values
 */
void Generate::recup_values(){
    accept();
}

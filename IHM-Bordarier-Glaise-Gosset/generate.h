#ifndef GENERATE_H
#define GENERATE_H

#include "ui_generate.h"

class Generate : public QDialog, private Ui::Generate
{
    Q_OBJECT

public:
    explicit Generate(QWidget *parent = nullptr);

    int getNb_rows() const;
    int getNb_columns() const;
    int getMin() const;
    int getMax() const;

private:
    Ui::Generate *ui;

protected slots:
   void recup_values();
};

#endif // GENERATE_H

#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QList>
#include <QWidget>

class StackedWidget : public QWidget
    {
        Q_OBJECT

        public:
            StackedWidget(QWidget * = 0, Qt::WindowFlags = 0);

            void addWidget(QWidget *);
            int count();
            QWidget * currentWidget();

            void setAutoResize(bool yes)
                { auto_resize = yes; }

            QSize sizeHint();

        protected:
            void showCurrentWidget();

        private:
            bool auto_resize;
            int curr_index;
            QList<QWidget *> widgets;

        public slots:
            void setCurrentIndex(int);
    };

#endif // STACKEDWIDGET_H

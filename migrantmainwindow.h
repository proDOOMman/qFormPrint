/*

    Copyright (c) 2010 by Stanislav (proDOOMman) Kosolapov <prodoomman@shell.tor.hu>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#ifndef MIGRANTMAINWINDOW_H
#define MIGRANTMAINWINDOW_H

#include "ui_migrantmainwindow.h"
#include <QRadioButton>
#include <QDateEdit>
#include <QLineEdit>
#include <QtXml/QtXml>
#include <QtXml>
#include <QXmlStreamReader>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QRadioButton>
#include <QFileDialog>
#include <QEvent>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintPreviewWidget>
#include <QPrintDialog>
#include <QPicture>
#include <QStringList>
#include <QPushButton>

class MigrantMainWindow : public QMainWindow, private Ui::MigrantMainWindow
{
    Q_OBJECT

    enum FieldType{
        String = 0,
        Date = 1,
        Enum = 2,
        Group = 3
    };

    struct Char{
        QString name;
        QString trans;
        QRect rect;
        QWidget *widget;
        QGraphicsPixmapItem *pixmap;
        QString bukva;
    };

    struct Field{
        QString name;
        QString trans;
        FieldType type;
        QList<Char*> chars;
        QList<Field> fields;
        Field *group;
        QVariant data;
        QWidget *widget;
    };

    struct Document{
        QString name;
        QString file;
        QList<Field> fields;
        QGraphicsScene *scene;
        QWidget *widget;
        QGraphicsItemGroup *charsGroup;
        QList<Char> chars;
    };

public:
    explicit MigrantMainWindow(QWidget *parent = 0);

protected:
    QMap<QString,Field*> fields;
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *, QEvent *);
    QList<Document> m_docs;
    QString current_filename;
    Field *current_field;
    Field *current_group;
    Document *current_doc;
    bool isChanged;
private slots:
    void on_actionFit_width_triggered();
    void on_actionZoom_out_triggered();
    void on_actionZoom_in_triggered();
    void createGroupWidget(Field *);
    void createFieldWidget(Field *);
    void createDocumentWidget(Document *);
    void saveData();
    void saveDataAs();
    void saveFields(Field *field, QXmlStreamWriter &w);
    void updatePixmaps();
    void updateFieldsPixs(Field *f, QGraphicsScene *scene);
    void setChanged(){isChanged = true;}
    void newData();
    void loadData();
    void printDocs(QPrinter *p);
    void printData();
    void printDobleSided();
    void printFront();
    void printBack();
    void printDocument(QPrinter *printer, int startPage, int endPage);
};

#endif // MIGRANTMAINWINDOW_H

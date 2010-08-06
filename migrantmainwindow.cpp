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

#include "migrantmainwindow.h"

#include <QDebug>
#include <QFormLayout>
#include <QFile>

MigrantMainWindow::MigrantMainWindow(QWidget *parent) :
    QMainWindow(parent){
    current_doc = 0;
    current_field = 0;
    current_group = 0;
    current_filename = "";

    setupUi(this);
    this->installEventFilter(this);
    fieldsBox->setLayout(new QFormLayout(this));
    this->showMaximized();

    actionNew->setIcon(QIcon::fromTheme("document-new",QIcon(":icons/document-new.png")));
    actionSave->setIcon(QIcon::fromTheme("document-save",QIcon(":icons/document-save.png")));
    actionOpen->setIcon(QIcon::fromTheme("document-open",QIcon(":icons/document-open.png")));
    menuPrint->setIcon(QIcon::fromTheme("document-print",QIcon(":icons/document-print.png")));
    actionPrintDual->setIcon(QIcon::fromTheme("document-print",QIcon(":icons/document-print.png")));
    actionPrint->setIcon(QIcon::fromTheme("document-print-preview",QIcon(":icons/document-print-preview.png")));
    actionSave_as->setIcon(QIcon::fromTheme("document-save-as",QIcon(":icons/document-save-as.png")));
    actionZoom_in->setIcon(QIcon::fromTheme("zoom-in",QIcon(":icons/zoom-in.png")));
    actionZoom_out->setIcon(QIcon::fromTheme("zoom-out",QIcon(":icons/zoom-out.png")));
    actionFit_width->setIcon(QIcon::fromTheme("zoom-fit-best",QIcon(":icons/zoom-fit-best.png")));
    actionAbout->setIcon(QIcon::fromTheme("help-about",QIcon(":icons/help-about.png")));
    actionExit->setIcon(QIcon::fromTheme("application-exit",QIcon(":icons/application-exit.png")));

    mainToolBar->addAction(actionNew);
    mainToolBar->addAction(actionSave);
    mainToolBar->addAction(actionSave_as);
    mainToolBar->addAction(actionOpen);
    mainToolBar->addAction(actionPrintDual);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionZoom_out);
    mainToolBar->addAction(actionZoom_in);
    mainToolBar->addAction(actionFit_width);

    connect(action_Panel,SIGNAL(triggered(bool)),mainToolBar,SLOT(setVisible(bool)));
    connect(mainToolBar,SIGNAL(visibilityChanged(bool)),action_Panel,SLOT(setChecked(bool))); //QT 4.7 needed!
    connect(tabWidget,SIGNAL(currentChanged(int)),this,SLOT(updatePixmaps()));
    connect(actionSave,SIGNAL(triggered()),this,SLOT(saveData()));
    connect(actionSave_as,SIGNAL(triggered()),this,SLOT(saveDataAs()));
    connect(actionNew,SIGNAL(triggered()),this,SLOT(newData()));
    connect(actionOpen,SIGNAL(triggered()),this,SLOT(loadData()));
    connect(actionPrint,SIGNAL(triggered()),this,SLOT(printData()));
    connect(actionPrintDual,SIGNAL(triggered()),this,SLOT(printDobleSided()));
    connect(actionPrint1,SIGNAL(triggered()),this,SLOT(printFront()));
    connect(actionPrint2,SIGNAL(triggered()),this,SLOT(printBack()));

    QMap<QString,Field*> groups;
    QStringList dirs;
    dirs << qApp->applicationDirPath()+QDir::separator()+"data"+QDir::separator()
#ifdef Q_OS_LINUX
            << "/usr/share/Migrant/data/"
#endif
            ;
    QFile f;
    foreach(QString dir, dirs)
        if(QFile::exists(dir+"data.xml"))
            f.setFileName(dir+"data.xml");
    if(!f.open(QIODevice::ReadOnly|QIODevice::Text))
        exit(3);
    QXmlStreamReader r(&f);
    while(!r.atEnd())
    {
        r.readNext();
        if(r.tokenType()==QXmlStreamReader::StartElement)
        {
            QString name = r.name().toString();
            if(name == "document")
            {
                Document doc;
                foreach(QString dir, dirs)
                    if(QFile::exists(dir+r.attributes().value("file").toString()))
                        doc.file = dir+r.attributes().value("file").toString();
                doc.name = r.attributes().value("name").toString();
                doc.fields = QList<Field>();
                m_docs.append(doc);
                current_doc = &m_docs.last();
                current_field = 0;
                current_group = 0;
                createDocumentWidget(current_doc);
            }
            else if(name == "group")
            {
                Field group;
                group.trans = r.attributes().value("trans").toString();
                group.type = Group;
                if(current_group)
                    group.group = current_group;
                else
                    group.group = 0;
                if(!groups.keys().contains(group.trans))
                {
                    emit createGroupWidget(&group);
                    groups[group.trans] = current_group;
                }
                else
                    group.widget = groups[group.trans]->widget;
                current_doc->fields.append(group);
                current_group = &current_doc->fields.last();
                current_field = 0;
            }
            else if(name == "field")
            {
                Field field;
                field.name = r.attributes().value("name").toString();
                field.trans = r.attributes().value("trans").toString();
                field.type = (FieldType)r.attributes().value("type").toString().toInt();
                if(current_group)
                {
                    field.group = current_group;
                    current_group->fields.append(field);
                    current_field = &current_group->fields.last();
                }
                else
                {
                    field.group = 0;
                    current_doc->fields.append(field);
                    current_field = &current_doc->fields.last();
                }
                if(fields.keys().contains(field.name))
                    current_field->widget = fields[field.name]->widget;
                else
                {
                    fields[field.name] = current_field;
                    emit createFieldWidget(current_field);
                }
            }
            else if(name == "char")
            {
                Char ch;
                ch.bukva="";
                ch.name = r.attributes().value("name").toString();
                ch.trans = r.attributes().value("trans").toString();
                ch.rect = QRect( r.attributes().value("x").toString().toInt(),
                                 r.attributes().value("y").toString().toInt(),
                                 r.attributes().value("w").toString().toInt(),
                                 r.attributes().value("h").toString().toInt() );
                QPixmap p(ch.rect.size());
                p.fill(Qt::transparent);
                ch.pixmap = current_doc->scene->addPixmap(p);
                ch.pixmap->setPos(ch.rect.topLeft());
                if(current_field)
                {
                    current_doc->chars.append(ch);
                    current_field->chars.append(&current_doc->chars.last());
                    if(current_field->type==Enum)
                    {
                        if(fields[current_field->name]==current_field)
                        {
                            QRadioButton *b = new QRadioButton(ch.trans,this);
                            connect(b,SIGNAL(toggled(bool)),this,SLOT(setChanged()));
                            current_field->widget->layout()->addWidget(b);
                            current_field->chars.last()->widget = b;
                        }
                        else
                        {
                            current_field->chars.last()->widget = 
                                    fields[current_field->name]->chars.at(current_field->chars.count()-1)->widget;
                        }
                    }
                }
            }
        }
        else if(r.tokenType()==QXmlStreamReader::EndElement)
        {
            QString name = r.name().toString();
            if(name == "group")
            {
                if(current_group)
                    current_group = current_group->group;
            }
            else if(name == "field")
            {
                if(current_field->type==String)
                {
                    QLineEdit *l = qobject_cast<QLineEdit*>(current_field->widget);
                    if(l->maxLength()==32767||l->maxLength()<current_field->chars.count())
                        l->setMaxLength(current_field->chars.count());
                }
                else if(current_field->type==Enum)
                    ((QRadioButton*)current_field->chars.first()->widget)->setChecked(true);
                current_field = 0;
            }
        }
    }
    isChanged = false;
}

void MigrantMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

bool MigrantMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type()==QEvent::Close && obj == this && isChanged)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, trUtf8("Закрыть документ"),
                                      trUtf8("Документ был изменен. Сохранить изменения или отклонить их?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch(reply)
        {
        case QMessageBox::Save:
            emit saveData();
        case QMessageBox::Discard:
            event->accept();
            break;
        default:
            event->ignore();
        }
        return true;
    }
    return QObject::event(event);
}

void MigrantMainWindow::printData()
{
    emit updatePixmaps();
    QWidget *previewWindow = new QWidget(this,Qt::Dialog);
    previewWindow->setWindowModality(Qt::ApplicationModal);
    previewWindow->setAttribute(Qt::WA_DeleteOnClose);
    previewWindow->setLayout(new QVBoxLayout(previewWindow));
    QPrintPreviewWidget *d = new QPrintPreviewWidget();
    QHBoxLayout *h = new QHBoxLayout(previewWindow);
    QPushButton *p1 = new QPushButton(trUtf8("Печать"),previewWindow);
    connect(p1,SIGNAL(released()),this,SLOT(printDobleSided()));
    QPushButton *p2 = new QPushButton(trUtf8("Увеличить"),previewWindow);
    connect(p2,SIGNAL(released()),d,SLOT(zoomIn()));
    QPushButton *p3 = new QPushButton(trUtf8("Уменьшить"),previewWindow);
    connect(p3,SIGNAL(released()),d,SLOT(zoomOut()));
    h->addWidget(p1);
    h->addWidget(p2);
    h->addWidget(p3);
    qobject_cast<QVBoxLayout*>(previewWindow->layout())->addLayout(h);
    previewWindow->layout()->addWidget(d);
    d->addAction(new QAction("asdasdasd",this));
    connect(d,SIGNAL(paintRequested(QPrinter*)),this,SLOT(printDocs(QPrinter*)));
    previewWindow->showMaximized();
}

void MigrantMainWindow::printDocs(QPrinter *printer)
{
    printDocument(printer,0,1);
}

void MigrantMainWindow::loadData()
{
    if(isChanged)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, trUtf8("Новый документ"),
                                      trUtf8("Текущий документ был изменен. Сохранить изменения или отклонить их?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch(reply)
        {
        case QMessageBox::Save:
            emit saveData();
        case QMessageBox::Discard:
            break;
        default:
            return;
        }
    }
    QString fname = QFileDialog::getOpenFileName(this,trUtf8("Открыть"),
                                                 QDir::homePath(),
                                                 trUtf8("XML файлы (*.xml)"));
    if(fname.isEmpty())
        return;
    QFile f(fname);
    if(!f.open(QIODevice::ReadOnly|QIODevice::Text))
        return;
    current_filename = fname;
    QXmlStreamReader r(&f);
    while(!r.atEnd())
    {
        r.readNext();
        if(r.tokenType()==QXmlStreamReader::StartElement)
        {
            QString name = r.name().toString();
            if(!name.compare("field",Qt::CaseInsensitive))
            {
                QString alias = r.attributes().value("alias").toString();
                QString value = r.attributes().value("value").toString();
                if(!fields.keys().contains(alias))
                    continue;
                Field *f = fields[alias];
                switch(f->type)
                {
                case String:
                    qobject_cast<QLineEdit*>(f->widget)->setText(value);
                    break;
                case Enum:
                    foreach(Char *ch, f->chars)
                        if(!ch->name.compare(value,Qt::CaseInsensitive))
                            qobject_cast<QRadioButton*>(ch->widget)->setChecked(true);
                    break;
                case Date:
                    qobject_cast<QDateEdit*>(f->widget)->setDate(QDate::fromString(value,Qt::ISODate));
                    break;
                default:
                    break;
                }
            }
        }
    }
    isChanged = false;
}

void MigrantMainWindow::newData()
{
    if(isChanged)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, trUtf8("Новый документ"),
                                      trUtf8("Текущий документ был изменен. Сохранить изменения или отклонить их?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch(reply)
        {
        case QMessageBox::Save:
            emit saveData();
        case QMessageBox::Discard:
            break;
        default:
            return;
        }
    }
    foreach(Field* f,fields)
    {
        switch(f->type)
        {
        case String:
            qobject_cast<QLineEdit*>(f->widget)->setText("");
            break;
        case Enum:
            qobject_cast<QRadioButton*>(f->chars.first()->widget)->setChecked(true);
            break;
        case Date:
            qobject_cast<QDateEdit*>(f->widget)->setDate(QDate::fromString("01012000","ddMMyyyy"));
            break;
        default:
            break;
        }
    }
    updatePixmaps();
    current_filename = "";
    isChanged = false;
}

void MigrantMainWindow::createDocumentWidget(Document *doc)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    QGraphicsView *w = new QGraphicsView(scene,this);
    w->setRenderHints(QPainter::Antialiasing|
                      QPainter::TextAntialiasing|
                      QPainter::SmoothPixmapTransform|
                      QPainter::HighQualityAntialiasing);
    scene->addPixmap(QPixmap(doc->file));
    doc->scene = scene;
    tabWidget->addTab(w,doc->name);
    doc->widget = fieldsBox;
    ((QFormLayout*)doc->widget->layout())->setRowWrapPolicy(QFormLayout::WrapLongRows);
    ((QFormLayout*)doc->widget->layout())->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
}

void MigrantMainWindow::createGroupWidget(Field *group)
{
    QLabel *g = new QLabel("<b>"+group->trans.replace("\\n","<br>")+"</b>",this);
    if(current_doc)
    {
        QFormLayout *l = qobject_cast<QFormLayout*>(current_doc->widget->layout());
        l->setWidget(l->rowCount()+1,QFormLayout::SpanningRole,g);
    }
    group->widget = g;
}

void MigrantMainWindow::createFieldWidget(Field *field)
{
    QFormLayout *l;
    l = qobject_cast<QFormLayout*>(current_doc->widget->layout());
    QWidget *w;
    switch(field->type)
    {
    case String:
        w = new QLineEdit(this);
        connect(w,SIGNAL(textChanged ( const QString &)),this,SLOT(setChanged()));
        break;
    case Date:
        w = new QDateEdit(this);
        w->setProperty("calendarPopup",true);
        connect(w,SIGNAL(dateChanged ( const QDate &)),this,SLOT(setChanged()));
        break;
    case Enum:
        w = new QWidget(this);
        w->setLayout(new QGridLayout(this));
        break;
    default:
        w = new QWidget(this);
        break;
    }
    field->widget = w;
    l->addRow(field->trans,w);
}

void MigrantMainWindow::saveDataAs()
{
    QString fname = QFileDialog::getSaveFileName(this,trUtf8("Сохранить"),
                                                 QDir::homePath()+QDir::separator()+
                                                 trUtf8("Документ-")+QDate::currentDate().toString("ddMMyyyy")+".xml",
                                                 trUtf8("XML файлы (*.xml)"));
    if(fname.isEmpty())
        return;
    current_filename = fname;
    emit saveData();
}

void MigrantMainWindow::saveData()
{
    if(current_filename.isEmpty())
    {
        emit saveDataAs();
        return;
    }
    QFile f(current_filename);
    f.open(QIODevice::WriteOnly|QIODevice::Text);
    QXmlStreamWriter w(&f);
    w.setAutoFormatting(true);
    w.writeStartDocument();
    w.writeComment("Writed by Chineese Counter v 0.1");
    w.writeStartElement("document");
    w.writeAttribute("timestamp",QDateTime::currentDateTime().toString(Qt::ISODate));
    w.writeAttribute("product",tr("Chineese Counter"));
    w.writeAttribute("productversion","1.0");
//    w.writeAttribute("vendor",);
    w.writeStartElement("report");
    w.writeAttribute("documentCode","1");
    w.writeAttribute("workplace","1");
    w.writeAttribute("date",QDate::currentDate().toString(Qt::ISODate));
    w.writeAttribute("UID","");
    foreach(Document doc, m_docs)
        for(QList<Field>::iterator it = doc.fields.begin(); it!=doc.fields.end(); it++)
            saveFields(&*it,w);
    w.writeEndDocument();
    isChanged = false;
}

void MigrantMainWindow::saveFields(Field *field, QXmlStreamWriter &w)
{
    switch(field->type)
    {
    case String:
        w.writeEmptyElement("field");
        w.writeAttribute("alias",field->name);
        w.writeAttribute("value",((QLineEdit*)field->widget)->text());
        break;
    case Date:
        w.writeEmptyElement("field");
        w.writeAttribute("alias",field->name);
        w.writeAttribute("value",((QDateEdit*)field->widget)->date().toString(Qt::ISODate));
        break;
    case Enum:
        w.writeEmptyElement("field");
        w.writeAttribute("alias",field->name);
        foreach(Char *ch, field->chars)
            if(((QRadioButton*)ch->widget)->isChecked())
                w.writeAttribute("value",ch->name);
        break;
    default:
        for(QList<Field>::iterator it = field->fields.begin(); it!=field->fields.end(); it++)
            saveFields(&*it,w);
    }
}

void MigrantMainWindow::updatePixmaps()
{
    foreach(Document doc, m_docs)
        for(QList<Field>::iterator it = doc.fields.begin(); it!=doc.fields.end(); it++)
            updateFieldsPixs(&*it,doc.scene);
}

void MigrantMainWindow::updateFieldsPixs(Field *field, QGraphicsScene *scene)
{
    QString text = 0;
    switch(field->type)
    {
    case String:
        text = ((QLineEdit*)field->widget)->text().toUpper();
        for(int i = 0; i<field->chars.count(); i++)
        {
            QPixmap pix(field->chars[i]->rect.size());
            pix.fill(Qt::transparent);
            field->chars[i]->bukva = "";
            if(text.length()>i&&pix.height()>0)
            {
                QPainter p(&pix);
                QFont f = p.font();
                f.setPixelSize(pix.height()*0.7);
                f.setFixedPitch(true);
                p.setFont(f);
                QTextOption to;
                to.setAlignment(Qt::AlignCenter);
                p.drawText(QRect(QPoint(0,0),pix.size()),text.at(i),to);
                field->chars[i]->bukva = text.at(i);
            }
            field->chars[i]->pixmap->setPixmap(pix);
        }
        break;
    case Date:
        text = ((QDateEdit*)field->widget)->date().toString("ddMMyyyy");
        for(int i = 0; i<field->chars.count(); i++)
        {
            QPixmap pix(field->chars[i]->rect.size());
            pix.fill(Qt::transparent);
            field->chars[i]->bukva = "";
            if(text.length()>i)
            {
                QPainter p(&pix);
                QFont f = p.font();
                f.setPixelSize(pix.height()*0.7);
                f.setFixedPitch(true);
                p.setFont(f);
                QTextOption to;
                to.setAlignment(Qt::AlignCenter);
                p.drawText(QRect(QPoint(0,0),pix.size()),text.at(i),to);
                field->chars[i]->bukva = QString(text.at(i));
            }
            field->chars[i]->pixmap->setPixmap(pix);
        }
        break;
    case Enum:
        for(int i = 0; i<field->chars.count(); i++)
        {
            QPixmap pix(field->chars[i]->rect.size());
            pix.fill(Qt::transparent);
            field->chars[i]->bukva = "";
            if(((QRadioButton*)field->chars[i]->widget)->isChecked())
            {
                QPainter p(&pix);
                QFont f = p.font();
                f.setPixelSize(pix.height()*0.7);
                f.setFixedPitch(true);
                p.setFont(f);
                QTextOption to;
                to.setAlignment(Qt::AlignCenter);
                p.drawText(QRect(QPoint(0,0),pix.size()),"X",to);
                field->chars[i]->bukva = "X";
            }
            field->chars[i]->pixmap->setPixmap(pix);
        }
        break;
    default:
        for(QList<Field>::iterator it = field->fields.begin(); it!=field->fields.end(); it++)
            updateFieldsPixs(&*it, scene);
    }
}

void MigrantMainWindow::on_actionZoom_in_triggered()
{
    if(tabWidget->currentIndex()>0)
    {
        QGraphicsView *w = (QGraphicsView*)tabWidget->currentWidget();
        w->scale(1.2,1.2);
    }
    else
        for(int i = 1; i<tabWidget->count(); i++)
        {
            QGraphicsView *w = (QGraphicsView*)tabWidget->widget(i);
            w->scale(1.2,1.2);
        }
}

void MigrantMainWindow::on_actionZoom_out_triggered()
{
    if(tabWidget->currentIndex()>0)
    {
        QGraphicsView *w = (QGraphicsView*)tabWidget->currentWidget();
        w->scale(0.8,0.8);
    }
    else
        for(int i = 1; i<tabWidget->count(); i++)
        {
            QGraphicsView *w = (QGraphicsView*)tabWidget->widget(i);
            w->scale(0.8,0.8);
        }
}

void MigrantMainWindow::on_actionFit_width_triggered()
{
    if(tabWidget->currentIndex()>0)
    {
        QGraphicsView *w = (QGraphicsView*)tabWidget->currentWidget();
        qreal r = w->viewport()->width()/qreal(m_docs[tabWidget->currentIndex()-1].scene->width());
        r -= 0.01;
        w->resetTransform();
        w->scale(r,r);
    }
    else
        for(int i = 1; i<tabWidget->count(); i++)
        {
            QGraphicsView *w = (QGraphicsView*)tabWidget->widget(i);
            qreal r = w->viewport()->width()/qreal(m_docs[i-1].scene->width());
            r -= 0.01;
            w->resetTransform();
            w->scale(r,r);
        }
}

void MigrantMainWindow::printDobleSided()
{
    updatePixmaps();
    QPrinter p(QPrinter::HighResolution);
    p.setDoubleSidedPrinting(true);
    QPrintDialog d(&p,this);
#ifndef Q_WS_X11
    qDebug() << "Only X11 supported now... sorry";
#endif
    d.setWindowTitle(trUtf8("Двусторонняя печать"));
    if(d.exec()!=QPrintDialog::Accepted)
        return;
    printDocument(&p, 0, 1);
}

void MigrantMainWindow::printFront()
{
    updatePixmaps();
    QPrinter p(QPrinter::HighResolution);
    QPrintDialog d(&p,this);
    d.setWindowTitle(trUtf8("Печать лицевой стороны"));
    if(d.exec()!=QPrintDialog::Accepted)
        return;
    printDocument(&p, 0, 0);
}

void MigrantMainWindow::printBack()
{
    updatePixmaps();
    QPrinter p(QPrinter::HighResolution);
    QPrintDialog d(&p,this);
    d.setWindowTitle(trUtf8("Печать оборотной стороны"));
    if(d.exec()!=QPrintDialog::Accepted)
        return;
    printDocument(&p, 1, 1);
}

void MigrantMainWindow::printDocument(QPrinter *printer, int startPage, int endPage)
{
    QPainter p;
    p.begin(printer);
    p.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform);
    for(int page = startPage; page<=endPage; page++)
    {
        QPixmap pix(m_docs[page].file);
        QSize size = pix.size();
        size.scale(printer->pageRect().size(),Qt::KeepAspectRatio);
        qDebug() << size;
        qreal multipler = pix.size().height()/(qreal)size.height();
        qDebug() << multipler;
        p.drawPixmap(QRect(QPoint(0,0),size),pix);
        for(QList<Char>::iterator ch = m_docs[page].chars.begin(); ch!=m_docs[page].chars.end(); ch++)
        {
            if(ch->bukva.isEmpty())
                continue;
            QFont f = p.font();
            f.setPixelSize(ch->rect.height()*0.7/multipler);
            f.setFixedPitch(true);
            p.setFont(f);
            QTextOption to;
            to.setAlignment(Qt::AlignCenter);
            QRect r(ch->rect.x()/multipler,
                    ch->rect.y()/multipler,
                    ch->rect.width()/multipler,
                    ch->rect.height()/multipler);
            p.drawText(r,QString(ch->bukva),to);
        }
        if(page!=endPage)printer->newPage();
    }
    p.end();
}

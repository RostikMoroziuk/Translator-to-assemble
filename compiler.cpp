#include "compiler.h"
#include "scaner.h"
#include <QtWidgets>
#include "codeeditor.h"
#include "Parser.h"

//using namespace Words;
Compiler::Compiler(QWidget *parent)
    : QWidget(parent)
{
    pmenuBar =  new QMenuBar(this);
    pmenu = new QMenu ("Меню");
    pmenuRun = new QMenu("Запуск");
    QAction* save, *open, *saveAs, *exit, *run, *build;
    save = new QAction("Зберегти", this);
    save->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_S));
    pmenu ->addAction(save);

    saveAs = new QAction("Зберегти як", this);
    pmenu ->addAction(saveAs);

    open = new QAction("Відкрити", this);
    open->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_O));
    pmenu ->addAction(open);

    pmenu->addSeparator();

    exit = new QAction("Вийти", this);
    exit->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_E));
    pmenu ->addAction(exit);

    run = new QAction("Запуск", this);
    run->setShortcut(QKeySequence(Qt::Key_F5));
    pmenuRun ->addAction(run);

    build = new QAction("Зібрати", this);
    build->setShortcut(QKeySequence(Qt::Key_F7));
    pmenuRun ->addAction(build);

    pmenuBar->addMenu(pmenu);
    pmenuBar ->addMenu(pmenuRun);

    toolBar = new QToolBar();
    toolBar->addActions(QList<QAction*>()<<save<<build<<run);

    //---------------------------------------------------
    m_process= new QProcess(this); //необхідно для запуску зовнішніх програм
    ptxt= new CodeEditor ;
    ptxt->appendPlainText(";");

    pStatusInfo = new QTextEdit();
    pStatusInfo->setMaximumHeight(100);
    QLabel* console_info = new QLabel ("Вивід програми:");
    lbl = new QLabel ("&Код програми .m15");
    lbl->setBuddy(ptxt);

    // connection
    connect(m_process,SIGNAL(readyReadStandardOutput()),SLOT(slotDataOnStdout()));
    connect(saveAs,SIGNAL(triggered(bool)),SLOT(slotSaveAs()));
    connect(save,SIGNAL(triggered(bool)),SLOT(slotSave()));
    connect(open, SIGNAL(triggered(bool)), SLOT(slotOpen()));
    connect(exit,SIGNAL(triggered(bool)),SLOT(slotExit()));
    connect(run,SIGNAL(triggered(bool)),SLOT(slotRun()));
    connect(build, SIGNAL(triggered(bool)), SLOT(slotBuild()));

    // Layout setup
    QVBoxLayout * vbox = new QVBoxLayout ;
    vbox->setMenuBar(pmenuBar);
    vbox ->addWidget(lbl);

    vbox->addWidget(toolBar);
    vbox ->addWidget(ptxt);
    vbox->addWidget(console_info);
    vbox->addWidget(pStatusInfo);

    setLayout(vbox);

    tbl = new QTableWidget;
}

Compiler::~Compiler()
{

}

void Compiler::pathChanger()
{
    if(!path.isEmpty())
    {
        for(int i =  0 ; i < path.length();++i)
        {
            if(path.at(i)=='/')
            {
                path.remove(i,1);
                path.insert(i,"\\");
            }
        }
        path.chop(nameNoPoth.length()+4);//видаляє останні символи з шляху (розширення)
        qDebug()<<path;
    }
}

bool Compiler::procSetup(const QString &msg , int n = 500)
{
    m_process->start(msg);
    Sleep(n);
    m_process->close();
    return true;
}


void Compiler::slotSave()
{
    if(Name.isEmpty())
    {
        slotSaveAs();
        return ;
    }
    QFile * pFile = new QFile(Name);
    if(pFile->open(QIODevice::WriteOnly))
    {
        QTextStream(pFile) << ptxt->toPlainText();
        pFile->close();
        return;
    }

}

bool Compiler::slotSaveAs()
{
    Name.clear();
    Name = QFileDialog::getSaveFileName(this,
                                        tr("Зберегти asm файл"),
                                        "Assembler.m15",
                                        "*.m15"
                                        );
    qDebug()<<Name;

    if(!Name.isEmpty())
    {
        path.clear();
        path=Name;
        pathChanger();
        QFile * pFile = new QFile(Name);
        if(pFile->open(QIODevice::WriteOnly))
        {
            QTextStream(pFile) << ptxt->toPlainText();
            QFileInfo inf (pFile->fileName());
            lbl->setText(inf.fileName());
            nameNoPoth.clear();
            nameNoPoth = inf.fileName();
            nameNoPoth.chop(4);
            qDebug()<< nameNoPoth;
            pFile->close();
            path.chop(nameNoPoth.length());
            return true;
        }
        pStatusInfo->setPlainText(Name);
    }
    return false;
}

void Compiler::slotAssembly()
{
    QString str = "cmd /k C:\\masm32\\bin\\ml /c /coff ";
    str+=path+nameNoPoth+".asm";

    procSetup(str);

    str="cmd /k copy "+nameNoPoth+".obj "+path;
    procSetup(str);

    str="cmd /k del "+nameNoPoth+".obj ";
    procSetup(str);

}

void Compiler::slotLink()
{
    QString str = "cmd /k C:\\masm32\\bin\\link /SUBSYSTEM:CONSOLE /OPT:NOREF ";
    str+=path+nameNoPoth+".obj";

    procSetup(str);

    str="cmd /k copy "+nameNoPoth+".exe "+path;
    procSetup(str,4000);

    str="cmd /k del "+nameNoPoth+".exe ";
    procSetup(str);
}

void Compiler::slotDataOnStdout()
{
    pStatusInfo->append(m_process->readAllStandardOutput());
}

void Compiler::slotOpen()
{
    Name = QFileDialog::getOpenFileName(this , tr("Відкрити файл"));
    if(!Name.isEmpty())
    {
        QFile * pFile = new QFile(Name);
        if(pFile->open(QIODevice::ReadOnly))
        {
            ptxt->setPlainText(pFile->readAll());
            QFileInfo inf (pFile->fileName());
            lbl->setText(inf.fileName());
            nameNoPoth = inf.fileName();
            nameNoPoth.chop(4);
            qDebug()<< nameNoPoth;
            path=Name;
            pathChanger();
            pFile->close();
        }


    }
}

void Compiler::slotRun()
{
    m_process->startDetached(path+nameNoPoth+".exe");
}

void Compiler::slotExit()
{
    QApplication :: quit();
}

bool Compiler::slotLexical()
{
    pStatusInfo->clear();
    pStatusInfo->append("Лексичний аналіз...");
    if(tbl!=NULL)
        delete tbl;
    QString text;//text - код програми
    text=ptxt->toPlainText();
    Scaner lex(text);

    connect(&lex, SIGNAL(error(QString)), this, SLOT(Error(QString)));

    lexems.clear();
    if(!lex.prewscan())
        return false;//попердній аналіз на лапки і скобки

    Token* look = new Token();
    while (look->type!=7 && lex.column<lex.GetText().length())
    {
        look=lex.scan();
        if(look->type!=32)
            lexems.push_back(look);
        else
        {
           qDebug()<<lex.hasError;
           if(lex.hasError)
               break;
        }
    }
    tbl=NULL;
    qDebug()<<lex.hasError;
    if(!lex.hasError)
    {
        tbl = new  QTableWidget (lexems.length(),2);
        QTableWidgetItem* ptwi = 0 ;
        QStringList lst ;
        lst  << "Лексема" << "Коментар";
        tbl->setHorizontalHeaderLabels(  lst);
        for(int i = 0 ; i<lexems.length();++i)
        {
            if (lexems[i]->type==31)
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Числова константа");
                tbl->setItem(i,1,ptwi);

            }
            else if (lexems[i]->type==29)
            {

                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Рядкова константа константа");
                tbl->setItem(i,1,ptwi);
            }
            else if (lexems[i]->type==27)
            {

                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex );
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Ідентифікатор");
                tbl->setItem(i,1,ptwi);
            }
            else if (lexems[i]->type==10)
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Оператор присвоювання");
                tbl->setItem(i,1,ptwi);
            }
            else if (lexems[i]->type==14 ||lexems[i]->type== 15 || lexems[i]->type==16 ||lexems[i]->type== 17 || lexems[i]->type==18)
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Бінарний оператор");
                tbl->setItem(i,1,ptwi);
            }
            else if (lexems[i]->type==23 ||lexems[i]->type== 24 ||lexems[i]->type==25 )
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Логічний оператор");
                tbl->setItem(i,1,ptwi);
            }
            else if (lexems[i]->type==19 ||lexems[i]->type== 20)
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Оператор порівняння");
                tbl->setItem(i,1,ptwi);
            }
            else if (lexems[i]->type==32)
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem  (t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem  ("Неопізнана лексема");
                tbl->setItem(i,1,ptwi);
            }
            else if (lexems[i]->type==1 ||lexems[i]->type==4 ||lexems[i]->type==5 ||lexems[i]->type==7 ||lexems[i]->type==8 ||
                     lexems[i]->type==9 ||lexems[i]->type==11 ||lexems[i]->type==12 ||lexems[i]->type==13 ||lexems[i]->type==26)
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem(t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem ("Ключове слово");
                tbl->setItem(i,1,ptwi);
            }

            else
            {
                Token* t = lexems[i];
                ptwi = new QTableWidgetItem(t->lex);
                tbl->setItem(i,0,ptwi);
                ptwi = new QTableWidgetItem ("Символ");
                tbl->setItem(i,1,ptwi);
            }
        }
        tbl->setColumnWidth(1,200);
        tbl->resize(350,350);
        tbl->setWindowTitle("Таблиця лексем");
        tbl->show();
        return true;
    }
    return false;
}

bool Compiler::slotParser()
{
    pStatusInfo->append("Синтаксичний аналіз...");
    Parser* pars = new Parser (lexems);
    connect(pars,SIGNAL(emitError(QString)),SLOT(Error(QString)));
    pars->analiz();

    if(!pars->hasError)
    {
        codeIn=pars->GetCodeTable();
        paramIn=pars->GetParamTable();
        idParam = pars->GetParamId();
        tokParam = pars->GetParamTok();
        pStatusInfo->append("Синтаксичний аналіз закінчено");
        if(!Name.isEmpty())
            slotSave();
        return true;
    }
    return false;
}

bool Compiler::generate()
{
    if(Name.isEmpty())
    {
        QMessageBox* pbox = new QMessageBox(QMessageBox::Information,
                                            "Збереження файла",
                                            "Зберегти файл?",
                                            QMessageBox::Yes|QMessageBox::No);
        int  n  = pbox->exec();
        if (n==QMessageBox::Yes)
        {
            if(!slotSaveAs())
                return false;
        }
        else
            return false;
    }
    Generator Gen(codeIn,paramIn, idParam, tokParam);
    Gen.generate();
    codeGen=Gen.GetOutput();

    QFile * pFile = new QFile(path +nameNoPoth+".asm");
    if(pFile->open(QIODevice::WriteOnly))
    {
        QTextStream(pFile) << codeGen;
        pFile->close();
    }
    return true;
}

void Compiler::slotBuild()
{
    if(!slotLexical())
    {
        Error("Виправіть помилки і попробуйте знову");
        return;
    }
    QApplication::processEvents();
    if(!slotParser())
    {
        Error("Виправіть помилки і попробуйте знову");
        return;
    }
    else
    {
        if(!generate())
        {
            pStatusInfo->append("Завершено...");
            return;
        }
        QApplication::processEvents();
        slotAssembly();
        QApplication::processEvents();
        slotLink();
    }
    QFile * pFile = new QFile(path +"info.txt");
    if(pFile->open(QIODevice::WriteOnly))
    {
        QTextStream(pFile) << pStatusInfo->toPlainText();
        pFile->close();
    }
}

void Compiler::Error(QString msg)
{
    pStatusInfo->append(msg);
}





#ifndef COMPILER_H
#define COMPILER_H

#include <QtWidgets>
#include "generator.h"
#include "codeeditor.h"
#include "scaner.h"
#include "parser.h"

class Compiler : public QWidget
{
    Q_OBJECT
private:
    QLabel* lbl; //лейбл над текстом програми
    CodeEditor* ptxt; //код програми
    QTextEdit* pStatusInfo;//вивід додаткової інформації (під console information)
    QString Name;//ім'я файлу з шляхом куди зберігаємо (зберігаємо для подальшого оновлення)
    QString nameNoPoth;//ім'я файла без розширення і без шляху
    QString path;//шлях(модифікуються слеші, для коректного зберігання)
    QString codeGen;//згенерований код
    QMenuBar* pmenuBar;//меню бар
    QMenu* pmenu ;//меню
    QMenu* pmenuRun;//меню запуску
    QToolBar* toolBar;
    QProcess* m_process ;//для запуску зовнішніх процесів
    QTableWidget* tbl; //таблиця виділених лексем
    QVector <Token*> lexems;//таблиця сформованих лексем
   //---------------------------
    QVector <short> codeIn;
    QVector <QString> paramIn;
    QVector<Id*> idParam;
    QVector<QVector<Token*> > tokParam;

public:
    Compiler(QWidget *parent = 0);
    ~Compiler();
    void pathChanger();//
signals :
private:
    bool procSetup(const QString & msg, int n);
public slots :
        void slotSave();
        bool slotSaveAs();
        void slotAssembly();
        void slotLink();
        void slotDataOnStdout();
        void slotOpen();
        void slotRun();
        void slotExit();
        bool slotLexical();
        bool slotParser();
        bool generate();
        void slotBuild();
        void Error(QString msg);
};



#endif // COMPILER_H


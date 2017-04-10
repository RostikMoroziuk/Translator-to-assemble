#ifndef PARSER_H
#define PARSER_H
#include "scaner.h"
#include <setjmp.h>
#include <QHash>
#include <QStack>

struct Id
{
    QString id, value, type;
};

class Parser: public QObject
{
    Q_OBJECT
private :
    QVector <short> codeIn;//команди для генерації коду
    //параметри для генерації коду
    QVector <QString> paramIn;
    QVector<Id*> idParam;
    QVector<QVector<Token*> > tokParam;

    QVector<Token*> lex;//таблиця лексем
    Token* look;
    jmp_buf jumpBuffer;
    QHash <QString,Id*> nameTable ;
    int i;
    QStack<Token*> magazin,//магазинний автомат
    mainLex;//для терміналів, які задають правило

    bool isOperator(Token*);
    bool isOperand(Token*);
    int order(Token*);
    bool isHigher(Token*, Token*);
    QVector<Token*> toPostfix(QStack<Token*>);
public:
    Parser(QVector<Token*> lexems);
    bool hasError;

        void error(QString msg, int line);
inline  bool match(int t, QString msg);

        void analiz();
        bool titleAnaliz();
        bool decl();
        bool bloc();
        bool checkTable();
        bool checkTable(QString name);
        bool assign();
        Id* getId(QString name);
        QVector <short> GetCodeTable()
        {
         return codeIn;
        }
        QVector <QString> GetParamTable()
        {
            return paramIn;
        }
        QVector <Id*> GetParamId()
        {
            return idParam;
        }
        QVector <QVector<Token*> > GetParamTok()
        {
            return tokParam;
        }
signals:
        void emitError(QString);
};

#endif // PARSER_H

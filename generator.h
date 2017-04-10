#ifndef GENERATOR_H
#define GENERATOR_H
#include <QtWidgets>
#include "parser.h"
class Generator
{
private:
    QString output;//код програми на ассемблері
    QVector <short> codes;//коди операторів
    QVector <QString> parametrs;
    QVector <Id*> idParam;
    QVector<QVector<Token*> > tokParam;
    int forCount;
    int cmpCount;
    int nextCount;
    void GenerateExp(QVector<Token*>);
    QString forId;
public:
    Generator(QVector <short> const& cod ,QVector <QString> const &parIn, QVector <Id*> const &idPar, QVector<QVector<Token*> > tokParam);
    void generate();
    void print(QString& variable,   short  state );
    QString GetOutput()
    {
        return output;
    }
};

#endif // GENERATOR_H

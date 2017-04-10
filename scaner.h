#ifndef Scaner_H
#define Scaner_H


struct Token
{
    Token(int t =0, QString lex = "", int line = 0):type(t), lex(lex), line(line){}
    Token& operator=(const Token& tok);
    int type ;
    QString lex;
    int line;
};

class Scaner:public QObject
{
    Q_OBJECT
private:
    QString text;//текст для лексичного аналізу
    short inMarks = 0;// inMarks = 0 meen" is not open",  inMarks =1 " is open" inMarks =3  was opened but now close
    Token word;
public:
    int line = 1;
    int column = 0;
    QChar peek = ' ';
    bool hasError;
    QHash <QString, Token> keyWords;
    void reserve (Token* word );
    Scaner(QString const &str);
    QString GetText();
    bool prewscan();//перевірка на кількість лапок і блоків
    void brackets_error(int row );//помилка кількості скобок
    void marks_error(int row);//помилка кількості лапок

    Token* scan();

signals:
    void error(QString msg);

};

#endif // Scaner_H

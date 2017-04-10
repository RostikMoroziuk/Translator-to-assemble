#include "scaner.h"

Token &Token::operator=(const Token &tok)
{
    this->type=tok.type;
    this->lex = tok.lex;
    return *this ;
}

void Scaner::reserve(Token *word)
{
    keyWords.insert(word->lex,*word);
}

Scaner::Scaner(const QString &str)
{
    hasError = false;
    text=str; //текст після простої перевірки на помилки
    // reserve words in HASH
    reserve (new Token (1, "Program"));
    reserve (new Token (4, "Start"));
    reserve (new Token (5, "Variable"));
    reserve (new Token (7, "Stop"));
    reserve (new Token (8, "Read"));
    reserve (new Token (9, "Write"));
    reserve (new Token (11, "For"));
    reserve (new Token (12, "To"));
    reserve (new Token (13, "Do"));
    reserve (new Token (16, "Mnl"));
    reserve (new Token (17, "Div"));
    reserve (new Token (18, "Mod"));
    reserve (new Token (19, "Eg"));
    reserve (new Token (20, "Ne"));
    reserve (new Token (24, "And"));
    reserve (new Token (25, "Or"));
    reserve (new Token (26, "Int16_t"));
}

QString Scaner::GetText()
{
    return text;
}

bool Scaner::prewscan()//видаляє всі зайві розділювачі
{
    hasError = false;
    QString str_ready;

    QStack <QChar> brackets;//{
    QStack <int> rowOfBrackets;
    QStack<QChar> brackets1;//(
    QStack<int> rowOfBrackets1;
    QStack <QChar> marks;
    QStack <int> rowOfMarks;
    int row  = 1;

    for (int i =0 ; i<text.length();++i)
    {
        if(text.at(i)=='$' && text.at(i+1)=='$')
        {
            for(;text.at(i)!='\n';i++);
        }
        if(text.at(i)==' ' || text.at(i)=='\t')
        {
            for(;(text.at(i)==' ' || text.at(i)=='\t') && (text.at(i+1)==' ' || text.at(i+1)=='\t');i++);
        }
        if (text.at(i)=='\n')
            ++row;
        else if(text.at(i)=='"')
        {;
            if(marks.isEmpty())
            {
                marks.push('"');
                rowOfMarks.push(row);
            }
            else
            {
                marks.pop();
                rowOfMarks.pop();
            }
        }
        else if(text.at(i)=='{')
        {
            brackets.push('{');
            rowOfBrackets.push(row);
        }
        else if(text.at(i)=='}')
        {
            if(brackets.isEmpty())
                brackets_error(row);
            else
            {
                brackets.pop();
                rowOfBrackets.pop();
            }
        }
        else if(text.at(i)=='(')
        {
            brackets1.push('(');
            rowOfBrackets1.push(row);
        }
        else if(text.at(i)==')')
        {
            if(brackets1.isEmpty())
                brackets_error(row);
            else
            {
                brackets1.pop();
                rowOfBrackets1.pop();
            }
        }
        str_ready+=text[i];//очищення від коментарів
    }
    if(!brackets.isEmpty())
    {
        while (!rowOfBrackets.isEmpty())
            brackets_error(rowOfBrackets.pop());
        return false ;
    }
    if(!marks.isEmpty())
    {
        while (!rowOfMarks.isEmpty())
            marks_error(rowOfMarks.pop());
        return false ;
    }
    text = str_ready ;
    return true;
}

void Scaner::brackets_error(int row)
{
    QString t = QString("Відсутні дужки в рядку - ") +QString::number(row ) + QString("\n");
    hasError = true;
    emit error(t);
}

void Scaner::marks_error(int row)
{
    hasError = true;
    emit error(QString("Відсутні лапки в рядку - ") +QString::number(row ) + QString("\n")) ;
}

Token *Scaner::scan()
{
    for(peek = text[column]; column<text.length(); )
    {
        if(peek == '\n')
        {
            line++;
            column++;
            peek=text[column];
            qDebug()<<peek;
        }
        if(inMarks==1)
        {
            QString temp;
            while (peek!='"')
            {
                qDebug()<<peek;
                if(peek=='\n')
                    ++line;
                peek = text[column];
                temp+=peek;
                column++;
            }
            --column;
            inMarks=2;
            return new Token(29, temp, line);
        }
        if(peek.isDigit())
        {
            int v = 0 ;
            do
            {
                v=10*v+peek.digitValue();
                column++;
                peek = text[column];
            } while (peek.isDigit());
            return new Token(31, QString::number(v), line);
        }
        if(peek.isLetter())
        {
            QString s ;
            do
            {
                s.append(peek);
                column++;
                peek = text[column];
                qDebug()<<peek;
            }while (peek.isLetter() || peek.isNumber());//перевіряємо слово (слоо обов"язково починається з літери а всередині можуть бути і цифри
            qDebug()<<s;
            if(s=="Int16")
            {
                if(peek=='_' && text[column+1]=='t')
                {
                    column+=2;
                    return new Token(26, "Int16_t", line);
                }
            }
            if (keyWords.contains(s))//перевірка чи це ключове слово
            {
                Token tmp = keyWords.value(s);
                return new Token(tmp.type, tmp.lex, line);
            }
            else if(s.length()<=6)
            {
                for(int i=0;i<s.length();i++)
                    if(s[i].isLower())
                    {
                        hasError = true;
                        emit error(QString("Неправильно оголошена змінна ") + s + QString(" в рядку") + QString::number(line));
                        return new Token(32, "", line);
                    }
                /*for(int i = 1 ; i<s.length();++i)
                {
                    if (s.at(i).isDigit() || !s.at(i).isLower())
                    {
                        hasError = true;
                        emit error(QString("Невідома змінна - ") + s + QString(" в рядку") + QString::number(line));
                        return new Token(32, "", line);
                    }
                }*/
                return new Token(27, s, line);
            }
            else
            {
                hasError = true;
                emit error(QString("Невідома змінна (забагато символів) - ") + s + QString(" в рядку") + QString::number(line));
                return new Token(32, "", line);
            }
        }
        if(peek==' ' || peek=='\t')
        {
            column++;
            peek=text[column];
            qDebug()<<peek;
            return new Token(32, "", line);
        }
        if(peek=='"')
        {
            if(inMarks==0)
                inMarks=1;
            else if(inMarks ==2)
            {
                inMarks=0;
            }
            column++;
            return new Token(30, "\"", line);
        }
        if(peek == '#')
        {
            column++;
            return new Token(0, "#", line);
        }
        if(peek == '<')
        {
            column++;
            return new Token(2, "<", line);
        }
        if(peek == '>')
        {
            column++;
            return new Token(3, ">", line);
        }
        if(peek == ';')
        {
            column++;
            return new Token(6, ";", line);
        }
        if(peek == ':')
        {
            if(text[column+1]==':', line)
            {
                column+=2;
                return new Token(10, "::", line);
            }
            else
            {
                hasError = true;
                emit error(QString("Невідомий символ в рядку - ") + QString::number(line));
                return new Token(32, "", line);
            }
        }
        if(peek == '+')
        {
            column++;
            return new Token(14, "+", line);
        }
        if(peek == '-')
        {
            column++;
            return new Token(15, "-", line);
        }
        if(peek=='(')
        {
            column++;
            return new Token(21, "(", line);
        }
        if(peek==')')
        {
            column++;
            return new Token(22, ")", line);
        }
        if(peek == '!')
        {
            column++;
            return new Token(23, "!", line);
        }
        if(peek=='{')
        {
            column++;
            return new Token(33, "{", line);
        }
        if(peek=='}')
        {
            column++;
            return new Token(34, "}", line);
        }
        column++;
        hasError = true;
        emit error(QString("Невідомий символ в рядку - ") + QString::number(line));
        return new Token(32, "", line);
    }
    return new Token(32, "", line);
}

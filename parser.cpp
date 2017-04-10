#include "Parser.h"


Parser::Parser(QVector<Token*> l):lex(l)
{
    hasError = false;
    i = 0;
    look = lex[i++];
}

void Parser::error(QString msg, int line = 0)
{
    hasError = true;
    codeIn.clear();
    paramIn.clear();
    idParam.clear();
    tokParam.clear();
    if(line==0)
        line = look->line;
    emit emitError("Рядок:"+QString::number(line)+": "+msg);
    longjmp(jumpBuffer,1);
}

bool Parser::match(int t , QString msg)
{
    if(look->type==t)
    {
        look = lex[i++];
        qDebug()<<look->lex;
    }
    else
    {
        hasError = true;
        error("Пропущено " + msg );
    }

    return !hasError; //повертає "чи без помилок"
}

void Parser::analiz()
{
    hasError = false;
    if(setjmp(jumpBuffer)==1)
        return;
    while(i<=lex.count())
    {
        qDebug()<<look->type;
        switch(look->type)
        {
        case 0://програма починається з решітки
            if(!mainLex.isEmpty())
            {
                error("Неправильне оголошення заголовку програми");
                return;
            }
            mainLex.push(look);
            look = lex[i++];
            if(!titleAnaliz())
            {
                return;
            }
            if(!match(6, ";"))
            {
                return;
            }
            break;
        case 4:
        {
            if(mainLex.pop()->type!=0)//якщо мітка старт іде не після заголовку
            {
                error("Неправильне використання ключового слова Start");
                return;
            }
            else
            {
                mainLex.push(look);
                look=lex[i++];
            }
            break;
        }
        case 5://Variable
            if(mainLex.top()->type!=4)
            {
                error("Пропущено Start");
                return;
            }
            else
            {
                mainLex.push(look);
                look = lex[i++];
                if(!decl())
                    return;
                look = lex[i++];
                if(!match(6, ";"))
                    return;
            }
            break;
        case 33://code block
            qDebug()<<mainLex.top()->type;
            if(mainLex.pop()->type!=5)//не був закінчений блок оголошення змінних
            {
                error("Пропущено блок оголошення змінних");
                return;
            }
            else
            {
                codeIn.push_back(2);
                mainLex.push(look);
                look = lex[i++];
                qDebug()<<look->lex;
                if(!bloc())
                {
                    return;
                }
            }
            break;
        case 7:
            if(mainLex.pop()->type!=4)
            {
                error("Пропущено мітку Start");
                return;
            }
            i++;
            break;
        default:
            error("Неропізнана лексема");
        }
    }
}

bool Parser::bloc()
{
    while(mainLex.top()->type==33 || mainLex.top()->type==11)//поки відкрита скобка
    {
        qDebug()<<look->lex;
        switch (look->type)
        {
        case 6://;
        {
            look = lex[i++];
            break;
        }
        case 27://id. Після нього обов'язково повинен йти ::, інакше буде видавати помилку
        {
            if(!nameTable.contains(look->lex))
            {
                error("Використано неоголошену змінну");
                return false;
            }
            qDebug()<<look->lex;
            magazin.push(look);
            if(!assign()) 
                return false;
            look = lex[i++];
            qDebug()<<look->lex;
            break;
        }
        case 8://Read
        {
            qDebug()<<look->lex;
            mainLex.push(look);
            look = lex[i++];
            qDebug()<<look->lex;
            while(look->type!=6)
            {
                magazin.push(look);
                look = lex[i++];
                qDebug()<<look->lex;
            }
            Token* curTok = magazin.top();
            Token* nextTok = mainLex.pop();//лівіший
            if(curTok->type==8 || curTok->type!=27)
            {
                error("Пропущено ідентифікатор");
                return false;
            }
            else if(nextTok->type!=8)
            {
                error("Неправильне використання Read");
                return false;
            }
            else
            {
                qDebug()<<curTok->lex;
                if(checkTable(curTok->lex))
                {
                    Id* id = getId(curTok->lex);
                    codeIn.push_back(4);
                    qDebug()<<curTok->type;
                    idParam.push_back(id);
                }
                else
                {
                    error("Використано неоголошену змінну");
                    return false;
                }
            }
            look = lex[i++];
            qDebug()<<look->lex;
            break;
        }
        case 9://Write
        {
            mainLex.push(look);
            magazin.push(look);
            qDebug()<<look->type;
            look = lex[i++];
            qDebug()<<look->lex;
            while(look->type!=6)
            {
                magazin.push(look);
                look = lex[i++];
                qDebug()<<look->lex;
            }
            Token* curTok = magazin.pop();
            if(curTok->type==9)
            {
                error("Пропущено операнди");
                return false;
            }
            else
            {
                if(curTok->type==27 && mainLex.top()->type==9)//операнд - змінна і головне правило  - write
                {
                    Id* id = getId(curTok->lex);
                    if(id)
                    {
                        codeIn.push_back(5);
                        paramIn.push_back(id->id);
                        look = lex[i++];
                        mainLex.pop();
                        magazin.pop();
                    }
                }
                else if(curTok->type == 30 && mainLex.top()->type==9)//операнд - рядкова константа і головне правило - write
                {
                    QString txt = curTok->lex;//" відкриваючі
                    curTok = magazin.pop();
                    qDebug()<<curTok->lex;
                    if(curTok->type!=29)
                    {
                        error("Неправильна рядкова константа", curTok->line);
                        return false;
                    }
                    txt += curTok->lex;//рядкова константа
                    qDebug()<<magazin.top()->lex;
                    if(magazin.top()->type!=30)
                    {
                        error("Неправильна рядкова константа", curTok->line);
                        return false;
                    }
                    txt+=magazin.pop()->lex;// лапки закриваючі
                    if(magazin.pop()!=mainLex.pop())
                    {
                        error("Невірні параметри");
                        return false;
                    }
                    codeIn.push_back(5);
                    paramIn.push_back(txt);
                    look = lex[i++];
                }
                else
                {
                    error("Нерозпізнана конструкція");
                    return false;
                }
            }
            
            break;
        }
        case 11://For
        {
            mainLex.push(look);
            qDebug()<<look->lex;
            while(look->type!=33)
            {
                magazin.push(look);
                look = lex[i++];
                qDebug()<<look->lex;
            }
            Token* curTok = magazin.pop();
            if(curTok->type!=13)
            {
                error("Пропущено do", curTok->line);
                return false;
            }
            Token* last = magazin.pop();
            if(last->type!=31 && last->type!=27)
            {
                error("Пропущено другий вираз", last->line);
                return false;
            }
            curTok = magazin.pop();
            if(curTok->type!=12)
            {
                error("Пропущено to", curTok->line);
                return false;
            }
            curTok = magazin.pop();
            if(curTok->type!=31)
            {
                error("Пропущено перший вираз", curTok->line);
                return false;
            }
            if(magazin.top()->type!=10)
            {
                error("Пропущено ::", magazin.pop()->line);
                return false;
            }
            magazin.pop();
            if(magazin.top()->type!=27)
            {
                error("Пропущено Ідентифікатор", magazin.pop()->line);
                return false;
            }
            Token* id = magazin.top();
            if(!nameTable.contains(id->lex))
            {
                    error("Невідома змінна", magazin.pop()->line);
                    return false;
            }
            magazin.pop();
            if(magazin.pop()->type!=11)
            {
                error("Невірна конструкція", magazin.pop()->line);
                return false;
            }
            codeIn.push_back(6);
            int loop;
            if(nameTable.contains(last->lex))
            {
                Id* id = getId(last->lex);
                loop =  id->value.toInt() - curTok->lex.toInt();
            }
            else
            {
                loop = last->lex.toInt() - curTok->lex.toInt();
            }
            if(loop>0)
            {
                paramIn.push_back(id->lex);
                paramIn.push_back(curTok->lex);//початкове значення
                paramIn.push_back(QString::number(loop));//кількість ітерацій
            }
            else
            {
                error("Невірні числові константи");
                return false;
            }
            look = lex[i++];
            break;
        }
        case 34:
        {
            if(mainLex.top()->type==11)
            {
                codeIn.push_back(7);
            }
            mainLex.pop();
            look = lex[i++];
            qDebug()<<look->lex;
            break;
        }
        default:
            error("Невірна конструкція");
            return false;
            break;
        }
    }
    return true;
}

bool Parser::isOperator(Token* lex)
{
    if(lex->type==14 || lex->type==15 ||lex->type==16 ||lex->type==17 ||lex->type==18 ||
            lex->type==19 ||lex->type==20 ||lex->type==21 ||lex->type==22 ||lex->type==23 ||
            lex->type==24 ||lex->type==25 || lex->type==2 ||lex->type==3)
        return true;
    else
        return false;
}

bool Parser::isOperand(Token * operand)
{
    if(operand->type== 31 || operand->type==27)//Якшо число або ідентифікатор
        return true;
    else
        return false;
}

int Parser::order(Token* lex)
{
    switch (lex->type)
    {
        case 25:
            return 1;
        case 24:
            return 1;
        case 23:
            return 1;
        case 20:
            return 1;
        case 19:
            return 1;
        case 2:
            return 1;
        case 3:
            return 1;
        case 14:
            return 2;
        case 15:
            return 2;
        case 16:
            return 3;
        case 17:
            return 3;
        case 18:
            return 3;
        case 21:
            return 0;

        default:
            hasError = true;
            error("Невірна операція", lex->line);
            return 0;
    }
}

bool Parser::isHigher(Token* l1, Token* l2)
{
    if(order(l1)>=order(l2))
        return true;
    else
        return false;
}

QVector<Token*> Parser::toPostfix(QStack<Token*> infix)
{
    for(int i=0;i<infix.size();i++)
    {
        qDebug()<<infix[i]->lex;
    }
    QVector<Token*> postfix;
    QStack<Token*> stack;
    bool operand = true;//повинен бути операнд
    bool oper = false;//повинен бути оператор
    postfix.push_back(infix.pop());//ідентифікатор
    postfix.push_back(infix.pop());//::

    int size = infix.size();
    qDebug()<<size;
    for(int i=0; i<size; ++i)
    {
        qDebug()<<infix.top()->lex;
        if(operand && infix.top()->type!=21 && infix.top()->type!=22)
        {
        //if operand
            if(isOperand(infix.top()))
            {
                operand = false;
                oper = true;
                postfix.push_back(infix.pop());
            }
            else
            {
                error("Відсутні операнд", infix.pop()->line);
                return QVector<Token*>();
            }
        }
        //if operator
        else if(oper || infix.top()->type==21 || infix.top()->type==22)
        {
            if(isOperator(infix.top()))
            {
                oper = false;
                operand = true;
                //if stack is empty
                if(stack.isEmpty())
                {
                    stack.push(infix.pop());
                }
                //if stack not empty
                else if(!stack.isEmpty())
                {
                    //if (
                    if(infix.top()->type==21)//(
                    {
                        stack.push(infix.pop());
                    }
                    else if(infix.top()->type == 22) //)
                    {
                        while(stack.top()->type != 21)
                            postfix.push_back(stack.pop());
                        stack.pop();
                        infix.pop();
                        operand = false;
                        oper = true;
                    }
                    else
                    {
                        //pop until tos has lesser precedence or tos is null.
                        while(isHigher(stack.top(),infix.top()))
                        {
                            postfix.push_back(stack.pop());
                            if(stack.isEmpty())
                                break;
                        }
                        stack.push(infix.pop());
                    }
                }
            }
            else
            {
                error("Відсутній оператор", infix.pop()->line);
                return QVector<Token*>();
            }
        }
    }

    while(!stack.isEmpty())
        postfix.push_back(stack.pop());

    for(int i=0;i<postfix.size();i++)
        qDebug()<<postfix[i]->lex;
    return postfix;
}

bool Parser::assign()
{
    QStack<Token*> param;
    look = lex[i++];
    qDebug()<<look->type;
    if(look->type!=10)
    {
        error("Пропущено ::");
        return false;
    }
    else
    {
        magazin.push(look);
        look = lex[i++];
        qDebug()<<look->lex;
        while(look->type!=6)//беремо всі лексеми з вираза закінчуємо на ;
        {
            magazin.push(look);
            look = lex[i++];
            qDebug()<<look->lex;
        }
        Token* cur = magazin.pop();
        if(cur->type==10)
        {
            error("Пропущено вираз");
            return false;
        }
        while(cur->type!=10)//поки не дійде до присвоєння
        {
            if(cur->type==27)
            {
                if(!nameTable.contains(cur->lex))
                {
                    error("Невідома змінна", cur->line);
                    return false;
                }
            }
            param.push(cur);
            cur = magazin.pop();
            qDebug()<<cur->lex;
        }
        qDebug()<<cur->lex;
        param.push(cur);
        qDebug()<<magazin.top()->lex;
        param.push(magazin.pop());
        tokParam.push_back(toPostfix(param));
        codeIn.push_back(3);
        if(hasError)
            return false;
        else
            return true;
    }
}


bool Parser::titleAnaliz()//аналіз заголовку
{
        //тут не може бути згорток, бо йдуть тільки ключові слова, тому немає сенсу використовувати магазинний автомат
    if(!match(1, "Program"))//Program
    {
        return false;
    }
    if(!match(2, "<"))
    {
        return false;
    }
    if(!match(27, "ID"))
    {
        return false;
    }
    if(!match(3, ">"))
    {
        return false;
    }
    return true;
}

bool Parser::decl()
{
    if(!match(33, "{"))
    {
        return false;
    }
    while(look->type!=34)
    {
        Id* idInfo = new Id();
        while(look->type!=6)
        {
            qDebug()<<look->lex;
            switch(look->type)
            {
            case 26:
                if(mainLex.top()->type==26)
                {
                    error("Пропущено ;", mainLex.pop()->line);
                    return false;
                }
                idInfo->type=look->lex;
                mainLex.push(look);
                look=lex[i++];
                break;
            case 27:
                if(mainLex.top()->type!=26)
                {
                    error("Пропущено тип змінної");
                    return false;
                }
                if(checkTable())
                {
                    idInfo->id = look->lex;
                    magazin.push(look);
                }
                else
                {
                    error("Змінна неоголошена");
                    return false;
                }
                look = lex[i++];
                break;
            case 10:
                if(magazin.top()->type!=27)
                {
                    error(QString("Невірне оголошення змінних"), mainLex.pop()->line);
                    return false;
                }
                magazin.push(look);
                look = lex[i++];
                break;
            case 31:
                if(magazin.top()->type!=10)
                {
                    error(QString("Невірне оголошення змінних"), mainLex.pop()->line);
                    return false;
                }
                magazin.push(look);
                idInfo->value = look->lex;
                look = lex[i++];
                break;
            default://неможливе оголошення якщо неправильна послідовність
                error(QString("Невірне оголошення змінних"), mainLex.pop()->line);
                return false;
            }
        }
        if(mainLex.pop()->type == 26)
        {
            Token* temp = magazin.pop();
            if(temp->type!=26)
            {
                if(temp->type == 31)//якщо є початкова ініціалізація
                {
                    temp = magazin.pop();
                    if(temp->type!=10)
                    {
                        error(QString("Пропущено ::"));
                        return false;
                    }
                    else
                    {
                        temp = magazin.pop();
                        if(temp->type!=27)
                        {
                            error(QString("Пропущено назву змінної"));
                            return false;
                        }
                    }
                }
            }
            else
            {
                error(QString("Пропущено назву зміної"));
                return false;
            }
            if(!idInfo->id.isEmpty())
            {
                codeIn.push_back(1);
                idParam.push_back(idInfo);
                nameTable.insert(idInfo->id, idInfo);
            }
        }
        look=lex[i++];
    }
    return true;
}

bool Parser::checkTable()//перевіряємо чи введений існуючий ідентифікатор
{
    if(nameTable.contains(look->lex))
    {
        error(look->lex+ " - змінна вже оголошена.");
        return false;
    }
    else
        return true;
}

bool Parser::checkTable(QString name)//перевіряємо чи введений існуючий ідентифікатор
{
    if(nameTable.contains(look->lex))
    {
        error(name+ " - змінна вже оголошена.");
        return false;
    }
    else
        return true;
}

Id* Parser::getId(QString name)
{
    Id* temp = nameTable[name];
    if(!nameTable.contains(name))
    {
        error(name+ " - змінна не знайдена.");
        return NULL;
    }
    else
        return temp;
}



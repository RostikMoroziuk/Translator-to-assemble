#include "generator.h"
#include "scaner.h"

Generator::Generator(QVector <short> const& cod ,QVector <QString> const &parIn, QVector <Id*> const &idPar, QVector<QVector<Token*> > tokParam):
    codes(cod),parametrs(parIn), idParam(idPar), tokParam(tokParam)
{
    output.append("    .386\n"
                  ".model flat, c, STDCALL\n"
                  "option casemap :none\n"
                  "include C:\\masm32\\include\\masm32.inc\n"
                  "include C:\\masm32\\include\\kernel32.inc\n"
                  "include C:\\masm32\\include\\msvcrt.inc\n"
                  "include C:\\masm32\\include\\user32.inc\n"
                  "include C:\\masm32\\macros\\macros.asm\n"

                  "includelib C:\\masm32\\lib\\masm32.lib\n"
                  "includelib C:\\masm32\\lib\\kernel32.lib\n"
                  "includelib C:\\masm32\\lib\\user32.lib\n"
                  "includelib C:\\masm32\\lib\\msvcrt.lib\n"
                  "EXTRN scanf:proc\n"

                  ".data\n"
                  "inp db \"%d\",0\n"
                  "temp dd 0\n"
                  "tempExp1 dd 0\n"
                  "tempExp2 dd 0\n"
                  );
    forCount = 0;
    cmpCount = 0;
    nextCount = 0;
}

void Generator::generate()
{
    short op;
    QString param, temp ;
    for(int i=0;i<codes.size();i++)
        qDebug()<<codes[i];
    while(!codes.isEmpty())
    {
        op = codes.front();
        codes.pop_front();
        switch (op)
        {
        case 2://початок сегменту коду
            output.append(".code \nstart : \n");
            break;
        case 4://read
            qDebug()<<idParam.front()->id;
            output.append("push offset "+QString("_")+idParam.front()->id+'\n');//назва змінної
            output.append("push offset inp \n");//параметр
            output.append("call scanf \n");
            output.append("add sp, 8 \n");
            idParam.pop_front();
            break;
        case 5://write
        {
            param = parametrs.front();
            qDebug()<<param;
            parametrs.pop_front();
            int i = 0;
            temp.clear();
            if(param.at(i)=='"')
            {
                ++i;
                while(param.at(i)!='"')
                {
                    temp.append(param.at(i));
                    ++i;
                }
                print(temp,1);
                ++i;
            }
            else  //змінна
            {
                print(param, 2);
            }
        }
            break;
        case 6:
        {
            QString op, loop;
            forId = parametrs.front();
            parametrs.pop_front();
            op = parametrs.front();
            qDebug()<<op;
            parametrs.pop_front();
            loop = parametrs.front();
            qDebug()<<loop;
            parametrs.pop_front();

            output.append("mov ecx, "+ loop +'\n');
            output.append("mov " + QString("_") + forId+ ", " + op +'\n');
            output.append("labelFor"+QString::number(forCount)+  ":\n" );
            output.append("push ecx\n" );
            break;
        }
        case 7:
            output.append("inc " + QString("_") + forId + "\n" );
            output.append("pop ecx\n" );
            output.append("loop labelFor"+QString::number(forCount)+  "\n" );
            forCount++;
            break;
        case 3://обчислення виразів
            GenerateExp(tokParam.front());
            tokParam.pop_front();
            break;
        case 1://декларація змінних
        {
            output.append("_" + idParam.front()->id +" dd ");
            if(idParam.front()->value.isEmpty())
                output.append("0 \n");
            else
                output.append(idParam.front()->value+" \n");
            idParam.pop_front();
        }
        default:
            break;
        }
    }
    output.append("print \"for exit write smtm and press Enter\" \n push offset temp\n"
                  "push offset inp\n call scanf \n");
    output.append("invoke ExitProcess , 0 \n end start\n");

}

void Generator::GenerateExp(QVector<Token *> exp)
{
    Token* tok, *id;
    for(int i=0;i<exp.size();i++)
        qDebug()<<exp[i]->lex;
    id = exp.front();//ідентифікатор, якому присвоюємо
    exp.pop_front();
    qDebug()<<exp.front()->lex;//::
    exp.pop_front();
    while(!exp.isEmpty())
    {
        tok = exp.front();
        qDebug()<<exp.front()->lex;
        exp.pop_front();
        switch(tok->type)
        {
        case 31://якшо число
            output.append("mov tempExp1, "+tok->lex + "\n");
            output.append("fild tempExp1\n");
            break;
        case 27:
            output.append("fild " + QString("_") + tok->lex + "\n");
            break;
        case 14://+
            output.append("fadd\n");
            break;
        case 15://-
            output.append("fsub\n");
            break;
        case 16://*
            output.append("fmul\n");
            break;
        case 17:// /
            output.append("fdiv\n");
            break;
        case 18:// %
            output.append("fprem\n");
            break;
        case 19:// Eq
            output.append("fistp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "fistp tempExp2\n"
                          "cmp eax, tempExp2\n"
                          "jne labelCmp"+QString::number(cmpCount)+"\n"
                          "mov tempExp1, 1\n"
                          "jmp next"+QString::number(nextCount)+"\n"
                          "labelCmp"+QString::number(cmpCount)+":\n"
                          "mov tempExp1, 0\n"
                          "next"+QString::number(nextCount)+":\n"
                          "fild tempExp1\n");
            cmpCount++;
            nextCount++;
            break;
        case 20:// Ne
            output.append("fistp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "fistp tempExp2\n"
                          "cmp eax, tempExp2\n"
                          "je labelCmp"+QString::number(cmpCount)+"\n"
                          "mov tempExp1, 1\n"
                          "jmp next"+QString::number(nextCount)+"\n"
                          "labelCmp"+QString::number(cmpCount)+":\n"
                          "mov tempExp1, 0\n"
                          "next"+QString::number(nextCount)+":\n"
                          "fild tempExp1\n");
            cmpCount++;
            nextCount++;
            break;
        case 2:// <
            output.append("fstp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "fstp tempExp2\n"
                          "cmp eax, tempExp2\n"
                          "jl labelCmp"+QString::number(cmpCount)+"\n"
                          "mov tempExp1, 1\n"
                          "jmp next"+QString::number(nextCount)+"\n"
                          "labelCmp"+QString::number(cmpCount)+":\n"
                          "mov tempExp1, 0\n"
                          "next"+QString::number(nextCount)+":\n"
                          "fild tempExp1\n");
            cmpCount++;
            nextCount++;
            break;
        case 3:// >
            output.append("fistp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "fistp tempExp2\n"
                          "cmp eax, tempExp2\n"
                          "jg labelCmp"+QString::number(cmpCount)+"\n"
                          "mov tempExp1, 1\n"
                          "jmp next"+QString::number(nextCount)+"\n"
                          "labelCmp"+QString::number(cmpCount)+":\n"
                          "mov tempExp1, 0\n"
                          "next"+QString::number(nextCount)+":\n"
                          "fild tempExp1\n");
            cmpCount++;
            nextCount++;
            break;
        case 24:// and
            output.append("fistp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "fistp tempExp2\n"
                          "cmp eax, 0\n"
                          "je labelCmp"+QString::number(cmpCount)+"\n"//якшо перше число  0
                          "fistp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "cmp eax, 0\n"
                          "je labelCmp"+QString::number(cmpCount)+"\n"//або друге число  0
                          "mov tempExp1, 1\n"
                          "jmp next"+QString::number(nextCount)+"\n"
                          "labelCmp"+QString::number(cmpCount)+":\n"
                          "mov tempExp1, 0\n"//то 0
                          "next"+QString::number(nextCount)+":\n"
                          "fild tempExp1\n");
            cmpCount++;
            nextCount++;
            break;
        case 25:// or
            output.append("fistp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "fistp tempExp2\n"
                          "cmp eax, 0\n"
                          "jne labelCmp"+QString::number(cmpCount)+"\n"//якшо перше число 0
                          "fistp tempExp1\n"
                          "mov eax, tempExp1\n"
                          "cmp eax, 0\n"
                          "jne labelCmp"+QString::number(cmpCount)+"\n"//і друге число  0
                          "mov tempExp1, 0\n"//то 1
                          "jmp next"+QString::number(nextCount)+"\n"
                          "labelCmp"+QString::number(cmpCount)+":\n"
                          "mov tempExp1, 1\n"//інакше - 0
                          "next"+QString::number(nextCount)+":\n"
                          "fild tempExp1\n");
            cmpCount++;
            nextCount++;
            break;
        }
    }
    output.append("fistp "+ QString("_") + id->lex + "\n");//вигружаємо результат у пам'ять
}

void Generator::print(QString &variable, short state)
{
    switch (state)
    {
    case 1:
        output.append("print \" \", 13, 10\n");
        output.append("print \""+variable+"\"\n");
        break;
    case 2:
        output.append("print str$("+QString("_")+variable+")\n");
        break;
    default:
        break;
    }

}





#include "Token.h"
#include "tokenType.h"
#include "Tree.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <map>
#include "LL1.h"
#include "LispEnvironment.h"

using namespace std;


string getSourceCodeFromStream( istream &in );
void getTokenStream( vector<Token> &Q, string sourceCode );
void outputtest( vector<Token> &Q );

p_AstNode CalculateAST( p_AstNode );

vector<Token> TokenStream;
int currentIndex = 0;


/* the main SymbolTable for the running stack */

void match( TokenType type )
{
    Token token = TokenStream[currentIndex];
    if ( token.getTokenType() == type )
    {
        currentIndex++;
        cout << "match " << token.getStrval() << endl;
    }
    else
    {
        cout << "Not match " << token.getStrval() << endl;
        exit(0);
    }

}

void match( string s )
{ 
    Token token = TokenStream[currentIndex];
    if ( token.getStrval() == s )
    {
        currentIndex++;
        cout << "match " << token.getStrval() << endl;
    }
    else
    {
        cout << "Not match " << token.getStrval() << endl;
        exit(0);
    }

}

void error( string s )
{
    cerr<<"error happened in "<<s<<endl;
}

p_AstNode LL1_Lisp()
{
    Token token = TokenStream[currentIndex];
    Token nextToken = TokenStream[currentIndex+1]; 
    
    p_AstNode root;
    if( token.getStrval() == "(" )
    {
        if( nextToken.getStrval() == "define" )
            root = LL1_DEF();
        else
            root = LL1_exp();
    }
    // 现在已经得到一棵语法树，是时候要对语法树进行解析了。
    

    return root;
}

p_AstNode LL1_exp()
{
    Token token = TokenStream[currentIndex];
    string str = token.getStrval();

    LispEnvironment env = LispEnvironment::getRunTimeEnv();
    /* open the calculation */
    env.TurnOnCalculation();

    p_AstNode expNode;

    if ( token.getStrval() == "(" )
    {
        match("(");
        /* special procedure */
        token = TokenStream[currentIndex];
        str = token.getStrval();
        if ( str == "cond" )
            expNode = LL1_COND();
        else if ( str == "if" )
        {
            expNode = LL1_IF();
        }
        else if ( str == "let" )
        {
            LL1_LET();
        }
        else if ( str == "cons" )
        {
            expNode = LL1_CONS();
        }
        else if ( str == "car" )
        {
            expNode = LL1_CAR();
        }
        else if ( str == "cdr" )
        {
            vector<p_AstNode> Q = LL1_CDR();
            expNode = new ASTNode( CDR,"cdr" );
            expNode->setChild( Q );
        }
        else if ( str == "list" )
        {
            expNode = LL1_LIST();
        }
        else
        {
            p_AstNode procNode = LL1_procedure();
            vector<p_AstNode> valueList = LL1_exp_();
            procNode -> setChild( valueList );

            /* execute the calculation in the ast
             * and return a single astnode with the
             * final value 
             */
            expNode = procNode;
            //    expNode = CalculateAST( procNode );
            //    delete procNode;
        }
        match(")");
    }
    else if (token.getStrval()=="\"" || 
            token.getTokenType()==NUM || token.getTokenType()==ID )
    {
        expNode = LL1_Value();
    }
    else
    {
        error("LL1_exp()");
    }
    return expNode;
}

p_AstNode LL1_Value()
{
    Token token = TokenStream[currentIndex];
    string s = token.getStrval();

    p_AstNode vnode;

    if ( token.getStrval() == "\"" )
        match("\"");
    else if ( token.getTokenType() == ID )
        match( ID );
    else if ( token.getTokenType() == NUM )
    {
        vnode = new ASTNode( NUM, s );
        match( NUM );
    }
    else 
        error("LL1_Value()");

    return vnode;

}

p_AstNode LL1_procedure()
{
    Token token = TokenStream[currentIndex];

    string str = token.getStrval();

    p_AstNode   procNode;

    if ( str=="+" || str=="-" || str=="*" || str=="/" )
        procNode = LL1_Operator();

    else if ( str=="<" || str=="<=" || str==">" || str==">=" || str=="=" )
        procNode = LL1_Rop();

    else if ( str=="and" || str=="or" || str=="not" )
        procNode = LL1_Boolop();

    else if ( str=="(" )
    {
        if( TokenStream[currentIndex+1].getStrval() == "lambda" )
            procNode = LL1_LAMB();
        else
            procNode = LL1_exp();
    }

    else if ( token.getTokenType() ==ID )
    {
        match(ID);
    }

    else
        error("LL1_procedure()");

    return procNode;
}

p_AstNode LL1_Operator()
{
    Token   token = TokenStream[currentIndex];
    string str = token.getStrval();
    p_AstNode newnode = new ASTNode( ARITH_OP, str );

    if ( str == "+" )
    {
        match("+");
    }
    else if ( str == "-" )
    {
        match("-");
    }
    else if ( str == "*" )
    {
        match("*");
    }
    else if ( str == "/" )
    {
        match("/");
    }
    else
    {
        error("LL1_Operator()");
    }
    return newnode;

}

p_AstNode LL1_Rop()
{
    Token   token = TokenStream[currentIndex];
    string str = TokenStream[currentIndex].getStrval();
    
    p_AstNode newnode = new ASTNode( BOOL_OP, str );
    if ( str == "<" )
    {
        match("<");
    }
    else if ( str == "<=" )
    {
        match("<=");
    }
    else if ( str == ">" )
    {
        match(">");
    }
    else if ( str == ">=" )
    {
        match(">=");
    }
    else if ( str=="=" )
    {
        match("=");
    }
    else
    {
        error("ROP()");
    }
    return newnode;
}

p_AstNode LL1_Boolop()
{
    Token   token = TokenStream[currentIndex];
    string str = TokenStream[currentIndex].getStrval();

    /* 不标注为　bool_op, 暂定为　普通过程　*/
    p_AstNode newnode = new ASTNode( PROC, str );

    if ( str == "and" )
    {
        match("and");
    }
    else if ( str == "or" )
    {
        match("or");
    }
    else if ( str == "not" )
    {
        match("not");
    }
    else
    {
        error("LL1_Boolop()");
    }
    return newnode;
}

p_AstNode LL1_LAMB()
{
    match( "(" );
    match( "lambda" );
    match( "(" );
    LL1_ArguRefList();
    match( ")" );
    LL1_exp();
    match( ")" );

}

/* 返回引用还是值返回? */
vector<p_AstNode> LL1_exp_()
{
    Token token = TokenStream[currentIndex];

    vector<p_AstNode> valueList;

    if ( token.getStrval()=="(" || token.getStrval()=="\"" || 
            token.getTokenType()==ID || token.getTokenType()==NUM )
    {
        p_AstNode newnode = LL1_exp();
        valueList = LL1_exp_();
        valueList.insert( valueList.begin(), newnode );
    }
    else if ( token.getStrval()==")" )
        return valueList;
    else
        error("LL1_exp()");

    return valueList;
}

/* the function try to match (<p1> <e1>) (<p2> <e2>) ... */
p_AstNode LL1_ConditionList()
{
    Token token = TokenStream[currentIndex];

    p_AstNode finalResult;

    if ( token.getStrval() == ")" )
    {
        return p_AstNode();
    }
    else if ( token.getStrval() == "(" )
    {
        match("(");
        token = TokenStream[currentIndex];
        if ( token.getStrval() == "(" )
        {
            /* predicate */
            p_AstNode boolvalue = LL1_exp();
            /* expression */
            p_AstNode curr_result = LL1_exp();
            match(")");

            p_AstNode next_result = LL1_ConditionList();

            if( boolvalue->getNumber() == 1 )
            {
                finalResult = curr_result;
            }
            else
            {
                finalResult = next_result;
            }
        }
        else if( token.getStrval() == "else" )
        {
            match("else");
            /* expression */
            finalResult = LL1_exp();
            match(")");
        }
        else
        {
            error("错误：cond子句中谓词形式不正确");
        }
    }
    else
    {
        error("cond子句形式不正确，应为(p,e)");
    }
    return finalResult;
}

p_AstNode LL1_ArguRefList()
{
    Token token = TokenStream[currentIndex];
    if ( token.getTokenType() == ID )
    {
        match( ID );
        LL1_ArguRefList();
    }
    else if ( token.getStrval() == ")" )
    {
        return p_AstNode();
    }
    else
        error("LL1_ArguRefList()");
}

/* DEF -> ( define DEFOBJ DEFBODY )
 */
p_AstNode LL1_DEF()
{
    match("(");    
    match("define");
    LL1_DEFOBJ();
    LL1_DEFBODY();
    match(")");
}

/* DEFOBJ -> ID | ( ID Argulist )
 */
p_AstNode LL1_DEFOBJ()
{
    Token token = TokenStream[currentIndex];
    if ( token.getTokenType() == ID )
        match( ID );
    else if ( token.getStrval() == "(" )
    {   
        match( "(" );
        match( ID );
        LL1_ArguRefList();
        match( ")" );
    }
    else
        error("LL1_DEFOBJ()");
}


/* DEFOBJ -> ID | ( ID Argulist )
 */
p_AstNode LL1_DEFBODY()
{
    Token token = TokenStream[currentIndex];
    if ( token.getStrval() == "(" )
    {
        Token next_token = TokenStream[currentIndex+1];
        if ( next_token.getStrval() == "define" )
        {
            LL1_DEF();
            LL1_DEFBODY();
        }
        else
        {
            LL1_exp();
            LL1_DEFBODY();
        }
    }
    else if ( token.getStrval()==")" )
    {
        return p_AstNode();
    }
    else if ( token.getTokenType() == ID )
    {
        match(ID);
    }
    else if ( token.getTokenType() == NUM )
    {
        match(NUM);
    }
    else
        error("LL1_DEFBODY()");

}

p_AstNode LL1_IF()
{
    /* the '(' before if has been matched in LL1_exp() */
    p_AstNode ifTree, bopNode, thenNode, elseNode;

    LispEnvironment env = LispEnvironment::getRunTimeEnv();

    match("if");
    ifTree = new ASTNode( IF, "if" );
    
    // judgement
    bopNode = LL1_exp();
        // then-part
    thenNode = LL1_exp();
        // else-part
    elseNode = LL1_exp();

    // the children of iftree in order is bopNode, thenNode, elseNode
    ifTree->addChild( bopNode );
    ifTree->addChild( thenNode );
    ifTree->addChild( elseNode );

    return ifTree;
}

p_AstNode LL1_COND()
{
    match("cond");
    return LL1_ConditionList();
}

p_AstNode LocalVariablePairs()
{
    Token token = TokenStream[currentIndex];
    if ( token.getStrval() == ")" )
    {
        return p_AstNode();
    }
    else
    {
        match("(");
        //locally defined variable
        LL1_exp();
        // value to the previous defined variable
        LL1_exp();
        match(")");
        LocalVariablePairs();
    }

}

p_AstNode LL1_LET()
{
    match("let");
    match("(");

    LocalVariablePairs();

    match(")");

    LL1_exp();
}

static vector<p_AstNode> VariableList()
{
    Token token = TokenStream[currentIndex];
    vector<p_AstNode> valueList;

    if( token.getStrval() == ")" )
    {
        // empty list
        return valueList;
    }
    else
    {
        p_AstNode newnode = LL1_exp();
        valueList = VariableList();
        // keep the order
        valueList.insert( valueList.begin(), newnode );
    }

    return valueList;
}

p_AstNode LL1_CONS()
{
    match("cons");

    p_AstNode consTypeNode = new ASTNode( CONS, "cons" );
    //left part
    p_AstNode leftNode = LL1_exp();
    //right part
    p_AstNode rightNode = LL1_exp();

    vector<p_AstNode> child;
    child.push_back( leftNode );
    child.push_back( rightNode );
    
    consTypeNode->setChild( child );

    return consTypeNode;
}

p_AstNode LL1_CAR()
{
    match("car");
    p_AstNode consNode = LL1_exp();
    // check whether it has only two childNode
    vector<p_AstNode> Q = consNode->getChild();

    if( consNode->getTokenType() != LIST && 
        consNode->getTokenType() != CONS )
    {
        cerr << "car操作针对错误的类型。" << endl;
        exit(0);
    }
    else
    {
        // get the first element in the cons
        return Q[0];
    }
}

vector<p_AstNode> LL1_CDR()
{
    match("cdr");
    p_AstNode consNode = LL1_exp();
    // check whether it has only two childNode
    vector<p_AstNode> Q;
    if( consNode->getTokenType() != LIST &&
        consNode->getTokenType() != CONS )
    {
        cerr << "car操作针对错误的类型。" << endl;
        exit(0);
    }
    else
    {
        // get the first element in the cons
        vector<p_AstNode>child = consNode->getChild();
        for( int i=1; i<child.size(); i++ )
            Q.push_back( child[i] ); 
        return Q;
    }

}

p_AstNode LL1_LIST()
{
    match("list");
    
    p_AstNode listTypeNode = new ASTNode( LIST, "list" );

    listTypeNode -> setChild( VariableList() );

    return listTypeNode;
}


int main()
{
    string sourceCode = getSourceCodeFromStream( cin ); 
    getTokenStream( TokenStream, sourceCode );
    p_AstNode result = LL1_Lisp();

    do_something();
}

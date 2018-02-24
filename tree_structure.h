#include"virtual-function-utilities\algebraic_virtual.h"



class Node;

class Literal;

class Variable;

class Call;


using elem = mu::algebraic<Node,Literal,Variable,Call>;



class Node
{
public:

    virtual bool is_literal() const
    {
        return false;
    }
    virtual bool is_variable() const
    {
        return false;
    }
    virtual bool is_call() const
    {
        return false;
    }
    
    virtual ~Node()
    {

    }
};


class Literal : public Node
{
public:

    bool is_literal() const override
    {
        return true;
    }


};

class Variable : public Node
{
public:

    bool is_variable() const override
    {
        return true;
    }
};

class Call : public Node
{
public:

    bool is_call() const override
    {
        return true;
    }
};

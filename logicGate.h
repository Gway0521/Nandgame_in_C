#ifndef LOGICGATE_H_INCLUDED
#define LOGICGATE_H_INCLUDED

#include <iostream>

using namespace std;

// ========== Logic Gate Template ==========

int temp = 0;

class Gate
{
public:
    Gate();
    Gate *input[2];
    virtual bool output() = 0;
    virtual void setValue(Gate *gate, int pin){input[pin] = gate;};
    virtual void setValue(bool, int);
};

// ========== Basic Logic Gate ==========

class TRUE : public Gate
{
public :
    virtual bool output() {return true;}
} ;

class FALSE : public Gate
{
public :
    virtual bool output() {return false;}
} ;

Gate *t = new TRUE;
Gate *f = new FALSE;

Gate::Gate()
{
    input[0] = f;
    input[1] = f;
}

void Gate::setValue(bool val, int pin)
{
    if (val) input[pin] = t;
    else input[pin] = f;
};

class NAND : public Gate
{
public:
    bool output()
    {
        return !(input[0]->output()) || !(input[1]->output());
    }
};

// ========== Implement Logic Gate ==========

class Invert : public Gate
{
private:
    Gate *component = new NAND;
public:
    bool output()
    {
        component->setValue(input[0], 0);
        component->setValue(input[0], 1);

        return component->output();
    }
};

class AND : public Gate
{
private:
    Gate *component[2];
public:
    AND()
    {
        component[0] = new NAND;
        component[1] = new Invert;
    }
    bool output()
    {
        component[0]->setValue(input[0], 0);
        component[0]->setValue(input[1], 1);
        component[1]->setValue(component[0]->output(), 0);

        return component[1]->output();
    }
};

class OR : public Gate
{
private:
    Gate *component[3];
public:
    OR()
    {
        component[0] = new Invert;
        component[1] = new Invert;
        component[2] = new NAND;
    }
    bool output()
    {
        component[0]->setValue(input[0], 0);
        component[1]->setValue(input[1], 0);
        component[2]->setValue(component[0]->output(), 0);
        component[2]->setValue(component[1]->output(), 1);

        return component[2]->output();
    }
};

class XOR : public Gate
{
private:
    Gate *component[4];
public:
    XOR()
    {
        for (int i=0; i<4; i++)
            component[i] = new NAND;
    }
    bool output()
    {
        component[0]->setValue(input[0], 0);
        component[0]->setValue(input[1], 1);
        component[1]->setValue(input[0], 0);
        component[1]->setValue(component[0], 1);
        component[2]->setValue(input[1], 0);
        component[2]->setValue(component[0], 1);
        component[3]->setValue(component[1], 0);
        component[3]->setValue(component[2], 1);

        return component[3]->output();
    }
};

struct Bits16
{
    Gate* arr[16];
    Bits16()
    {
        for (int i=0; i<16; i++)
            arr[i] = f;
    }
    Bits16(bool input[])
    {
        for (int i=0; i<16; i++)
            arr[i] = (input[i]) ? t : f;
    }
    Gate* &operator[](int n){return arr[n];}
};

#endif // LOGICGATE_H_INCLUDED

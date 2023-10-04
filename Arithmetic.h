#ifndef ARITHMETIC_H_INCLUDED
#define ARITHMETIC_H_INCLUDED

// ========== Adder Template ==========

class Adder
{
public:
    virtual void setValue(bool val, int pin) = 0;
    virtual void setValue(Gate *gate, int pin) = 0;
};

class OneBitAdder : Adder
{
protected:
    Gate *input[3];
public:
    OneBitAdder()
    {
        for (int i=0; i<3; i++)
            input[i] = f;
    }
    virtual void setValue(bool val, int pin)
    {
        if (val) input[pin] = t;
        else input[pin] = f;
    }
    virtual void setValue(Gate *gate, int pin)
    {
        input[pin] = gate;
    }
    virtual Gate *sum() = 0;
    virtual Gate *carryOut() = 0;
};

class SixteenBitsAdder : Adder
{
protected:
    Bits16 Bits16Input[2];
public:
    SixteenBitsAdder()
    {
        for (int i=0; i<2; i++)
            for (int j=0; j<16; j++)
                Bits16Input[i][j] = f;
    }
    virtual void setValue(bool val, int pin)
    {
        if (val)
            Bits16Input[pin/16][pin%16] = t;
        else
            Bits16Input[pin/16][pin%16] = f;
    }
    virtual void setValue(Gate *gate, int pin)
    {
        Bits16Input[pin/16][pin%16] = gate;
    }
    virtual void setValue(Bits16 bits, int loc = 0)
    {
        Bits16Input[loc] = bits;
    }
    virtual Bits16 output() = 0;
    virtual Gate *carryOut() = 0;
};

// ========== Adder Implement ==========

class HalfAdder : public OneBitAdder
{
private:
    Gate *component[2];
public:
    HalfAdder()
    {
        component[0] = new XOR;
        component[1] = new AND;
    }
    virtual Gate *sum()
    {
        component[0]->setValue(input[0]->output(), 0);
        component[0]->setValue(input[1]->output(), 1);

        return component[0];
    }
    virtual Gate *carryOut()
    {
        component[1]->setValue(input[0]->output(), 0);
        component[1]->setValue(input[1]->output(), 1);

        return component[1];
    }
};

class FullAdder : public OneBitAdder
{
private:
    OneBitAdder *addComp[2];
    Gate *component;
public:
    FullAdder()
    {
        addComp[0] = new HalfAdder;
        addComp[1] = new HalfAdder;
        component = new XOR;
    }
    virtual Gate *sum()
    {
        addComp[0]->setValue(input[0]->output(), 0);
        addComp[0]->setValue(input[1]->output(), 1);
        addComp[1]->setValue(input[2]->output(), 0);
        addComp[1]->setValue(addComp[0]->sum()->output(), 1);

        return addComp[1]->sum();
    }
    virtual Gate *carryOut()
    {
        addComp[0]->setValue(input[0]->output(), 0);
        addComp[0]->setValue(input[1]->output(), 1);
        addComp[1]->setValue(input[2]->output(), 0);
        addComp[1]->setValue(addComp[0]->sum()->output(), 1);
        component->setValue(addComp[0]->carryOut()->output(), 0);
        component->setValue(addComp[1]->carryOut()->output(), 1);

        return component;
    }
};

class RippleAdder : public SixteenBitsAdder
{
private:
    OneBitAdder *addComp[16];
public:
    RippleAdder()
    {
        for (int i=0; i<16; i++)
            addComp[i] = new FullAdder;
    }
    virtual Bits16 output()
    {
        _out();
        Bits16 outBits;
        for (int i=0; i<16; i++)
            outBits[i] = addComp[i]->sum();

        return outBits;
    }
    virtual Gate *operator[](int n)
    {
        _out();

        return addComp[n]->sum();
    }
    virtual Gate *carryOut()
    {
        _out();

        return addComp[15]->carryOut();
    }
    void _out()
    {
        for (int i=0; i<16; i++)
        {
            addComp[i]->setValue(Bits16Input[0][i]->output(), 0);
            addComp[i]->setValue(Bits16Input[1][i]->output(), 1);
        }

        addComp[0]->setValue(false, 2);
        for (int i=0; i<15; i++)
            addComp[i+1]->setValue(addComp[i]->carryOut()->output(), 2);
    }
};

class Increment : public SixteenBitsAdder
{
private:
    RippleAdder *addComp;
    Gate *gateComp;
public:
    Increment()
    {
        addComp = new RippleAdder;
        gateComp = new Invert;
    }
    virtual Bits16 output()
    {
        _out();

        return addComp->output();
    }
    virtual Gate *operator[](int n)
    {
        _out();

        return (*addComp)[n];
    }
    virtual Gate *carryOut()
    {
        _out();

        return addComp->carryOut();
    }
    void _out()
    {
        addComp->setValue(Bits16Input[0], 0);
        addComp->setValue(gateComp->output(), 16);
    }
};

class Subtraction : public SixteenBitsAdder
{
private:
    RippleAdder *addComp;
    Increment *incComp;
    Gate *gateComp[16];
public:
    Subtraction()
    {
        addComp = new RippleAdder;
        incComp = new Increment;
        for (int i=0; i<16; i++)
            gateComp[i] = new Invert;
    }
    virtual Bits16 output()
    {
        _out();

        return addComp->output();
    }
    virtual Gate *operator[](int n)
    {
        _out();

        return (*addComp)[n];
    }
    virtual Gate *carryOut()
    {
        _out();

        return addComp -> carryOut();
    }
    void _out()
    {
        addComp->setValue(Bits16Input[0], 0);
        for (int i=0; i<16; i++)
            gateComp[i]->setValue(Bits16Input[1][i]->output(), 0);
        for (int i=0; i<16; i++)
            incComp->setValue(gateComp[i]->output(), i);
        addComp->setValue(incComp->output(), 1);
    }
};

class EqualZero
{
private:
    Gate *gateComp[16];
public:
    EqualZero()
    {
        for (int i=0; i<15; i++)
            gateComp[i] = new OR;
        gateComp[15] = new Invert;
    }
    virtual void setValue(bool val, int pin){gateComp[pin/2]->setValue(val, pin%2);}
    virtual void setValue(Gate *gate, int pin){gateComp[pin/2]->setValue(gate, pin%2);}
    virtual void setValue(Bits16 bits, int loc = 0)
    {
        for (int i=0; i<16; i++)
            setValue(bits[i], i);
    }
    virtual Gate *output()
    {
        gateComp[8]->setValue(gateComp[0]->output(), 0);
        gateComp[8]->setValue(gateComp[1]->output(), 1);
        gateComp[9]->setValue(gateComp[2]->output(), 0);
        gateComp[9]->setValue(gateComp[3]->output(), 1);
        gateComp[10]->setValue(gateComp[4]->output(), 0);
        gateComp[10]->setValue(gateComp[5]->output(), 1);
        gateComp[11]->setValue(gateComp[6]->output(), 0);
        gateComp[11]->setValue(gateComp[7]->output(), 1);
        gateComp[12]->setValue(gateComp[8]->output(), 0);
        gateComp[12]->setValue(gateComp[9]->output(), 1);
        gateComp[13]->setValue(gateComp[10]->output(), 0);
        gateComp[13]->setValue(gateComp[11]->output(), 1);
        gateComp[14]->setValue(gateComp[12]->output(), 0);
        gateComp[14]->setValue(gateComp[13]->output(), 1);
        gateComp[15]->setValue(gateComp[14]->output(), 0);

        return gateComp[15];
    }
};

class LessThanZero
{
public:
    Gate *ans;
public:
    virtual void setValue(bool val, int pin){if (pin==15 && val) ans = t; else if (pin == 15 && !val) ans = f;}
    virtual void setValue(Gate *gate, int pin){if (pin==15) ans = gate;}
    virtual void setValue(Bits16 bits, int loc = 0)
    {
        for (int i=0; i<16; i++)
            setValue(bits[i]->output(), i);
    }
    virtual Gate *output(){return ans;}
};

#endif // ARITHMETIC_H_INCLUDED

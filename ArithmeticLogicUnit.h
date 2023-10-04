#ifndef ARITHMETICLOGICUNIT_H_INCLUDED
#define ARITHMETICLOGICUNIT_H_INCLUDED

class LogicUnit
{
private:
    Gate *invt[16];
    Gate *xorComp[16];
    Gate *orComp[16];
    Gate *andComp[16];
    Mux4_1 *mux4_1;
public:
    LogicUnit()
    {
        for (int i=0; i<16; i++)
        {
            invt[i] = new Invert;
            xorComp[i] = new XOR;
            orComp[i] = new OR;
            andComp[i] = new AND;
        }
        mux4_1 = new Mux4_1;
    }
    virtual void setValue(bool val, int pin)
    {
        if (pin < 16)
        {
            invt[pin]->setValue(val, 0);
            xorComp[pin]->setValue(val, 0);
            orComp[pin]->setValue(val, 0);
            andComp[pin]->setValue(val, 0);
        }
        else if (pin < 32)
        {
            xorComp[pin-16]->setValue(val, 1);
            orComp[pin-16]->setValue(val, 1);
            andComp[pin-16]->setValue(val, 1);
        }
    }
    virtual void setValue(Gate *gate, int pin)
    {
        if (pin < 16)
        {
            invt[pin]->setValue(gate, 0);
            xorComp[pin]->setValue(gate, 0);
            orComp[pin]->setValue(gate, 0);
            andComp[pin]->setValue(gate, 0);
        }
        else if (pin < 32)
        {
            xorComp[pin-16]->setValue(gate, 1);
            orComp[pin-16]->setValue(gate, 1);
            andComp[pin-16]->setValue(gate, 1);
        }
    }
    virtual void setValue(Bits16 bits, int loc)
    {
        for (int i=0; i<16; i++)
            setValue(bits[i], i+loc*16);
    }
    virtual void setOP(bool  val,  int pin){mux4_1->setSelect(val, pin);}
    virtual void setOP(Gate *gate, int pin){mux4_1->setSelect(gate, pin);}
    virtual Bits16 output()
    {
        _out();

        return mux4_1->output();
    }
    virtual Gate *operator[](int n)
    {
        _out();
        return (*mux4_1)[n];
    }
    void _out()
    {
        for (int i=0; i<16; i++)
            mux4_1->setValue(andComp[i]->output(), i);
        for (int i=0; i<16; i++)
            mux4_1->setValue(orComp[i]->output(), i+16);
        for (int i=0; i<16; i++)
            mux4_1->setValue(xorComp[i]->output(), i+32);
        for (int i=0; i<16; i++)
            mux4_1->setValue(invt[i]->output(), i+48);
    }
};

class ArithmeticUnit
{
private:
    RippleAdder *add16[2];
    Subtraction *sub16[2];
    Mux4_1 *mux4_1;
    Gate *invt;
public:
    ArithmeticUnit()
    {
        add16[0] = new RippleAdder;
        add16[1] = new RippleAdder;
        sub16[0] = new Subtraction;
        sub16[1] = new Subtraction;
        mux4_1 = new Mux4_1;
        invt = new Invert;
    }
    virtual void setValue(bool val, int pin)
    {
        if (pin < 16)
        {
            add16[0]->setValue(val, pin);
            add16[1]->setValue(val, pin);
            sub16[0]->setValue(val, pin);
            sub16[1]->setValue(val, pin);
        }
        else if (pin < 32)
        {
            add16[0]->setValue(val, pin);
            sub16[0]->setValue(val, pin);
        }
    }
    virtual void setValue(Gate *gate, int pin)
    {
        if (pin < 16)
        {
            add16[0]->setValue(gate, pin);
            add16[1]->setValue(gate, pin);
            sub16[0]->setValue(gate, pin);
            sub16[1]->setValue(gate, pin);
        }
        else if (pin < 32)
        {
            add16[0]->setValue(gate, pin);
            sub16[0]->setValue(gate, pin);
        }
    }
    virtual void setValue(Bits16 bits, int loc)
    {
        for (int i=0; i<16; i++)
            setValue(bits[i], i+loc*16);
    }
    virtual void setOP(bool val, int pin){mux4_1->setSelect(val, pin);}
    virtual void setOP(Gate *gate, int pin){mux4_1->setSelect(gate, pin);}
    virtual Bits16 output()
    {
        _out();

        return mux4_1->output();
    }
    virtual Gate *operator[](int n)
    {
        _out();

        return (*mux4_1)[n];
    }
    void _out()
    {
        add16[1]->setValue(invt->output(), 16);
        sub16[1]->setValue(invt->output(), 16);
        mux4_1->setValue(add16[0]->output(), 0);
        mux4_1->setValue(add16[1]->output(), 1);
        mux4_1->setValue(sub16[0]->output(), 2);
        mux4_1->setValue(sub16[1]->output(), 3);
    }
};

class ALU
{
private:
    Mux2_1 *mux2_1[4];
    LogicUnit *lu;
    ArithmeticUnit *au;
public:
    ALU()
    {
        for (int i=0; i<4; i++)
            mux2_1[i] = new Mux2_1;
        lu = new LogicUnit;
        au = new ArithmeticUnit;
    }
    virtual void setValue(bool val, int pin)
    {
        if (pin < 16)
        {
            mux2_1[0]->setValue(val, pin);
            mux2_1[1]->setValue(val, pin+16);
        }
        else if (pin < 32)
        {
            mux2_1[0]->setValue(val, pin);
            mux2_1[1]->setValue(val, pin-16);
        }
    }
    virtual void setValue(Gate *gate, int pin)
    {
        if (pin < 16)
        {
            mux2_1[0]->setValue(gate, pin);
            mux2_1[1]->setValue(gate, pin+16);
        }
        else if (pin < 32)
        {
            mux2_1[0]->setValue(gate, pin);
            mux2_1[1]->setValue(gate, pin-16);
        }
    }
    virtual void setValue(Bits16 bits, int loc)
    {
        for (int i=0; i<16; i++)
            setValue(bits[i], i+loc*16);
    }
    virtual void setOP(bool val, int pin)
    {
        lu->setOP(val, pin);
        au->setOP(val, pin);
    }
    virtual void setOP(Gate *gate, int pin)
    {
        lu->setOP(gate, pin);
        au->setOP(gate, pin);
    }
    virtual void setU(bool val){mux2_1[3]->setSelect(val);}
    virtual void setU(Gate *gate){mux2_1[3]->setSelect(gate);}
    virtual void setZX(bool val){mux2_1[2]->setSelect(val);}
    virtual void setZX(Gate *gate){mux2_1[2]->setSelect(gate);}
    virtual void setSW(bool val)
    {
        mux2_1[0]->setSelect(val);
        mux2_1[1]->setSelect(val);
    }
    virtual void setSW(Gate *gate)
    {
        mux2_1[0]->setSelect(gate);
        mux2_1[1]->setSelect(gate);
    }
    virtual Gate *operator[](int n)
    {
        _out();

        return (*mux2_1[3])[n];
    }
    virtual Bits16 output()
    {
        _out();

        return mux2_1[3]->output();
    }
    void _out()
    {
        mux2_1[2]->setValue(mux2_1[0]->output(), 0);
        au->setValue(mux2_1[2]->output(), 0);
        au->setValue(mux2_1[1]->output(), 1);
        lu->setValue(mux2_1[2]->output(), 0);
        lu->setValue(mux2_1[1]->output(), 1);
        mux2_1[3]->setValue(lu->output(), 0);
        mux2_1[3]->setValue(au->output(), 1);
    }
};

class Condition
{
private:
    Gate *andComp[3];
    Gate *orComp[3];
    Gate *invt;
    EqualZero *isZero;
    LessThanZero *isNeg;
public:
    Condition()
    {
        for (int i=0; i<3; i++)
        {
            andComp[i] = new AND;
            orComp[i] = new OR;
        }
        invt = new Invert;
        isZero = new EqualZero;
        isNeg = new LessThanZero;
    }
    virtual void setValue(bool val, int pin)
    {
        isZero->setValue(val, pin);
        isNeg->setValue(val, pin);
    }
    virtual void setValue(Gate *gate, int pin)
    {
        isZero->setValue(gate, pin);
        isNeg->setValue(gate, pin);
    }
    virtual void setValue(Bits16 bits, int loc)
    {
        for (int i=0; i<16; i++)
            setValue(bits[i], i+loc*16);
    }
    virtual void setLT(bool val){andComp[0]->setValue(val, 0);}
    virtual void setLT(Gate *gate){andComp[0]->setValue(gate, 0);}
    virtual void setEQ(bool val){andComp[1]->setValue(val, 0);}
    virtual void setEQ(Gate *gate){andComp[1]->setValue(gate, 0);}
    virtual void setGT(bool val){andComp[2]->setValue(val, 0);}
    virtual void setGT(Gate *gate){andComp[2]->setValue(gate, 0);}
    virtual Gate *output()
    {
        _out();

        return orComp[2];
    }
    void _out()
    {
        orComp[0]->setValue(isZero->output()->output(), 0);
        orComp[0]->setValue(isNeg->output()->output(), 1);
        invt->setValue(orComp[0]->output(), 0);
        andComp[2]->setValue(invt->output(), 1);
        andComp[0]->setValue(isNeg->output()->output(), 1);
        andComp[1]->setValue(isZero->output()->output(), 1);
        orComp[1]->setValue(andComp[0]->output(), 0);
        orComp[1]->setValue(andComp[1]->output(), 1);
        orComp[2]->setValue(orComp[1]->output(), 0);
        orComp[2]->setValue(andComp[2]->output(), 1);
    }
};

#endif // ARITHMETICLOGICUNIT_H_INCLUDED

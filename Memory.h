#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

// ========== Memory Template ==========

class Memory
{
protected:
    Gate *store;
    Gate *clk;
public:
    Memory()
    {
        store = f;
        clk = f;
    }
    virtual void clock(){};
    virtual void setStore(Gate *gate){store = gate;clock();};
    virtual void setStore(bool val){if (val) store = t; else store = f;clock();};
    virtual void setclk(Gate *gate){clk = gate;clock();};
    virtual void setclk(bool val){if (val) clk = t; else clk = f;clock();};
};

class OneBitMemory : public Memory
{
protected:
    Gate *data;
public:
    OneBitMemory()
    {
        data = f;
    }
    virtual void clock() = 0;
    virtual Gate *output() = 0;
    virtual void setValue(Gate *gate, int pin = 0){data = gate;};
    virtual void setValue(bool val, int pin = 0){if (val) data = t; else data = f;};
};

class SixteenBitsMemory : public Memory
{
public:
    virtual void clock() = 0;
    virtual Bits16 output() = 0;
    virtual void setValue(Gate *gate, int pin) = 0;
    virtual void setValue(bool val, int pin) = 0;
    virtual void setValue(Bits16 bits, int loc) = 0;
};

// ========== Memory Implement ==========
/*
class Latch : public OneBitMemory
{
private:
    Gate *component[4];
public:
    Latch()
    {
        for (int i=0; i<4; i++)
            component[i] = new NAND;
        clock();
    }
    void clock()
    {
        component[0]->setValue(store, 0);
        component[1]->setValue(store, 0);
        component[1]->setValue(data, 1);
        component[0]->setValue(component[1]->output(), 1);

        component[2]->setValue(component[0]->output(), 0);
        component[3]->setValue(component[1]->output(), 0);
        component[3]->setValue(component[2]->output(), 1);
        component[2]->setValue(component[3]->output(), 1);

        component[3]->setValue(component[2]->output(), 1);
    }
    Gate *output()
    {
        clock();

        if (component[3]->output())
            return t;
        else
            return f;
    }
};

class FlipFlop : public OneBitMemory
{
private:
    Latch *latchComp[2];
    Gate *logicComp[3];
public:
    FlipFlop()
    {
        latchComp[0] = new Latch;
        latchComp[1] = new Latch;
        logicComp[0] = new Invert;
        logicComp[1] = new AND;
        logicComp[2] = new AND;
        clock();
    }
    void clock()
    {
        logicComp[0]->setValue(clk, 0);
        logicComp[1]->setValue(store, 0);
        logicComp[1]->setValue(logicComp[0]->output(), 1);
        logicComp[2]->setValue(store, 0);
        logicComp[2]->setValue(clk, 1);

        latchComp[0]->setValue(data);
        latchComp[0]->setStore(logicComp[1]->output());
        latchComp[1]->setValue(latchComp[0]->output());
        latchComp[1]->setStore(logicComp[2]->output());
    }
    Gate *output()
    {
        clock();

        return latchComp[1]->output();
    }
};

class Register : public SixteenBitsMemory
{
private:
    FlipFlop *FFComp[16];
public:
    Register()
    {
        for (int i=0; i<16; i++)
            FFComp[i] = new FlipFlop;
        clock();
    }
    virtual void setValue(Gate  *gate, int pin)    {FFComp[pin]->setValue(gate);};
    virtual void setValue(bool   val,  int pin)    {if (val) FFComp[pin]->setValue(t); else FFComp[pin]->setValue(f);};
    virtual void setValue(Bits16 bits, int loc = 0){for (int i=0; i<16; i++) setValue(bits[i], i+loc*16);}
    void clock()
    {
        for (int i=0; i<16; i++)
        {
            FFComp[i]->setStore(store);
            FFComp[i]->setclk(clk);
        }
    }
    virtual Gate *operator[](int n)
    {
        clock();

        return FFComp[n]->output();
    }
    virtual Bits16 output()
    {
        clock();
        Bits16 bits;
        for (int i=0; i<16; i++)
            bits[i] = FFComp[i]->output();

        return bits;
    }
};

class Counter : public SixteenBitsMemory
{
private:
    Register *reg;
    Mux2_1 *sel;
    Increment *inc;
    Gate *invt;
public:
    Counter()
    {
        reg = new Register;
        sel = new Mux2_1;
        inc = new Increment;
        invt = new Invert;
    }
    virtual void setValue (Gate  *gate, int pin)    {sel->setValue(gate, 16+pin);};
    virtual void setValue (bool   val,  int pin)    {if (val) sel->setValue(t, 16+pin); else sel->setValue(f, 16+pin);};
    virtual void setValue (Bits16 bits, int loc = 0){for (int i=0; i<16; i++) setValue(bits[i], i+loc*16);}
    void clock()
    {
        inc->setValue(reg->output());
        sel->setValue(inc->output(), 0);
        sel->setSelect(store);
        reg->setValue(sel->output());
        reg->setStore(invt);
        reg->setclk(clk);
    }
    virtual Gate *operator[](int n)
    {
        clock();

        return (*reg)[n];
    }
    virtual Bits16 output()
    {
        clock();

        return reg->output();
    }
};

class RAM : public SixteenBitsMemory
{
private:
    Mux32_1 *mux;
    Decoder5_32 *dec;
public:
    Register *reg[32];

    RAM()
    {
        for (int i=0; i<32; i++)
            reg[i] = new Register;
        mux = new Mux32_1;
        dec = new Decoder5_32;
    }
    virtual void setValue  (Gate  *gate, int pin)    {for (int i=0; i<32; i++) reg[i]->setValue(gate, pin);}
    virtual void setValue  (bool   val,  int pin)    {for (int i=0; i<32; i++) reg[i]->setValue(val, pin);}
    virtual void setValue  (Bits16 bits, int loc = 0){for (int i=0; i<32; i++) reg[i]->setValue(bits);}
    virtual void setAddress(Gate  *gate, int pin)
    {
        if (pin < 5)
        {
            dec->setValue(gate, pin);
            mux->setSelect(gate, pin);
        }
    };
    virtual void setAddress(bool val, int pin)
    {
        if (pin < 5)
        {
            dec->setValue(val, pin);
            mux->setSelect(val, pin);
        }
    };
    virtual void setAddress(Bits16 bits)
    {
        for (int i=0; i<5; i++)
            setAddress(bits[i], i);
    };
    void clock()
    {
        dec->setEnable(store);
        Bits16 bits0 = dec->output(0);
        Bits16 bits1 = dec->output(1);
        for (int i=0; i<16; i++)
            reg[i]->setStore(bits0[i]);
        for (int i=0; i<16; i++)
            reg[i+16]->setStore(bits1[i]);
        for (int i=0; i<32; i++)
            reg[i]->setclk(clk);
        for (int i=0; i<32; i++)
            mux->setValue(reg[i]->output(), i);
    }
    virtual Gate *operator[](int n)
    {
        clock();

        return (*mux)[n];
    }
    virtual Bits16 output()
    {
        clock();

        return mux->output();
    }
};
*/



class Latch : public OneBitMemory
{
private:
    Gate *component[4];
public:
    Latch()
    {
        for (int i=0; i<4; i++)
            component[i] = new NAND;
        clock();
    }
    void clock()
    {
        component[0]->setValue(store, 0);
        component[1]->setValue(store, 0);
        component[1]->setValue(data, 1);
        component[0]->setValue(component[1]->output(), 1);

        component[2]->setValue(component[0]->output(), 0);
        component[3]->setValue(component[1]->output(), 0);
        component[3]->setValue(component[2]->output(), 1);
        component[2]->setValue(component[3]->output(), 1);

        component[3]->setValue(component[2]->output(), 1);
    }
    Gate *output()
    {
        clock();

        if (component[3]->output())
            return t;
        else
            return f;
    }
};

class FlipFlop : public OneBitMemory
{
private:
    Latch *latchComp[2];
    Gate *logicComp[3];
public:
    FlipFlop()
    {
        latchComp[0] = new Latch;
        latchComp[1] = new Latch;
        logicComp[0] = new Invert;
        logicComp[1] = new AND;
        logicComp[2] = new AND;
        clock();
    }
    void clock()
    {
        logicComp[0]->setValue(clk, 0);
        logicComp[1]->setValue(store, 0);
        logicComp[1]->setValue(logicComp[0]->output(), 1);
        logicComp[2]->setValue(store, 0);
        logicComp[2]->setValue(clk, 1);

        latchComp[0]->setValue(data);
        latchComp[0]->setStore(logicComp[1]->output());
        latchComp[1]->setValue(latchComp[0]->output());
        latchComp[1]->setStore(logicComp[2]->output());
    }
    Gate *output()
    {
        clock();

        return latchComp[1]->output();
    }
};

class Register : public SixteenBitsMemory
{
private:
    FlipFlop *FFComp[16];
public:
    Register()
    {
        for (int i=0; i<16; i++)
            FFComp[i] = new FlipFlop;
        clock();
    }
    virtual void setValue(Gate  *gate, int pin)    {FFComp[pin]->setValue(gate);};
    virtual void setValue(bool   val,  int pin)    {if (val) FFComp[pin]->setValue(t); else FFComp[pin]->setValue(f);};
    virtual void setValue(Bits16 bits, int loc = 0){for (int i=0; i<16; i++) setValue(bits[i], i+loc*16);}
    void clock()
    {
        for (int i=0; i<16; i++)
        {
            FFComp[i]->setStore(store);
            FFComp[i]->setclk(clk);
        }
    }
    virtual Gate *operator[](int n)
    {
        clock();

        return FFComp[n]->output();
    }
    virtual Bits16 output()
    {
        clock();
        Bits16 bits;
        for (int i=0; i<16; i++)
            bits[i] = FFComp[i]->output();

        return bits;
    }
};

class Counter : public SixteenBitsMemory
{
private:
    Register *reg;
    Mux2_1 *sel;
    Increment *inc;
    Gate *invt;
public:
    Counter()
    {
        reg = new Register;
        sel = new Mux2_1;
        inc = new Increment;
        invt = new Invert;
    }
    virtual void setValue (Gate  *gate, int pin)    {sel->setValue(gate, 16+pin);};
    virtual void setValue (bool   val,  int pin)    {if (val) sel->setValue(t, 16+pin); else sel->setValue(f, 16+pin);};
    virtual void setValue (Bits16 bits, int loc = 0){for (int i=0; i<16; i++) setValue(bits[i], i+loc*16);}
    void clock()
    {
        inc->setValue(reg->output());
        sel->setValue(inc->output(), 0);
        sel->setSelect(store);
        reg->setValue(sel->output());
        reg->setStore(invt);
        reg->setclk(clk);
    }
    virtual Gate *operator[](int n)
    {
        clock();

        return (*reg)[n];
    }
    virtual Bits16 output()
    {
        clock();

        return reg->output();
    }
};

class RAM : public SixteenBitsMemory
{
private:
    Mux32_1 *mux;
    Decoder5_32 *dec;
public:
    Register *reg[32];

    RAM()
    {
        for (int i=0; i<32; i++)
            reg[i] = new Register;
        mux = new Mux32_1;
        dec = new Decoder5_32;
    }
    virtual void setValue  (Gate  *gate, int pin)    {for (int i=0; i<32; i++) reg[i]->setValue(gate, pin);}
    virtual void setValue  (bool   val,  int pin)    {for (int i=0; i<32; i++) reg[i]->setValue(val, pin);}
    virtual void setValue  (Bits16 bits, int loc = 0){for (int i=0; i<32; i++) reg[i]->setValue(bits);}
    virtual void setAddress(Gate  *gate, int pin)
    {
        if (pin < 5)
        {
            dec->setValue(gate, pin);
            mux->setSelect(gate, pin);
        }
    };
    virtual void setAddress(bool val, int pin)
    {
        if (pin < 5)
        {
            dec->setValue(val, pin);
            mux->setSelect(val, pin);
        }
    };
    virtual void setAddress(Bits16 bits)
    {
        for (int i=0; i<5; i++)
            setAddress(bits[i], i);
    };
    void clock()
    {
        dec->setEnable(store);
        Bits16 bits0 = dec->output(0);
        Bits16 bits1 = dec->output(1);
        for (int i=0; i<16; i++)
            reg[i]->setStore(bits0[i]);
        for (int i=0; i<16; i++)
            reg[i+16]->setStore(bits1[i]);
        for (int i=0; i<32; i++)
            reg[i]->setclk(clk);
        for (int i=0; i<32; i++)
            mux->setValue(reg[i]->output(), i);
    }
    virtual Gate *operator[](int n)
    {
        clock();

        return (*mux)[n];
    }
    virtual Bits16 output()
    {
        clock();

        return mux->output();
    }
};


#endif // MEMORY_H_INCLUDED

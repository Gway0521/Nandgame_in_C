#ifndef PROCESSOR_H_INCLUDED
#define PROCESSOR_H_INCLUDED

class CombinedMemory : public Memory
{
private:
public:
    Register *reg[2];
    RAM *ram;

    CombinedMemory()
    {
        reg[0] = new Register;
        reg[1] = new Register;
        ram = new RAM;
    }
    virtual void setValue(bool val, int pin)
    {
        reg[0]->setValue(val, pin);
        reg[1]->setValue(val, pin);
        ram->setValue(val, pin);
    }
    virtual void setValue(Gate *gate, int pin)
    {
        reg[0]->setValue(gate, pin);
        reg[1]->setValue(gate, pin);
        ram->setValue(gate, pin);
    }
    virtual void setValue(Bits16 bits, int loc = 0)
    {
        reg[0]->setValue(bits);
        reg[1]->setValue(bits);
        ram->setValue(bits);
    }
    virtual void setRegA(bool val){reg[0]->setStore(val);}
    virtual void setRegA(Gate *gate){reg[0]->setStore(gate);}
    virtual void setRegD(bool val){reg[1]->setStore(val);}
    virtual void setRegD(Gate *gate){reg[1]->setStore(gate);}
    virtual void setRegLA(bool val){ram->setStore(val);}
    virtual void setRegLA(Gate *gate){ram->setStore(gate);}
    void clock()
    {
        ram->setAddress(reg[0]->output());
        reg[0]->setclk(clk);
        reg[1]->setclk(clk);
        ram->setclk(clk);
    }
    virtual Gate *operator[](int n)
    {
        clock();
        if (n < 16)
            return (*reg[0])[n];
        else if (n < 32)
            return (*reg[1])[n-16];
        else if (n < 48)
            return (*ram)[n-32];
    }
    virtual Bits16 outputRegA() {clock(); return reg[0]->output();}
    virtual Bits16 outputRegD() {clock(); return reg[1]->output();}
    virtual Bits16 outputRegLA(){clock(); return ram->output();}
};

class ALUInstruction
{
private:
    ALU *alu;
    Condition *cond;
    Mux2_1 *mux2_1;
    Gate *dst[3];
public:
    ALUInstruction()
    {
        alu = new ALU;
        cond = new Condition;
        mux2_1 = new Mux2_1;
    }
    virtual void setValue(bool val, int pin)
    {
        if (pin < 16)
            mux2_1->setValue(val, pin);
        else if (pin < 32)
            alu->setValue(val, pin-16);
        else if (pin < 48)
            mux2_1->setValue(val, pin-16);
    }
    virtual void setValue(Gate *gate, int pin)
    {
        if (pin < 16)
            mux2_1->setValue(gate, pin);
        else if (pin < 32)
            alu->setValue(gate, pin-16);
        else if (pin < 48)
            mux2_1->setValue(gate, pin-16);
    }
    virtual void setValue(Bits16 bits, int loc)
    {
        if      (loc == 0) mux2_1->setValue(bits, 0);
        else if (loc == 1) alu->setValue(bits, 0);
        else if (loc == 2) mux2_1->setValue(bits, 1);
    }
    virtual void setInstruction(bool val, int pin)
    {
        switch(pin)
        {
            case 0  : cond->setGT(val); break;
            case 1  : cond->setEQ(val); break;
            case 2  : cond->setLT(val); break;
            case 3  : dst[0] = (val) ? t : f; break;
            case 4  : dst[1] = (val) ? t : f; break;
            case 5  : dst[2] = (val) ? t : f; break;
            case 6  : alu->setSW(val); break;
            case 7  : alu->setZX(val); break;
            case 8  : alu->setOP(val, 0); break;
            case 9  : alu->setOP(val, 1); break;
            case 10 : alu->setU(val); break;
            case 12 : mux2_1->setSelect(val); break;
            default : ;
        }
    }
    virtual void setInstruction(Gate *gate, int pin)
    {
        switch(pin)
        {
            case 0  : cond->setGT(gate); break;
            case 1  : cond->setEQ(gate); break;
            case 2  : cond->setLT(gate); break;
            case 3  : dst[0] = gate; break;
            case 4  : dst[1] = gate; break;
            case 5  : dst[2] = gate; break;
            case 6  : alu->setSW(gate); break;
            case 7  : alu->setZX(gate); break;
            case 8  : alu->setOP(gate, 0); break;
            case 9  : alu->setOP(gate, 1); break;
            case 10 : alu->setU(gate); break;
            case 12 : mux2_1->setSelect(gate); break;
            default : ;
        }
    }
    virtual void setInstruction(Bits16 bits)
    {
        for (int i=0; i<13; i++)
            setInstruction(bits[i], i);
    }
    virtual Bits16 output()
    {
        alu->setValue(mux2_1->output(), 1);

        return alu->output();
    }
    virtual Gate *operator[](int n)
    {
        alu->setValue(mux2_1->output(), 1);

        return (*alu)[n];
    }
    virtual Gate *getRegLA(){return dst[0];}
    virtual Gate *getRegD(){return dst[1];}
    virtual Gate *getRegA(){return dst[2];}
    virtual Gate *getJ()
    {
        alu->setValue(mux2_1->output(), 1);
        cond->setValue(alu->output(), 0);

        return cond->output();
    }
};

class ControlUnit
{
private:
    Mux2_1 *mux2_1;
    Selector *sel[4];
    ALUInstruction *aluI;
    Gate *invt;
public:
    ControlUnit()
    {
        mux2_1 = new Mux2_1;
        for (int i=0; i<4; i++)
            sel[i] = new Selector;
        aluI = new ALUInstruction;
        invt = new Invert;
    }
    virtual void setValue(bool   val,  int pin){aluI->setValue(val, pin);}
    virtual void setValue(Gate  *gate, int pin){aluI->setValue(gate, pin);}
    virtual void setValue(Bits16 bits, int loc){aluI->setValue(bits, loc);}
    virtual void setInstruction(bool val, int pin)
    {
        aluI->setInstruction(val, pin);
        mux2_1->setValue(val, pin);
        if (pin == 15)
        {
            mux2_1->setSelect(val);
            for (int i=0; i<4; i++)
                sel[i]->setSelect(val);
        }
    }
    virtual void setInstruction(Gate *gate, int pin)
    {
        aluI->setInstruction(gate, pin);
        mux2_1->setValue(gate, pin);
        if (pin == 15)
        {
            mux2_1->setSelect(gate);
            for (int i=0; i<4; i++)
                sel[i]->setSelect(gate);
        }
    }
    virtual void setInstruction(Bits16 bits)
    {
        for (int i=0; i<16; i++)
            setInstruction(bits[i], i);
    }
    virtual Gate *operator[](int n)
    {
        _out();

        return (*mux2_1)[n];
    }
    virtual Bits16 output()
    {
        _out();

        return mux2_1->output();
    }
    virtual Gate *getRegA(){_out(); return sel[0]->output();}
    virtual Gate *getRegD(){_out(); return sel[1]->output();}
    virtual Gate *getRegLA(){_out(); return sel[2]->output();}
    virtual Gate *getJ(){_out(); return sel[3]->output();}
    void _out()
    {
        mux2_1->setValue(aluI->output(), 1);
        sel[0]->setValue(invt, 0);
        sel[0]->setValue(aluI->getRegA(), 1);
        sel[1]->setValue(aluI->getRegD(), 1);
        sel[2]->setValue(aluI->getRegLA(), 1);
        sel[3]->setValue(aluI->getJ(), 1);
    }
};

class Computer
{
private:

public:
    ControlUnit *cu;
    Counter *ct;
    Gate* clk;
    RAM *rom;
    CombinedMemory *memory;

    Computer()
    {
        rom = new RAM;
        cu = new ControlUnit;
        memory = new CombinedMemory;
        ct = new Counter;
        clk = f;
    }
    void clock()
    {
        for (int i=0; i<2; i++)
        {
            rom->setAddress(ct->output());
            cu->setInstruction(rom->output());
            cu->setValue(memory->outputRegA(), 0);
            cu->setValue(memory->outputRegD(), 1);
            cu->setValue(memory->outputRegLA(), 2);
            memory->setValue(cu->output());
            memory->setRegA(cu->getRegA());
            memory->setRegD(cu->getRegD());
            memory->setRegLA(cu->getRegLA());
            ct->setValue(memory->outputRegA());
            ct->setStore(cu->getJ());
            memory->setclk(clk);
            ct->setclk(clk);

            if (clk->output())
                clk = f;
            else
                clk = t;
        }
    }
};

#endif // PROCESSOR_H_INCLUDED

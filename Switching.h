#ifndef SWITCHING_H_INCLUDED
#define SWITCHING_H_INCLUDED

// ========== Decoder and Mux Template ==========

class Decoder
{
protected:
    Gate *enable;
public:
    virtual void setValue(bool, int) = 0;
    virtual void setValue(Gate *, int) = 0;
    virtual void setEnable (bool  val){if (val) enable = t; else enable = f;}
    virtual void setEnable (Gate *gate){enable = gate;}
    virtual Gate *operator[](int) = 0;
};

class Mux
{
protected:
    Gate *select;
public:
    virtual void setSelect(bool val)
    {
        if (val) select = t;
        else select = f;
    }
    virtual void setSelect(Gate *gate)
    {
        select = gate;
    }
};

class OneBitMux : public Mux
{
protected:
    Gate *input[2];
public:
    OneBitMux()
    {
        input[0] = f;
        input[1] = f;
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
    virtual Gate *output() = 0;
};

class SixteenBitsMux : public Mux
{
public:
    virtual void setValue(bool val, int pin) = 0;
    virtual void setValue(Gate *gate, int pin) = 0;
    virtual void setValue(Bits16 bits, int loc) = 0;
    virtual Bits16 output() = 0;
};

// ========== Decoder and Mux Implement ==========

class Selector : public OneBitMux
{
private:
    Gate *component[4];
public:
    Selector()
    {
        component[0] = new AND;
        component[1] = new AND;
        component[2] = new Invert;
        component[3] = new OR;
    }
    virtual Gate *output()
    {
        component[1]->setValue(select->output(), 1) ;
        component[2]->setValue(select->output(), 0) ;
        component[0]->setValue(component[2]->output(), 1);

        component[0]->setValue(input[0]->output(), 0);
        component[1]->setValue(input[1]->output(), 0);

        component[3] -> setValue(component[0]->output(), 0) ;
        component[3] -> setValue(component[1]->output(), 1) ;
        return component[3] ;
    }
};

class Mux2_1 : public SixteenBitsMux
{
private:
    Selector *component[16];
public:
    Mux2_1()
    {
        for (int i=0; i<16; i++)
            component[i] = new Selector;
    }
    virtual void  setValue  (bool   val,  int pin){component[pin%16]->setValue(val, pin/16);}
    virtual void  setValue  (Gate  *gate, int pin){component[pin%16]->setValue(gate, pin/16);}
    virtual void  setValue  (Bits16 bits, int loc){for (int i=0; i<16; i++) component[i]->setValue(bits[i], loc);}
    virtual void  setSelect (bool   val)          {for (int i=0; i<16; i++) component[i]->setSelect(val);}
    virtual void  setSelect (Gate  *gate)         {for (int i=0; i<16; i++) component[i]->setSelect(gate);}
    virtual Gate *operator[](int    n)            {return component[n]->output();}
    virtual Bits16 output()
    {
        Bits16 outBits;
        for (int i=0; i<16; i++)
            outBits[i] = component[i]->output();

        return outBits;
    };
};

class Mux4_1 : public SixteenBitsMux
{
private:
    Mux2_1 *mux2_1[3];
public:
    Mux4_1()
    {
        for (int i=0; i<3; i++)
            mux2_1[i] = new Mux2_1;
    }
    virtual void setValue (bool   val,  int pin){mux2_1[pin/32]->setValue(val, pin%32);}
    virtual void setValue (Gate  *gate, int pin){mux2_1[pin/32]->setValue(gate, pin%32);}
    virtual void setValue (Bits16 bits, int loc){mux2_1[loc/2]->setValue(bits, loc%2);}
    virtual void setSelect(bool   val,  int pin)
    {
        switch(pin)
        {
            case 0  : mux2_1[0] -> setSelect(val) ;
                      mux2_1[1] -> setSelect(val) ; break ;
            case 1  : mux2_1[2] -> setSelect(val) ; break ;
            default : ;
        }
    }
    virtual void setSelect(Gate *gate, int pin)
    {
        switch(pin)
        {
            case 0  : mux2_1[0] -> setSelect(gate) ;
                      mux2_1[1] -> setSelect(gate) ; break ;
            case 1  : mux2_1[2] -> setSelect(gate) ; break ;
            default : ;
        }
    }
    virtual Bits16 output()
    {
        mux2_1[2]->setValue(mux2_1[0]->output(), 0);
        mux2_1[2]->setValue(mux2_1[1]->output(), 1);

        return mux2_1[2]->output();
    }
    virtual Gate *operator[](int n)
    {
        mux2_1[2]->setValue(mux2_1[0]->output(), 0);
        mux2_1[2]->setValue(mux2_1[1]->output(), 1);

        return (*mux2_1[2])[n];
    }
};

class Mux16_1 : public SixteenBitsMux
{
private:
    Mux4_1 *mux4_1[5];
public:
    Mux16_1()
    {
        for (int i=0; i<5; i++)
            mux4_1[i] = new Mux4_1;
    }
    virtual void setValue  (bool  val,   int pin){mux4_1[pin/64]->setValue(val, pin%64);}
    virtual void setValue  (Gate *gate,  int pin){mux4_1[pin/64]->setValue(gate, pin%64);}
    virtual void setValue  (Bits16 bits, int loc){mux4_1[loc/4]->setValue(bits, loc%4);}
    virtual void setSelect (bool  val,   int pin)
    {
        if (pin < 2)
            for (int i=0; i<4; i++)
                mux4_1[i] -> setSelect(val, pin);
        else if (pin < 4)
            mux4_1[4] -> setSelect(val, pin-2);
    }
    virtual void setSelect(Gate *gate, int pin)
    {
        if (pin < 2)
            for (int i=0; i<4; i++)
                mux4_1[i] -> setSelect(gate, pin);
        else if (pin < 4)
            mux4_1[4] -> setSelect(gate, pin-2);
    }
    virtual Bits16 output()
    {
        for (int i=0; i<4; i++)
            mux4_1[4]->setValue(mux4_1[i]->output(), i);

        return mux4_1[4]->output();
    }
    virtual Gate *operator[](int n)
    {
        for (int i=0; i<4; i++)
            mux4_1[4]->setValue(mux4_1[i]->output(), i);

        return (*mux4_1[4])[n];
    }
};

class Mux32_1 : public SixteenBitsMux
{
private:
    Mux16_1 *mux16_1[2];
    Mux2_1 *mux2_1;
public:
    Mux32_1()
    {
        mux16_1[0] = new Mux16_1;
        mux16_1[1] = new Mux16_1;
        mux2_1 = new Mux2_1;
    }
    virtual void setValue  (bool val,    int pin){mux16_1[pin/256]->setValue(val, pin%256);}
    virtual void setValue  (Gate *gate,  int pin){mux16_1[pin/256]->setValue(gate, pin%256);}
    virtual void setValue  (Bits16 bits, int loc){mux16_1[loc/16]->setValue(bits, loc%16);}
    virtual void setSelect (bool val,    int pin)
    {
        if (pin < 4)
        {
            mux16_1[0] -> setSelect(val, pin);
            mux16_1[1] -> setSelect(val, pin);
        }
        else if (pin == 4)
            mux2_1 -> setSelect(val);
    }
    virtual void setSelect(Gate *gate, int pin)
    {
        if (pin < 4)
        {
            mux16_1[0] -> setSelect(gate, pin);
            mux16_1[1] -> setSelect(gate, pin);
        }
        else if (pin == 4)
            mux2_1 -> setSelect(gate);
    }
    virtual Bits16 output()
    {
        mux2_1->setValue(mux16_1[0]->output(), 0);
        mux2_1->setValue(mux16_1[1]->output(), 1);

        return mux2_1->output();
    }
    virtual Gate *operator[](int n)
    {
        mux2_1->setValue(mux16_1[0]->output(), 0);
        mux2_1->setValue(mux16_1[1]->output(), 1);

        return (*mux2_1)[n];
    }
};

class Decoder1_2 : public Decoder
{
private:
    Gate *component[3];
public:
    Decoder1_2()
    {
        component[0] = new AND;
        component[1] = new AND;
        component[2] = new Invert;
    }
    virtual void setValue(bool val, int pin = 0)
    {
        component[1]->setValue(val, 0);
        component[2]->setValue(val, 0);
    }
    virtual void setValue(Gate *gate, int pin = 0)
    {
        component[1]->setValue(gate, 0);
        component[2]->setValue(gate, 0);
    }
    virtual void setValue(Bits16 bits, int loc = 0)
    {
        component[1]->setValue(bits[0], 0);
        component[2]->setValue(bits[0], 0);
    }
    virtual void setEnable(bool val)
    {
        component[0]->setValue(val, 0);
        component[1]->setValue(val, 1);
        component[0]->setValue(component[2], 1);
    }
    virtual void setEnable(Gate *gate)
    {
        component[0]->setValue(gate, 0);
        component[1]->setValue(gate, 1);
        component[0]->setValue(component[2], 1);
    }
    virtual Bits16 output()
    {
        Bits16 bits;
        bits[0] = component[0];
        bits[1] = component[1];

        return bits;
    }
    virtual Gate *operator[](int n){return component[n];}
};

class Decoder2_4 : public Decoder
{
private:
    Gate *component[6];
    Gate *enables[4];
public:
    Decoder2_4()
    {
        for(int i = 0 ; i < 2 ; i++)
            component[i] = new Invert;
        for(int i = 2 ; i < 6 ; i++)
            component[i] = new AND;
        for(int i = 0 ; i < 4 ; i++)
            enables[i] = new AND;
    }
    virtual void setValue(bool val, int pin)
    {
        if (pin == 0)
        {
            component[0]->setValue(val, 0);
            component[3]->setValue(val, 0);
            component[5]->setValue(val, 0);
        }
        else if (pin == 1)
        {
            component[1]->setValue(val, 0);
            component[4]->setValue(val, 0);
            component[5]->setValue(val, 1);
        }
    }
    virtual void setValue(Gate *gate, int pin)
    {
        if (pin == 0)
        {
            component[0]->setValue(gate, 0);
            component[3]->setValue(gate, 0);
            component[5]->setValue(gate, 0);
        }
        else if (pin == 1)
        {
            component[1]->setValue(gate, 0);
            component[4]->setValue(gate, 0);
            component[5]->setValue(gate, 1);
        }
    }
    virtual void setValue(Bits16 bits)
    {
        setValue(bits[0], 0);
        setValue(bits[1], 1);
    }
    virtual Bits16 output()
    {
        _out();
        Bits16 bits;
        for (int i=0; i<4; i++)
            bits[i] = enables[i];

        return bits;
    }
    virtual Gate *operator[](int n)
    {
        _out();
        return enables[n];
    }
    void _out()
    {
        component[2]->setValue(component[0], 0);
        component[2]->setValue(component[1], 1);
        component[3]->setValue(component[1], 1);
        component[4]->setValue(component[0], 1);
        for(int i = 0 ; i < 4 ; i++)
        {
            enables[i]->setValue(enable, 0);
            enables[i]->setValue(component[i + 2], 1);
        }
    }
};

class Decoder3_8 : public Decoder
{
private:
    Gate *component[19];
    Gate *enables[8];
public:
    Decoder3_8()
    {
        for(int i = 0 ; i < 3 ; i++)
            component[i] = new Invert;
        for(int i = 3 ; i < 19 ; i++)
            component[i] = new AND;
        for(int i = 0 ; i < 8 ; i++)
            enables[i] = new AND;
    }
    virtual void setValue(bool val, int pin)
    {
        if (pin == 0)
        {
            component[0]->setValue(val, 0);
            for (int i=4; i<=10; i+=2)
                component[i]->setValue(val, 0);
        }
        else if (pin == 1)
        {
            component[1]->setValue(val, 0);
            component[5]->setValue(val, 1);
            component[6]->setValue(val, 1);
            component[9]->setValue(val, 1);
            component[10]->setValue(val, 1);
        }
        else if (pin == 2)
        {
            component[2]->setValue(val, 0);
            component[15]->setValue(val, 1);
            component[16]->setValue(val, 1);
            component[17]->setValue(val, 1);
            component[18]->setValue(val, 1);
        }
    }
    virtual void setValue(Gate *gate, int pin)
    {
        if (pin == 0)
        {
            component[0]->setValue(gate, 0);
            for (int i=4; i<=10; i+=2)
                component[i]->setValue(gate, 0);
        }
        else if (pin == 1)
        {
            component[1]->setValue(gate, 0);
            component[5]->setValue(gate, 1);
            component[6]->setValue(gate, 1);
            component[9]->setValue(gate, 1);
            component[10]->setValue(gate, 1);
        }
        else if (pin == 2)
        {
            component[2]->setValue(gate, 0);
            component[15]->setValue(gate, 1);
            component[16]->setValue(gate, 1);
            component[17]->setValue(gate, 1);
            component[18]->setValue(gate, 1);
        }
    }
    virtual void setValue(Bits16 bits)
    {
        for (int i=0; i<3; i++)
            setValue(bits[i], i);
    }
    virtual Bits16 output()
    {
        _out();
        Bits16 bits;
        for (int i=0; i<8; i++)
            bits[i] = enables[i];

        return bits;
    }
    virtual Gate *operator[](int n)
    {
        _out();
        return enables[n];
    }
    void _out()
    {
        component[3]->setValue(component[0], 0);
        component[3]->setValue(component[1], 1);
        component[4]->setValue(component[1], 1);
        component[5]->setValue(component[0], 0);
        component[7]->setValue(component[0], 0);
        component[7]->setValue(component[1], 1);
        component[8]->setValue(component[1], 1);
        component[9]->setValue(component[0], 0);

        component[11]->setValue(component[3], 0);
        component[11]->setValue(component[2], 1);
        component[12]->setValue(component[4], 0);
        component[12]->setValue(component[2], 1);
        component[13]->setValue(component[5], 0);
        component[13]->setValue(component[2], 1);
        component[14]->setValue(component[6], 0);
        component[14]->setValue(component[2], 1);
        component[15]->setValue(component[7], 0);
        component[16]->setValue(component[8], 0);
        component[17]->setValue(component[9], 0);
        component[18]->setValue(component[10], 0);

        for(int i = 0; i < 8; i++)
        {
            enables[i]->setValue(enable, 0);
            enables[i]->setValue(component[i + 11], 1);
        }
    }
};

class Decoder5_32 : public Decoder
{
private:
    Decoder2_4 *dec2_4;
    Decoder3_8 *dec3_8[4];
public:
    Decoder5_32()
    {
        dec2_4 = new Decoder2_4;
        for (int i=0; i<4; i++)
            dec3_8[i] = new Decoder3_8;
    }
    virtual void setValue(bool val, int pin)
    {
        if(pin < 3)
            for (int i=0; i<4; i++)
                dec3_8[i] -> setValue(val, pin);
        else if (pin < 5)
            dec2_4 -> setValue(val, (pin-1) % 2);
    }
    virtual void setValue(Gate *gate, int pin)
    {
        if(pin < 3)
            for (int i=0; i<4; i++)
                dec3_8[i] -> setValue(gate, pin);
        else if (pin < 5)
            dec2_4 -> setValue(gate, (pin-1) % 2);
    }
    virtual void setValue(Bits16 bits)
    {
        for (int i=0; i<5; i++)
            setValue(bits[i], i);
    }
    virtual Gate *operator[](int n)
    {
        _out();
        if (n >= 0 && n < 32)
            return (*dec3_8[n/8])[n%8];
        else
            return nullptr ;
    }
    virtual Bits16 output(int loc)
    {
        _out();
        Bits16 bits0 = dec3_8[0]->output();
        Bits16 bits1 = dec3_8[1]->output();
        Bits16 bits2 = dec3_8[2]->output();
        Bits16 bits3 = dec3_8[3]->output();
        for (int i=0; i<8; i++)
        {
            bits0[i+8] = bits1[i];
            bits2[i+8] = bits3[i];
        }

        if (loc == 0)
            return bits0;
        else
            return bits2;
    }
    void _out()
    {
        dec2_4 -> setEnable(this -> enable);
        Bits16 bits = dec2_4->output();

        for (int i=0; i<4; i++)
            dec3_8[i]->setEnable(bits[i]);
    }
};

#endif // SWITCHING_H_INCLUDED

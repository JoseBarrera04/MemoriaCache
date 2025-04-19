#ifndef DATALINE_H
#define DATALINE_H

class DataLine {
private:
    bool dirty;
    bool valid;
    unsigned int tag;
    unsigned int data;
    unsigned int lfu;

public:
    //Constructor
    DataLine();

    //Funciones

    //Getters y Setters
    bool getDirty();
    bool getValid();
    unsigned int getTag();
    unsigned int getData();
    unsigned int getLFU();

    void setDirty(bool dirty);
    void setValid(bool valid);
    void setTag(unsigned int tag);
    void setData(unsigned int data);
    void setLFU(unsigned int lfu);
};

#endif //DATALINE_H